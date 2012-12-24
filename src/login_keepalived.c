#include "login_keepalived.h"

int main(int argc, char *argv[]) {
   char *str_index;
   extern char *optarg;
   int c, i;

   Progname = (str_index = strrchr(argv[0], '/')) ? str_index + 1 : argv[0];
   if (argc < 2) exit_usage();

   /* set defaults */
   options.sleep_sec	= SLEEP_SEC_DEFAULT;
   options.debug        = 0;
   options.verbose      = 0;
   options.private_mode = -1;

   /* Get args */
   while ((c = getopt(argc, argv, "pgs:vd")) != -1) {
      switch(c) {
      case 'p':
	 options.private_mode = 1;
	 break;
      case 'g':
	 options.private_mode = 0;
	 break;
      case 's':
	 options.sleep_sec = atoi(optarg);
	 if (options.sleep_sec <= 0) {
	    fprintf(stderr, "  Invalid argument: -s %s\n  -s needs positive integer value\n\n", optarg);
	    exit_usage();
	 }
	 break;
      case 'd':
	 options.debug = 1;
	 break;
      case 'v':
	 options.verbose = 1;
	 break;
      default:
	 exit_usage();
      }
   }
   if (options.private_mode < 0) exit_usage();
   if (options.private_mode) {
      run_private_daemon();
   }
   else {
      if (geteuid() != 0) {
	 fprintf(stderr, "Only root can run global daemon\n\n");
	 exit_usage();
      }
      run_global_daemon();
   }
   exit (EXIT_SUCCESS);
}

void exit_usage() {
   fprintf(stderr, 
	   "login_keepalived %s (c) johan@ekenberg.se\n"
	   "usage: %s [-p | -g] [-s <sec> -d -v]\n"
	   "  -p = private mode (keeps you alive)\n"
	   "  -g = global mode  (keeps everybody alive)\n"
	   "  -s <sec> = seconds to sleep between each keepalive-write (default = %d)\n"
	   "  -v = be verbose when starting (mostly for private mode)\n"
	   "  -d = logs debug messages through syslogd (daemon.debug)\n",
	   VERSION, Progname, SLEEP_SEC_DEFAULT);
   exit (EXIT_FAILURE);
}

