#include "NihongoNoTangoCli.h"
#include "DiceMachine.h"
#include "TermColor.h"

#include <cstdio>
#include <vector>
#include <algorithm>
#include <random>
#include <sstream>

class NihongoNoTango {

	struct Record {
		std::string kanji;
		std::string kana;
		std::string translation;

		bool read(const std::string& line) {
			auto list = split_by_char(line, ';');
			bool result = (list.size() == 3u);
			if(result) {
				kanji = list[0];
				kana = list[1];
				translation = list[2];
				kanji = trim(kanji, " \t");
				kana = trim(kana, " \t");
				translation = trim(translation, " \t");
				result = (not kana.empty()) && (not translation.empty());
			}
			return result;
		}

		void dump() {
			printf("%s;%s;%s\n", kanji.c_str(), kana.c_str(), translation.c_str());
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
		FILE* file = fopen(_cli.dic_file.value.c_str(), "r");
		if(file) {
			std::string line;
			size_t line_cnt = 1;
			while(read_line(file, line, false)) {
				if(line.empty() || line[0] == '/') {
					continue;
				}
				Record rec;
				if(rec.read(line)) {
					_dic.push_back(rec);
				} else {
					fprintf(stderr, "Line %zu cannot be parsed : %s.\n", line_cnt, line.c_str());
				}
				++line_cnt;
			}
			fclose(file);
			printf("%zu lines loaded.\n", _dic.size());
		} else {
			err = EXIT_FAILURE;
			fprintf(stderr, "Dictionary file '%s' is not available for reading.\n", _cli.dic_file.value.c_str());
		}
		return err;
	}

	void run() {
		auto tm_before = time(nullptr);

		unsigned rounds_left = _cli.rounds;
		unsigned cnt_total = 0;
		unsigned cnt_mistakes = 0;

		while(rounds_left--) {
			auto rng = std::default_random_engine(time(nullptr));
			std::shuffle(_dic.begin(), _dic.end(), rng);

			for(const auto& item : _dic) {

				switch(_cli.action.method.value) {
					case NihongoNoTangoCli::EnumMethod::KANJI:
						if(not item.kanji.empty()) {
							printf("%s ", item.kanji.c_str());
							fflush(stdout);
						} else {
							continue;
						}
						break;

					case NihongoNoTangoCli::EnumMethod::TRANSLATION:
						printf("%s ", item.translation.c_str());
						fflush(stdout);
						break;

					case NihongoNoTangoCli::EnumMethod::AUDIO:
						if(not item.kanji.empty()) {
							say(item.kanji);
						} else {
							say(item.kana);
						}
						break;

					default:
						break;
				}

				++cnt_total;

				// Read the output.
				std::string output;
				read_line(stdin, output, true);

				// // Check the result.
				if(output != item.kana) {
					++cnt_mistakes;
					printf("%s", TermColor::front(TermColor::RED));
					printf("'%s'", item.kana.c_str());
					printf("\n%s", TermColor::reset());
				}
				printf("\n");

			}

		}

		double cnt_mistakes_percent = cnt_mistakes;
		cnt_mistakes_percent /= cnt_total;
		cnt_mistakes_percent *= 100.;
		printf("Mistakes : %u (%.2f%%).", cnt_mistakes, cnt_mistakes_percent);
		const unsigned seconds_total = time(nullptr) - tm_before;
		printf(" %u seconds.\n", seconds_total);
	}

private:

	bool read_line(FILE* input, std::string& buf, const bool skip_spaces) {
		buf.resize(0);
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
		return ch != EOF;
	}

	static std::vector<std::string> split_by_char(const std::string &str, const char delim) {
		std::vector<std::string> result;
		std::stringstream ss (str);
		std::string item;

		while (getline (ss, item, delim)) {
			result.push_back (item);
		}

		return result;
	}

	static std::string trim_right(std::string str, const char* space) {
		str.erase(str.find_last_not_of(space) + 1);
		return str;
	}

	static std::string trim_left(std::string str, const char* space) {
		str.erase(0, str.find_first_not_of(space));
		return str;
	}

	static std::string trim(std::string str, const char* space) {
		return trim_right(trim_left(std::move(str), space), space);
	}

	static std::string trim_left(const std::string& str, const std::string& whitespace = " \t") {
		const auto strBegin = str.find_first_not_of(whitespace);
		if (strBegin == std::string::npos) {
			return ""; // no content
		}

		const auto str_end = str.find_last_not_of(whitespace);
		const auto str_range = str_end - strBegin + 1;

		return str.substr(strBegin, str_range);
	}

	void say(const std::string& to_say) {
		std::string command("trans -b -p  :en :jpn \"");
		command.append(to_say);
		command.append("\" >> /dev/null");

		const auto err = system(command.c_str());
		if(err != EXIT_SUCCESS) {
			fprintf(stderr, "system(\"%s\") fails\n", command.c_str());
			exit(err);
		}
	}

};

int main(int argc, char** argv) {
	NihongoNoTangoCli cli;

	if(not cli.parse_args(argc, argv)) {
		cli.print_usage(stderr);
		return EXIT_FAILURE;
	}

	NihongoNoTango app(cli);
	const int err = app.load();
	if(err == EXIT_SUCCESS) {
		app.run();
	}

	return EXIT_SUCCESS;
}
