#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <netinet/in.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>

#include "wrtdListen.h"
#include "wrtd-common.h"

extern int wrtdShowClock(const char *clock_name)
{
    unsigned int clkid;
    struct timespec cur_time;
    if (strcmp (clock_name, "CLOCK_TAI") == 0)
        clkid = CLOCK_TAI;
    else if (strcmp (clock_name, "CLOCK_REALTIME") == 0)
       clkid = CLOCK_REALTIME;
    else if (strcmp (clock_name, "CLOCK_MONOTONIC") == 0)
        clkid = CLOCK_MONOTONIC;
    else if (strlen(clock_name) == 0)
	clkid = 0;
    else
    {
        const unsigned char *ptp_front = "/dev/ptp";
        if (strncmp(clock_name, ptp_front, strlen(ptp_front)) == 0)
	{
    	    int fd = open(clock_name, O_RDWR);
            if (fd < 0)
	    {
	        perror("could not open PtP clock");
	        return fd;
	    }
	    clkid = get_clockid(fd); 	
    	}
	else
	{
	    printf("Unrecognized clock '%s' must be one of ('', 'CLOCK_REALTIME', 'CLOCK_TAI', 'CLOCK_MONOTONIC', '/dev/ptpN')\n", clock_name);
	    return -1;
	}
    }
    if (clock_gettime(clkid, &cur_time) == 0)
    {
        printf("Time from %s (%u) is %lu sec %lu nsec\n", clock_name, clkid, cur_time.tv_sec, cur_time.tv_nsec);
    }
    else
    {
	printf("error in clock_gettime");
	return errno;
    }
    return 0;

}

extern int wrtdListenDTacq(const char *event_regx, double delay, unsigned int verbose)
{
/*
 * int wrtdListenDTacq(const char *event_regular_expression, unsigned int verbose)
 *
 * args: 
 *    event_regular_expression - regex to match event_id against
 * returns:
 *    0 - success
 *    1 - failure
 * description:
 *   call wrtdListen with defaults that make sense for D-Tacq wrtd messages
 *     port=5044              follow with multicast port
 *     group=224.0.23.159     follow with multicast group address
 *     delay=0                follow signed number of nanoseconds to wait
 *     PtP=/dev/ptp0          follow with device to use PTP for time calls
 *     leapseconds=37         follow signed number of leapseconds to subtract
 */
  int status;
  unsigned int clock_id;
  unsigned const char *ptp = "/dev/ptp0";
  int fd = open(ptp, O_RDWR);
  if (fd < 0) {
    fprintf(stderr, "opening %s: %s reverting to CLOCK_TAI\n", ptp, strerror(errno));
    clock_id = CLOCK_TAI;
  }
  clock_id = get_clockid(fd);
 
  status = wrtdListen("224.0.23.159", 5044, event_regx, delay, clock_id, 37, verbose);
  if (fd > 0)
    close(fd);
  return status;
}

extern int wrtdListen(const char *group, unsigned int port, const char *event_regex, unsigned int clock_id, double delay, int leapseconds, unsigned int verbose)
{
  regex_t reegex;
  unsigned int ptp = ((clock_id != CLOCK_TAI) &&
                      (clock_id != CLOCK_REALTIME) &&
                      (clock_id != CLOCK_MONOTONIC));

  if (verbose)
  {
	  printf("wrtdListen(%s, %u, %s, %d, %f, %d)\n", group, port, event_regex, clock_id, delay, leapseconds);
  }
  // Compile the regular expression
  if (regcomp( &reegex, event_regex, 0)) {
    perror("Could not parse regex");
    return 1;
  }

  // create what looks like an ordinary UDP socket
  //
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) {
    perror("socket");
    return 1;
  }

  // allow multiple sockets to use the same PORT number
  //
  u_int yes = 1;
  if (setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, (char*) &yes, sizeof(yes)) < 0)
  {
   perror("Reusing ADDR failed");
   return 1;
  }

  // set up destination address
  //
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY); // differs from sender
  addr.sin_port = htons(port);

  // bind to receive address
  //
  if (bind(fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
    perror("bind");
    return 1;
  }

  // use setsockopt() to request that the kernel join a multicast group
  //
  struct ip_mreq mreq;
  mreq.imr_multiaddr.s_addr = inet_addr(group);
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);
  if ( setsockopt( fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &mreq, sizeof(mreq) ) < 0 )
  {
    perror("setsockopt");
    return 1;
  }
  int status = 1;
  while (1)
  {
    struct wrtd_message msgbuf;
    socklen_t addrlen = sizeof(addr);
    int nbytes = recvfrom( fd, (char *)&msgbuf, sizeof(msgbuf), 0, (struct sockaddr *) &addr, &addrlen );
    if (nbytes < 0) {
      perror("recvfrom");
      break;
    }
    if (nbytes != sizeof(msgbuf))
    {
      if (verbose)
        printf("recfrom expected %d bytes got %d\n", (int)sizeof(msgbuf), nbytes);
      continue;
    }
    if (strncmp(msgbuf.hw_detect, "LXI", 3))
    {
      if (verbose)
        printf("Expected LXI message got -%3.3s-\n", msgbuf.hw_detect);
      continue;
    }
    if (regexec( &reegex, msgbuf.event_id, 0, NULL, 0))
    {
      if (verbose)
        printf("LXI message -%s- is not for us -%s-\n", msgbuf.event_id, event_regex);
      continue;
    }
    if (verbose)
      printf("hw_detect -%3.3s-\nevent_id -%s-\nseq %d\nts_sec %d\nts_ns %d\nts_frac %d\nts_hi_sec %d\n",
              msgbuf.hw_detect,msgbuf.event_id, msgbuf.seq, msgbuf.ts_sec, msgbuf.ts_ns, msgbuf.ts_frac, msgbuf.ts_hi_sec);
    struct timespec desired_tp, cur_tp;
    double delay_secs = floor(delay);
    desired_tp.tv_sec=msgbuf.ts_sec - leapseconds + (int)delay_secs;
    desired_tp.tv_nsec = msgbuf.ts_ns + (delay - delay_secs)*1E9;
    if (clock_gettime(clock_id, &cur_tp) == 0)
    {
      struct timespec remaining_tp={0,0};
      if (verbose)
      {
        printf("CURRENT: tv_sec=%ld tv_nsec=%ld\n", cur_tp.tv_sec, cur_tp.tv_nsec);
        printf("DESIRED: tv_sec=%ld tv_nsec=%ld\n", desired_tp.tv_sec, desired_tp.tv_nsec);
        printf("sleeping\n");      
      }
      if(! ptp)
      {
        while (status = clock_nanosleep(clock_id, TIMER_ABSTIME, &desired_tp, &remaining_tp))
          if (status != EINTR)
            break;
        if (verbose)
        {
          printf("tv_sec=%ld tv_nsec=%ld\n", desired_tp.tv_sec, desired_tp.tv_nsec);
          printf("\tawake status = %d\n", status);
        }
        status = 0;
        break;
      }
      else
      {
        if (verbose)
          printf("PTP Sleeping\n");
        while((cur_tp.tv_sec < desired_tp.tv_sec) || (cur_tp.tv_nsec < desired_tp.tv_nsec))
        {
          if (clock_gettime(clock_id, &cur_tp) != 0)
            {
              printf("Failed to get PTP time\n");
              break;
            }
        }
        if (verbose)
          printf("\tAwake\n");
        status = 0;
        break;
      }
    }
    else 
    {
      perror("gettime");
      break;
    }
  }
  return status;
}
