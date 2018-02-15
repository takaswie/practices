#!/usr/bin/env python3

import sys, signal, gi

# Sample-1.0 gir
gi.require_version('Sample', '1.0')
from gi.repository import Sample

gi.require_version('GLib', '2.0')
from gi.repository import GLib

from array import array

loop = GLib.MainLoop()
def handle_unix_signal():
    loop.quit()
GLib.unix_signal_add(GLib.PRIORITY_HIGH, signal.SIGINT, handle_unix_signal)

def handle_notice(listener, count):
    print('  notified: {0}'.format(count))

    if count % 2 == 0:
        ret = None
    else:
        ret = (0, 1, 2, 3, 4, 5)

    return ret

def handle_notice32(listener, count):
    def _get_array():
        # The width with 'L' parameter is depending on environment.
        arr = array('L')
        if arr.itemsize is not 4:
            arr = array('I')
            if arr.itemsize is not 4:
                raise RuntimeError('Platform has no representation \
                                    equivalent to quadlet.')
        return arr

    print('  notified32: {0}'.format(count))

    if count % 2 == 0:
        ret = None
    else:
        ret = _get_array()
        for i in range(4):
            ret.append(i)

    return ret

listener = Sample.Listener()

try:
    listener.connect('notified', handle_notice)
    listener.connect('notified32', handle_notice32)
except Exception as e:
    print(e)
    sys.exit()

try:
    listener.listen()
except Exception as e:
    print(e)
    sys.exit()

loop.run()

listener.unlisten()
del listener
