#include <cstdio>
#include "log.h"
//void log (int log_level, const char *file_name, int line_num, const char *format, ...)
//__LINE__：在源代码中插入当前源代码行号；
//
//__FILE__：在源文件中插入当前源文件名；

int main ()
{
  int a = 1;

  log (LOG_INFO, __FILE__, __LINE__, "hello");
}
