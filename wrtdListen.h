#ifndef __WRTDLISTEN__
#define __WRTDLISTEN__

#ifdef __cplusplus
extern "C" {
#endif

extern int wrtdShowClock(const char *clock_name);
extern int wrtdListenDTacq(const char *event_regx, double delay, unsigned int verbose);
extern double wrtdGetDTacqTime(const char *event_regx, double delay, unsigned int verbose);
extern int wrtdWaitDTacq(const char *event_regx, double delay, unsigned int verbose);
extern int wrtdWait(const char *group, unsigned int port, const char *event_regex, unsigned int clock_id, double delay, int leapseconds, unsigned int verbose);
extern double wrtdGetTime(const char *group, unsigned int port, const char *event_regex, unsigned int clock_id, double delay, int leapseconds, unsigned int verbose);
extern int wrtdListen(const char *group, unsigned int port, const char *event_regex, unsigned int clock_id, double delay, int leapseconds, unsigned int verbose);

#define CLOCKFD 3
static clockid_t get_clockid(int fd)
{
  return (((unsigned int) ~fd) << 3) | CLOCKFD;
}
#ifdef __cplusplus
} // extern "C"
#endif

#endif
