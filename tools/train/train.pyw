#!/usr/bin/env python

# Train Simulator
# hacked by Arno Puder, 4/2003
# modified by Haijie Xiao, 3/2004

from Tkinter import *
import thread, socket, sys, time, os, re, random

debug = 1

def usage ():
    print 'usage: train.pyw [--layout1|--layout2]'
    sys.exit (-1)

layout = 1
if len (sys.argv) > 1:
    if len (sys.argv) != 2:
        usage()
    if sys.argv[1] == '--layout1':
        layout = 1
    elif sys.argv[1] == '--layout2':
        layout = 2
    else:
        usage()


if layout == 1:
    tile_width = 30
else:
    tile_width = 40

train_size = 10
train_color = 'red'
wagon_color = 'yellow'
zamboni_color = 'black'
even_contact_color = 'blue4'
odd_contact_color = 'brown'

train = 20
wagon = 1
zamboni = 78

max_vehicle_speed = 5
zamboni_speed = max_vehicle_speed 

sleep_time = 0.05
train_stop_delay = 0.2
command_processing_delay_factor = 0.1
serial_line_delay = 0.1 
wait_for_command_sleep_time = 0.3

# The track layout is defined through a two-dimensional grid. Each
# tile of this chessboard-like arrangement can have multiple track
# segments. There are eight access point to each tile, denoted by
# their location:
#                        NW--N--NE
#                        |       |
#                        W       E
#                        |       |
#                        SW--S--SE
#
# A track segment lists the two access points that should be connected.
# E.g., "SW:E" will draw a track segment from SW to E.
# Each tile can have multiple segments which must be separated by ';'.
# E.g., "W:E;N:S" will put a cross in the tile.

#track_layout_1 simulates the train model layout
track_layout_1 = [
    ['', '', 'sw:e', 'w:e', 'w:e;w:se', 'w:e', 'w:e', 'w:e', 'w:e', 'w:e', 'w:e;sw:e', 'w:e', 'w:se', '', '', ''],
    ['', 'sw:ne', '', '', '', 'nw:e;nw:se', 'w:se', '', '', 'sw:ne', '', '', '', 'nw:se', '', ''],
    ['ne:s', '', '', '', '', '', 'nw:s', 'nw:se', 's:ne', '', '', '', '', '', 'nw:se', ''],
    ['n:s', '', '', '', '', '', 'n:s', '', 'nw:s;n:s', '', '', '', '', '', '', 'nw:s'],
    ['n:s', '', '', '', '', '', 'n:s', '', 'n:se', '', '', '', '', '', '', 'n:s'],
    ['n:s', '', '', '', '', '', 'n:s', '', '', 'nw:se', '', '', '', '', '', 'n:s;n:sw'],
    ['n:se', '', '', '', '', '', '', '', '', '', 'nw:se', '', '', '', 'sw:ne', 'n:s'],
    ['', 'nw:se', '', '', '', '', '', 'sw:ne', '', 'sw:e', 'w:e', 'w:e;nw:e', 'w:e', 'sw:ne;w:ne', '', 'n:sw'],
    ['', '', 'nw:se;nw:e', 'w:se;w:e', 'w:e', 'w:e', 'w:ne', '', 'sw:ne', '', '', '', 'sw:ne', '', 'sw:ne', ''],
    ['', '', '', 'nw:se', 'nw:e', 'w:e', 'w:e', 'w:ne', '', '', '', 'sw:ne', '', 'sw:ne', '', ''],
    ['', '', '', '', 'nw:e', 'w:e', 'w:e', 'w:e', 'w:e', 'w:e', 'w:ne', '', '', '', '', '']]


# track_labels_1 defines the switch labels, contacts (sensors), and contact labels.
# The first element of track_labels_1 (track-labels_1[0]) defines switch labels.
# The second element of track_labels_1 (track-labels_1[1]) defines contacts and contact labels.
#
# For switch labels: 
#     A label is defined using format: 'type:label number:tile:position'
#        type         - the type of the label. 's' for switches and 'c' for contacts
#        label number - the label number
#        tile         - the tile where the switch locates. It is in the format "tile_column, tile_row"
#        position     - the position inside the tile where to display the label.
#                       Can be: 's', 'w', 'e', 'n', 'sw', 'se', 'nw', 'ne'
#     E.g., 's:1:4:0:s' defines switch #1. The switch is in the tile(4,0).
#     The label of the switch will be placed at the south of the tile.
#
# For contacts and contack labels:
#     The format used is similar to the format that defines switch labels, except a tile list is used
#     instead of a single tile. (A contact can cross multiply tiles.) A tile list is a set of tile
#     seperated by ';'. The first tile in the list is the tile where the label will be displayed.
#     E.g., 'c:1:8,8;9,7;10,7:se' defines contact #1. It occupies tile (8,8), (9,7), and (10,7). The
#     label of this contact will be displayed in the sourth-east part of tile (8,8).

track_labels_1 =[
    ['s:1:13,7:s', 's:2:11,7:s', 's:3:3,8:n', 's:4:2,8:w', 's:5:4,0:sw', 's:6:5,1:w',
     's:7:8,3:e','s:8:10,0:se','s:9:15,5:e'],
    ['c:1:8,8;9,7;10,7:se', 'c:2:5,9;4,9;6,9;7,9:s', 'c:3:10,10;8,10;9,10;11,9;12,8:s',
     'c:4:5,10;3,9;4,10;6,10;7,10:s', 'c:5:5,8;7,7;4,8;6,8:n', 'c:6:0,6;0,5;1,7:ne',
     'c:7:1,1;0,4;0,3;0,2;2,0;3,0:se','c:8:6,3;6,2;6,4;6,5:w','c:9:7,2;6,1:ne','c:10:7,0;5,0;6,0;8,0;9,0:s',
     'c:11:9,1;8,2:se','c:12:9,5;8,4;10,6:ne','c:13:12,0;11,0;13,1:sw','c:14:14,2;15,3;15,4:w',
     'c:15:14,6:nw','c:16:14,8;15,6;15,7;13,9:se']]

track_layout_2 = [
    ['', 'sw:e', 'w:e;w:se', 'w:e', 'w:e;sw:e', 'w:se', '', '', '', '', ''],
    ['s:ne', '', '', 'nw:s;ne:s', '', '', 'nw:s', '', 'n:s', '', ''],
    ['n:s;n:se', '', '', 'n:s', '', '', 'n:s', '', 's:n', '', 'n:s'],
    ['n:s', 'nw:s', '', 'n:se', '', '', 'n:s', '', 'sw:n', '', 'n:s'],
    ['n:s', 'n:s', '', '', 'nw:e', 'w:e', 'w:e;n:s', 'w:e;w:ne', 'w:e', 'w:se', 'n:s'],
    ['n:s', 'n:s', '', '', '', '', 'n:s', '', '', '', 'nw:s;n:s'],
    ['n:se', '', '', 'sw:e', 'w:e', '', 'n:sw;n:se', '', '', '', 'n:sw'],
    ['', 'nw:e', 'w:e;w:ne', 'w:e', 'w:e', 'w:e;w:ne', 'w:e', 'w:e;nw:e', 'w:e', 'w:ne', '']]

# There are three vehicles on the track: the controllable train,
# the uncontrollable zamboni (another train) and a wagon that
# can not move on its own. The following three variables define
# the initial default locations of those three vehicles. The
# x,y coordiates define the tile and the direction where the
# vehicle will be placed initially.

configurations_1 = [
    # Wagon        Train       Zamboni
    ([6, 9, 'e'], [6, 3, 'n'], [5, 10, 'e']),
    ([6, 9, 'e'], [6, 3, 'n'], [6, 10, 'w']),
    ([8, 2, 's'], [5, 8, 'w'], [5, 10, 'e']),
#   ([8, 2, 's'], [5, 8, 'w'], [6, 10, 'w'])
    ([15, 7, 'sw'], [5, 8, 'w'], [6, 10, 'w'])
    ]


configurations_2 = [
    # Wagon        Train       Zamboni
    ([8, 1, 's'], [1, 4, 'n'], [2, 0, 'ne']),
    ([8, 1, 's'], [1, 4, 'n'], [2, 0, 'sw']),
    ([10, 2, 's'], [4, 6, 'w'], [2, 0, 'w']),
    ([10, 2, 's'], [4, 6, 'w'], [2, 0, 'e'])]


if layout == 1:
    track_layout = track_layout_1
    track_labels = track_labels_1
    configurations = configurations_1
else:
    track_layout = track_layout_2
    configurations = configurations_2


half_tile_width = tile_width / 2

# Function f(x) computes the coordinates of a track segment along
# a curve. This happens for example for a track segment going from
# SW to E. In order to fit seamlessly with neighboring tiles, the
# function f(x) has the following properties:
#   f(0) = 0
#   f(tile_width) = tile_width / 2
#   f'(0) = 1
#   f'(tile_width) = 0

def f (x):
    return x - (x * x / (2 * tile_width))

def draw_dot (canvas, x, y, tags):
    obj_id = canvas.create_line (x - 1, y - 1, x + 1, y + 1, width=3)
    #obj_id = canvas.create_oval (x, y, x, y, width=5, fill='black')
    for i in range(len(tags)):
       canvas.addtag_withtag (tags[i], obj_id)

def coordinates_within_tile (x, y):
    return x < tile_width and x >= 0 and y < tile_width and y >= 0

def line (x, y, dx, dy, reverse_direction=0):
    if reverse_direction:
        dx = -dx
        dy = -dy
    x += dx
    y += dy
    return (x, y, dx, dy)

def curve1 (x, y, mx, dy, reverse_direction=0):
    if reverse_direction:
        dy = -dy
    y += dy
    x = f (tile_width - y - 1)
    x = (x - half_tile_width) * mx + half_tile_width
    return (x, y, mx, dy)

def curve2 (x, y, mx, dy, reverse_direction=0):
    if reverse_direction:
        dy = -dy
    y += dy
    x = tile_width - f (y)
    x = (x - half_tile_width) * mx + half_tile_width
    return (x, y, mx, dy)

def curve3 (x, y, dx, my, reverse_direction=0):
    if reverse_direction:
        dx = -dx
    x += dx
    y = tile_width - f (x)
    y = (y - half_tile_width) * my + half_tile_width
    return (x, y, dx, my)

def curve4 (x, y, dx, my, reverse_direction=0):
    if reverse_direction:
        dx = -dx
    x += dx
    y = tile_width - f (tile_width - x - 1)
    y = (y - half_tile_width) * my + half_tile_width
    return (x, y, dx, my)

w = tile_width - 1
w2 = half_tile_width

funcs = {
    'n':  [w2, 0,  {'s':  [line, 0, +1],
                    'sw': [curve1, +1, +1],
                    'se': [curve1, -1, +1]}],
    'ne': [w, 0,   {'sw': [line, -1, +1],
                    's':  [curve2, +1, +1],
                    'w':  [curve4, -1, -1]}],
    'e':  [w, w2,  {'w':  [line, -1, 0],
                    'sw': [curve3, -1, +1],
                    'nw': [curve3, -1, -1]}],
    'se': [w, w,   {'nw': [line, -1, -1],
                    'w':  [curve4, -1, +1],
                    'n':  [curve1, -1, -1]}],
    's':  [w2, w,  {'n':  [line, 0, -1],
                    'ne': [curve2, +1, -1],
                    'nw': [curve2, -1, -1]}],
    'sw': [0, w,   {'e':  [curve3, +1, +1],
                    'ne': [line, +1, -1],
                    'n':  [curve1, +1, -1]}],
    'w':  [0, w2,  {'e':  [line, 1, 0],
                    'se': [curve4, 1, +1],
                    'ne': [curve4, +1, -1]}],
    'nw': [0, 0,   {'se': [line, +1, +1],
                    's':  [curve2, -1, +1],
                    'e':  [curve3, +1, -1]}]}


class Vehicle:
    def __init__ (self, track_gui, name, color):
        self.track_gui = track_gui
        self.track = track_gui.get_track()
        self.name = name
        self.canvas = self.track.get_canvas()
        obj_id = self.canvas.create_oval (0, 0, train_size,
                                          train_size, fill=color)
        self.canvas.addtag_withtag (name, obj_id)
        self.curr_pos_x = 0
        self.curr_pos_y = 0
        self.set_speed (0)

    def remove_from_canvas (self):
        self.canvas.delete (self.name)

    def set_location (self, tile_x, tile_y, entry_point, exit_point, func, x, y, dx, dy):
        self.tile_x = tile_x
        self.tile_y = tile_y
        self.entry_point = entry_point
        self.exit_point = exit_point
        self.func = func
        self.x = x
        self.y = y
        self.dx = dx
        self.dy = dy
        self.draw()

    def collides (self, vehicle):
        # Check if this vehicle collides with another vehicle
        dx = abs (self.curr_pos_x - vehicle.curr_pos_x)
        dy = abs (self.curr_pos_y - vehicle.curr_pos_y)
        distance = dx * dx + dy * dy
        return distance <= train_size * train_size

    def is_in_same_tile (self, vehicle):
        return self.tile_x == vehicle.tile_x and self.tile_y == vehicle.tile_y

    def is_in_tile (self, x, y):
        return self.tile_x == x and self.tile_y == y

    def set_speed (self, speed):
        self.speed = speed
        self.ticks = (max_vehicle_speed + 1) - speed

    def get_speed (self):
        return self.speed

    def draw (self):
        new_pos_x = self.x + self.tile_x * tile_width - train_size / 2
        new_pos_y = self.y + self.tile_y * tile_width - train_size / 2
        move_x = new_pos_x - self.curr_pos_x
        move_y = new_pos_y - self.curr_pos_y
        self.canvas.move (self.name, move_x, move_y)
        self.curr_pos_x = new_pos_x
        self.curr_pos_y = new_pos_y

    def move_to_next_segment (self):
        next_tile_x = self.tile_x
        next_tile_y = self.tile_y
        if self.exit_point == 'n':
            next_entry_point = 's'
            next_tile_y -= 1
        elif self.exit_point == 'ne':
            next_entry_point = 'sw'
            next_tile_x += 1
            next_tile_y -= 1
        elif self.exit_point == 'e':
            next_entry_point = 'w'
            next_tile_x += 1
        elif self.exit_point == 'se':
            next_entry_point = 'nw'
            next_tile_x += 1
            next_tile_y += 1
        elif self.exit_point == 's':
            next_entry_point = 'n'
            next_tile_y += 1
        elif self.exit_point == 'sw':
            next_entry_point = 'ne'
            next_tile_x -= 1
            next_tile_y += 1
        elif self.exit_point == 'w':
            next_entry_point = 'e'
            next_tile_x -= 1
        elif self.exit_point == 'nw':
            next_entry_point = 'se'
            next_tile_x -= 1
            next_tile_y -= 1
        else:
            assert 0
        self.track.enter_vehicle (self, next_tile_x, next_tile_y,
                                  next_entry_point)

    def move (self):
        if not coordinates_within_tile (self.x, self.y):
            try:
                self.move_to_next_segment()
            except:
                # if we got an exception, then we ran out of track
                self.track_gui.abort_simulation ('Ran out of track')
            return
        (self.x, self.y, self.dx, self.dy) = self.func (self.x,
                                                        self.y,
                                                        self.dx,
                                                        self.dy)
        (self.x, self.y, self.dx, self.dy) = self.func (self.x,
                                                        self.y,
                                                        self.dx,
                                                        self.dy)
        self.draw()

    def reverse_direction (self):
        (self.x, self.y, self.dx, self.dy) = self.func (self.x,
                                                        self.y,
                                                        self.dx,
                                                        self.dy,
                                                        1)
        tmp = self.entry_point
        self.entry_point = self.exit_point
        self.exit_point = tmp

    def next_simulation_step (self):
        if self.speed == 0:
            return
        if self.ticks == 0:
            self.ticks = (max_vehicle_speed + 1) - self.speed
            self.move()
        self.ticks -= 1

class Train (Vehicle):
    def __init__ (self, track, wagon):
        Vehicle.__init__ (self, track, 'train', train_color)
        self.wagon = wagon
        self.wagon_is_attached = 0

    def move (self):
        Vehicle.move (self)
        if self.collides (self.wagon) and not self.wagon_is_attached:
            self.wagon_is_attached = 1
            # We have to check from which side the train approaches
            # the wagen. Depending on the direction, we need to reverse
            # the direction of the wagon (which, by default, initially
            # points to the inside of the tile)
            if self.is_in_same_tile (self.wagon):
                self.wagon.reverse_direction()
        if self.wagon_is_attached:
            self.wagon.move()

    def reverse_direction (self):
        Vehicle.reverse_direction (self)
        if self.wagon_is_attached:
            self.wagon.reverse_direction()

class Wagon (Vehicle):
    def __init__ (self, track):
        Vehicle.__init__ (self, track, 'wagon', wagon_color)

class Zamboni (Vehicle):
    def __init__ (self, track_gui, vehicle1, vehicle2):
        Vehicle.__init__ (self, track_gui, 'zamboni', zamboni_color)
        self.vehicle1 = vehicle1
        self.vehicle2 = vehicle2
        self.set_speed (zamboni_speed)

    def move (self):
        Vehicle.move (self)
        if self.collides (self.vehicle1) or self.collides (self.vehicle2):
            # Zamboni crashed into another vehicle. Abort the simulation
            self.track_gui.abort_simulation ('Collision with Zamboni')


def animate_switch (canvas, tag):
    original_color = canvas.itemcget(tag, "fill")
    if original_color == 'black':
        flashing_color = 'red'
    else:
        flashing_color = 'black'
        
    for i in range (10):
        canvas.itemconfig (tag, fill=flashing_color)
        #Misc.update (canvas)
        time.sleep (0.1)
        canvas.itemconfig (tag, fill=original_color)
        #Misc.update (canvas)
        time.sleep (0.1)


class Tile:
    def __init__ (self, track, x, y, layout):
        self.track = track
        self.x = x
        self.y = y
        self.layout = layout
        self.set_switch (random.randint(0, 1))

    def set_switch (self, switch, visual_feedback=0):
        self.switch = switch
        if visual_feedback:
            tag = '%d_%d_%d' % (self.x, self.y, switch)
            canvas = self.track.get_canvas()
            thread.start_new (animate_switch, (canvas, tag))
            #for i in range (15):
            #    canvas.itemconfig (tag, fill='red')
            #    Misc.update (canvas)
            #    time.sleep (0.1)
            #    canvas.itemconfig (tag, fill='black')
            #    Misc.update (canvas)
            #    time.sleep (0.1)

    def draw (self, canvas):
        if self.layout == '':
            return
        segments = self.layout.split (';')
        x_ofs = self.x * tile_width
        y_ofs = self.y * tile_width
        seg = 0
        for s in segments:
            corners = s.split (':')
            frm = corners[0]
            to = corners[1]
            spec = funcs[frm]
            x = spec[0]
            y = spec[1]
            to_spec = spec[2][to]
            func = to_spec[0]
            dx = to_spec[1]
            dy = to_spec[2]

            tags = []
            tags.append("track_segment")
            if (len(segments) == 1 ):
                contact_id = self.get_contact_id()
                if contact_id != 0 and contact_id % 2 == 0:
                    tags.append("even_contact")
                elif contact_id != 0:
                    tags.append("odd_contact")
                else:
                    tags.append("non_contact")
            else:
                if seg == 0:
                    tags.append("switch_green")
                else:
                    tags.append("switch_red")
                tags.append ('%d_%d_%d' % (self.x, self.y, seg))
                
            seg += 1
            while coordinates_within_tile (x, y):
                draw_dot (canvas, x_ofs + x, y_ofs + y, tags)
                (x, y, dx, dy) = func (x, y, dx, dy)

    def get_contact_id(self):
        contact_id = 0
        s = '[^0-9]' + str(self.x) + ',' + str(self.y) + '[^0-9]'
        for i in range(len(track_labels_1[1])):
            if re.search(s, track_labels_1[1][i]) != None:
                contact_id = i + 1
                break;
        return contact_id
    
    def enter_vehicle (self, vehicle, entry_point):
        assert self.layout != ''
        segments = self.layout.split (';')
        exit_point = None
        switch = self.switch
        for s in segments:
            corners = s.split (':')
            frm = corners[0]
            to = corners[1]
            if frm == entry_point:
                exit_point = to
                if switch == 0:
                    break
                switch -= 1
            elif to == entry_point:
                exit_point = frm
                if switch == 0:
                    break
                switch -= 1
            
        assert exit_point != None
        spec = funcs[entry_point]
        v_x = spec[0]
        v_y = spec[1]
        exit_spec = spec[2][exit_point]
        v_func = exit_spec[0]
        v_dx = exit_spec[1]
        v_dy = exit_spec[2]
        vehicle.set_location (self.x, self.y, entry_point, exit_point,
                              v_func, v_x, v_y, v_dx, v_dy)


class Track:
    def __init__ (self, canvas):
        self.canvas = canvas
        self.track_layout = []
        self.vehicles = []
        self.show_grid = 0
        self.show_labels = 0

    def get_canvas (self):
        return self.canvas

    def expand (self, x, y):
        # Make sure that the array track_layout has enough x and y components
        while len (self.track_layout) < x + 1:
            self.track_layout.append ([])
        l = self.track_layout[x]
        while len (l) < y + 1:
            l.append (None)

    def add_track_segment (self, x, y, layout):
        self.expand (x, y)
        self.track_layout[x][y] = Tile (self, x, y, layout)

    def set_switch (self, x, y, s, animate_switch):
        self.track_layout[x][y].set_switch (s, animate_switch)

    def reset_switches (self):
        for x in self.track_layout:
            for y in x:
                y.set_switch (random.randint(0, 1))

    def draw (self):
        for x in self.track_layout:
            for y in x:
                y.draw (self.canvas)

    def toggle_grid (self):
        self.show_grid = not self.show_grid
        if self.show_grid:
            self.draw_grid()
        else:
            self.remove_grid()

    def draw_grid (self):
        cols = len (self.track_layout)
        rows = len (self.track_layout[0])
        for x in range (cols):
            x_pos = x * tile_width
            y_pos = tile_width * rows
            obj_id = self.canvas.create_text (tile_width / 2 + x_pos, 5,
                                              anchor='n', text= x)
            self.canvas.addtag_withtag ('grid', obj_id)
            if x == 0:
                continue
            obj_id = self.canvas.create_line (x_pos, 0, x_pos, y_pos, dash='.')
            self.canvas.addtag_withtag ('grid', obj_id)
        for y in range (rows):
            x_pos = tile_width * cols
            y_pos = y * tile_width
            obj_id = self.canvas.create_text (7, tile_width / 2 + y_pos,
                                              anchor='w', text= y)
            self.canvas.addtag_withtag ('grid', obj_id)
            if y == 0:
                continue
            obj_id = self.canvas.create_line (0, y_pos, x_pos, y_pos, dash='.')
            self.canvas.addtag_withtag ('grid', obj_id)
        self.canvas.lower ('grid')

    def remove_grid (self):
        self.canvas.delete ('grid')

    def toggle_labels (self):
        self.show_labels = not self.show_labels
        if self.show_labels:
            self.draw_labels()
            self.color_track()
        else:
            self.remove_labels()
            self.uncolor_track()

    def draw_labels (self):
        sn_position_x = ew_position_y = tile_width/2
        n_position_y = 0
        s_position_y = tile_width - 12
        w_position_x = 5
        e_position_x = tile_width - 5

        for i in range (len(track_labels)):
            for j in range (len(track_labels[i])):
                tokens = track_labels[i][j].split(':')
                type = tokens[0]
                no = tokens[1]
                tiles = tokens[2].split(';')
                position = tokens[3]

                display_tile = tiles[0]
                xy = display_tile.split(',')
                x = int(xy[0])
                y = int(xy[1])

                if type == 's':
                   color = 'red'
                   text_color = 'black'
                else:
                   if (j+1) %2 == 0:
                       color = even_contact_color
                   else:
                       color = odd_contact_color
                   text_color = 'white'

                if position == 'n':
                    x_position = x * tile_width + sn_position_x
                    y_position = y * tile_width + n_position_y
                elif position == 'e':
                    x_position = x * tile_width + e_position_x
                    y_position = y * tile_width + ew_position_y
                elif position == 's':
                    x_position = x * tile_width + sn_position_x
                    y_position = y * tile_width + s_position_y
                elif position == 'w':
                    x_position = x * tile_width + w_position_x
                    y_position = y * tile_width + ew_position_y
                elif position == 'se':
                    x_position = x * tile_width + e_position_x
                    y_position = y * tile_width + s_position_y                 
                elif position == 'sw':
                    x_position = x * tile_width + w_position_x
                    y_position = y * tile_width + s_position_y
                elif position == 'ne':
                    x_position = x * tile_width + e_position_x
                    y_position = y * tile_width + n_position_y
                else:
                    x_position = x * tile_width + w_position_x
                    y_position = y * tile_width + n_position_y 
                    
                if type == 's':
                    obj_id = self.canvas.create_oval(x_position-6,
                                                     y_position,
                                                     x_position+6,
                                                     y_position+12,
                                                     outline = color,
                                                     fill = color)
                else:
                    obj_id = self.canvas.create_rectangle(x_position-6,
                                                          y_position,
                                                          x_position+6,
                                                          y_position+12,
                                                          width=1,
                                                          outline = color,
                                                          fill = color)
                self.canvas.addtag_withtag('label', obj_id)
                obj_id = self.canvas.create_text (x_position, y_position,
                                                  anchor='n', text= no,
                                                  fill = text_color)
                self.canvas.addtag_withtag ('label', obj_id)


        self.canvas.lower ('label')


    def remove_labels (self):
        self.canvas.delete ('label')

    def color_track(self):
        self.canvas.itemconfig('odd_contact', fill = odd_contact_color)
        self.canvas.itemconfig('even_contact', fill = even_contact_color)
        self.canvas.itemconfig('non_contact', fill = 'red')
        self.canvas.itemconfig('switch_green', fill = 'green')
        self.canvas.itemconfig('switch_red', fill = 'red')
    
    def uncolor_track(self):
        self.canvas.itemconfig('track_segment', fill = 'black')
        
    def enter_vehicle (self, vehicle, x, y, entry_point):
        if vehicle not in self.vehicles:
            self.vehicles.append (vehicle)
        self.track_layout[x][y].enter_vehicle (vehicle, entry_point)

    def remove_vehicles (self):
        for v in self.vehicles:
            v.remove_from_canvas()
        self.vehicles = []

    def next_simulation_step (self):
        for v in self.vehicles:
            v.next_simulation_step()


class StatusWindow (Frame):
    def __init__ (self, parent=None, text=''):
        Frame.__init__ (self, parent)
        self.pack (expand=YES, fill=BOTH)
        self.makewidgets()

    def makewidgets (self):
        sbar = Scrollbar (self)
        text = Text (self, height=5, width=40, relief=SUNKEN)
        sbar.config (command=text.yview)
        text.config (yscrollcommand=sbar.set)
        sbar.pack (side=RIGHT, fill=Y)
        text.pack (side=LEFT, expand=YES, fill=BOTH)
        self.text = text
        self.clear()

    def clear (self):
        self.empty = 1
        self.text.config (state='normal')
        self.text.delete ('1.0', END)
        self.text.config (state='disabled')

    def log (self, msg):
        self.text.config (state='normal')
        if not self.empty:
            self.text.insert (END, '\n')
        self.text.insert (END, msg)
        self.text.yview (END)
        self.text.config (state='disabled')
        self.empty = 0

    def remove_last_logs(self, count):
        self.text.config (state='normal')
        for i in range(count):
            self.text.delete("end -1 lines", END)

        if self.text.get(1.0, END) == '\n':
            print("empty")
            self.empty = 1
            
        #haijie: have problem preventing flashing when deleting from a full window
        self.text.config (state='disabled')


class TrainGUI (Frame):
    def __init__ (self, track_layout):
        Frame.__init__ (self)
        self.master.title ('Train Simulator')
        tiles_x = len (track_layout[0])
        tiles_y = len (track_layout)

        # Title
        title_frame = Frame (self)
        title_frame.pack (side=TOP)
        
        title = Label (title_frame, text='Train Simulator       ')
        title.config (pady=10, font=('times', 30, 'bold italic'))
        title.pack (side=LEFT)

        # Logo
	giffile = None
	for fname in ['tos.gif', 'tools/train/tos.gif']:
		if os.path.exists(fname):
			giffile = fname
	if giffile == None:
		print('Cannot find tos.gif or tools/train/tos.gif');
		sys.exit(-1);
        self.pic = PhotoImage (file=giffile)
        logo = Label (title_frame, image=self.pic)
        logo.pack (side=RIGHT, padx=10, pady=10)
        
        # Track
        track_frame = Frame (self)
        track_frame.pack (side=TOP)

        canvas = Canvas (track_frame, height=tile_width*tiles_y,
                         width=tile_width*tiles_x, bd=5, relief=RAISED)
        canvas.pack (side=TOP, padx=10, pady=10)
        track = Track (canvas)

        # Buttons: Reset, Exit
        control_frame = Frame (self)
        control_frame.pack (side=RIGHT, padx=10, pady=10)

        button_frame = Frame (control_frame)
        button_frame.pack (side=BOTTOM, padx=10, pady=10)
        reset = Button (button_frame, text='Reset', command=self.reset)
        reset.pack (side=LEFT, fill=BOTH, pady=5)

        exit = Button (button_frame, text='Exit', command=self.quit)
        exit.pack (side=LEFT, fill=BOTH, padx=10, pady=5)

        conf_list = []
        for i in range (len (configurations)):
            conf_list.append ('Configuration %d' % (i+1))
        self.active_configuration = StringVar()
        self.active_configuration.set (conf_list[0])
        conf = OptionMenu (control_frame, self.active_configuration,
                           *conf_list)
        conf.pack (side=TOP, anchor=W, pady=10)

        zamboni = Checkbutton (control_frame, text='Zamboni',
                               command=self.toggle_show_zamboni)
        zamboni.pack (side=TOP, anchor=W)

        if debug:
            grid = Checkbutton (control_frame, text='Show grid',
                                command=track.toggle_grid)
            grid.pack (side=TOP, anchor=W)

        label = Checkbutton(control_frame, text = 'Show labels', 
                                    command=self.toggle_labels)
        label.pack (side=TOP, anchor=W)

        animate_switch = Checkbutton (control_frame, text='Animate switch',
                                      command=self.toggle_animate_switch)
        animate_switch.pack (side=TOP, anchor=W)

        # Command
        command_frame = Frame (self)
        command_frame.pack (side=TOP, anchor=W, padx=10)
        Label (command_frame, text='Command:').pack (side=LEFT)
        entry = Entry (command_frame)
        entry.bind ('<Return>', lambda event: self.exec_command (entry))
        entry.focus_set()
        entry.pack (side=LEFT)

        # Status window
        self.sw = StatusWindow (self)
        self.sw.pack (side=BOTTOM, padx=10, pady=10)

        self.pack()

        # Init tiles of track
        for row in range (tiles_y):
            for col in range (tiles_x):
                track.add_track_segment (col, row, track_layout[row][col])

        track.draw()
        self.track = track
        self.show_zamboni = 0
        self.animate_switch = 0
        self.setup_vehicles()
        self.simulation_running = 0
        self.start_simulation()
        self.command_buffer = []

    def setup_vehicles (self):
        # Get the configuration number
        i = int (self.active_configuration.get().split (' ')[1]) - 1
        wagon_location, train_location, zamboni_location = \
                        configurations[i]
        self.track.remove_vehicles()
        wagon = Wagon (self)
        self.wagon = wagon
        self.track.enter_vehicle (wagon,
                                  wagon_location[0],
                                  wagon_location[1],
                                  wagon_location[2])
        train = Train (self, wagon)
        self.train = train
        self.track.enter_vehicle (train,
                                  train_location[0],
                                  train_location[1],
                                  train_location[2])
        train.reverse_direction()
        zamboni = None
        if self.show_zamboni:
            zamboni = Zamboni (self, train, wagon)
            self.track.enter_vehicle (zamboni,
                                      zamboni_location[0],
                                      zamboni_location[1],
                                      zamboni_location[2])
        self.zamboni = zamboni

    def reset_switches (self):
        self.track.reset_switches()

    def get_track (self):
        return self.track

    def toggle_show_zamboni (self):
        self.show_zamboni = not self.show_zamboni

    def toggle_animate_switch (self):
        self.animate_switch = not self.animate_switch

    def toggle_labels (self):
        self.track.toggle_labels()
        
    def exec_command (self, entry):
        cmd = entry.get()
        entry.delete ('0', END)
        if cmd == '':
            return
        self.process_train_command (cmd)

    def process_train_command (self, cmd):
        if not self.simulation_running:
            self.sw.log ('Simulation is currently not running')
            return ''
            
        if cmd[0] == 'L':
            return_value = self.process_L_command(cmd)
        elif cmd[0] == 'M':
            return_value = self.process_M_command(cmd)
        elif cmd[0] == 'C':
            return_value = self.process_C_command(cmd)
        elif cmd[0] == 'R':
            return_value = self.process_R_command(cmd)
        else:
            self.abort_simulation('Illegal command (%s)' % cmd)
            return ''

        self.pre_pre_cmd = self.pre_cmd
        self.pre_cmd = cmd

        return return_value

    def process_L_command (self, cmd):
        is_format_correct = 1
        
        if cmd.count('L') > 1:
            is_format_correct = 0

        else:    
            stripped_cmd = cmd.lstrip('L')

            #"L#S#" format
            if stripped_cmd.count('S') == 1:
                tokens_s = stripped_cmd.split('S')
                format = 'S'
                try:
                    vehicle = int (tokens_s[0])
                    speed = int (tokens_s[1])
                except:
                   is_format_correct = 0

            #"L#D" format       
            elif stripped_cmd[len(stripped_cmd)- 1 ] == 'D' and stripped_cmd.count('D')== 1:
                format = 'D'
                try:
                    vehicle = int (stripped_cmd.rstrip('D'))
                except:
                    is_format_correct = 0

            else:
                is_format_correct = 0
                
        if is_format_correct == 0:
            self.abort_simulation('"L" command format: "L # S #" or "L # D" (%s) ' %cmd)
            return ''
            
        if vehicle != train:
            self.abort_simulation ('Only the train can be manipulated (%s)' %cmd)
            return ''

        if format == 'S':
            if speed < 0 or speed > max_vehicle_speed:
                self.abort_simulation('Speed out of range (%s) ' %cmd)
                return ''
            else:
                if speed == 0:
                    time.sleep (train_stop_delay)
                self.train.set_speed(speed)
                self.sw.log('Changed train velocity to %d (%s)' % (speed, cmd))

        else: 
            if self.train.get_speed() != 0:
                self.abort_simulation ('Can not reverse direction while train is moving (%s) ' %cmd)
                return ''
            self.train.reverse_direction()
            self.sw.log ('Reversed direction of train (%s)' % cmd)
        return ''

    def process_M_command(self, cmd):
        is_format_correct = 1

        if cmd.count('M') > 1:
            is_format_corret = 0
                
        else:    
            stripped_cmd = cmd.lstrip('M')
            if (stripped_cmd[len(stripped_cmd)-1]) == 'R' and stripped_cmd.count('R') == 1:
                switch_setting = 1
                try:
                    switchID = int (stripped_cmd.rstrip('R'))
                except:
                    is_format_correct = 0
            elif (stripped_cmd[len(stripped_cmd)-1]) == 'G' and stripped_cmd.count('G') == 1:
                switch_setting = 0
                try:
                    switchID = int (stripped_cmd.rstrip('G'))
                except:
                    is_format_correct = 0
            else:
                is_format_correct = 0
                
        if is_format_correct == 0:
           self.abort_simulation ('"M" command format: "M#[G][R]" (%s)' % cmd)
           return ''   

        if switchID < 1 or switchID > len (track_labels[0]):
           self.abort_simulation ('Switch ID out of range (%s)' % cmd )
           return ''
            
        tokens = track_labels[0][switchID-1].split(':')
        tile = tokens[2]
        tile_xy = tile.split(',')
        tile_x = int(tile_xy[0])
        tile_y = int(tile_xy[1])
        self.track.set_switch(tile_x, tile_y, switch_setting, self.animate_switch)
        self.sw.log ('Changed switch %d to %s (%s)' % (switchID, cmd[2], cmd))
        return ''

    def process_C_command(self, cmd):
        is_format_correct = 1
   
        if self.pre_cmd != 'R':
            self.abort_simulation('s88 memory buffer has not been cleaned (%s)' % cmd)
            return ''

        if cmd.count('C') > 1:
            is_format_corret = 0
        else:
            stripped_cmd = cmd.lstrip('C')
            try:
                contactID = int(stripped_cmd)
            except:
                is_format_correct = 0

        if is_format_correct == 0:
            self.abort_simulation ('"C" command format "C#" (%s)' % cmd)
            return ''

        if contactID < 1 or contactID > len(track_labels[1]):
            self.abort_simulation('Contact ID out of range (%s)' %cmd)
            return ''
            
        tokens = track_labels[1][contactID-1].split(':')
        tiles = tokens[2].split(';')
        status = 0

        for i in range(len(tiles)):
            tile_xy = tiles[i].split(',')
            tile_x = int(tile_xy[0])
            tile_y = int(tile_xy[1])
            if self.train.is_in_tile (tile_x, tile_y) or self.wagon.is_in_tile (tile_x, tile_y):
                status = 1
                break
            if self.zamboni != None and self.zamboni.is_in_tile (tile_x, tile_y):
                status = 1
                break

        if self.pre_pre_cmd == cmd and self.pre_status == status:
            self.probe_count = self.probe_count + 1
            self.sw.remove_last_logs(2)
            self.sw.log('Probe result of [%d]: %d (%s).....%d times' %(contactID, status, cmd, self.probe_count))
        else:
            self.probe_count = 1
            self.sw.log ('Probe result of [%d]: %d (%s)' % (contactID, status, cmd))

        self.pre_status = status    
        return "*" + str (status) + "\015"

    def process_R_command(self, cmd):
        if len(cmd) != 1:
            self.abort_simulation('"R" command format "R" (%s) ' %cmd)
            return ''
        self.sw.log('s88 memory buffer cleaned. (%s)' % cmd )
        return ''

    def reset (self):
        self.setup_vehicles()
        self.reset_switches()
        self.sw.clear()
        del self.command_buffer
        self.command_buffer = []
        self.start_simulation()

    def start_simulation (self):
        # Init pre_cmd, pre_pre_cmd, pre_status
        self.pre_pre_cmd = ''
        self.pre_cmd = ''
        self.pre_status = -1
        
        if not self.simulation_running:
            self.simulation_running = 1

    def abort_simulation (self, msg):
        self.simulation_running = 0
        self.sw.log (msg)
        self.sw.log ('Simulation aborted')

    def next_simulation_step (self):
        if self.simulation_running:
            self.track.next_simulation_step()

    def add_command_to_buffer(self, cmd, time_interval):
        self.command_buffer.append((cmd, time_interval))

    def get_next_command(self):
        if len(self.command_buffer) != 0:
            return self.command_buffer.pop(0)
        else:
            return () 
        


class Socket_manager:
    def __init__ (self, inet):
        ip_port = inet.split (':')
        if len (ip_port) != 2:
            print 'Bad inet argument (%s)' % inet
            thread.exit_thread()
        self.ip = ip_port[0]
        self.port = int (ip_port[1])
        self.is_connected = 0
        self.glock = thread.allocate_lock()
        self.rlock = thread.allocate_lock()

                
    def get_connection (self):
        self.glock.acquire()
        if self.is_connected == 0:
            self.connect()
        self.glock.release()
        return self.conn
    
    def connect (self):
        try:
            s = socket.socket (socket.AF_INET, socket.SOCK_STREAM)
            s.bind ((self.ip, self.port))
            s.listen (1)
            self.conn, addr = s.accept()
                                   
        except:
            print "Could not connect to '%s'" % inet
            thread.exit_thread()

        self.is_connected = 1

    def re_connect(self, oldconn):
        self.rlock.acquire()
        if self.conn == oldconn:
            self.conn.close()
            self.connect()
        self.rlock.release()
        
        return self.conn

def calculate_time_interval(conn):
    i = 0
    end_time = 0
    
    for i in range(6):
        while 42:
            try:
                ch = conn.recv (1)
            except:
                conn = socket_man.re_connect(conn)
                i = -2
                break
            if not ch:
                conn = socket_man.re_connect(conn)
                i = -2
                break
            if ch == '\015':
                break
                       
        if i == 0:
           start_time = time.time()
        if i == 5:
            end_time = time.time()         
        i = i + 1
        
    return (end_time - start_time)/5.0 - 0.0055 
            

def read_commands_from_socket(train_gui, socket_man):
    conn = socket_man.get_connection()
    last_time = time.time() - 1
    
    while 42:
        cmd = ''
        while 42:
            try:
                ch = conn.recv (1)
            except:
                conn = socket_man.re_connect(conn)
                cmd = "invalid"
                break
            if not ch:
                conn = socket_man.re_connect(conn)
                cmd = "invalid"
                break
            if ch == '\015':
                break
            
            cmd += ch
                    
        this_time = time.time()
        #print this_time - last_time

        global detected_time_interval
 
        #the first command from boot loader
        if cmd == 'M':
            detected_time_interval = calculate_time_interval(conn)
            train_gui.reset()
            print "detected time interval: %f " %(detected_time_interval)

        elif cmd != "invalid":
            #AP: remove timing code. Something is not working.
            #if this_time - last_time < detected_time_interval: 
            #    train_gui.abort_simulation("Commands are sent too fast. Not able to process. (%s)" % (cmd))
            #else:
                train_gui.add_command_to_buffer(cmd, this_time - last_time)
                
        last_time = this_time
        

def process_train_command(train_gui, socket_man):
    conn = socket_man.get_connection()
    last_factor = 1 
    
    while 42:
        temp = train_gui.get_next_command()
        if len(temp) > 0:
            cmd = temp[0]

            # calculate the delay between two commands. (calculate how much
            # time the user has delayed before sending this command.)
            #
            # time_interaval: the interval between the time when this command
            # was received and the time when the last command was received.  
            # the last command. It corresponding to the delay a user put
            # betweent the two commands.
            #  
            # factor: the factor comparing time_interval to 
            # detected_time_interval, which is the detected time interval
            # between two commands that were sent minimal delay in between.   i
            # 
            # delay_time: the time the simulator must delay before processing
            # this command. It is corresponding to factor, which corresponding 
            # to the delay a user put between two commands. 
 
            time_interval = temp[1] 
            factor = time_interval/detected_time_interval
            if factor < 10: 
               last_factor = factor 
            delay_time = command_processing_delay_factor * last_factor

            time.sleep( serial_line_delay + delay_time)
            ret = train_gui.process_train_command (cmd)       
                
            if ret != '':
                time.sleep(serial_line_delay)
                try:
                    conn.send(ret)
                except:
                    conn = socket_man.get_connection()
                    try:
                        conn.send(ret)
                    except:
                        print "abort action: sending result of %s" %(cmd)
        else:
            time.sleep(wait_for_command_sleep_time)

def run_simulation (train_gui):
    while 42:
        current_time = time.time()
        train_gui.next_simulation_step()
        processing_time = time.time() - current_time
        delta = sleep_time - processing_time
        if delta > 0:
            time.sleep (delta)

try:
    bochsrc = os.path.expanduser ('~/.bochsrc')
    bochsrc_f = open (bochsrc, 'r')
    lines = bochsrc_f.readlines()
    bochsrc_f.close()
    inet = None
    for l in lines:
        if l[0] == '#':
            continue
        i = l.find ('com1: ')
        if i != -1:
            if l.find('mode=socket') == -1:
                print 'com1 not redirected to a socket'
                continue
            i = l.find ('dev=')
            if i == -1:
                print 'com1 not followed by dev argument'
                continue
            inet = l[i+4:-1]
    if inet == None:
        print 'com1 not configured properly'
        throw
except:
    inet = "localhost:8888"
    
random.seed()

train_gui = TrainGUI (track_layout)
socket_man = Socket_manager(inet)
detected_time_interval = 0.2
#0.2 is a guess. Will be modified later when bochs is booting
    
thread.start_new (read_commands_from_socket, (train_gui, socket_man))
thread.start_new (process_train_command, (train_gui, socket_man))
thread.start_new (run_simulation, (train_gui,))
mainloop()

