#pragma once

#include "FieldReader.h"
#include "FieldWriter.h"

#include <cassert>
#include <set>
#include <map>
#include <cstdio>
#include <cstring>
#include <getopt.h>

struct OptionBase {

	const bool has_default_value;
	bool has_user_value;
	const bool has_argument;
	const char name;
	const unsigned print_priority;
	std::string desc;

	OptionBase(bool _has_def_value, const bool _has_arg, const char _name, std::string _desc, const unsigned _print_priority) :
		has_default_value(_has_def_value), has_user_value(false), has_argument(_has_arg), name(_name), print_priority(_print_priority), desc(std::move(_desc))
	{}

	virtual ~OptionBase() = default;

	bool parse(const char* arg) {
		has_user_value = parse_impl(arg);
		return has_user_value;
	}

	bool presented() const {
		return has_default_value || has_user_value;
	}

	virtual bool parse_impl(const char* arg) = 0;
	virtual void print(FILE* out) = 0;

};

template <typename T>
struct Option : public OptionBase {
	using Base_t = OptionBase;
	const T value_def;
	T value_user;

	Option(const char _name, std::string _desc, const unsigned _print_priority) :
		Base_t(false, true, _name,  std::move(_desc), _print_priority), value_def(T()) {}

	Option(const char _name, std::string _desc, const unsigned _print_priority, T _def_value) :
		Base_t(true, true, _name,  std::move(_desc), _print_priority), value_def(std::move(_def_value)) {}

	bool parse_impl(const char* arg) override {
		return FieldReader::read(value_user, arg);
	}

	operator const T&() const {
		return value();
	}

	const T& value() const {
		return has_user_value ? value_user : value_def;
	}

	void print(FILE* out) final {
		fprintf(out, "\t -%c %s", name, desc.c_str());
		if(has_default_value) {
			std::string buf;
			FieldWriter::write(buf, value_def);
			fprintf(out, " default='%s'", buf.c_str());
		}
		fprintf(out, "\n");
	}

};

struct OptionFlag : public OptionBase {
	using Base_t = OptionBase;

	OptionFlag(const char _name, std::string _desc, const unsigned _print_priority) : Base_t(false, false, _name,  std::move(_desc), _print_priority) {}

	bool parse_impl(const char* arg) override { return true; }
	void print(FILE* out) final { fprintf(out, "\t -%c %s\n", name, desc.c_str()); }
};

struct OptionItem {
	OptionBase* opt;
	bool mandatory;

	bool operator<(const OptionItem& rv) const {
		return opt->print_priority < rv.opt->print_priority;
	}
};

struct Method {

	std::set<OptionItem> options;
	std::string _desc;

	Method& desc(std::string desc) noexcept {
		_desc = std::move(desc);
		return *this;
	}

	template <typename T>
	Method& mand(Option<T>& op) noexcept {
		OptionItem value{&op, true};
		if(not options.emplace(value).second) {
			fprintf(stderr,"'%c' option is duplicated.\n", op.name);
			assert(false);
		}
		return *this;
	}

	template <typename T, typename... Args>
	Method& mand(Option<T>& op, Args&... args) noexcept {
		mand(op);
		mand(args...);
		return *this;
	}

	Method& opt(OptionBase& op) noexcept {
		OptionItem value{&op, false};
		if(not options.emplace(value).second) {
			fprintf(stderr,"'%c' option is duplicated.\n", op.name);
			assert(false);
		}
		return *this;
	}

	template <typename... Args>
	Method& opt(OptionBase& op, Args&... args) noexcept {
		opt(op);
		opt(args...);
		return *this;
	}
};


class AppCliBasic {

protected:

	std::map<char, OptionBase*> _opt_map;
	std::string _opt_names;

	virtual bool post_validate() const = 0;
	virtual void build_option_map() = 0;

public:

	void finalize() {
		_opt_names.clear();
		_opt_map.clear();
		build_option_map();

		for(const auto& item : _opt_map) {
			_opt_names.push_back(item.first);
			if(item.second->has_argument) {
				_opt_names.push_back(':');
			}
		}
	}

	bool parse_args(int argc, char** argv) {
		bool result = true;
		int opt;

		while((opt = getopt(argc, argv, _opt_names.c_str())) != EOF) {
			const char opt_char = static_cast<char>(opt);
			const auto it = _opt_map.find(opt_char);
			bool opt_parse_reault = false;

			if(it != _opt_map.end()) {
				opt_parse_reault = it->second->has_argument ? it->second->parse(optarg) : it->second->parse(nullptr);
			} else {
				fprintf(stderr,"Unknown option '%c'\n", opt_char);
			}

			if(not opt_parse_reault) {
				fprintf(stderr,"Option '%c' parsing failure.\n", opt_char);
			}

			result = result && opt_parse_reault;
		}

		return result && post_validate();
	}

	std::string options_string() const {
		std::string result;
		result.push_back('-');
		for(const auto& item : _opt_map) {
			result.push_back(item.first);
		}
		return result;
	}

	void print_options(FILE* out) {
		if(not _opt_map.empty()) {
			std::map<unsigned, OptionBase*> print_map;
			for(const auto& item : _opt_map) {
				print_map.emplace(item.second->print_priority, item.second);
			}

			for(const auto& item : print_map) {
				item.second->print(out);
			}
		}
	}

protected:

	void draw_border(FILE* out, int width_total) const {
		fprintf(out, "\t");
		for(int i = 0; i < width_total; ++i) {
			fprintf(out, "-");
		}
		fprintf(out, "\n");
	}

	bool validate_method(const Method& meth) const {
		bool result = true;
		for(const auto& item : meth.options) {
			if(item.mandatory && (not item.opt->presented())) {
				fprintf(stderr,"Mandatory option '%c' is not presented.\n", item.opt->name);
				result = false;
			}
		}
		return result;
	}
};


class AppCliSimple : public AppCliBasic {
	Method _default;

public:

	Method& configure() {
		return _default;
	}

	void build_option_map() final {
		for(const auto& item : _default.options) {
			const char name = item.opt->name;
			const auto it = _opt_map.find(name);
			if(it != _opt_map.end() && it->second != item.opt) {
				fprintf(stderr,"Option name '%c' is duplicated.\n", name);
				assert(false);
			}
			_opt_map.emplace(name, item.opt);
		}
	}

	bool post_validate() const final {
		return validate_method(_default);
	}

	void print_usage(FILE* out, const char* name) {
		fprintf(out, "\n%s :\n", name);
		print_options(out);
	}

};


template <typename EF>
struct AppCliMethod : AppCliBasic {

	using E = typename EF::Enum_t;
	static constexpr char METHOD_NAME = 'm';

	static constexpr const char* const HEADER_METHOD = "Method";
	static constexpr const char* const HEADER_DESCRIPTION = "Description";
	static constexpr const char* const HEADER_OPTIONS = "Options";

	const int _width_method;
	int _width_description;
	int _width_option;

	Method _methods[EF::size()];
	Option<EF> _method_opt = Option<EF>(METHOD_NAME, "Method to run.", 0);

	AppCliMethod() : _width_method(calc_method_width()) {}

	Method& operator[](const size_t idx) {
		if(idx < EF::size()) {
			return _methods[idx];
		}
		assert(false);
	}

	const EF& action() const {
		return _method_opt.value();
	}

	void build_option_map() final {
		_opt_map.emplace(METHOD_NAME, &_method_opt);

		for(size_t i = 0; i < EF::size(); ++i) {
			if(_methods[i]._desc.empty()) {
				fprintf(stderr,"Method '%s' description is empty.\n", EF::to_cstr(static_cast<E>(i)));
				assert(false);
			}
			for(const auto& item : _methods[i].options) {
				const char name = item.opt->name;
				const auto it = _opt_map.find(name);
				if(it != _opt_map.end() && it->second != item.opt) {
					fprintf(stderr,"Option name '%c' is duplicated.\n", name);
					assert(false);
				}
				_opt_map.emplace(name, item.opt);
			}
		}

		_width_description = calc_description_width();
		_width_option = calc_option_width();
	}

	void print_usage(FILE* out, const char* name) {
		fprintf(out, "\n%s :\n", name);

		const int width_total = _width_method + _width_description + _width_option + 10;
		draw_border(out, width_total);

		// header
		fprintf(out, "\t");
		fprintf(out, "| %-*s ", _width_method, HEADER_METHOD);
		fprintf(out, "| %-*s ", _width_description, HEADER_DESCRIPTION);
		fprintf(out, "| %-*s ", _width_option, HEADER_OPTIONS);
		fprintf(out, "|\n");

		draw_border(out, width_total);

		// content
		for (size_t i = 0; i < EF::size(); ++i) {
			size_t cnt_man = 0;
			int opt_width_left = _width_option + 1;
			fprintf(out, "\t");
			fprintf(out, "| %-*s ", _width_method, EF::to_cstr(static_cast<E>(i)));
			fprintf(out, "| %-*s ", _width_description, _methods[i]._desc.c_str());
			fprintf(out, "| ");

			for (const auto& item: _methods[i].options) {
				if (item.mandatory) {
					fprintf(out, "%c", item.opt->name);
					++cnt_man;
					--opt_width_left;
				}
			}
			if (cnt_man < _methods[i].options.size()) {
				opt_width_left -= 2;
				fprintf(out, "[");
				for (const auto& item: _methods[i].options) {
					if (not item.mandatory) {
						fprintf(out, "%c", item.opt->name);
						--opt_width_left;
					}
				}
				fprintf(out, "]");
			}
			fprintf(out, "%-*s", opt_width_left, "");
			fprintf(out, "|\n");
		}
		draw_border(out, width_total);
		print_options(out);
	}

private:

	static int calc_method_width() {
		size_t max = strlen(HEADER_METHOD);
		for(size_t i = 0; i < EF::size(); ++i) {
			const auto sl = strlen(EF::to_cstr(static_cast<E>(i)));
			if(sl > max) {
				max = sl;
			}
		}
		return static_cast<int>(max);
	}

	int calc_description_width() const {
		size_t max = strlen(HEADER_DESCRIPTION);
		for(size_t i = 0; i < EF::size(); ++i) {
			const auto sl = _methods[i]._desc.size();
			if(sl > max) {
				max = sl;
			}
		}
		return static_cast<int>(max);
	}

	int calc_option_width() const {
		size_t max = strlen(HEADER_OPTIONS);
		bool has_opt = false;
		for(size_t i = 0; i < EF::size(); ++i) {
			auto sl = _methods[i].options.size();
			for(const auto& it : _methods[i].options) {
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

	bool post_validate() const final {
		bool result;
		if(_method_opt.value().get() != EF::size()) {
			const auto idx = static_cast<size_t>(_method_opt.value().value);
			result = validate_method(_methods[idx]);
		} else {
			fprintf(stderr, "Method is not presented.\n");
			result = false;
		}
		return result;
	}

};
