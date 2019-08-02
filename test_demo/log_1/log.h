//
// Created by S on 2019/7/26.
//

#ifndef _LOG_H_
#define _LOG_H_

#include <syslog.h>
#include <cstdarg>

void set_loglevel (int log_level = LOG_DEBUG);
void log (int log_level, const char *file_name, int line_num, const char *format, ...);

#endif //_LOG_H_
