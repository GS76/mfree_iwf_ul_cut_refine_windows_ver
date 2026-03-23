#ifndef MFREE_CONFIG_JSON_H_
#define MFREE_CONFIG_JSON_H_

#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace mfree::config {

struct json_value {
	using array = std::vector<json_value>;
	using object = std::map<std::string, json_value>;

	std::variant<std::nullptr_t, bool, double, std::string, array, object> v;

	json_value();
	json_value(std::nullptr_t);
	json_value(bool b);
	json_value(double n);
	json_value(std::string s);
	json_value(const char* s);
	json_value(array a);
	json_value(object o);

	bool is_null() const;
	bool is_bool() const;
	bool is_number() const;
	bool is_string() const;
	bool is_array() const;
	bool is_object() const;

	const bool& as_bool() const;
	const double& as_number() const;
	const std::string& as_string() const;
	const array& as_array() const;
	const object& as_object() const;

	bool& as_bool();
	double& as_number();
	std::string& as_string();
	array& as_array();
	object& as_object();
};

struct json_error : public std::runtime_error {
	int line = 0;
	int col = 0;
	std::string path;
	explicit json_error(const std::string& msg, int line, int col, std::string path);
};

json_value parse_json(const std::string& input);
std::string dump_json(const json_value& v, int indent = 2);

}

#endif

