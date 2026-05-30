#include "config/json.h"

#include <cctype>
#include <cstdint>
#include <cmath>
#include <cstdio>
#include <sstream>
#include <string_view>

namespace mfree::config {

json_value::json_value() : v(nullptr) {}
json_value::json_value(std::nullptr_t) : v(nullptr) {}
json_value::json_value(bool b) : v(b) {}
json_value::json_value(double n) : v(n) {}
json_value::json_value(std::string s) : v(std::move(s)) {}
json_value::json_value(const char *s) : v(std::string(s)) {}
json_value::json_value(array a) : v(std::move(a)) {}
json_value::json_value(object o) : v(std::move(o)) {}

bool json_value::is_null() const { return std::holds_alternative<std::nullptr_t>(v); }
bool json_value::is_bool() const { return std::holds_alternative<bool>(v); }
bool json_value::is_number() const { return std::holds_alternative<double>(v); }
bool json_value::is_string() const { return std::holds_alternative<std::string>(v); }
bool json_value::is_array() const { return std::holds_alternative<array>(v); }
bool json_value::is_object() const { return std::holds_alternative<object>(v); }

const bool &json_value::as_bool() const { return std::get<bool>(v); }
const double &json_value::as_number() const { return std::get<double>(v); }
const std::string &json_value::as_string() const { return std::get<std::string>(v); }
const json_value::array &json_value::as_array() const { return std::get<array>(v); }
const json_value::object &json_value::as_object() const { return std::get<object>(v); }

bool &json_value::as_bool() { return std::get<bool>(v); }
double &json_value::as_number() { return std::get<double>(v); }
std::string &json_value::as_string() { return std::get<std::string>(v); }
json_value::array &json_value::as_array() { return std::get<array>(v); }
json_value::object &json_value::as_object() { return std::get<object>(v); }

json_error::json_error(const std::string &msg, int line, int col, std::string path)
	: std::runtime_error(msg), line(line), col(col), path(std::move(path)) {}

struct parser {
	const std::string &s;
	size_t i = 0;
	int line = 1;
	int col = 1;
	std::vector<std::string> path;

	explicit parser(const std::string &s) : s(s) {}

	char peek() const { return i < s.size() ? s[i] : '\0'; }

	char get() {
		char c = peek();
		if (c == '\0')
			return c;
		i++;
		if (c == '\n') {
			line++;
			col = 1;
		} else {
			col++;
		}
		return c;
	}

	[[noreturn]] void fail(const std::string &msg) const {
		std::string p;
		for (size_t k = 0; k < path.size(); k++) {
			if (k)
				p += ".";
			p += path[k];
		}
		throw json_error(msg, line, col, p);
	}

	void skip_ws() {
		while (true) {
			char c = peek();
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
				get();
				continue;
			}
			return;
		}
	}

	bool consume(char expected) {
		if (peek() != expected)
			return false;
		get();
		return true;
	}

	void expect(char expected) {
		if (!consume(expected)) {
			std::string msg = "Expected '";
			msg.push_back(expected);
			msg += "'";
			fail(msg);
		}
	}

	static bool is_num_char(char c) { return std::isdigit((unsigned char)c) || c == '-' || c == '+' || c == '.' || c == 'e' || c == 'E'; }

	json_value parse_value() {
		skip_ws();
		char c = peek();
		if (c == '{')
			return parse_object();
		if (c == '[')
			return parse_array();
		if (c == '"')
			return json_value(parse_string());
		if (c == 't')
			return parse_true();
		if (c == 'f')
			return parse_false();
		if (c == 'n')
			return parse_null();
		if (c == '-' || std::isdigit((unsigned char)c))
			return json_value(parse_number());
		fail("Unexpected token");
	}

	json_value parse_object() {
		expect('{');
		skip_ws();
		json_value::object obj;
		if (consume('}'))
			return json_value(std::move(obj));
		while (true) {
			skip_ws();
			if (peek() != '"')
				fail("Expected object key string");
			std::string key = parse_string();
			skip_ws();
			expect(':');
			path.push_back(key);
			json_value val = parse_value();
			path.pop_back();
			obj.emplace(std::move(key), std::move(val));
			skip_ws();
			if (consume('}'))
				break;
			expect(',');
		}
		return json_value(std::move(obj));
	}

	json_value parse_array() {
		expect('[');
		skip_ws();
		json_value::array arr;
		if (consume(']'))
			return json_value(std::move(arr));
		int idx = 0;
		while (true) {
			path.push_back(std::to_string(idx));
			arr.push_back(parse_value());
			path.pop_back();
			idx++;
			skip_ws();
			if (consume(']'))
				break;
			expect(',');
		}
		return json_value(std::move(arr));
	}

	std::string parse_string() {
		expect('"');
		std::string out;
		while (true) {
			char c = get();
			if (c == '\0')
				fail("Unexpected end of input in string");
			if (c == '"')
				break;
			if (c == '\\') {
				char e = get();
				if (e == '\0')
					fail("Unexpected end of input in string escape");
				switch (e) {
				case '"':
					out.push_back('"');
					break;
				case '\\':
					out.push_back('\\');
					break;
				case '/':
					out.push_back('/');
					break;
				case 'b':
					out.push_back('\b');
					break;
				case 'f':
					out.push_back('\f');
					break;
				case 'n':
					out.push_back('\n');
					break;
				case 'r':
					out.push_back('\r');
					break;
				case 't':
					out.push_back('\t');
					break;
				case 'u': {
					auto hex_val = [&](char h) -> int {
						if (h >= '0' && h <= '9')
							return h - '0';
						if (h >= 'a' && h <= 'f')
							return 10 + (h - 'a');
						if (h >= 'A' && h <= 'F')
							return 10 + (h - 'A');
						return -1;
					};

					auto parse_hex4 = [&]() -> uint16_t {
						uint16_t v = 0;
						for (int k = 0; k < 4; k++) {
							// Explicit bounds check: unicode escapes require exactly 4 hex digits; avoid reading past the std::string end.
							if (i >= s.size())
								fail("Unexpected end of input in unicode escape");
							char h = get();
							int hv = hex_val(h);
							if (hv < 0)
								fail("Invalid hex digit in unicode escape");
							v = static_cast<uint16_t>((v << 4) | static_cast<uint16_t>(hv));
						}
						return v;
					};

					auto append_utf8 = [&](uint32_t cp) {
						if (cp <= 0x7Fu) {
							out.push_back((char)cp);
						} else if (cp <= 0x7FFu) {
							out.push_back((char)(0xC0u | (cp >> 6)));
							out.push_back((char)(0x80u | (cp & 0x3Fu)));
						} else if (cp <= 0xFFFFu) {
							out.push_back((char)(0xE0u | (cp >> 12)));
							out.push_back((char)(0x80u | ((cp >> 6) & 0x3Fu)));
							out.push_back((char)(0x80u | (cp & 0x3Fu)));
						} else if (cp <= 0x10FFFFu) {
							out.push_back((char)(0xF0u | (cp >> 18)));
							out.push_back((char)(0x80u | ((cp >> 12) & 0x3Fu)));
							out.push_back((char)(0x80u | ((cp >> 6) & 0x3Fu)));
							out.push_back((char)(0x80u | (cp & 0x3Fu)));
						} else {
							fail("Unicode code point out of range");
						}
					};

					uint16_t u1 = parse_hex4();
					if (u1 >= 0xD800u && u1 <= 0xDBFFu) {
						if (get() != '\\' || get() != 'u') {
							fail("Missing low surrogate after high surrogate");
						}
						uint16_t u2 = parse_hex4();
						if (!(u2 >= 0xDC00u && u2 <= 0xDFFFu)) {
							fail("Invalid low surrogate");
						}
						uint32_t cp = 0x10000u + (((uint32_t)u1 - 0xD800u) << 10) + ((uint32_t)u2 - 0xDC00u);
						append_utf8(cp);
					} else if (u1 >= 0xDC00u && u1 <= 0xDFFFu) {
						fail("Unexpected low surrogate without preceding high surrogate");
					} else {
						append_utf8((uint32_t)u1);
					}
					break;
				}
				default:
					fail("Invalid string escape");
				}
				continue;
			}
			if ((unsigned char)c < 0x20)
				fail("Control character in string");
			out.push_back(c);
		}
		return out;
	}

	double parse_number() {
		skip_ws();
		size_t start = i;
		if (peek() == '-')
			get();
		if (!std::isdigit((unsigned char)peek()))
			fail("Invalid number");
		if (peek() == '0') {
			get();
		} else {
			while (std::isdigit((unsigned char)peek()))
				get();
		}
		if (peek() == '.') {
			get();
			if (!std::isdigit((unsigned char)peek()))
				fail("Invalid number");
			while (std::isdigit((unsigned char)peek()))
				get();
		}
		if (peek() == 'e' || peek() == 'E') {
			get();
			if (peek() == '+' || peek() == '-')
				get();
			if (!std::isdigit((unsigned char)peek()))
				fail("Invalid number");
			while (std::isdigit((unsigned char)peek()))
				get();
		}
		size_t end = i;
		std::string_view sv(s.data() + start, end - start);
		char *ep = nullptr;
		double v = std::strtod(std::string(sv).c_str(), &ep);
		if (!std::isfinite(v))
			fail("Non-finite number");
		return v;
	}

	json_value parse_true() {
		expect('t');
		if (get() != 'r' || get() != 'u' || get() != 'e')
			fail("Invalid literal");
		return json_value(true);
	}

	json_value parse_false() {
		expect('f');
		if (get() != 'a' || get() != 'l' || get() != 's' || get() != 'e')
			fail("Invalid literal");
		return json_value(false);
	}

	json_value parse_null() {
		expect('n');
		if (get() != 'u' || get() != 'l' || get() != 'l')
			fail("Invalid literal");
		return json_value(nullptr);
	}
};

json_value parse_json(const std::string &input) {
	parser p(input);
	json_value v = p.parse_value();
	p.skip_ws();
	if (p.peek() != '\0') {
		p.fail("Trailing data after JSON value");
	}
	return v;
}

static void dump_escaped(std::ostringstream &os, const std::string &s) {
	os << '"';
	for (char c : s) {
		switch (c) {
		case '"':
			os << "\\\"";
			break;
		case '\\':
			os << "\\\\";
			break;
		case '\b':
			os << "\\b";
			break;
		case '\f':
			os << "\\f";
			break;
		case '\n':
			os << "\\n";
			break;
		case '\r':
			os << "\\r";
			break;
		case '\t':
			os << "\\t";
			break;
		default:
			if ((unsigned char)c < 0x20) {
				char buf[8];
				std::snprintf(buf, sizeof(buf), "\\u%04x", (unsigned char)c);
				os << buf;
			} else {
				os << c;
			}
		}
	}
	os << '"';
}

static void dump_impl(std::ostringstream &os, const json_value &v, int indent, int level) {
	if (v.is_null()) {
		os << "null";
		return;
	}
	if (v.is_bool()) {
		os << (v.as_bool() ? "true" : "false");
		return;
	}
	if (v.is_number()) {
		os.setf(std::ios::fmtflags(0), std::ios::floatfield);
		os.precision(17);
		os << v.as_number();
		return;
	}
	if (v.is_string()) {
		dump_escaped(os, v.as_string());
		return;
	}
	if (v.is_array()) {
		const auto &a = v.as_array();
		if (a.empty()) {
			os << "[]";
			return;
		}
		os << "[";
		for (size_t i = 0; i < a.size(); i++) {
			if (i)
				os << ",";
			if (indent > 0)
				os << "\n" << std::string((level + 1) * indent, ' ');
			dump_impl(os, a[i], indent, level + 1);
		}
		if (indent > 0)
			os << "\n" << std::string(level * indent, ' ');
		os << "]";
		return;
	}
	const auto &o = v.as_object();
	if (o.empty()) {
		os << "{}";
		return;
	}
	os << "{";
	bool first = true;
	for (const auto &[k, vv] : o) {
		if (!first)
			os << ",";
		first = false;
		if (indent > 0)
			os << "\n" << std::string((level + 1) * indent, ' ');
		dump_escaped(os, k);
		os << (indent > 0 ? ": " : ":");
		dump_impl(os, vv, indent, level + 1);
	}
	if (indent > 0)
		os << "\n" << std::string(level * indent, ' ');
	os << "}";
}

std::string dump_json(const json_value &v, int indent) {
	std::ostringstream os;
	dump_impl(os, v, indent, 0);
	if (indent > 0)
		os << "\n";
	return os.str();
}

} // namespace mfree::config
