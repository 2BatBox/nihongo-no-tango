#pragma once

#include "AppCli.h"

#include <string>

struct NihongoNoTangoCli {

	enum EnumMethod : unsigned {
		LEARN,
		TEST,
		__SIZE
	};

	struct EnumMethodToCStr {
		static const char* to_cstr(const EnumMethod& value) {
			switch(value) {
				case EnumMethod::LEARN: return "learn";
				case EnumMethod::TEST: return "test";
				default: return "[UNKNOWN]";
			}
		}
	};

	using Method = EnumField<EnumMethod, EnumMethodToCStr>;

	enum class EnumAnswer : unsigned {
		KANA,
		KANJI,
		TRANSLATION,
		__SIZE
	};

	struct EnumAnswerCStr {
		static const char* to_cstr(const EnumAnswer& value) {
			switch(value) {
				case EnumAnswer::KANA: return "kana";
				case EnumAnswer::KANJI: return "kanji";
				case EnumAnswer::TRANSLATION: return "translation";
				default: return "[UNKNOWN]";
			}
		}
	};

	using Answer = EnumField<EnumAnswer, EnumAnswerCStr>;

	unsigned pr = 1;
	Option<size_t> rounds = Option<size_t>('r', "Rounds.", ++pr);
	Option<std::string> dic_file = Option<std::string>('d', "Dictionary file.", ++pr);
	OptionFlag show_kanji = OptionFlag('j', "Show kanji.", ++pr);
	OptionFlag show_kana = OptionFlag('k', "Show kana.", ++pr);
	OptionFlag show_translation = OptionFlag('t', "Show translation.", ++pr);
	OptionFlag play_audio = OptionFlag('p', "Play audio.", ++pr);
	OptionFlag katakana_filter = OptionFlag('f', "Katakana filter.", ++pr);
	Option<Answer> answer = Option<Answer>('a', Answer::description(), ++pr);

	AppCliMethod<Method> action;

	NihongoNoTangoCli() {
		action[EnumMethod::LEARN]
			.desc("Learning.")
			.mand(rounds, dic_file)
			.opt(show_kanji, show_kana, show_translation, play_audio, katakana_filter);

		action[EnumMethod::TEST]
			.desc("Testing.")
			.mand(rounds, dic_file, answer)
			.opt(show_kanji, show_kana, show_translation, play_audio, katakana_filter);

		action.finalize();
	}

	bool parse_args(int argc, char** argv) {
		return action.parse_args(argc, argv) && validate();
	}

	bool validate() const {
		bool result = (not dic_file.value().empty());
		result = result && (show_kanji.presented() || show_kana.presented() || play_audio.presented());
		result = result && (rounds > 0);
		return result;
	}

	void print_usage(FILE* out, const char* bin) {
		action.print_usage(out, bin);
	}

	std::string options_string() const {
		return action.options_string();
	}

};
