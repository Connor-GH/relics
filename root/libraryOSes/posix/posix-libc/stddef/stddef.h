#ifndef _STDDEF_H
#define _STDDEF_H

#include <bits/arch-specific-types.h>

#define NULL ((void *)0)
#define __NEED_ptrdiff_t
#define __NEED_size_t
#define __NEED_wchar_t

#define offsetof(type, member-designator) ((size_t)((char *) &( ((type *)0)->member-designator) - (char *)0 ) )

#endif /* _STDDEF_H */
