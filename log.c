#include "hemalloc.h"
#include "log.h"

static FILE *sys_log_fp;

const char *log_level_name(LogLevel _VAL_)
{
    int i;
    if (_VAL_ != SYSLOG_LEVEL_NOSET) {
        for (i = 0; log_levels[i].val != SYSLOG_LEVEL_NOSET; i++) {
            if (log_levels[i].val == _VAL_) {
                return log_levels[i].name;
            }//end if
        }//end for
    }//end if

    return NULL;
}

static void err_doit(LogLevel _VAL_, const char *fmt, va_list ap)
{
    int err_no, n;

    char buf[ERRORLEN + 1] = {0};
    char time[TIMELEN + 1] = {0};
    get_sys_time(time, TIMELEN);

    err_no = errno;
    
    snprintf(buf, ERRORLEN, "%s %s :", time, log_level_name(_VAL_));
    n = strlen(buf);
    
    vsnprintf(buf + n, ERRORLEN - n, fmt, ap);
    n = strlen(buf);

    if (_VAL_ <= 3) {
        if (_VAL_ == SYSLOG_LEVEL_ERROR)
            snprintf(buf+n, ERRORLEN - n, ": %s", strerror(err_no));
        
        strcat(buf, "\n");
        sys_log_fp = fopen("/mnt/syslog", "a+");
        
        if ( !fwrite(buf, strlen(buf), 1, sys_log_fp))
            fprintf(stderr, "write logfile error.\n");
        
        fclose(sys_log_fp);
    }
    else
        fprintf(stdout, "%s\n", buf);
    
    return;
}

void get_sys_time(char *buf, int len)
{
    time_t now;

    struct tm *tm_now;

    time(&now);
    tm_now = localtime(&now);

    strftime(buf, len, "%Z %Y %a %b %H:%M:%S ", tm_now);
}

void err_sys(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(SYSLOG_LEVEL_ERROR, fmt, ap);
    va_end(ap);

    exit(-1);
}

void err_quit(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(SYSLOG_LEVEL_QUIT, fmt, ap);
    va_end(ap);

    exit(-1);
}

void err_fatal(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(SYSLOG_LEVEL_FATAL, fmt, ap);
    va_end(ap);

    exit(-1);
}

void err_msg(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(SYSLOG_LEVEL_ERRINFO, fmt, ap);
    va_end(ap);

    return;
}


void debug1_msg(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(SYSLOG_LEVEL_DEBUG1, fmt, ap);
    va_end(ap);

    return;
}

void debug2_msg(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(SYSLOG_LEVEL_DEBUG2, fmt, ap);
    va_end(ap);

    return;
}

void debug3_msg(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(SYSLOG_LEVEL_DEBUG3, fmt, ap);
    va_end(ap);

    return;
}
