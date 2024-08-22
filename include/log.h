#ifndef __LOG_H
#define __LOG_H

#include <stdarg.h>

#define LOG_DEBUG_TYPE  0
#define LOG_MSG_TYPE    1
#define LOG_WARN_TYPE   2
#define LOG_ERR_TYPE    3

void ember_log(int, const char*);
void ember_logx(int, const char *errstr, const char*fmt,va_list);
void ember_msgx(const char* fmt,...);
void ember_debugx(const char* fmt,...);

#define LOG_MSG(msg) ember_log(LOG_MSG_TYPE, msg)
#define LOG_ERR(msg) ember_log(LOG_DEBUG_TYPE, msg)

#endif