#!/usr/bin/env python3

import sys
import signal
import gi

import random

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

class Lasso():
    def __init__(self):
        self.actor = None
        self.x = 0
        self.y = 0

def random_color_component():
    return 155 + (100.0 * random.random() / 2.0)

def button_pressed_cb(actor, event, lasso):
    lasso.actor = Clutter.Actor.new()
    color = Clutter.Color.new(0xaa, 0xaa, 0xaa, 0x33)
    lasso.actor.set_background_color(color)
    lasso.x, lasso.y = Clutter.Event.get_coords(event)
    actor.add_child(lasso.actor)

    return True

def button_released_cb(stage, event, lasso):
    if not lasso.actor:
        return True

    random_color = Clutter.Color.new(random_color_component(),
                                     random_color_component(),
                                     random_color_component(),
                                     random_color_component())
    rectangle = Clutter.Actor.new()
    rectangle.set_background_color(random_color)

    x, y = lasso.actor.get_position()
    width, height = lasso.actor.get_size()

    rectangle.set_position(x, y)
    rectangle.set_size(width, height)

    stage.add_child(rectangle)

    lasso.actor = None

    stage.queue_redraw()

    return True

def pointer_motion_cb(stage, event, lasso):
    if not lasso.actor:
        return True

    pointer_x, pointer_y = Clutter.Event.get_coords(event)

    new_x = min(pointer_x, lasso.x)
    new_y = min(pointer_y, lasso.y)
    width = max(pointer_x, lasso.x) - new_x
    height = max(pointer_y, lasso.y) - new_y

    lasso.actor.set_position(new_x, new_y)
    lasso.actor.set_size(width, height)

    return True

# Handle Unix signal to terminate this application by your terminal.
def handle_unix_signal():
    Clutter.main_quit()
GLib.unix_signal_add(GLib.PRIORITY_HIGH, signal.SIGINT, handle_unix_signal)

if Clutter.init(sys.argv)[0] != Clutter.InitError.SUCCESS:
    print('fail')
    sys.exit()

lasso = Lasso()

stage = Clutter.Stage.new()
stage.set_size(320, 240)
stage.set_background_color(Clutter.Color.new(0x33, 0x33, 0x55, 0xff))
stage.connect('destroy', lambda x:Clutter.main_quit())

stage.connect('button-press-event', button_pressed_cb, lasso)
stage.connect('button-release-event', button_released_cb, lasso)
stage.connect('motion-event', pointer_motion_cb, lasso)

stage.show()

Clutter.main()

del lasso
