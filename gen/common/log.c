#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/klog.h>
#include <stdint.h>

#include "log.h"
#include "lock.h"

#define LOG_BUF_MAX 512
#define MAX_LOG_FILE_SIZE (0x200000)

enum LOG_TYPE{
    LOG_TYPE_CONSOLE = 0,
    LOG_TYPE_SYSLOGD,
    LOG_TYPE_TEXT_FILE,
    LOG_TYPE_MAX,
};

static int log_type = LOG_TYPE_CONSOLE;
static int log_file_fd = -1;
static int log_file_size = 0;
static int s_log_level = DBG_CUR_LEVEL;

static lock_t s_log_lock;

static char s_log_file[128] = {0};
void log_write(int level, const char *fmt, ...)
{
    char buf[LOG_BUF_MAX];
    va_list ap;
    int len = 0;

    if (level > s_log_level) return;
    if(log_type == LOG_TYPE_SYSLOGD){
        return ;
    }

    lock(&s_log_lock);
    va_start(ap, fmt);
    vsnprintf(buf + len, LOG_BUF_MAX, fmt, ap);
    buf[LOG_BUF_MAX - 1] = 0;
    va_end(ap);
    unlock(&s_log_lock);
    log_file_size += strlen(buf);
    if(log_file_size >= MAX_LOG_FILE_SIZE){
        log_file_size = strlen(buf);
        lseek(log_file_fd, 0, SEEK_SET);
    }
    //write(log_file_fd, buf, strlen(buf));
    //if ((log_type == LOG_TYPE_CONSOLE) || (log_file_fd < 0))
    {
        fprintf(stderr, "%s", buf);
        //return ;
    }

}


int log_level_set(int level)
{
    if(level >= 0){
        s_log_level = level;
    }

    return 0;
}

int log_read(char *log, int size)
{
    int len = 0;

    if(strlen(s_log_file) > 0){
        int fd;

        fd = open(s_log_file, O_RDONLY, 0655);
        if(fd > 0){
            len = read(fd, log, size);
            if(len < 0){
                len = 0;
            }

            close(fd);
        }
    }
    return len;
}

int log_init(const char *file, int level)
{
    log_file_fd = -1;

    lock_init(&s_log_lock);
    s_log_level = level;

    if(file){
        int temp_fd;
        char dir[128];
        char *tmp;

        if(!strcasecmp(file, "syslogd")){
            log_type = LOG_TYPE_SYSLOGD;
            openlog("broadvis", LOG_NDELAY, LOG_DAEMON);

            return 0;
        }
        if(!strcasecmp(file, "console")){
            log_type = LOG_TYPE_CONSOLE;

            return 0;
        }
        strcpy(dir, file);
        strncpy(s_log_file, file, sizeof(s_log_file));
        if((tmp = strrchr(dir, '/')) != NULL){
            char cmd[128];

            tmp[0] = 0;
            sprintf(cmd, "mkdir -p %s", dir);
            system(cmd);
        }
        temp_fd = open(file, O_RDWR | O_APPEND | O_SYNC | O_CREAT, 0655);
        if(temp_fd < 0){
            DBG(DBG_ERR, "can't open log file %s\n", file);
            log_type = LOG_TYPE_CONSOLE;

            return -1;
        }
        dup2(temp_fd, 1);
        dup2(temp_fd, 2);
        log_file_fd = temp_fd;
        log_type = LOG_TYPE_TEXT_FILE;
    }

    return 0;
}

int log_exit(void)
{
    if(log_type == LOG_TYPE_SYSLOGD){
        closelog();

        return 0;
    }
    if(log_file_fd > 0){
        close(log_file_fd);
        log_file_fd =  -1;
    }

    return 0;
}
