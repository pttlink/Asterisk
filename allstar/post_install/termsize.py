#!/usr/bin/env python

# Get the current size of the terminal window, and set stty size accordingly.
# A replacement for xterm's resize program, with no X dependency.
# Useful when logged in over a serial line.
# Copyright 2013 by Akkana Peck -- share and enjoy under the GPL v2 or later.

import os, sys
import fcntl
import posix
import struct
import time
import re
import termios
import select

tty = open('/dev/tty', 'r+')
tty.write('\033[7\033[r\033[999;999H\033[6n')
tty.flush()

fd = sys.stdin.fileno()

oldterm = termios.tcgetattr(fd)
newattr = oldterm[:]
newattr[3] = newattr[3] & ~termios.ICANON & ~termios.ECHO
termios.tcsetattr(fd, termios.TCSANOW, newattr)

oldflags = fcntl.fcntl(fd, fcntl.F_GETFL)
fcntl.fcntl(fd, fcntl.F_SETFL, oldflags | os.O_NONBLOCK)

try:
    while True:
        r, w, e = select.select([fd], [], [])
        if r:
            output = sys.stdin.read()
            break
finally:
    termios.tcsetattr(fd, termios.TCSAFLUSH, oldterm)
    fcntl.fcntl(fd, fcntl.F_SETFL, oldflags)

rows, cols = map(int, re.findall(r'\d+', output))

fcntl.ioctl(fd, termios.TIOCSWINSZ,
            struct.pack("HHHH", rows, cols, 0, 0))

print "\nReset the terminal to", rows, "rows", cols, "cols"

