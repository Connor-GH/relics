#include <ps2_keyboard.h>
#include <typedefs.h>
#include <orbit-kernel/stdbool.h>

static bool char_pressed_state = false;
static char most_recent_char = '\0';

void
add_character_pressed(void)
{
	char_pressed_state = !char_pressed_state;
}
void
add_most_recent_char(char c)
{
	most_recent_char = c;
}
char
get_most_recent_char(void)
{
	char tmp = most_recent_char;
	most_recent_char = '\0'; // invalidate most recent char
	return tmp;
}

static bool is_shifted = false;
static bool is_alted = false;
static bool is_ctrled = false;
static bool is_caps_locked = false;

char
to_character(uint8_t scan_code)
{
	if (scan_code == 0x3A) {
		is_caps_locked = !is_caps_locked;
		return 0;
	} else if (scan_code == 0x38) {
		is_alted = true;
		return 0;
	} else if (scan_code == 0xB8) {
		is_alted = false;
		return 0;
	} else if (scan_code == 0x2A) {
		is_shifted = true;
		return 0;
	} else if (scan_code == 0xAA) {
		is_shifted = false;
		return 0;
	} else if (scan_code == 0x1D) {
		is_ctrled = true;
		return 0;
	} else if (scan_code == 0x9D) {
		is_ctrled = false;
		return 0;
	}
	if ((!is_shifted && !is_caps_locked) || (is_shifted && is_caps_locked)) {
		switch (scan_code) {
		default:
			return 0;
		case 1:
			return 1;
		case 2:
			return '1';
		case 3:
			return '2';
		case 4:
			return '3';
		case 5:
			return '4';
		case 6:
			return '5';
		case 7:
			return '6';
		case 8:
			return '7';
		case 9:
			return '8';
		case 10:
			return '9';
		case 11:
			return '0';
		case 12:
			return '-';
		case 13:
			return '=';
		case 14:
			return '\b';
		case 15:
			return '\t';
		case 16:
			return 'q';
		case 17:
			return 'w';
		case 18:
			return 'e';
		case 19:
			return 'r';
		case 20:
			return 't';
		case 21:
			return 'y';
		case 22:
			return 'u';
		case 23:
			return 'i';
		case 24:
			return 'o';
		case 25:
			return 'p';
		case 26:
			return '[';
		case 27:
			return ']';
		case 28:
			return '\n';
		case 30:
			return 'a';
		case 31:
			return 's';
		case 32:
			return 'd';
		case 33:
			return 'f';
		case 34:
			return 'g';
		case 35:
			return 'h';
		case 36:
			return 'j';
		case 37:
			return 'k';
		case 38:
			return 'l';
		case 39:
			return ';';
		case 40:
			return '\'';
		case 41:
			return '`';
		case 43:
			return '\\';
		case 44:
			return 'z';
		case 45:
			return 'x';
		case 46:
			return 'c';
		case 47:
			return 'v';
		case 48:
			return 'b';
		case 49:
			return 'n';
		case 50:
			return 'm';
		case 51:
			return ',';
		case 52:
			return '.';
		case 53:
			return '/';
		case 57:
			return ' ';
		}

	} else if (is_caps_locked || is_shifted) {
		switch (scan_code) {
		default:
			return 0;
		case 1:
			return 1;
		case 2:
			return '!';
		case 3:
			return '@';
		case 4:
			return '#';
		case 5:
			return '$';
		case 6:
			return '%';
		case 7:
			return '^';
		case 8:
			return '&';
		case 9:
			return '*';
		case 10:
			return '(';
		case 11:
			return ')';
		case 12:
			return '_';
		case 13:
			return '+';
		case 14:
			return '\b';
		case 15:
			return '\t';
		case 16:
			return 'Q';
		case 17:
			return 'W';
		case 18:
			return 'E';
		case 19:
			return 'R';
		case 20:
			return 'T';
		case 21:
			return 'Y';
		case 22:
			return 'U';
		case 23:
			return 'I';
		case 24:
			return 'O';
		case 25:
			return 'P';
		case 26:
			return '{';
		case 27:
			return '}';
		case 28:
			return '\n';
		case 30:
			return 'A';
		case 31:
			return 'S';
		case 32:
			return 'D';
		case 33:
			return 'F';
		case 34:
			return 'G';
		case 35:
			return 'H';
		case 36:
			return 'J';
		case 37:
			return 'K';
		case 38:
			return 'L';
		case 39:
			return ':';
		case 40:
			return '\"';
		case 41:
			return '~';
		case 43:
			return '|';
		case 44:
			return 'Z';
		case 45:
			return 'X';
		case 46:
			return 'C';
		case 47:
			return 'V';
		case 48:
			return 'B';
		case 49:
			return 'N';
		case 50:
			return 'M';
		case 51:
			return '<';
		case 52:
			return '>';
		case 53:
			return '?';
		case 57:
			return ' ';
		}
	}
  return 0;
}
