#ifndef PS2_KEYBOARD_H
#define PS2_KEYBOARD_H
#include <typedefs.h>
char
to_character(uint8_t scan_code);
void
add_character_pressed(void);
#endif /* PS2_KEYBOARD_H */
