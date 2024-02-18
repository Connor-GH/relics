#ifndef PS2_KEYBOARD_H
#define PS2_KEYBOARD_H
#include <typedefs.h>
char
to_character(uint8_t scan_code);
void
add_character_pressed(void);
void
add_most_recent_char(char);
char
get_most_recent_char(void);
#endif /* PS2_KEYBOARD_H */
