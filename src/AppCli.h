#pragma once

#include "FieldReader.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <set>
#include <map>
#include <cassert>
#include <getopt.h>

struct CliOptionBase {
	const char name;
	std::string desc;
	bool presented;

	CliOptionBase(const char _name, std::string _desc) : name(_name), desc(std::move(_desc)), presented(false) {}
	virtual ~CliOptionBase() = default;

	bool parse(const char* arg) {
		presented = parse_impl(arg);
		return presented;
	}

	void reset() {
		presented = false;
	}

	virtual bool parse_impl(const char* arg) = 0;

};

template <typename T>
struct CliOption : public CliOptionBase {
	using Base_t = CliOptionBase;
	T value;

	constexpr CliOption(const char _name, std::string _desc) : Base_t(_name, std::move(_desc)) {}

	bool parse_impl(const char* arg) override {
		return FieldReader::read(value, arg);
	}

	operator const T&() const {
		return value;
	}

};

struct OptMeth {
	CliOptionBase* opt;
	bool mandatory;

	bool operator<(const OptMeth& rv) const {
		return opt->name < rv.opt->name;
	}
};

struct AppCliItem {
	std::set<OptMeth> opts;
	std::string desc;

	AppCliItem& description(std::string _desc) noexcept {
		desc = std::move(_desc);
		return *this;
	}

	template <typename V>
	AppCliItem& mandatory(CliOption<V>& op) noexcept {
		OptMeth value{&op, true};
		if(not opts.emplace(value).second) {
			printf("'%c' option is duplicated.\n", op.name);
			assert(false);
		}
		return *this;
	}

	template <typename V, typename... Args>
	AppCliItem& mandatory(CliOption<V>& op, Args&... args) noexcept {
		mandatory(op);
		mandatory(args...);
		return *this;
	}

	template <typename V>
	AppCliItem& optional(CliOption<V>& op) noexcept {
		OptMeth value{&op, false};
		if(not opts.emplace(value).second) {
			printf("'%c' option is duplicated.\n", op.name);
			assert(false);
		}
		return *this;
	}

	template <typename V, typename... Args>
	AppCliItem& optional(CliOption<V>& op, Args&... args) noexcept {
		optional(op);
		optional(args...);
		return *this;
	}
};

template <typename T>
struct AppCli {
	using E = typename T::Enum_t;
	static constexpr char METHOD_NAME = 'm';

	const char* const HEADER_METHOD = "Method";
	const char* const HEADER_DESCRIPTION = "Description";
	const char* const HEADER_OPTIONS = "Options";

	const int width_method;
	int width_description;
	int width_option;

	T method;

	AppCliItem _items[T::size()];
	std::map<char, CliOptionBase*> _opt_map;
	std::string _opt_names;

	AppCli() : width_method(calc_method_width()) {}

	AppCliItem& operator[](const size_t idx) {
		if(idx < T::size()) {
			return _items[idx];
		}
		assert(false);
	}

	void finilize() {
		_opt_names.clear();
		_opt_map.clear();
		_opt_map.emplace(METHOD_NAME, nullptr);

		for(size_t i = 0; i < T::size(); ++i) {
			if(_items[i].desc.empty()) {
				printf("Method '%s' description is empty.\n", T::to_cstr(static_cast<E>(i)));
				assert(false);
			}
			for(const auto& item : _items[i].opts) {
				const char name = item.opt->name;
				const auto it = _opt_map.find(name);
				if(it != _opt_map.end() && it->second != item.opt) {
					printf("Option name '%c' is duplicated.\n", name);
					assert(false);
				}
				_opt_map.emplace(name, item.opt);
			}
		}

		for(const auto& item : _opt_map) {
			_opt_names.push_back(item.first);
			_opt_names.push_back(':');
		}

		width_description = calc_description_width();
		width_option = calc_option_width();

		_opt_map.erase(METHOD_NAME);
	}

	bool parse_args(int argc, char** argv) {
		bool result = true;
		int opt;

		while((opt = getopt(argc, argv, _opt_names.c_str())) != EOF) {
			const char opt_char = static_cast<char>(opt);
			const auto it = _opt_map.find(opt_char);
			bool opt_parse_reault = false;

			if(it != _opt_map.end()) {
				opt_parse_reault = it->second->parse(optarg);
			} else if(opt_char == METHOD_NAME) {
				opt_parse_reault = FieldReader::read(method, optarg);
			} else {
				printf("Unknown option '%c'\n", opt_char);
			}

			if(not opt_parse_reault) {
				printf("Option '%c' parsing failure.\n", opt_char);
			}

			result = result && opt_parse_reault;
		}

		return result && validate();
	}

	void print_usage(FILE* out) {
		const int width_total = width_method + width_description + width_option + 10;
		fprintf(out, "\t -%c Enum. EnumMethod to do :\n", METHOD_NAME);
		draw_line(out, width_total);

		// header
		fprintf(out, "\t");
		fprintf(out, "| %-*s ", width_method, HEADER_METHOD);
		fprintf(out, "| %-*s ", width_description, HEADER_DESCRIPTION);
		fprintf(out, "| %-*s ", width_option, HEADER_OPTIONS);
		fprintf(out, "|\n");

		draw_line(out, width_total);

		// content
		for(size_t i = 0; i < T::size(); ++i) {
			size_t cnt_man = 0;
			int opt_width_left = width_option + 1;
			fprintf(out, "\t");
			fprintf(out, "| %-*s ", width_method, T::to_cstr(static_cast<E>(i)));
			fprintf(out, "| %-*s ", width_description, _items[i].desc.c_str());
			fprintf(out, "| ");

			for(const auto& item : _items[i].opts) {
				if(item.mandatory) {
					fprintf(out, "%c", item.opt->name);
					++cnt_man;
					--opt_width_left;
				}
			}
			if(cnt_man < _items[i].opts.size()) {
				opt_width_left -= 2;
				fprintf(out, "[");
				for(const auto& item : _items[i].opts) {
					if(not item.mandatory) {
						fprintf(out, "%c", item.opt->name);
						--opt_width_left;
					}
				}
				fprintf(out, "]");
			}
			fprintf(out, "%-*s", opt_width_left, "");
			fprintf(out, "|\n");
		}
		draw_line(out, width_total);

		if(not _opt_map.empty()) {
			fprintf(out, "\nOptions :\n");
			for(const auto& item : _opt_map) {
				fprintf(out, "\t -%c %s\n", item.first, item.second->desc.c_str());
			}
		}
	}

	std::string options_string() const {
		std::string result;
		result.push_back('-');
		for(const auto& item : _opt_map) {
			result.push_back(item.first);
		}
		return result;
	}

private:

	void draw_line(FILE* out, int width_total) const {
		fprintf(out, "\t");
		for(int i = 0; i < width_total; ++i) {
			fprintf(out, "-");
		}
		fprintf(out, "\n");
	}

	int calc_method_width() const {
		size_t max = strlen(HEADER_METHOD);
		for(size_t i = 0; i < T::size(); ++i) {
			const auto sl = strlen(T::to_cstr(static_cast<E>(i)));
			if(sl > max) {
				max = sl;
			}
		}
		return static_cast<int>(max);
	}

	int calc_description_width() const {
		size_t max = strlen(HEADER_DESCRIPTION);
		for(size_t i = 0; i < T::size(); ++i) {
			const auto sl = _items[i].desc.size();
			if(sl > max) {
				max = sl;
			}
		}
		return static_cast<int>(max);
	}

	int calc_option_width() const {
		size_t max = strlen(HEADER_OPTIONS);
		bool has_opt = false;
		for(size_t i = 0; i < T::size(); ++i) {
			auto sl = _items[i].opts.size();
			for(const auto& it : _items[i].opts) {
				has_opt = has_opt || it.mandatory;
			}
			if(has_opt) {
				sl += 2u;
			}
			if(sl > max) {
				max = sl;
			}
		}
		return static_cast<int>(max);
	}

	bool validate() const {
		bool result = true;
		if(method != E::__SIZE) {
			const auto idx = static_cast<size_t>(method.get());
			for(const auto& item : _items[idx].opts) {
				if(item.mandatory && (not item.opt->presented)) {
					printf("Mandatory option '%c' is not presented.\n", item.opt->name);
					result = false;
				}
			}
		} else {
			printf("Method is not presented.\n");
			result = false;
		}
		return result;
	}

};
