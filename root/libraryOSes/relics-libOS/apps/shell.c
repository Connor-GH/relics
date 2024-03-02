#include <apps/shell.h>
#include <orbit-kernel/kio.h>
#include <orbit-kernel/string.h>
#include <orbit-kernel/cpu.h>
#include <orbit-kernel/panic.h>
#include <orbit-kernel/pit.h>

static void
sleep(int x)
{
	millisleep((uint64_t)x * 1000);
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
	printk("\n");
}

static void
cursor_builtin(const char *buf)
{
	/* command goes as follows:
     * COMMAND -E/D
     */
	if (buf[7] == '-') {
		switch (buf[8]) {
		case 'e':
			enable_cursor(1, 1);
			break;
		case 'd':
			disable_cursor();
			break;
		default:
			break;
		}
	} else {
		printk("Options:\n"
			   "CURSOR [options]\n"
			   "  -e: enables blinking cursor\n"
			   "  -d: disables blinking cursor\n");
	}
}
static void
sleep_builtin(const char *buf)
{
	if ('1' <= buf[7] && buf[7] <= '9')
		sleep(buf[7] - 0x30);
}

void
relics_shell(const char *prompt)
{
	char command_buf[4096];
	reset_video_memory();
	printk("%s ", prompt);
	for (int i = 0; i < (int)sizeof(command_buf); i++) {
		char c = (char)getchark();
		if (c != '\n')
			command_buf[i] = c;
		printk("%c", c);
		if (c == '\n') {
			if (strncmp(command_buf, "fetchme", 7) == 0) {
				fetchme_builtin();
			} else if (strncmp(command_buf, "echo ", 5) == 0) {
				echo_builtin(command_buf);
			} else if (strncmp(command_buf, "cursor", 6) == 0) {
				cursor_builtin(command_buf);
			} else if (strncmp(command_buf, "panic", 5) == 0) {
				panic(GENERIC_ISSUE);
			} else if (strncmp(command_buf, "sleep", 5) == 0) {
				sleep_builtin(command_buf);
			} else {
				printk("`%s' is not a valid command.\n", command_buf);
			}
			for (; i >= 0; i--) {
				command_buf[i] = 0;
			}
			printk("%s ", prompt);
		}
	}
}
