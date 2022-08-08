#!/usr/bin/python3
from ctypes import cdll, c_char_p, c_double, c_int
wrtdListen = cdll.LoadLibrary("libwrtdListen.so")
wrtd = wrtdListen.wrtdShowClock
wrtd("CLOCK_TAI".encode())
wrtd("CLOCK_REALTIME".encode())
wrtd("CLOCK_MONOTONIC".encode())
wrtd("/dev/ptp0".encode())
wrtd("".encode())
