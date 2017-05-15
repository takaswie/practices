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

def button_event_cb(actor, event):
    x, y = Clutter.Event.get_coords(event)

    if Clutter.Event.type(event) == Clutter.EventType.BUTTON_PRESS:
        event_type = 'pressed'
    else:
        event_type = 'released'

    button_pressed = Clutter.Event.get_button(event)

    state = Clutter.Event.get_state(event)

    if state & Clutter.ModifierType.CONTROL_MASK:
        ctrl_pressed = 'ctrl pressed'
    else:
        ctrl_pressed = 'ctrl not pressed'

    click_count = Clutter.Event.get_click_count(event)

    print('button {0} {1} at {2} {3}; {4}; click count {5}'.format(button_pressed,
          event_type, x, y, ctrl_pressed, click_count))

    return True

stage = Clutter.Stage.new()
color = Clutter.Color.new(0x33, 0x33, 0x55, 0xff)
stage.set_background_color(color)
stage.set_user_resizable(True)
stage.connect('destroy', lambda x:Clutter.main_quit())

red = Clutter.Actor.new()
color = Clutter.Color.new(0xff, 0x00, 0x00, 0xff)
red.set_background_color(color)
red.set_size(100, 100)
red.set_position(50, 150)
red.set_reactive(True)

green = Clutter.Actor.new()
color = Clutter.Color.new(0x00, 0xff, 0x00, 0xff)
green.set_background_color(color)
green.set_size(100, 100)
green.set_position(250, 150)
green.set_reactive(True)

red.connect('button-press-event', button_event_cb)
red.connect('button-release-event', button_event_cb)
green.connect('button-press-event', button_event_cb)
red.connect('button-release-event', button_event_cb)

stage.add_child(red)
stage.add_child(green)

stage.show()

Clutter.main()
