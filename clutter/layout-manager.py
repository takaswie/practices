#!/usr/bin/env python3

import sys, signal, math

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

class MultiLayout(Clutter.LayoutManager):
    def __init__(self):
        super(Clutter.LayoutManager, self).__init__()

        self.state = 0
        self.cell_width = -1.0
        self.cell_height = -1.0
        self.spacing = 0.0

    def set_spacing(self, spacing):
        self.spacing = spacing
        self.layout_changed()

    def get_state(self):
        return self.state

    def set_state(self, state):
        self.state = state
        self.layout_changed()

    def do_get_preferred_width(self, container, for_width):
        minimum = 0
        natural = 0
        max_natural_width = 0
        n_children = 0

        for child in container.get_children():
            if not child.is_visible():
                continue

            child_minimum, child_natural = child.get_preferred_width(-1.0)

            max_natural_width = max(max_natural_width, child_natural)

            if self.state == 0:
                minimum += child_minimum
                natural += child_natural
            elif self.state == 1:
                minimum = max(minimum, child_minimum)
                natural = max(natural, child_natural)

            n_children += 1

        self.cell_width = max_natural_width

        minimum += self.spacing * (n_children - 1)
        natural += self.spacing * (n_children - 1)

        return (minimum, natural)

    def do_get_preferred_height(self, container, for_height):

        minimum = self.spacing * 2
        natural = self.spacing * 2
        n_children = 0

        for child in container.get_children():
            if not child.is_visible():
                continue

            child_minimum, child_natural = child.get_preferred_height(-1.0)

            minimum = max(minimum, child_minimum)
            natural = max(natural, child_natural)

            n_children += 1

        self.cell_height = natural

        minimum += self.spacing * (n_children - 1)
        natural += self.spacing * (n_children - 1)

        return (minimum, natural)

    @staticmethod
    def _get_items_per_row(for_width, cell_width, spacing):
        if for_width < 0:
            return 1

        if cell_width <= 0:
            return 1

        n_columns = math.ceil((for_width + spacing) / (cell_width + spacing))

        return max(n_columns, 1)

    @staticmethod
    def _get_visible_children(container):
        n_visible_children = 0

        for child in container.get_children():
            if child.is_visible():
                n_visible_children += 1

        return n_visible_children

    def do_allocate(self, container, allocation, flags):
        n_items = MultiLayout._get_visible_children(container)
        if n_items == 0:
            return

        x_offset, y_offset = allocation.get_origin()
        avail_width, avail_height = allocation.get_size()

        self.get_preferred_width(container, avail_width)
        self.get_preferred_height(container, avail_height)

        item_index = 0

        if self.state == 0:
            n_items_per_row = MultiLayout._get_items_per_row(avail_width, self.cell_width, self.spacing)
            item_x = x_offset
            item_y = y_offset
        elif self.state == 1:
            center = Clutter.Point()
            center.x = allocation.x2 // 2
            center.y = allocation.y2 // 2
            radius = min((avail_width - self.cell_width) // 2,
                         (avail_height - self.cell_height) // 2)

        for child in container.get_children():
            child_allocation = Clutter.ActorBox()

            if not child.is_visible():
                continue

            if self.state == 0:
                if item_index == n_items_per_row:
                    item_index = 0
                    item_x = x_offset
                    item_y += self.cell_height + self.spacing

                child_allocation.x1 = item_x
                child_allocation.y1 = item_y
                child_allocation.x2 = child_allocation.x1 + self.cell_width
                child_allocation.y2 = child_allocation.y1 + self.cell_height

                item_x += self.cell_width + self.spacing
            elif self.state == 1:
                # Should be arythmetic operation for floating point.
                theta = 2 * math.pi / n_items * item_index
                child_allocation.x1 = center.x + radius * math.sin(theta) - (self.cell_width // 2)
                child_allocation.y1 = center.y + radius * -1 * math.cos(theta) - (self.cell_height // 2)
                child_allocation.x2 = child_allocation.x1 + self.cell_width
                child_allocation.y2 = child_allocation.y1 + self.cell_height

            child.allocate(child_allocation, flags)

            item_index += 1

# Handle Unix signal to terminate this application by your terminal.
def handle_unix_signal():
    Clutter.main_quit()
GLib.unix_signal_add(GLib.PRIORITY_HIGH, signal.SIGINT, handle_unix_signal)

if Clutter.init(sys.argv)[0] != Clutter.InitError.SUCCESS:
    print('fail')
    sys.exit()

stage = Clutter.Stage.new()
stage.set_title('Multi-layout')
stage.connect('destroy', lambda x:Clutter.main_quit())
stage.show()

N_RECTS = 16
RECT_SIZE = 64.0
N_ROWS = 4
PADDING = 12.0
BOX_SIZE = RECT_SIZE * (N_RECTS // N_ROWS) + PADDING * (N_RECTS // N_RECTS - 1)

manager = MultiLayout()
manager.set_spacing(PADDING)

margin = Clutter.Margin.new()
margin.top = PADDING
margin.bottom = PADDING
margin.left = PADDING
margin.right = PADDING

box = Clutter.Actor.new()
box.set_margin(margin)
box.set_layout_manager(manager)
box.set_size(BOX_SIZE, BOX_SIZE)

constr = Clutter.AlignConstraint.new(stage, Clutter.AlignAxis.BOTH, 0.5)
box.add_constraint(constr)
stage.add_child(box)

def on_enter(rect, event):
    rect.set_scale(1.2, 1.2)
    return Clutter.EVENT_STOP

def on_leave(rect, event):
    rect.set_scale(1.0, 1.0)
    return Clutter.EVENT_STOP

for i in range(N_RECTS):
    rect = Clutter.Actor.new()

    color = Clutter.Color.from_hls(360.0 // N_RECTS * i, 0.5, 0.8)
    color.alpha = 128 + 128 // N_RECTS * i

    rect.set_size(RECT_SIZE, RECT_SIZE)
    rect.set_pivot_point(0.5, 0.5)
    rect.set_background_color(color)
    rect.set_opacity(0)
    rect.set_reactive(True)

    transition = Clutter.PropertyTransition.new('opacity')
    transition.set_duration(250)
    transition.set_delay(i * 50)
    transition.set_from(0)
    transition.set_to(255)
    rect.add_transition('fadeIn', transition)
    del transition

    rect.set_easing_duration(250)
    rect.set_easing_mode(Clutter.AnimationMode.EASE_OUT_CUBIC)

    box.add_child(rect)

    rect.connect('enter-event', on_enter)
    rect.connect('leave-event', on_leave)

label = Clutter.Text.new()
label.set_text('Press t\t\342\236\tToggle layout\nPress q\t\342\236\236\tQuit')
constr = Clutter.AlignConstraint.new(stage, Clutter.AlignAxis.X_AXIS, 0.5)
label.add_constraint(constr)
constr = Clutter.AlignConstraint.new(stage, Clutter.AlignAxis.Y_AXIS, 0.5)
label.add_constraint(constr)
stage.add_child(label)

def on_key_press(stage, event, box):
    layout = box.get_layout_manager()

    if   event.keyval == Clutter.KEY_q:
        Clutter.main_quit()
    elif event.keyval == Clutter.KEY_t:
        state = layout.get_state()
        if state == 0:
            layout.set_state(1)
        if state == 1:
            layout.set_state(0)

    return Clutter.EVENT_STOP

stage.connect('key-press-event', on_key_press, box)

Clutter.main()
