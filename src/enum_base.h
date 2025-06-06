#pragma once

#include <string>

using EnumBase_t = int;

template <typename E, typename ToCStr, bool PedanticRead = true, typename std::enable_if_t<std::is_enum<E>::value, int> = 0>
struct EnumField {
	using Enum_t = E;
	E value;

	EnumField() : value(E::__SIZE) {}

	EnumField(const E& val) : value(val) {}

	bool operator<(const E& val) const {
		return std::string(to_cstr()) < std::string(ToCStr::to_cstr(val));
	}

	bool operator<(const EnumField& val) const {
		return std::string(to_cstr()) < std::string(val.to_cstr());
	}

	bool operator!=(const E& val) const {
		return value != val;
	}

	bool operator==(const E& val) const {
		return value == val;
	}

	bool operator==(const EnumField& val) const {
		return value == val.value;
	}

	const E& get() const {
		return value;
	}

	const char* to_cstr() const {
		return ToCStr::to_cstr(value);
	}

	static const char* to_cstr(const E& val) {
		return ToCStr::to_cstr(val);
	}

	static constexpr std::string description() {
		std::string result("Enum : ");
		const auto SIZE = static_cast<size_t>(E::__SIZE);
		result.push_back('[');
		for(size_t i = 0; i < SIZE; ++i) {
			if(i > 0) {
				result.append(", ");
			}
			result.append(to_cstr(static_cast<E>(i)));
		}
		result.push_back(']');
		return result;
	}

	static constexpr size_t size() {
		return static_cast<size_t>(E::__SIZE);
	}

	void write(std::string& buf) const {
		buf.append(to_cstr());
	}

};
