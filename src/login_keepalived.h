/*
 * login_keepalived.h
*/

#ifndef LOGIN_KEEPALIVED_H
#define LOGIN_KEEPALIVED_H

/*
  This is how long we sleep by default between each write
*/
#define SLEEP_SEC_DEFAULT 60

/*
  This is the character written to each terminal
  (or the users terminal if run in private mode).
  Using anything other than '\0' will probably produce
  unwanted/odd/ugly behaviour. On the other hand it
  might be interesting, especially in global mode
  on a multiuser system :)
*/
#define NOOP '\0'

/*
  This is where we write our pid when running as a global
  daemon, to stop multiple global instances
  (Not implemented yet)
*/
#define PIDFILE "/var/run/login_keepalived.pid"

/* Includes */
#include "config.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utmp.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <stdarg.h>

/* Global variables */
const char* Progname;
struct {
   int sleep_sec;
   int private_mode;
   int debug;
   int verbose;
} options;

/* Function declarations. */
void exit_usage();
void become_daemon();
void run_private_daemon();
void run_global_daemon();
void sighandler(int signum);
void setsignalhandlers();
void debug_log(const char*, ...);
void verbose(const char*, ...);

#endif /* LOGIN_KEEPALIVED_H */
