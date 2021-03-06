/* Copyright (c) 2012 Wayne Tran <wayne@tran.io>
 * License: MIT
 */

#ifndef _LOG_C_H_
#define _LOG_C_H_
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <syslog.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif
/*Max length per message*/
#define MAXLENGTH 4096

extern int debugflag;

typedef unsigned long DWORD;

typedef enum {
	LOGFLAG_FILE	=1 << 0,  /* log to a file with the name specified by domain */
	LOGFLAG_STDOUT	=1 << 1,  /* log to stdout */
	LOGFLAG_STDERR	=1 << 2,  /* log to stderr for ERROR and WARNING messages */
	LOGFLAG_SYSLOG	=1 << 3,  /* log to system logs */
	LOGFLAG_ERROR	=1 << 4, /* log error messages */
	LOGFLAG_INFO	=1 << 5, /* log info messages  */
	LOGFLAG_DEBUG	=1 << 6,  /* log debug messages */
	LOGFLAG_TRACE	=1 << 7,  /* log trace messages */
	LOGFLAG_WARN	=1 << 8 /* log warning messages */
} LOGFLAGS;

/*Initializes the logger with default flags:
 *		LOGFLAG_STDOUT|LOGFLAG_INFO|LOGFLAG_ERROR|
		LOGFLAG_DEBUG|LOGFLAG_TRACE|LOGFLAG_WARN
 */
bool logInit(const char *domain);
bool logClose();

const char * logGetDomain();

void logSetFlags(DWORD logflags);
DWORD logGetFlags();
DWORD logRemoveFlags(DWORD mask);
DWORD logAddFlags(DWORD mask);

/*Will be redirected to STDERR if the flag LOGFLAG_STDERR is set.*/
void logError(const char *format, ...);

/*Will be redirected to STDERR if the flag LOGFLAG_STDERR is set.*/
void logWarn(const char *format, ...);

void logInfo(const char *format, ...);
void logDebug(const char *format, ...);
void logTrace(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif // LOG_H
