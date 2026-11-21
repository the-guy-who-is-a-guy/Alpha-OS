#include "string.h"
#include "heap.h"
#include "string.h"
#include "stdarg.h"

size_t strlen(const char *s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

int strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n) {
    while (n && *a && *a == *b) { a++; b++; n--; }
    if (n == 0) return 0;
    return (unsigned char)*a - (unsigned char)*b;
}

char * strdup(const char *s) {
    size_t n = strlen(s) + 1;
    char *p = kmalloc(n);
    if (!p) return NULL;
    for (size_t i = 0; i < n; i++) p[i] = s[i];
    return p;
}

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args) {
    size_t written = 0;
    size_t i = 0;

    #define PUT(c) do { \
        if (written + 1 < size) buf[written] = (c); \
        written++; \
    } while (0)

    while (fmt[i]) {
        if (fmt[i] != '%') { PUT(fmt[i]); i++; continue; }
        i++;   /* skip '%' */

        switch (fmt[i]) {
            case 'd': {
                int64_t v = va_arg(args, int);
                if (v < 0) {
                    PUT('-');
                    v = -v;
                }
                /* convert to decimal */
                char tmp[20];
                int n = 0;
                if (v == 0) {
                    tmp[n++] = '0';
                } else {
                    while (v > 0) {
                        tmp[n++] = (char)('0' + (v % 10));
                        v /= 10;
                    }
                }
                while (n > 0) {
                    PUT(tmp[--n]);
                }
                break;
            }
            case 'u': {
                uint64_t v = va_arg(args, unsigned int);
                char tmp[20];
                int n = 0;
                if (v == 0) {
                    tmp[n++] = '0';
                } else {
                    while (v > 0) {
                        tmp[n++] = (char)('0' + (v % 10));
                        v /= 10;
                    }
                }
                while (n > 0) {
                    PUT(tmp[--n]);
                }
                break;
            }
            case 'x': {
                uint64_t v = va_arg(args, unsigned int);
                int started = 0;
                for (int shift = 60; shift >= 0; shift -= 4) {
                    uint8_t nib = (v >> shift) & 0xF;
                    if (nib != 0 || started || shift == 0) {
                        started = 1;
                        PUT(nib < 10 ? '0' + nib : 'a' + nib - 10);
                    }
                }
                break;
            }
            case 'X': {
                uint64_t v = va_arg(args, unsigned int);
                int started = 0;
                for (int shift = 60; shift >= 0; shift -= 4) {
                    uint8_t nib = (v >> shift) & 0xF;
                    if (nib != 0 || started || shift == 0) {
                        started = 1;
                        PUT(nib < 10 ? '0' + nib : 'A' + nib - 10);
                    }
                }
                break;
            }
            case 'p': {
                void *p = va_arg(args, void *);
                PUT('0');
                PUT('x');
                uint64_t v = (uint64_t)p;
                int started = 0;
                for (int shift = 60; shift >= 0; shift -= 4) {
                    uint8_t nib = (v >> shift) & 0xF;
                    if (nib != 0 || started || shift == 0) {
                        started = 1;
                        PUT(nib < 10 ? '0' + nib : 'a' + nib - 10);
                    }
                }
                break;
            }
            case 's': {
                const char *s = va_arg(args, const char *);
                if (!s) s = "(null)";
                while (*s) { PUT(*s); s++; }
                break;
            }
            case 'c': {
                PUT((char)va_arg(args, int));
                break;
            }
            case '%': {
                PUT('%');
                break;
            }
            default: {
                PUT('%');
                PUT(fmt[i]);
                break;
            }
        }
        i++;
    }

    if (size > 0) {
        buf[written < size ? written : size - 1] = '\0';
    }
    return (int)written;

    #undef PUT
}
int snprintf(char *buf, size_t size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int r = vsnprintf(buf, size, fmt, args);
    va_end(args);
    return r;
}
