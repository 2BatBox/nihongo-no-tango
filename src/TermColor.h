#pragma once

struct TermColor {

	static constexpr const char* FRONT_RED = "\033[1;31m";
	static constexpr const char* FRONT_GREEN = "\033[1;32m";
	static constexpr const char* FRONT_YELLOW = "\033[1;33m";
	static constexpr const char* FRONT_BLUE = "\033[1;34m";
	static constexpr const char* FRONT_PURPLE = "\033[1;35m";
	static constexpr const char* FRONT_CYAN = "\033[1;36m";
	static constexpr const char* FRONT_WHITE = "\033[1;37m";

	static constexpr const char* FRONT_RED_LIGHT = "\033[1;91m";
	static constexpr const char* FRONT_GREEN_LIGHT = "\033[1;92m";
	static constexpr const char* FRONT_YELLOW_LIGHT = "\033[1;93m";
	static constexpr const char* FRONT_BLUE_LIGHT = "\033[1;94m";
	static constexpr const char* FRONT_PURPLE_LIGHT = "\033[1;95m";
	static constexpr const char* FRONT_CYAN_LIGHT = "\033[1;96m";
	static constexpr const char* FRONT_WHITE_LIGHT = "\033[1;97m";

	static constexpr const char* BACK_RED = "\033[1;41m";
	static constexpr const char* BACK_GREEN = "\033[1;42m";
	static constexpr const char* BACK_YELLOW = "\033[1;43m";
	static constexpr const char* BACK_BLUE = "\033[1;44m";
	static constexpr const char* BACK_PURPLE = "\033[1;45m";
	static constexpr const char* BACK_CYAN = "\033[1;46m";
	static constexpr const char* BACK_WHITE = "\033[1;47m";

	static constexpr const char* BACK_RED_LIGHT = "\033[1;101m";
	static constexpr const char* BACK_GREEN_LIGHT = "\033[1;102m";
	static constexpr const char* BACK_YELLOW_LIGHT = "\033[1;103m";
	static constexpr const char* BACK_BLUE_LIGHT = "\033[1;104m";
	static constexpr const char* BACK_PURPLE_LIGHT = "\033[1;105m";
	static constexpr const char* BACK_CYAN_LIGHT = "\033[1;106m";
	static constexpr const char* BACK_WHITE_LIGHT = "\033[1;107m";

	static constexpr const char* RESET = "\033[0m";

	enum ColorCode : unsigned {
		RED,
		GREEN,
		YELLOW,
		BLUE,
		PURPLE,
		CYAN,
		WHITE,

		RED_LIGHT,
		GREEN_LIGHT,
		YELLOW_LIGHT,
		BLUE_LIGHT,
		PURPLE_LIGHT,
		CYAN_LIGHT,
		WHITE_LIGHT,

		NONE
	};

	static constexpr const char* front(const ColorCode clr) {
		switch(clr) {
			case ColorCode::RED :			return FRONT_RED;
			case ColorCode::GREEN :			return FRONT_GREEN;
			case ColorCode::YELLOW :		return FRONT_YELLOW;
			case ColorCode::BLUE :			return FRONT_BLUE;
			case ColorCode::PURPLE :		return FRONT_PURPLE;
			case ColorCode::CYAN :			return FRONT_CYAN;
			case ColorCode::WHITE :			return FRONT_WHITE;

			case ColorCode::RED_LIGHT :		return FRONT_RED_LIGHT;
			case ColorCode::GREEN_LIGHT :	return FRONT_GREEN_LIGHT;
			case ColorCode::YELLOW_LIGHT :	return FRONT_YELLOW_LIGHT;
			case ColorCode::BLUE_LIGHT :	return FRONT_BLUE_LIGHT;
			case ColorCode::PURPLE_LIGHT :	return FRONT_PURPLE_LIGHT;
			case ColorCode::CYAN_LIGHT :	return FRONT_CYAN_LIGHT;
			case ColorCode::WHITE_LIGHT :	return FRONT_WHITE_LIGHT;

			default:
				return RESET;
		}
	}

	static const char* back(const ColorCode clr) {
		switch(clr) {
			case ColorCode::RED :			return BACK_RED;
			case ColorCode::GREEN :			return BACK_GREEN;
			case ColorCode::YELLOW :		return BACK_YELLOW;
			case ColorCode::BLUE :			return BACK_BLUE;
			case ColorCode::PURPLE :		return BACK_PURPLE;
			case ColorCode::CYAN :			return BACK_CYAN;
			case ColorCode::WHITE :			return BACK_WHITE;

			case ColorCode::RED_LIGHT :		return BACK_RED_LIGHT;
			case ColorCode::GREEN_LIGHT :	return BACK_GREEN_LIGHT;
			case ColorCode::YELLOW_LIGHT :	return BACK_YELLOW_LIGHT;
			case ColorCode::BLUE_LIGHT :	return BACK_BLUE_LIGHT;
			case ColorCode::PURPLE_LIGHT :	return BACK_PURPLE_LIGHT;
			case ColorCode::CYAN_LIGHT :	return BACK_CYAN_LIGHT;
			case ColorCode::WHITE_LIGHT :	return BACK_WHITE_LIGHT;

			default:
				return RESET;
		}
	}

	static const char* reset() {
		return RESET;
	}

};
