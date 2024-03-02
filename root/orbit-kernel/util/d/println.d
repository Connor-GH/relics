module println;
private import gcc.builtins;

alias __builtin_va_list va_list;

extern(C) T va_arg(T)(ref va_list _argptr) => T.init;

deprecated("To be removed in 0.0.7 for being redundant")
extern (C) int println_d(char *fmt, va_list *argp) {
	va_arg!int(*argp);
	return 0;
}

