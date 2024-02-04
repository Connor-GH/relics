extern(C) void printk(const char *fmt, ...);

static void safe_println_real(T...)(T args) {
	foreach (arg; args) {
		if (is(arg == typeof(string)))
			printk("%s", arg);
	}
}
extern(C) void safe_println(const char *fmt ...) {
	safe_println_real(fmt);
}
