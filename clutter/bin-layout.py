#!/usr/bin/env python3

import sys
import signal
import gi

try:
    from gi.repository import GLib
except:
    print('Please install "gir1.2-glib-2.0" to your Ubuntu.')
    sys.exit()

try:
    gi.require_version('Clutter', '1.0')
    from gi.repository import Clutter
except:
    print('Please install "gir1.2-clutter-1.0" to your Ubuntu.')
    sys.exit()

# Handle Unix signal to terminate this application by your terminal.
def handle_unix_signal():
    Clutter.main_quit()
GLib.unix_signal_add(GLib.PRIORITY_HIGH, signal.SIGINT, handle_unix_signal)

if Clutter.init(sys.argv)[0] != Clutter.InitError.SUCCESS:
    print('fail')
    sys.exit()

stage = Clutter.Stage.new()
stage.set_size(400, 400)
stage.connect('destroy', lambda x:Clutter.main_quit())

layout = Clutter.BinLayout.new(Clutter.BoxAlignment.START,
                               Clutter.BoxAlignment.START)

box = Clutter.Actor.new()
box.set_layout_manager(layout)

color = Clutter.Color.new(0x66, 0x66, 0x66, 0xff)
rect1 = Clutter.Actor.new()
rect1.set_background_color(color)
rect1.set_size(400, 200)

color = Clutter.Color.new(0xcc, 0xcc, 0xcc, 0xff)
rect2 = Clutter.Actor.new()
rect2.set_background_color(color)
rect2.set_size(200, 400)

box.add_child(rect1)
box.add_child(rect2)

for align_x in range(2, 5):
    for align_y in range(2, 5):
        diff_x = align_x - 1
        if align_x == 3:
            diff_x = 3
        elif align_x == 4:
            diff_x = 2

        diff_y = align_y - 1
        if align_y == 3:
            diff_y = 3
        elif align_y == 4:
            diff_y = 2

        color = Clutter.Color.new(255 - diff_x * 50, 100 + diff_y *50, 0, 255)
        rect = Clutter.Actor.new()
        rect.set_background_color(color)
        rect.set_size(100, 100)
        layout.set_alignment(rect, align_x, align_y)
        box.add_child(rect)

stage.add_child(box)

stage.show()

Clutter.main()
