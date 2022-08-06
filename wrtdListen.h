extern int wrtdListen(const char *group, unsigned int port, const char *event_regex, unsigned int clock_id, double delay, int leapseconds, unsigned int verbose);
extern int wrtdListenDTacq(const char *event_regx, double delay, unsigned int verbose);
#define CLOCKFD 3
static clockid_t get_clockid(int fd)
{
  return (((unsigned int) ~fd) << 3) | CLOCKFD;
}
