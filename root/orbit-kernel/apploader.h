#ifndef APPLOADER_H
#define APPLOADER_H
#include <orbit.h>

/* This may look confusing, but it is an
 * early attempt at loading "apps" without
 * building an ELF loader (yet), since there
 * is not a filesystem. */
#define RELICS_SHELL "LOAD-APP-0908"
void
load_app(char *__owned appcode, long *__borrowed address);

#endif /* APPLOADER_H */
