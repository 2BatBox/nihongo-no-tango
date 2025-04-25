#pragma once

#include "AppCli.h"

#include <string>

struct NihongoNoTangoCli {

	enum EnumMethod : unsigned {
		KANJI,
		TRANSLATION,
		AUDIO,
		__SIZE
	};

	struct EnumMethodToCStr {
		static const char* to_cstr(const EnumMethod& value) {
			switch(value) {
				case EnumMethod::KANJI: return "kanji";
				case EnumMethod::TRANSLATION: return "trans";
				case EnumMethod::AUDIO: return "audio";
				default: return "[UNKNOWN]";
			}
		}
	};

	using Method = EnumField<EnumMethod, EnumMethodToCStr>;

	CliOption<unsigned> rounds = CliOption<unsigned>('r', "Rounds.");
	CliOption<std::string> dic_file = CliOption<std::string>('d', "Dictionary file.");

	AppCli<Method> action;

	NihongoNoTangoCli() {
		action[EnumMethod::KANJI]
			.description("赤い -> あかい")
			.mandatory(rounds, dic_file);

		action[EnumMethod::TRANSLATION]
			.description("красный -> あかい")
			.mandatory(rounds, dic_file);

		action[EnumMethod::AUDIO]
			.description("(voice '赤い') -> あかい")
			.mandatory(rounds, dic_file);

		action.finilize();
	}

	bool parse_args(int argc, char** argv) {
		return action.parse_args(argc, argv) && validate();
	}

	bool validate() const {
		return (not dic_file.value.empty()) && rounds.value > 0;
	}

	void print_usage(FILE* out) {
		action.print_usage(out);
	}

	std::string options_string() const {
		return action.options_string();
	}

};
