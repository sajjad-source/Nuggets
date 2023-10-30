/* 
 * log module - a simple way to log messages to a file
 * 
 * David Kotz, May 2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include "log.h"

/**************** flog_init ****************/
/* Initialize the logging module.
 */
void flog_init(FILE* fp)
{
  flog_v(fp, "START OF LOG");
}

/**************** flog_s ****************/
/* 
 * log a string to the logfile, if logging is enabled.
 * The string `format` can reference '%s' to incorporate `str`.
 */
void
flog_s(FILE* fp, const char* format, const char* str)
{
  if (fp != NULL && format != NULL && str != NULL) {
    fprintf(fp, format, str);
    fputc('\n', fp);
    fflush(fp);
  }
}

/**************** flog_d ****************/
/* 
 * log an integer to the logfile, if logging is enabled.
 * The string `format` can reference '%d' to incorporate `num`.
 */
void
flog_d(FILE* fp, const char* format, const int num)
{
  if (fp != NULL && format != NULL) {
    fprintf(fp, format, num);
    fputc('\n', fp);
    fflush(fp);
  }
}

/**************** flog_c ****************/
/* 
 * log a character to the logfile, if logging is enabled.
 * The string `format` can reference '%c' to incorporate `ch`.
 */
void
flog_c(FILE* fp, const char* format, const char ch)
{
  if (fp != NULL && format != NULL) {
    fprintf(fp, format, ch);
    fputc('\n', fp);
    fflush(fp);
  }
}

/**************** flog_v ****************/
/* 
 * log a message to the logfile, if logging is enabled.
 */
void
flog_v(FILE* fp, const char* str)
{
  if (fp != NULL && str != NULL) {
    fputs(str, fp);
    fputc('\n', fp);
    fflush(fp);
  }
}

/**************** flog_e ****************/
/* 
 * log an error to the logfile, if logging is enabled.
 * Expects the global variable errno (sys/errno.h) to indicate the error,
 * so this is best used immediately after a system call.
 */
void
flog_e(FILE* fp, const char* str)
{
  if (fp != NULL && str != NULL) {
    fprintf(fp, "%s: %s\n", str, strerror(errno));
    fflush(fp);
  }
}

/**************** flog_done ****************/
/* 
 * Done with logging.  Notes this, then disables logging. 
 */
void
flog_done(FILE* fp)
{
  flog_v(fp, "END OF LOG");
}
