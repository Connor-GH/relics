#include <stddef.h>
#include <kio.h>
#include <panic.h>
#include <orbit.h>

#define ERR_MSG_DEFAULT                                \
	("An unrecoverable error has occurred.\n"          \
	 "The system will now go into an infinite loop.\n" \
	 "Please restart the system through hardware.")

ATTR(noreturn) void panic2(int i, const char *errmsg)
{
	const char *issue;
	switch (i) {
	case CPU_ISSUE:
		issue = "processor-related";
		break;
	case MEMORY_ISSUE:
		issue = "memory-related";
		break;
	case GRAPHICS_ISSUE:
		issue = "graphics-related";
		break;
	case STORAGE_ISSUE:
		issue = "storage-related";
		break;
	default: /* fall though */
	case GENERIC_ISSUE:
		issue = "generic";
		break;
	}

	printk("%s\n", errmsg);
	printk("Error code: %d (%s)\n", i, issue);

	while (1) {
		ASM("cli; hlt");
	}
}

ATTR(noreturn) void panic(int i)
{
	panic2(i, ERR_MSG_DEFAULT);
}
