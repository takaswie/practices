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

try:
    gi.require_version('Gdk', '3.0')
    from gi.repository import Gdk
except:
    print('Please install "gir1.2-gtk-3.0" to your Ubuntu.')
    sys.exit()

try:
    gi.require_version('GdkPixbuf', '2.0')
    from gi.repository import GdkPixbuf
except:
    print('Please install "gir1.2-gdkpixbuf-2.0" to your Ubuntu.')
    sys.exit()

try:
    gi.require_version('Cogl', '1.0')
    from gi.repository import Cogl
except:
    print('Please install "gir1.2-cogl-1.0" to your Ubuntu.')
    sys.exit()

def create_content_actor():
    content = Clutter.Actor.new()
    content.set_size(720, 720)

    filepath = '/usr/share/images/desktop-base/spacefun-grub.png'
    pixbuf = GdkPixbuf.Pixbuf.new_from_file(filepath)
    image = Clutter.Image.new()
    if pixbuf.get_has_alpha():
        pixel_format = Cogl.PixelFormat.RGBA_8888
    else:
        pixel_format = Cogl.PixelFormat.RGB_888
    image.set_data(pixbuf.get_pixels(), pixel_format, pixbuf.get_width(),
                   pixbuf.get_height(), pixbuf.get_rowstride())
    del pixbuf

    content.set_content_scaling_filters(Clutter.ScalingFilter.TRILINEAR,
                                        Clutter.ScalingFilter.LINEAR)
    content.set_content_gravity(Clutter.ContentGravity.RESIZE_ASPECT)
    content.set_content(image)
    del image

    return content

def on_pan(action, scroll, is_interpolated):
    if is_interpolated:
        distance, delta_x, delta_y = action.get_interpolated_data()
        event = None
    else:
        distance, delta_x, delta_y = action.get_motion_delta(0)
        event = action.get_last_event(0)

    if not event:
        event_type = 'INTERPOLATED'
    elif event.type() == Clutter.EventType.MOTION:
        event_type = 'MOTION'
    elif event.type() == Clutter.EventType.TOUCH_UPDATE:
        event_type = 'TOUCH_UPDATE'
    else:
        event_type = '?'

    print('[{0}] panning dx:{1} dy:{2}'.format(event_type, delta_x, delta_y))

    return True

def create_scroll_actor(stage):
    scroll = Clutter.Actor.new()
    scroll.set_name('scroll')

    constr = Clutter.AlignConstraint.new(stage, Clutter.AlignAxis.X_AXIS, 0)
    scroll.add_constraint(constr)
    constr = Clutter.BindConstraint.new(stage, Clutter.BindCoordinate.SIZE, 0)
    scroll.add_constraint(constr)

    content = create_content_actor()
    scroll.add_child(content)

    pan_action = Clutter.PanAction.new()
    pan_action.set_interpolate(True)
    pan_action.connect('pan', on_pan)
    scroll.add_action_with_name('pan', pan_action)
    scroll.set_reactive(True)

    return scroll

def on_key_press(stage, event):
    scroll = stage.get_first_child()

    if Clutter.Event.get_key_symbol(event):
        scroll.save_easing_state()
        scroll.set_easing_duration(1000)
        scroll.set_child_transform(None)
        scroll.restore_easing_state()

    return Clutter.Event.STOP

def label_clicked_cb(label, event, scroll):
    action = scroll.get_action('pan')
    label_text = label.get_text()

    if label_text == 'X AXIS':
        action.set_pan_axis(Clutter.PanAxis.X_AXIS)
    elif label_text == 'Y AXIS':
        action.set_pan_axis(Clutter.PanAxis.Y_AXIS)
    elif label_text == 'AUTO':
        action.set_pan_axis(Clutter.PanAxis.AXIS_AUTO)
    else:
        action.set_pan_axis(Clutter.PanAxis.AXIS_NONE)

    return False

def add_label(text, box, scroll):
    label = Clutter.Text.new_with_text(None, text)
    label.set_reactive(True)
    label.set_x_align(Clutter.ActorAlign.START)
    label.set_x_expand(True)

    box.add_child(label)

    label.connect('clicked', label_clicked_cb, scroll)

# Handle Unix signal to terminate this application by your terminal.
def handle_unix_signal():
    Clutter.main_quit()
GLib.unix_signal_add(GLib.PRIORITY_HIGH, signal.SIGINT, handle_unix_signal)

if Clutter.init(sys.argv)[0] != Clutter.InitError.SUCCESS:
    print('fail')
    sys.exit()

stage = Clutter.Stage.new()
stage.set_title('Pan Action')
stage.set_user_resizable(True)

scroll = create_scroll_actor(stage)
stage.add_child(scroll)

box = Clutter.Actor.new()
stage.add_child(box)
box.set_position(12, 12)

layout = Clutter.BoxLayout.new()
layout.set_orientation(Clutter.Orientation.VERTICAL)
box.set_layout_manager(layout)

line = 'Press <space> to reset the image position.'
info = Clutter.Text.new_with_text(None, line)
box.add_child(info)

line = 'Click labels below to change AXIS pinning.'
info = Clutter.Text.new_with_text(None, line)
box.add_child(info)

add_label('NONE', box, scroll)
add_label('X AXIS', box, scroll)
add_label('Y AXIS', box, scroll)
add_label('AUTO', box, scroll)

stage.connect('destroy', lambda x:Clutter.main_quit())
stage.connect('key-press-event', on_key_press, scroll)

stage.show()

Clutter.main()
