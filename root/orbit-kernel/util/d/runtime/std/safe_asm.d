module safe_asm;
enum safe_asm(string fmt) = "asm @trusted @nogc nothrow {\"" ~ fmt ~ "\";}";
