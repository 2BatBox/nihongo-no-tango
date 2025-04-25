#pragma once

#include "enum_base.h"

#include <charconv>
#include <cstdlib>
#include <string>
#include <string_view>
#include <type_traits>

class FieldReader {

	static constexpr bool PEDANTIC = true;

public:

	// -----------------------------------------------------------------
	// Built-in types.
	// -----------------------------------------------------------------
	static bool read(std::string& value, const std::string_view& str) {
		value = str;
		return true;
	}

	template <typename V, std::enable_if_t<std::is_integral_v<V> && std::is_signed_v<V>, int> = 0>
	static bool read(V& value, const std::string_view& str) {
		auto fcr = std::from_chars<V>(str.begin(), str.end(), value);
		const bool result = (fcr.ec == std::errc()) && (size_t(fcr.ptr - str.begin()) == str.length());
		if(PEDANTIC && (not result)) {
			printf("%.*s -> %zd\n", int(str.length()), str.begin(), ssize_t(value));
		}
		return result;
	}

	template <typename V, std::enable_if_t<std::is_integral_v<V> && std::is_unsigned_v<V>, int> = 0>
	static bool read(V& value, const std::string_view& str) {
		auto fcr = std::from_chars<V>(str.begin(), str.end(), value);
		const bool result = (fcr.ec == std::errc()) && (size_t(fcr.ptr - str.begin()) == str.length());
		if(PEDANTIC && (not result)) {
			printf("%.*s -> %zu\n", int(str.length()), str.begin(), size_t(value));
		}
		return result;
	}

	static bool read(float& value, const std::string_view& str) {
		char* endptr = nullptr;
		errno = EXIT_SUCCESS;
		float tmp = std::strtof(str.begin(), &endptr);
		const bool result = (size_t(endptr - str.begin()) == str.length()) && (errno == 0);
		if(result) {
			value = tmp;
		}
		if(PEDANTIC && (not result)) {
			printf("%.*s -> %f\n", int(str.length()), str.begin(), value);
		}
		return result;
	}

	static bool read(double& value, const std::string_view& str) {
		char* endptr = nullptr;
		errno = EXIT_SUCCESS;
		double tmp = std::strtod(str.begin(), &endptr);
		const bool result = (size_t(endptr - str.begin()) == str.length()) && (errno == 0);
		if(result) {
			value = tmp;
		}
		if(PEDANTIC && (not result)) {
			printf("%.*s -> %f\n", int(str.length()), str.begin(), value);
		}
		return result;
	}

	// -----------------------------------------------------------------
	// Basic JTF types.
	// -----------------------------------------------------------------
	template <typename Enum, typename Converter, bool Pedantic>
	static bool read(EnumField<Enum, Converter, Pedantic>& value, const std::string_view& str) {
		value.value = as_enum<Enum, Converter, Pedantic>(value, str);
		const bool result = (value != Enum::__SIZE);
		if(PEDANTIC && Pedantic && (not result)) {
			printf("%.*s -> %s\n", int(str.length()), str.begin(), value.to_cstr());
		}
		return result;
	}

private:

	// TODO: linear time complexity
	template <typename Enum, typename Converter, bool Pedantic>
	static Enum as_enum(EnumField<Enum, Converter, Pedantic>&, const std::string_view& str) {
		auto result = Enum::__SIZE;
		for(EnumBase_t i = 0; i < static_cast<EnumBase_t>(Enum::__SIZE); ++i) {
			const auto val = static_cast<Enum>(i);
			if(str == EnumField<Enum,Converter>::to_cstr(val)) {
				result = val;
				break;
			}
		}
		return result;
	}

};
