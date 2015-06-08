#ifndef __WARN_H__
#define __WARN_H__

#ifdef __GNUC__
#define FORMAT(a,b) __attribute__((format(printf, a, b)))
#define NORETURN __attribute__((noreturn))
#else
#define FORMAT(a,b)
#define NORETURN
#endif

void warn(const char* fmt, ...) FORMAT(1,2);
void die(const char* fmt, ...) FORMAT(1,2) NORETURN;

#endif
