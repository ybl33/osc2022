#include "string.h"

void *memcpy (void *dst, void *src, size_t n) {

    unsigned char *d = (unsigned char *)dst;
    unsigned char *s = (unsigned char *)src;

    for (int i = 0; i < n; i++) {
        *d = *s;
        d++; s++;
    }

    return dst;
}

void *memset (void *dst, int value, size_t n) {

    unsigned char *d = (unsigned char *)dst;

    for (int i = 0; i < n; i++) {
        *d = (unsigned char) value;
        d++;
    }

    return dst;
}

long strlen (char *s) {

    long n = 0;
    
    while (*s) {
        s++; n++;
    } 

    return n;
}

char *strcpy (char *dst, char *src) {

    char *d = dst;

    while (*src) {
        *d = *src;
        d++; src++;
    }

    *d = '\0';

    return dst;
}

char *strncpy (char *dst, char *src , size_t n) {

    char *d = dst;

    while (n > 0 && *src) {
        *d = *src;
        d++; src++; n--;
    }

    while (n > 0) {
        *d = '\0';
        d++; n--;
    }

    return dst;
}

char *strcat (char *dst, char *src) {

    char *d = dst;
    
    while (*d) d++;

    while (*src) {
        *d = *src; 
        d++; src++;
    }

    *d = '\0';

    return dst;
}

char *strncat (char *dst, char *src, size_t n) {
    
    char *d = dst;
    
    while (*d) d++;

    while (n > 0 && *src) {
        *d = *src; 
        d++; src++; n--;
    }

    *d = '\0';

    return dst;
}


int strcmp (char *s1, char*s2) {

    int v;

    s1--; s2--;

    do {
        s1++; s2++;
        if (*s1 == *s2)
        {
            v = 0;
        }
        else if (*s1 < *s2) 
        {
            v = -1;
            break;
        } 
        else if (*s1 > *s2) 
        {
            v = 1;
            break;
        }

    } while (*s1 != '\0' && *s2 != '\0');

    return v;
}

int strncmp (char *s1, char *s2, size_t n) {

    int v;

    s1--; s2--;
    
    do {

        s1++; s2++;
        
        if (*s1 == *s2)
        {
            v = 0;
        }
        else if (*s1 < *s2) 
        {
            v = -1;
            break;
        } 
        else if (*s1 > *s2) 
        {
            v = 1;
            break;
        }

        n--;
        
    } while (n > 0 && *s1 != '\0' && *s2 != '\0');

    return v;
}

char *strinsert (char *s, char c, int pos) {

    char *p;
    char tmp;
    
    if (pos <= strlen(s)) 
    {
        p = s + pos;

        do
        {

            tmp = *p;
            *p = c;
            c = tmp;
            p++;

        } while (c);

        *p = c;
    
    }
    
    return s;
}

char *strpullout (char *s, int pos) {

    char *p;
    
    if (pos < strlen(s)) 
    {
        p = s + pos;

        do { *p = *(p+1); p++;} while (*p);

        *p = '\0';
    
    }
    
    return s;
}