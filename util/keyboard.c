#include "include/keyboard.h"
#include "include/kio.h"
#include "include/get_ascii_char.h"


static uint8_t inb(uint16_t port) {

    uint8_t ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "d"(port));

    return ret;
}

static void outb(uint16_t port, uint8_t data) {

    __asm__ __volatile__("outb %0, %1" : "=a"(data) : "d"(port));
}

static char get_input_keycode(void) {

    char ch = 0;
    while((ch = inb(KEYBOARD_PORT)) != 0) {

        if(ch > 0) return ch;
    }
    return ch;
}

/*
keep the cpu busy for doing nothing(nop)
so that io port will not be processed by cpu
here timer can also be used, but lets do this in looping counter
*/
static void wait_for_io(uint32_t timer_count) {


    while(1) {
/*
 * important notes on sleep timers:
 * NO-OP xor (xor %eax, %ebx; xor %ebx, %eax)
 * NO-OP add (add $0, %esp)
 * NO-OP nop (xchg %eax, %eax)
 *
 * speed:
 * fast        slow
 * ----------------
 * add < nop < xor
 */

        __asm__ __volatile__(
        "xchg %eax, %eax");
        timer_count--;

        if(timer_count <= 0) break;
    }
}
void sleep(uint32_t timer_count) {
    wait_for_io(timer_count);
}

void test_input(void) {

    char ch = 0;
    char keycode = 0;
    sleep(0x2FFFFFF);

    do {
        keycode = get_input_keycode();

        if(keycode == KEY_ENTER) print_new_line();

        else {
            ch = get_ascii_char(keycode);
            print_char(ch);
        }
        sleep(0x2FFFFFF);


    } while (ch > 0);
}
