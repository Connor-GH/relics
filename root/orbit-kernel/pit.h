#ifndef PIT_H
#define PIT_H
#include <typedefs.h>
#include <stdbool.h>
void millisleep(uint64_t);
void reprogram_timer(uint16_t);
bool PIT_enabled(void);
#endif /* PIT_H */
