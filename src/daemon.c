#include "login_keepalived.h"

void verbose(const char *format, ...) {
   va_list ap;
   va_start(ap, format);

   if (! options.verbose) return;
   vprintf(format, ap);
}

void debug_log(const char *format, ...) {
   char message[1024];
   va_list ap;
   va_start(ap, format);
   if (options.debug) {
      vsnprintf(message, 1023, format, ap);
      syslog(LOG_DEBUG, message);
   }
}

void run_global_daemon() {
   struct utmp *utmp_rec;
   FILE *tty;
   char ttynam[32];		/* "/dev/tty?, /dev/pts/* etc" */
   uint sleep_sec;

   openlog(Progname, LOG_PID, LOG_DAEMON);
   syslog(LOG_NOTICE, "starting global daemon (keeping your world alive)");
   verbose("Starting %s in global daemon mode (keeping everybody alive)...\n", Progname);
   become_daemon();
   while (1) {
      setutent();
      while ((utmp_rec = getutent()) != NULL) {
	 if (utmp_rec->ut_type == USER_PROCESS && (utmp_rec->ut_addr != 0 || strlen(utmp_rec->ut_host))) {
	    debug_log("found user %s on tty /dev/%s", utmp_rec->ut_user, utmp_rec->ut_line);
	    snprintf(ttynam, 32, "/dev/%s", utmp_rec->ut_line);
	    if ((tty = fopen(ttynam, "w")) == NULL) continue;
	    fputc(NOOP, tty);
	    fclose(tty);
	    debug_log("wrote NOOP to %s on tty /dev/%s", utmp_rec->ut_user, utmp_rec->ut_line);
	 }
      }
      endutent();
      debug_log("going to sleep for %d seconds", options.sleep_sec);
      sleep_sec = options.sleep_sec;
      while ((sleep_sec = sleep (sleep_sec)) > 0);
      debug_log("waking up from sleep");
   }
}

void run_private_daemon() {
   struct utmp *utmp_rec;
   FILE *tty;
   char *tty_fullname, *tty_device_name;
   pid_t parent_pid = 0;
   int parent_pid_status, found_utmp_rec = 0;

   /* We assume parent_pid is the pid of the shell */
   parent_pid = getppid();
   verbose("Parent has pid %d. When it disappears, we exit\n", parent_pid);

   if ((tty_fullname = ttyname(STDIN_FILENO)) == NULL) {
      fprintf(stderr, "Can't find ttyname(STDIN_FILENO)\n");
      exit (EXIT_FAILURE);
   }
   verbose("Found tty: %s\n", tty_fullname);
   tty_device_name = tty_fullname;
   if (strncmp(tty_fullname, "/dev/", 5) == 0) {
      tty_device_name = (char *) (tty_fullname + 5);
   }
   setutent();
   while ((utmp_rec = getutent()) != NULL) {
      /* find our entry. If ut_addr == 0, this is no remote login so we exit */
      if (utmp_rec->ut_type == USER_PROCESS && strcmp(utmp_rec->ut_line, tty_device_name) == 0) {
	 if (utmp_rec->ut_addr == 0) {
	    fprintf(stderr, "This is not a remote login!\n");
	    exit (EXIT_SUCCESS);
	 }
	 found_utmp_rec = 1;
	 break;
      }
   }
   endutent();
   /* don't run unless found utmp record */
   if (! found_utmp_rec) {
      fprintf(stderr, "Couldn't find utmp record for this login!\n");
      exit (EXIT_FAILURE);
   }
   verbose("Entering daemon mode...\n");
   become_daemon();
   while (1) {
      if ((tty = fopen(tty_fullname, "w")) == NULL) exit (EXIT_FAILURE);
      if (fputc(NOOP, tty) == EOF) break;
      fclose(tty);
      sleep (options.sleep_sec);
      /* check to see that the pid using the tty (our shell or sshd) is still alive */
      parent_pid_status = kill(parent_pid, 0);
      if (parent_pid_status != 0 && (errno == EPERM || errno == ESRCH)) break;
   }
}

void become_daemon() {
   pid_t newpid, newsid;
   int i;

   debug_log("forking to become a daemon");
   newpid = fork();
   if (newpid < 0)			exit (EXIT_FAILURE);
   if (newpid > 0)			exit (EXIT_SUCCESS);
   debug_log("starting a new session with setsid()");
   if ((newsid = setsid()) < 0)		exit (EXIT_FAILURE);
   debug_log("changing working dir to /");
   if (chdir("/") < 0)			exit (EXIT_FAILURE);
   umask(0);
   debug_log("closing stdin, stdout, stderr");
   if (close(STDIN_FILENO) != 0)	exit (EXIT_FAILURE);
   if (close(STDOUT_FILENO) != 0)	exit (EXIT_FAILURE);
   if (close(STDERR_FILENO) != 0)	exit (EXIT_FAILURE);
   debug_log("now I'm a daemon");

   /* Catch signals */
   setsignalhandlers();
}

