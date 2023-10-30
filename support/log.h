/* 
 * log module - a simple way to log messages to a file.
 * 
 * Users of this module should call log_init(fp); stderr is one option,
 * but it could be any file open for writing. Then call a sequence of 
 * log_s, log_d, log_c, log_v, log_e functions to write formatted information
 * to that log.  Finally, call log_done.
 * 
 * If the user of the module does not call log_init(), or calls log_init(NULL),
 * the log_x functions will be ignored and nothing will be logged.
 * 
 * The flog_x functions should not be called by the module user.
 * 
 * See the note below about file-local global variables; if log.h is included
 * by multiple source files within a single program, *each* such file has
 * its own logging fp and thus can independently control whether to log and
 * where to log.
 * 
 * David Kotz, May 2019
 */

#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>
#include <stdlib.h>

/*********** file-local global variable ****************/
/* Here is an example of a judicious use of a global variable.
 * This module stashes a file pointer(FP) for use in all the logging 
 * functions, so the module user need not pass the FP to every call.
 * This approach is unusual in that there is a copy of this variable local to 
 * *each file* that #includes "log.h" as well as to the module itself.
 * Each file that includes log.h will be able to log to its own file,
 * and thus *must* call log_init to provide that file descriptor.
 * Default is NULL, which means "do not log". 
 */
static FILE* logFP = NULL;

/*********** logging-related functions ****************/
/* Module users should call the inline log_x functions; these simply provide
 * the logFP to the flog_x functions that are coded in log.c.
 */

void flog_init(FILE* fp);
static inline void log_init(FILE* fp) { logFP = fp; flog_init(logFP); }
/* log_init: to begin logging, provide an fp open for writing;
 * to do no logging, provide logFP = NULL.
 * Call log_done() before the program exits.
 */

void flog_s(FILE* fp, const char* format, const char* str);
static inline void log_s(const char* f, const char* s) { flog_s(logFP, f, s); }
/* log_s: printf a string to the log, using the given format string.
 * Expects exactly one format specifier within the string,
 * corresponding to the one argument.  A newline is added.
 * Example:
 *   char* userName = ...; LOG_S("Your name is '%s'", userName);
 */

void flog_d(FILE* fp, const char* format, const int  num);
static inline void log_d(const char* f, const int n) { flog_d(logFP, f, n); }
/* log_c: like the above, but to print an integer. Example:
 *   int age = ...;        log_d("You are %d years old.", age);
 */

void flog_c(FILE* fp, const char* format, const char ch);
static inline void log_c(const char* f, const char c) { flog_c(logFP, f, c); }
/* log_c: like the above, but to print a character. Example:
 *   char player = ...;    log_c("Player %c is winning.", player);
 */

void flog_v(FILE* fp, const char* str);
static inline void log_v(const char* str) { flog_v(logFP, str); }
/* log_v: like the above, but used when no additional argument is needed.
 * Thus v stands for 'void'.
 */

void flog_e(FILE* fp, const char* str);
static inline void log_e(const char* str) { flog_e(logFP, str); }
/* log_e: print the given string to the log, with a message representing
 * an internal error.  See 'man errno' and 'man perror';
 * This function is best used immediately after a system call.
 */

void flog_done(FILE* fp);
static inline void log_done(void) { flog_done(logFP); logFP = NULL; }
/* log_done: call this when finished logging, or when you want to pause
 * logging for a while.  Call log_init() to resume.
 * It is the caller's responsibility to close the file, if desired.
 */

#endif // _LOG_H_
