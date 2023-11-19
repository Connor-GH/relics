#include <keyboard.h>
#include <kio.h>
#include <get_ascii_char.h>


char get_input_keycode(void) {

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

void test_input(void) {

    char ch = 0;
    char keycode = 0;
    wait_for_io(0x2FFFFFF);

    do {
        keycode = get_input_keycode();

        if(keycode == KEY_ENTER) printk("\n");

        else {
            ch = get_ascii_char(keycode);
            printk("%c", ch);
        }
        wait_for_io(0x2FFFFFF);


    } while (ch > 0);
}
