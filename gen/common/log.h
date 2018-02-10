#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <syslog.h>

#define DBG_LOG_MAX_LINE_SIZE   (1024)
#define DBG_LOG_MAX_LINES       (1024)

#define DBG_DEBUG      7
#define DBG_INFO       6
#define DBG_NOTICE     5
#define DBG_WARN       4
#define DBG_ERR        3
#define DBG_CRIT       2
#define DBG_FATAL      1
#define DBG_NONE       0

#define DBG_CUR_LEVEL  DBG_DEBUG

#define DBG(level, fmt...) \
    do{ \
        struct timeval __tv;\
        char __p[64];\
        struct tm *__t;\
        gettimeofday(&__tv, NULL);\
        __t = localtime(&__tv.tv_sec);\
        sprintf(__p, "%04d-%02d-%02d %02d:%02d:%02d", __t->tm_year + 1900, __t->tm_mon + 1, __t->tm_mday, __t->tm_hour, __t->tm_min, __t->tm_sec); \
        log_write(level, "%s.%7ld <%d>%s, %s, %d:", __p, __tv.tv_usec, level, __FILE__, __FUNCTION__, __LINE__);\
        log_write(level, fmt);\
    }while(0)

int log_init(const char *file, int level);
int log_exit(void);
int log_level_set(int level);
int log_read(char *log, int size);

void log_write(int level, const char *fmt, ...)
    __attribute__ ((format(printf, 2, 3)));

#endif //__LOG_H__
