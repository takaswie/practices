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

def clicked_cb(action, actor):
    button = action.get_button()
    name = actor.get_name()

    print('Pointer button {0} clicked on actor {1}'.format(button, name))

# Handle Unix signal to terminate this application by your terminal.
def handle_unix_signal():
    Clutter.main_quit()
GLib.unix_signal_add(GLib.PRIORITY_HIGH, signal.SIGINT, handle_unix_signal)

if Clutter.init(sys.argv)[0] != Clutter.InitError.SUCCESS:
    print('fail')
    sys.exit()

stage = Clutter.Stage.new()
stage.set_title('Click Action')
stage.set_background_color(Clutter.Color.new(0x33, 0x33, 0x55, 0xff))
stage.set_user_resizable(True)
stage.connect('destroy', lambda x:Clutter.main_quit())

actor1 = Clutter.Actor.new()
actor1.set_name('Red Button')
actor1.set_background_color(Clutter.Color.get_static(Clutter.StaticColor.RED))
actor1.set_size(100, 100)
actor1.set_reactive(True)
actor1.set_position(50, 150)
stage.add_child(actor1)

actor2 = Clutter.Actor.new()
actor2.set_name('Blue Button')
actor2.set_background_color(Clutter.Color.get_static(Clutter.StaticColor.BLUE))
actor2.set_size(100, 100)
actor2.set_position(250, 150)
actor2.set_reactive(True)
stage.add_child(actor2)

action1 = Clutter.ClickAction.new()
actor1.add_action(action1)

action2 = Clutter.ClickAction.new()
actor2.add_action(action2)

action1.connect('clicked', clicked_cb)
action2.connect('clicked', clicked_cb)

stage.show()

Clutter.main()
