#ifndef APPLOADER_H
#define APPLOADER_H

/* This may look confusing, but it is an
 * early attempt at loading "apps" without
 * building an ELF loader (yet), since there
 * is not a filesystem. */
#define RELICS_SHELL "LOAD-APP-0908"
__attribute__((noreturn)) void
load_app(char *appcode, long *address);

#endif /* APPLOADER_H */
