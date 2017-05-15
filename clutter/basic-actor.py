#!/usr/bin/env python3

import sys, signal

try:
    import gi
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
stage.connect('destroy', lambda x:Clutter.main_quit())
stage.set_title('Three Flowers in a vase')
stage.set_user_resizable(True)

vase = Clutter.Actor.new()
vase.set_name('vase')
vase.set_layout_manager(Clutter.BoxLayout.new())
vase.set_background_color(Clutter.Color.get_static(Clutter.StaticColor.SKY_BLUE_LIGHT))
vase.add_constraint(Clutter.AlignConstraint.new(stage, Clutter.AlignAxis.BOTH, 0.5))
stage.add_child(vase)

def animate_color(flower, event):
    color = Clutter.Color.get_static(Clutter.StaticColor.BLUE)

    if (flower.get_background_color().equal(color)):
        color = Clutter.Color.get_static(Clutter.StaticColor.RED)

    flower.save_easing_state()
    flower.set_easing_duration(500)
    flower.set_easing_mode(Clutter.AnimationMode.LINEAR)
    flower.set_background_color(color)
    flower.restore_easing_state()

    return Clutter.EVENT_STOP

def on_crossing(flower, event):
    if Clutter.Event.type(event) == Clutter.EventType.ENTER:
        zpos = -250.0
    else:
        zpos = 0.0

    flower.save_easing_state()
    flower.set_easing_duration(500)
    flower.set_easing_mode(Clutter.AnimationMode.EASE_OUT_BOUNCE)
    flower.set_z_position(zpos)
    flower.restore_easing_state()

    return Clutter.EVENT_STOP

def on_transition_stopped(flower, name, is_finished):
    flower.save_easing_state()
    flower.set_rotation_angle(Clutter.AlignAxis.Y_AXIS, 0.0)
    flower.restore_easing_state()
    flower.handler_block_by_func(on_transition_stopped)

def animate_rotation(flower, event):
    flower.save_easing_state()
    flower.set_easing_duration(1000)
    flower.set_rotation_angle(Clutter.AlignAxis.Y_AXIS, 360.0)
    flower.restore_easing_state()

    flower.handler_unblock_by_func(on_transition_stopped)

    return Clutter.EVENT_STOP

bgcolors = (
    Clutter.StaticColor.RED,
    Clutter.StaticColor.YELLOW,
    Clutter.StaticColor.GREEN,
)

flowers = [Clutter.Actor.new() for i in range(3)]

for i, flower in enumerate(flowers):
    flower.set_name('flower.{0}'.format(i + 1))
    flower.set_size(128, 128)
    flower.set_background_color(Clutter.Color.get_static(bgcolors[i]))
    flower.set_reactive(True)
    vase.add_child(flower)

flowers[0].set_margin_left(12)
flowers[0].connect('button-press-event', animate_color)

flowers[1].set_margin_top(12)
flowers[1].set_margin_left(6)
flowers[1].set_margin_right(6)
flowers[1].set_margin_bottom(12)
flowers[1].connect('enter-event', on_crossing)
flowers[1].connect('leave-event', on_crossing)

flowers[2].set_margin_right(12)
flowers[2].set_pivot_point(0.5, 0.5)
flowers[2].connect('button-press-event', animate_rotation)
flowers[2].connect('transition-stopped::rotation-angle-y',
                   on_transition_stopped)
flowers[2].handler_block_by_func(on_transition_stopped)

stage.show()

Clutter.main()
