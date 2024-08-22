#include "../include/log.h"
#include <stdio.h>
#include <string.h>
#include <syslog.h>

#define MAXLINE 4096

void ember_log(int severity, const char *msg) {
    const char *severity_str;
    switch(severity){
        case LOG_DEBUG_TYPE:
            severity_str = "\33[1;35mdebug\33[0m";
            break;
        case LOG_MSG_TYPE:
            severity_str = "\33[1;36mmsg\33[0m";
            break;
        case LOG_ERR_TYPE:
            severity_str = "\33[1;37merr\33[0m";
            break;
        default:
            severity_str = "!!!";
            break;
    }

    fprintf(stdout,"[%s] %s\n",severity_str, msg);
}

void ember_logx(int severity, const char* errstr, const char *fmt, va_list ap) {
    char buf[1024];
    size_t len;

    if(fmt)
        vsnprintf(buf, sizeof(buf), fmt, ap);
    else
        buf[0] = '\0';

    if(errstr) {
        len = strlen(buf);
        if (len < sizeof(buf) - 3)  {
            snprintf(buf + len, sizeof(buf) - len, ": %s", errstr);
        }
    }
    ember_log(severity, buf);
}

void ember_msgx(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    // printf("\33[1;36m");
    ember_logx(LOG_MSG_TYPE, NULL, fmt, ap);
    // printf("\33[0m");
    va_end(ap);
}

void ember_debugx(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    // printf("\33[1;35m");
    ember_logx(LOG_DEBUG_TYPE, NULL, fmt, ap);
    // printf("\33[0m");
    va_end(ap);
}

