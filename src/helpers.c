#include "../include/kernel.h"

size_t strlen(const char* str)
{
    size_t len = 0;
    while (str[len])
    {
        len++;
    }
    return len;
}

char* strcpy(char* dest, const char* src)
{
    char* original_dest = dest;
    while (*src)
    {
        *dest++ = *src++;
    }
    *dest = '\0'; 
    return original_dest;
}

int strcmp(const char* str1, const char* str2)
{
	while (*str1 && (*str1 == *str2))
    {
		str1++;
		str2++;
	}
	return *(const unsigned char*)str1 - *(const unsigned char*)str2;
}

void* memcpy(void* dest, const void* src, size_t n)
{
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    for (size_t i = 0; i < n; i++)
    {
        d[i] = s[i];
    }
    return dest;
}

void* memset(void* s, int c, size_t n)
{
    unsigned char* p = (unsigned char*)s;
    
    for (size_t i = 0; i < n; i++)
    {
        p[i] = (unsigned char)c;
    }
    return s;
}


int strncmp(const char* str1, const char* str2, size_t n)
{
    while (n && *str1 && (*str1 == *str2))
    {
        str1++;
        str2++;
        n--;
    }
    
    if (n == 0) return 0;
    return *(const unsigned char*)str1 - *(const unsigned char*)str2;
}

char* strncpy(char* dest, const char* src, size_t n)
{
    char* original_dest = dest;
    
    while (n-- && *src)
    {
        *dest++ = *src++;
    }
    while (n-- > 0)
    {
        *dest++ = '\0';
    }
    
    return original_dest;
}