#include <apps/shell.h>
#include <orbit-kernel/kio.h>
#include <orbit-kernel/string.h>
#include <orbit-kernel/memory.h>
#include <orbit-kernel/keyboard.h>
#include <orbit-kernel/get_ascii_char.h>
#include <orbit-kernel/cpu.h>
#include <orbit-kernel/panic.h>

static void
sleep(int x)
{
	for (int i = 0; i < x; i++) {
		__asm__ __volatile__("nop");
	}
}
static void
fetchme_builtin(void)
{
	set_cpu_vendor_name();
	printk("OS: Relics version %s\n"
		   "CPU: %s\n"
		   "Kernel: Orbit ExoKernel\n"
		   "LibOS: Relics builtin libOS\n"
		   "Shell: Relics builtin shell\n",
		   VERSION, cpu_vendor_name);
}
static void
echo_builtin(char *buf)
{
	int i = 5; /* 6th byte in command sequence */
	while (buf[i] != '\n' && buf[i] != '\0') {
		printk("%c", buf[i]);
		i++;
	}
}

static void
cursor_builtin(const char *buf)
{
	/* command goes as follows:
     * COMMAND -E/D
     */
	if (buf[7] == '-') {
		switch (buf[8]) {
		case 'E':
			enable_cursor(1, 1);
			break;
		case 'D':
			disable_cursor();
			break;
		default:
			break;
		}
	} else {
		printk("Options:\n"
			   "CURSOR [options]\n"
			   "  -E: enables blinking cursor\n"
			   "  -D: disables blinking cursor\n");
	}
}

void
relics_shell(const char *prompt)
{
	char command_buf[4096];
	char keycode = 0;
	reset_video_memory();
	printk("%s ", prompt);
	for (int i = 0; i < (int)sizeof(command_buf); i++) {
		sleep(0x2FFFFFF);
		sleep(0x2FFFFFF);

		keycode = get_input_keycode();
		command_buf[i] = get_ascii_char(keycode);
		printk("%c", command_buf[i]);
		if (keycode == KEY_ENTER) {
			printk("\n");
			if (strncmp(command_buf, "FETCHME", 7) == 0) {
				fetchme_builtin();
			} else if (strncmp(command_buf, "ECHO ", 5) == 0) {
				echo_builtin(command_buf);
			} else if (strncmp(command_buf, "CURSOR", 6) == 0) {
				cursor_builtin(command_buf);
			} else if (strncmp(command_buf, "PANIC", 5) == 0) {
				panic(GENERIC_ISSUE);
			} else {
				printk("`%s' is not a valid command.\n", command_buf);
			}
			for (i; i >= 0; i--) {
				command_buf[i] = 0;
			}
			printk("%s ", prompt);
		}
	}
}
