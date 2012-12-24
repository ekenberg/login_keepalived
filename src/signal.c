#include "login_keepalived.h"

/* signals to catch */
struct _siglist {
   int signum;
   char* signame;
};

struct _siglist siglist[] = {{SIGHUP,  "SIGHUP"},
			     {SIGINT,  "SIGINT"},
			     {SIGTERM, "SIGTERM"},
			     {SIGPIPE, "SIGPIPE"},
			     {0, (char *) 0}};
struct sigaction sa;

void setsignalhandlers() {
   int i;
   memset (&sa, 0, sizeof (sa));
   sa.sa_handler = sighandler;

   for (i = 0; siglist[i].signum > 0; i++) {
      debug_log("setting signal handler: %s (%d)\n", siglist[i].signame, siglist[i].signum);
      sigaction(siglist[i].signum, &sa, NULL);
   }
}

void sighandler(int signum) {
   int i;

   if (options.private_mode) return; // logging only for global daemon

   for (i = 0; siglist[i].signum > 0; i++) {
      if (signum == siglist[i].signum) {
	 syslog(LOG_NOTICE, "Caught (ignoring) signal %s (%d)", siglist[i].signame, siglist[i].signum);
	 return;
      }
   }
}

