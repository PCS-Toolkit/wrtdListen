# wrtdListen
wrtd message tester and callable library

```
#include "wrtdListen.h"
status = wrtdListenDTacq(const char *event_regx, unsigned int verbose);
status = wrtdListen(const char *group, unsigned int port, const char *event_regex, unsigned int clock_id, int leapseconds, unsigned int verbose);
```
# returns 
- 0 - success
- 1 - failure
# arguments
- const char *event_regx - regular expression to match against event_id in the message 
- unsigned int verbose - 0 -> quiet, 1 -> verbose
- const char *group - multicast address to listen on
- unsigned int port - network port to listen on
- unsigned int clock_id {CLOCK_TAI | CLOCK_REALTIME | CLOCK_MONOTONIC | clockid made from fd of ptp device} 
- int leapseconds - number of leap seconds to subract (or is it add) 
```
```
Usage: wrtdWait [-Pv?] [-p|--port=5044 or ...] [-g|--group=STRING] [-e|--event_id=STRING] [-d|--delay=0] [-P|--PtP] [-v|--verbose] [-?|--help] [--usage]
```

```
./wrtdWait --help
Usage: wrtdWait [OPTION...]
  -p, --port=5044 or ...     follow with multicast port
  -g, --group=STRING         follow with multicast group address
  -e, --event_id=STRING      follow with regex to match event string
  -d, --delay=0              follow signed number of nanoseconds to wait
  -P, --PtP                  Use PTP for time calls
  -v, --verbose              be verbose

Help options:
  -?, --help                 Show this help message
      --usage                Display brief usage message
```


Program to test reciept of WRTD messages.  

Test program from D-TACQ to send messages:
https://github.com/petermilne/ACQ420FMC.git

```
ACQ420FMC$ ./soft_wrtd --help
Usage: soft_wrtd [OPTION...]
      --tickns=INT             tick size nsec
  -d, --dns=INT                nsec to add to current time
  -d, --delta_ns=INT           nsec to add to current time
  -p, --rt_prio=INT            real time priority
  -n, --on_next_second=INT     trigger next second, on the second, for comparison with PPS
  -v, --verbose=INT            debug
      --max_tx=INT             maximum transmit count
      --tx_id=STRING           txid: default is $(hostname)
      --at=STRING              at [+UT]sss[:.]ttt
at: +: relative, U: absolute UTC T: absolute TAI
at: tx at +s[:nsec] or [UT]sec-since-epoch[:nsec]
at: tx at +s[.frac] or
                               [UT]sec-since-epoch[.frac]

      --delay01=INT            in double tap, delay to second trigger
      --tx_mask=INT            mask for TIGA trigger tx

Help options:
  -?, --help                   Show this help message
      --usage                  Display brief usage message
```
For example to send a message labeled 'START' at the current time + 2.5 seconds:
```
WRTD_ID=START ./soft_wrtd --at +2:500000000 -v3 txa
```
wrtdWait will respond:
```
sleeping
        awake status = 0
```
or with the verbose flag:
```
./wrtdWait -v
After processing, options have values:
	 port holds 5044
	 ptp flag holds 0
	 group holds [224.0.23.159]
	 event regx holds [^START$]
	 delay holds 0
hw_detect -LXI-
event_id -START-
seq 62
ts_sec 1658762453
ts_ns 500000000
ts_frac 0
ts_hi_sec 0
CURRENT: tv_sec=1658762413 tv_nsec=932496668
DESIRED: tv_sec=1658762416 tv_nsec=500000000
sleeping
tv_sec=1658762416 tv_nsec=500000000
	awake status = 0
