#include <orbit-kernel/string.h>
/*
 * strlen():
 *
 * There could possibly be an issue here
 * if the string length is outside of a signed
 * int's max size, but this is architecture
 * dependent.
 */
uint32_t
strlen(const char *__borrowed s)
{
	uint32_t length = 0;
	while (s[length])
		length++;
	return length;
}
/*
 * strcmp() and strncmp():
 *
 * We can safely assume that these strings will
 * match only if the lengths are the same, so
 * checking every array element in regards to
 * s1 is no issue.
 */

int memcmp(const void *__borrowed s1, const void *__borrowed s2, size_t n) {
	for (size_t i = 0; i < n; i++) {
		if (((const char *)s1)[i] == ((const char *)s2)[i])
			continue;
		return (((const char *)s1)[i] - ((const char *)s2)[i]);
	}
  return 0;
}
int
strcmp(const char *__borrowed s1, const char *__borrowed s2)
{
  return memcmp(s1, s2, strlen(s1));
}

int
strncmp(const char *__borrowed s1, const char *__borrowed s2, size_t n)
{
  return memcmp(s1, s2, n);
}

void *
memcpy(void *__inout dest, void *__borrowed src, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		((char *)dest)[i] = ((char *)src)[i];
	}
	return (void *)dest;
}

void *
memset(void *__inout s, int c, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		((uint8_t *)s)[i] = (uint8_t)c;
	}
	return s;
}

char *
strcpy(char *__inout dest, const char *__borrowed src)
{
	size_t i;
	for (i = 0; src[i] != '\0'; i++)
		dest[i] = src[i];

	return dest;
}

char *
kernel_strncpy(char *__inout dest, const char *__borrowed src, size_t n)
{
	size_t i;
	for (i = 0; i < n && src[i] != '\0'; i++)
		dest[i] = src[i];
	for (; i < n; i++)
		dest[i] = '\0';

	return dest;
}
