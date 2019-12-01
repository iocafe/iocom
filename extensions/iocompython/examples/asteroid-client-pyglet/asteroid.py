import pyglet, random, math, time
from pyglet.window import key
from game import load, player, resources
from iocompython import Root, MemoryBlock, Connection, Signal, json2bin

signal_conf = ('{'
  '"mblk": ['
  '{'
    '"name": "exp",'
    '"groups": ['
       '{'
         '"name": "control",'
         '"signals": ['
           '{"name": "userctrl", "type": "float", "array": 5}'
         ']'
       '}'
    ']'
  '},'
  '{'
    '"name": "imp",'
    '"groups": ['
      '{'
        '"name": "world",'
        '"signals": ['
           '{"name": "coords", "type": "short", "array": 40}'
         ']'
      '}'
    ']'
  '}'
  ']'
'}')


my_player_nr = int(time.time()) % 9998 + 1 # Bad way to make unique device number (not really unique)
max_players = 50 
data_vector_n = 8

root = Root('mygame', device_nr=my_player_nr, network_name='pekkanet')
exp = MemoryBlock(root, 'upward,auto', 'exp', nbytes=32)
imp = MemoryBlock(root, 'downward,auto', 'imp', nbytes=2*max_players*data_vector_n + 2)
data = json2bin(signal_conf)
info = MemoryBlock(root, 'upward,auto', 'info', nbytes=len(data))
info.publish(data)
# connection = Connection(root, "192.168.1.220", "socket,upward")
connection = Connection(root, "127.0.0.1", "socket,upward")
myworld = Signal(root, "coords", "pekkanet")
userctrl = Signal(root, "userctrl", "pekkanet")

# Set up a window
game_window = pyglet.window.Window(900, 600)

main_batch = pyglet.graphics.Batch()

# Set up the two top labels
score_label = pyglet.text.Label(text="Score: 0", x=10, y=575, batch=main_batch)
level_label = pyglet.text.Label(text="Version 3: Basic Collision",
                                x=400, y=575, anchor_x='center', batch=main_batch)

# Store all objects that update each frame in a list
space_ships = []

# Let pyglet handle keyboard events for us
key_handler = key.KeyStateHandler()

# Tell the main window who responds to events
game_window.push_handlers(key_handler)

# Set some easy-to-tweak constants
my_thrust = 300.0
my_rotate_speed = 200.0

# We control the game by sending force and rotation
my_rotation = 0.0
my_force_x = 0.0
my_force_y = 0
my_engine_visible = False
resurrect_me = False

@game_window.event
def on_draw():
    game_window.clear()
    main_batch.draw()

def keyboard_input(dt):
    global resurrect_me, my_rotation, my_force_x, my_force_y
    global my_engine_visible, my_rotate_speed, my_thrust
    global key_handler, userctrl

    if key_handler[key.LEFT]:
        my_rotation -= my_rotate_speed * dt
    if key_handler[key.RIGHT]:
        my_rotation += my_rotate_speed * dt

    if key_handler[key.UP]:
        angle_radians = -math.radians(my_rotation)
        my_force_x = math.cos(angle_radians) * my_thrust * dt
        my_force_y = math.sin(angle_radians) * my_thrust * dt
        my_engine_visible = True
    else:
        my_force_x = 0.0
        my_force_y = 0.0
        my_engine_visible = False

    if key_handler[key.SPACE]:
        resurrect_me = True

    userctrl.set( (my_rotation, my_force_x, my_force_y, my_engine_visible, resurrect_me) )
     
def set_player(ship, player_ix0, data):
    ix = player_ix0 * data_vector_n + 1
    if player_ix0 == data[ix]:
        ship.mysetplayer(data[ix+1], data[ix+2], data[ix+3], my_rotation, my_engine_visible, data[ix+6], data[ix+7], main_batch);

    else:
        ship.mysetplayer(data[ix+1], data[ix+2], data[ix+3], data[ix+4], data[ix+5], data[ix+6],  data[ix+7], main_batch);

def update(dt):
    keyboard_input(dt)

    state_bits, data = myworld.get()

    if state_bits & 2:
        nro_players = data[0]

        for player_ix0 in range(len(space_ships), nro_players): 
            ship = player.Player(batch=main_batch)
            space_ships.append(ship)

        for player_ix0 in range(nro_players): 
            set_player(space_ships[player_ix0], player_ix0, data)

        while len(space_ships) > nro_players:
          space_ships[-1].delete()
          del space_ships[-1]

if __name__ == "__main__":
    # Update the game 60 times per second
    pyglet.clock.schedule_interval(update, 1 / 60.0)
    pyglet.app.run()

