#ifndef __LOG_H_
#define __LOG_H_

#define ERRORLEN  1024
#define TIMELEN   128


typedef enum {
    SYSLOG_LEVEL_QUIT   ,
    SYSLOG_LEVEL_ERROR  ,
    SYSLOG_LEVEL_FATAL  ,
    SYSLOG_LEVEL_ERRINFO,
    SYSLOG_LEVEL_DEBUG1 ,
    SYSLOG_LEVEL_DEBUG2 ,
    SYSLOG_LEVEL_DEBUG3 ,
    SYSLOG_LEVEL_NOSET  = -1
} LogLevel;

struct {
    const char *name;
    LogLevel    val;
}log_levels[] = 
{
    { "QUIT"   , SYSLOG_LEVEL_QUIT   } ,
    { "ERROR"  , SYSLOG_LEVEL_ERROR  } ,
    { "FATAL"  , SYSLOG_LEVEL_FATAL  } ,
    { "ERRINFO", SYSLOG_LEVEL_ERRINFO} ,
    { "DEBUG1" , SYSLOG_LEVEL_DEBUG1 } ,
    { "DEBUG2" , SYSLOG_LEVEL_DEBUG2 } ,
    { "DEBUG3" , SYSLOG_LEVEL_DEBUG3 } ,
    { "NULL"   , SYSLOG_LEVEL_NOSET  }
};

const char *log_level_name(LogLevel _VAL_);
static void err_doit(LogLevel _VAL_, const char *fmt, va_list ap);
void  err_quit(const char *fmt, ...);
void  err_fatal(const char *fmt, ...);
void  err_sys(const char *fmt, ...);
void  get_sys_time(char *buf, int len);
void  err_msg(const char *fmt, ...);
void  debug1_msg(const char *fmt, ...);
void  debug2_msg(const char *fmt, ...);
void  debug3_msg(const char *fmt, ...);


#endif// end __log_h
