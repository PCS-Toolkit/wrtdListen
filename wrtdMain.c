/* Processes command-line options. */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <popt.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
#include <string.h>
#include <time.h>
//#include <sys/time.h>
//#include <regex.h>
//
//#include <sys/types.h>
//#include <sys/socket.h>
//#include "wrtd-common.h"
#include "wrtdListen.h"

/* Data values for the options. */
static int port = 5044;
static int leapseconds = 37;
static int delay = 0;
static char *ptp = NULL;
static int verbose = 0;
static int realtime = 0;
static int tai = 0;
static int monotonic = 0;
static char *group = "224.0.23.159";
static char *event_id = "^START$";

/* Set up a table of options. */
static struct poptOption optionsTable[] = {
  {(const char *) "port", (char) 'p', POPT_ARG_INT, (void *) &port, 0,
   (const char *) "follow with multicast port", (const char *) "5044"},
  {(const char *) "group", (char) 'g', POPT_ARG_STRING, (void *) &group, 0,
   (const char *) "follow with multicast group address", (const char *)"224.0.23.159"},
  {(const char *) "event_id", (char) 'e', POPT_ARG_STRING, (void *) &event_id, 0,
   (const char *) "follow with regex to match event string", NULL},
  {(const char *) "delay", (char) 'd', POPT_ARG_INT, (void *) &delay, 0,
   (const char *) "follow signed number of nanoseconds to wait", (const char *) "0"},
  {(const char *) "PtP", (char) 'P', POPT_ARG_STRING, &ptp, 0,
   (const char *) "follow with device to use PTP for time calls", (const char *)"/dev/ptp0"},
  {(const char *) "verbose", (char) 'v', POPT_ARG_NONE, &verbose, 0,
   (const char *) "be verbose", NULL},
  {(const char *) "tai", (char) 't', POPT_ARG_NONE, &tai, 0,	
   (const char *) "Use CLOCK_TAI", NULL},
  {(const char *) "realtime", (char) 'r', POPT_ARG_NONE, &realtime, 0,
   (const char *) "Use CLOCK_REALTIME", NULL},
  {(const char *) "monotonic", (char) 'm', POPT_ARG_NONE, &monotonic, 0,
   (const char *) "Use CLOCK_MONOTONIC", NULL},
  {(const char *) "leapseconds", (char) 'l', POPT_ARG_INT, (void *) &leapseconds, 0,
   (const char *) "follow signed number of leapseconds to subtract", (const char *) "37"},

  POPT_AUTOALIAS POPT_AUTOHELP POPT_TABLEEND
};

int main (int argc, const char *argv[])
{
  poptContext context = poptGetContext ((const char *) argv[0],
					argc,
					argv,
					(const struct poptOption *)
					&optionsTable,
					0);
  int option = poptGetNextOpt (context);
  if(verbose){
    printf ("After processing, options have values:\n");
    printf ("\t port holds %d\n", port);
    printf ("\t ptp flag holds %s\n", ptp);
    printf ("\t group holds [%s]\n", group);
    printf ("\t event regx holds [%s]\n", event_id);
    printf ("\t delay holds %d\n", delay);
  }
  poptFreeContext (context);
  
  // if ptp flag set then use a clock_id made from /dev/ptp0
  // otherwise use CLOCK_TAI
  clockid_t clkid;
  if (ptp) 
  {
    int fd = open(ptp, O_RDWR);
    if (fd < 0) {
      fprintf(stderr, "opening %s: %s\n", ptp, strerror(errno));
      return -1;
    }
    clkid = get_clockid(fd);
  }
  else if (tai)
    clkid = CLOCK_TAI;
  else if (realtime)
    clkid = CLOCK_REALTIME;
  else if (monotonic)
    clkid = CLOCK_MONOTONIC;
  else 
    clkid = CLOCK_REALTIME;

  while (wrtdListen(group, port, event_id, clkid, (double)0.0, leapseconds, verbose) == 0);
  return 0;
}
