#pragma once

#include <string>
#include <type_traits>

struct FieldWriter {

	// -----------------------------------------------------------------
	// Built-in types.
	// -----------------------------------------------------------------
	template <typename V, std::enable_if_t<(std::is_integral_v<V>), int> = 0>
	static void write(std::string& buf, const V& value) {
		buf.append(std::to_string(value));
	}

	static void write(std::string& buf, const std::string& value) {
		buf.append(value);
	}

	static void write(std::string& buf, const char* value) {
		buf.append(value);
	}

	template <typename V, std::enable_if_t<(std::is_class_v<V>), int> = 0>
	static void write(std::string& buf, const V& value) {
		value.write(buf);
	}

};
