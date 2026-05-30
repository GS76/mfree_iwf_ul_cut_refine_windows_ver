#include "config/json.h"

#include <cassert>
#include <string>

int main() {
	using namespace mfree::config;

	const std::string json = "{ \"s\": \"A\\\\u00E9\\\\uD83D\\\\uDE00\" }";
	json_value root = parse_json(json);
	assert(root.is_object());
	const auto& o = root.as_object();
	auto it = o.find("s");
	assert(it != o.end());
	assert(it->second.is_string());

	const std::string expected = std::string("A") + std::string("\xC3\xA9") + std::string("\xF0\x9F\x98\x80");
	assert(it->second.as_string() == expected);
	return 0;
}

