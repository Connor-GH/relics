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
strlen(const char *s)
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
int
strcmp(const char *s1, const char *s2)
{
	for (size_t i = 0; i < strlen(s1); i++) {
		if (s1[i] - s2[i] == 0)
			continue;
		return s1[i] - s2[i];
	}
	return 0;
}

int
strncmp(const char *s1, const char *s2, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		if (s1[i] == s2[i])
			continue;
		return s1[i] - s2[i];
	}
	return 0;
}

void *
memcpy(void *dest, void *src, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		((char *)dest)[i] = ((char *)src)[i];
	}
	return (void *)dest;
}

void *
memset(void *s, int c, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		((uint8_t *)s)[i] = (uint8_t)c;
	}
	return s;
}

char *
strcpy(char *dest, const char *src)
{
	size_t i;
	for (i = 0; src[i] != '\0'; i++)
		dest[i] = src[i];

	return dest;
}

char *
kernel_strncpy(char *dest, const char *src, size_t n)
{
	size_t i;
	for (i = 0; i < n && src[i] != '\0'; i++)
		dest[i] = src[i];
	for (; i < n; i++)
		dest[i] = '\0';

	return dest;
}
