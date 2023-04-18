#ifndef PORT_FUNCTIONS_H
#define PORT_FUNCTIONS_H

#include <inttypes.h>
uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t data);
#endif /* PORT_FUNCTIONS_H */
