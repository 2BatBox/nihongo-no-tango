#include "NihongoNoTangoCli.h"
#include "DiceMachine.h"
#include "TermColor.h"

#include <cstdio>
#include <vector>
#include <algorithm>
#include <random>
#include <sstream>
#include <locale>
#include <codecvt>

class NihongoNoTango {

	using String_t = std::u32string;

	struct Record {
		String_t kanji;
		String_t kana;
		String_t translation;

		bool read(const String_t& line) {
			auto list = split_by_char(line, U";");
			bool result = (list.size() == 3u);
			if(result) {
				kanji = list[0];
				kana = list[1];
				translation = list[2];
				kanji = trim(kanji, U" \t");
				kana = trim(kana, U" \t");
				translation = trim(translation, U" \t");
				result = (not kana.empty()) && (not translation.empty());
			}
			return result;
		}

	};

	using Buffer_t = std::vector<Record>;

	const NihongoNoTangoCli _cli;
	DiceMachine _dm;
	Buffer_t _dic;

public:
	NihongoNoTango(const NihongoNoTangoCli& cli) :
		_cli(cli), _dm(time(nullptr)) {}

	int load() {
		int err = EXIT_SUCCESS;
		FILE* file = fopen(_cli.dic_file.value().c_str(), "r");
		if(file) {
			String_t line;
			size_t line_cnt = 1;
			while(read_line(file, line, false)) {
				if(line.empty() || line[0] == '/') {
					continue;
				}
				Record rec;
				if(rec.read(line)) {
					_dic.push_back(rec);
				} else {
					fprintf(stderr, "Line %zu cannot be parsed : %s.\n", line_cnt, to_basic_string(line).c_str());
				}
				++line_cnt;
			}
			fclose(file);
			printf("%zu lines loaded.\n", _dic.size());
		} else {
			err = EXIT_FAILURE;
			fprintf(stderr, "Dictionary file '%s' is not available for reading.\n", _cli.dic_file.value().c_str());
		}
		return err;
	}

	void do_read(const Record& rec, const String_t& str) {
		printf("%s", to_basic_string(str).c_str());
		if(_cli.with_translation.presented()) {
			printf("  (%s)", to_basic_string(rec.translation).c_str());
		}
		fflush(stdout);
		String_t output;
		read_line(stdin, output, true);
		if(not _cli.with_translation.presented()) {
			printf("(%s)\n", to_basic_string(rec.translation).c_str());
		}
		if(rec.kanji.empty()) {
			say(rec.kana);
		} else {
			say(rec.kanji);
		}
	}

	void run() {
		auto tm_before = time(nullptr);

		unsigned cnt_total = 0;
		unsigned cnt_mistakes = 0;

		auto rng = std::default_random_engine(time(nullptr));
		std::shuffle(_dic.begin(), _dic.end(), rng);
		auto rounds_max = std::min(_cli.rounds.value(), _dic.size());

		for(size_t idx = 0; idx < rounds_max; ++ idx) {
			const auto& item = _dic[idx];

			switch(_cli.action.action().value) {

				case NihongoNoTangoCli::EnumMethod::READ_KANA:
					do_read(item, item.kana);
					break;

				case NihongoNoTangoCli::EnumMethod::READ_KANJI:
					if(item.kanji.empty()) {
						continue;
					}
					do_read(item, item.kanji);
					break;

				case NihongoNoTangoCli::EnumMethod::WRITE_KANA: {
					printf("%s  (%s)\n", to_basic_string(item.kanji).c_str(), to_basic_string(item.translation).c_str());
					fflush(stdout);
					say(item.kana);

					String_t output;
					read_line(stdin, output, true);
					if(_cli.katakana.presented()) {
						output = filter_katakana(output);
					}
					while(output != item.kana) {
						++cnt_mistakes;
						printf("%s", TermColor::front(TermColor::RED));
						printf("'%s'", to_basic_string(item.kana).c_str());
						printf("\n%s", TermColor::reset());
						say(item.kana);
						read_line(stdin, output, true);
					}

				}
					break;

				default:
					exit(-1);
					break;
			}

			++cnt_total;
		}

		double cnt_mistakes_percent = cnt_mistakes;
		cnt_mistakes_percent /= cnt_total;
		cnt_mistakes_percent *= 100.;
		printf("Mistakes : %u (%.2f%%).", cnt_mistakes, cnt_mistakes_percent);
		const unsigned seconds_total = time(nullptr) - tm_before;
		printf(" %u seconds.\n", seconds_total);
	}

private:

	String_t filter_katakana(const String_t& in) const {
		String_t result;
		for(const auto ch : in) {
			switch(ch) {
				case U'力':
					result.push_back(U'カ');
					break;

				case U'口':
					result.push_back(U'ロ');
					break;

				case U'二':
					result.push_back(U'ニ');
					break;

				case U'一':
					result.push_back(U'ー');
					break;

				case U'へ':
					result.push_back(U'ヘ');
					break;

				case U'べ':
					result.push_back(U'ベ');
					break;

				case U'ぺ':
					result.push_back(U'ペ');
					break;

				default:
					result.push_back(ch);
					break;
			}
		}
		return result;
	}

	bool read_line(FILE* input, String_t& result, const bool skip_spaces) {
		std::string buf;
		int ch;
		while((ch = getc(input)) != EOF) {
			if(ch == '\n') {
				break;
			}
			if(skip_spaces && isspace(ch)) {
				continue;
			}
			buf.push_back(ch);
		}
		result = to_u32_string(buf);
		return ch != EOF;
	}

	static std::vector<String_t> split_by_char(const String_t& str, const String_t& delim) {
		std::vector<String_t> result;
		size_t start = 0;

		for (size_t found = str.find(delim); found != String_t::npos; found = str.find(delim, start)) {
			result.emplace_back(str.begin() + start, str.begin() + found);
			start = found + 1u;
		}

		if (start != str.size()) {
			result.emplace_back(str.begin() + start, str.end());
		}
		return result;
	}

	static String_t trim_right(String_t str, const String_t& space) {
		str.erase(str.find_last_not_of(space) + 1);
		return str;
	}

	static String_t trim_left(String_t str, const String_t& space) {
		str.erase(0, str.find_first_not_of(space));
		return str;
	}

	static String_t trim(String_t str, const String_t& space) {
		return trim_right(trim_left(std::move(str), space), space);
	}

	void say(const String_t& to_say) const {
		std::string command("trans -b -p  :en :jpn \"");
		command.append(to_basic_string(to_say));
		command.append("\" >> /dev/null");

		const auto err = system(command.c_str());
		if(err != EXIT_SUCCESS) {
			fprintf(stderr, "system(\"%s\") fails\n", command.c_str());
			exit(err);
		}
	}

	static std::string to_basic_string(const std::u32string& str) {
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
		return conv.to_bytes(str);
	}

	static std::u32string to_u32_string(const std::string& str) {
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
		return conv.from_bytes(str);
	}

};

int main(int argc, char** argv) {
	NihongoNoTangoCli cli;

	if(not cli.parse_args(argc, argv)) {
		cli.print_usage(stderr, argv[0]);
		return EXIT_FAILURE;
	}

	NihongoNoTango app(cli);
	const int err = app.load();
	if(err == EXIT_SUCCESS) {
		app.run();
	}

	return EXIT_SUCCESS;
}
