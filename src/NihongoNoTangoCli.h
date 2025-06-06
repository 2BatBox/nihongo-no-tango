#pragma once

#include "AppCli.h"

#include <string>

struct NihongoNoTangoCli {

	enum EnumMethod : unsigned {
		READ_KANA,
		WRITE_KANA,
		READ_KANJI,
		WRITE_KANJI,
		TRANSLATION,
		AUDIO,
		__SIZE
	};

	struct EnumMethodToCStr {
		static const char* to_cstr(const EnumMethod& value) {
			switch(value) {
				case EnumMethod::READ_KANA: return "read-kana";
				case EnumMethod::WRITE_KANA: return "write-kana";
				case EnumMethod::READ_KANJI: return "read-kanji";
				case EnumMethod::WRITE_KANJI: return "write-kanji";
				case EnumMethod::TRANSLATION: return "trans";
				case EnumMethod::AUDIO: return "audio";
				default: return "[UNKNOWN]";
			}
		}
	};

	using Method = EnumField<EnumMethod, EnumMethodToCStr>;

	unsigned pr = 1;
	Option<size_t> rounds = Option<size_t>('r', "Rounds.", ++pr);
	Option<std::string> dic_file = Option<std::string>('d', "Dictionary file.", ++pr);
	OptionFlag with_translation = OptionFlag('t', "Show translation.", ++pr);
	OptionFlag katakana = OptionFlag('k', "Katakana filter.", ++pr);

	AppCliMethod<Method> action;

	NihongoNoTangoCli() {
		action[EnumMethod::READ_KANA]
			.desc("あかい + (voice '赤い')")
			.mand(rounds, dic_file)
			.opt(with_translation, katakana);

		action[EnumMethod::WRITE_KANA]
			.desc("(voice '赤い') -> あかい")
			.mand(rounds, dic_file)
			.opt(with_translation, katakana);

		action[EnumMethod::READ_KANJI]
			.desc("赤い + (voice '赤い')")
			.mand(rounds, dic_file);

		action[EnumMethod::WRITE_KANJI]
			.desc("(voice '赤い') -> 赤い")
			.mand(rounds, dic_file)
			.opt(with_translation);

		action[EnumMethod::TRANSLATION]
			.desc("красный -> あかい")
			.mand(rounds, dic_file);

		action[EnumMethod::AUDIO]
			.desc("(voice '赤い') -> あかい")
			.mand(rounds, dic_file);

		action.finalize();
	}

	bool parse_args(int argc, char** argv) {
		return action.parse_args(argc, argv) && validate();
	}

	bool validate() const {
		return (not dic_file.value().empty()) && rounds > 0;
	}

	void print_usage(FILE* out, const char* bin) {
		action.print_usage(out, bin);
	}

	std::string options_string() const {
		return action.options_string();
	}

};
