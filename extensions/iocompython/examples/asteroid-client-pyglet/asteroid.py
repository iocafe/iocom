import pyglet, random, math, time
from pyglet.window import key
from game import physicalobject, resources
from iocompython import Root, MemoryBlock, Connection, Signal, json2bin

my_player_nr = 9000  # 9000 = select device number automatically 
data_vector_n = 0

with open('resources/asteroid-signals.json', 'r') as file:
    signal_conf = file.read()

root = Root('spacepilot', device_nr=my_player_nr, network_name='cafenet', security='certchainfile=myhome-bundle.crt')
exp = MemoryBlock(root, 'up', 'exp')
imp = MemoryBlock(root, 'down', 'imp')
data = json2bin(signal_conf)
info = MemoryBlock(root, 'up', 'info', nbytes=len(data))
info.publish(data)
# connection = Connection(root, "192.168.1.220", "tls,up")
connection = Connection(root, "127.0.0.1", "tls,up")
# connection = Connection(root, "3.135.236.95", "tls,up")

# Setup Python access to exported IO signals. These pass user keyboard control to the asteroid service.
exp_force_x = Signal(root, "force_x")
exp_force_y = Signal(root, "force_y")
exp_rotation = Signal(root, "rotation")
exp_engine_visible = Signal(root, "engine_visible")
exp_shoot = Signal(root, "shoot")

# Imported IO signals contain the object matrix to draw
imp_nro_objects = Signal(root, "nro_objects")
imp_object_data = Signal(root, "object_data")

# Set up a window and batch to the draw sprites
game_window = pyglet.window.Window(800, 600)
main_batch = pyglet.graphics.Batch()

# Set up the two top labels
level_label = pyglet.text.Label(text="asteroids vs space ships - collison course",
                                x=400, y=575, anchor_x='center', batch=main_batch)

# Store all objects that update each frame in a list
game_objects = []

# Let pyglet handle keyboard events for us
key_handler = key.KeyStateHandler()

# Tell the main window who responds to the window events
game_window.push_handlers(key_handler)

# Set some easy-to-tweak constants
my_thrust = 300.0
my_rotate_speed = 300.0
my_engine_visible = False

# Current rotation position
my_rotation = 0.0

@game_window.event
def on_draw():
    game_window.clear()
    main_batch.draw()

def keyboard_input(dt):
    global my_rotation, my_engine_visible, my_rotate_speed, my_thrust
    global key_handler

    if key_handler[key.LEFT]:
        my_rotation -= my_rotate_speed * dt
    if key_handler[key.RIGHT]:
        my_rotation += my_rotate_speed * dt

    if key_handler[key.UP]:
        angle_radians = -math.radians(my_rotation)
        force_x = math.cos(angle_radians) * my_thrust
        force_y = math.sin(angle_radians) * my_thrust
        my_engine_visible = True
    else:
        force_x = 0
        force_y = 0
        my_engine_visible = False

    shoot = False
    if key_handler[key.SPACE]:
        shoot = True

    exp_force_x.set(force_x)
    exp_force_y.set(force_y)
    exp_rotation.set(my_rotation)
    exp_engine_visible.set(my_engine_visible)
    exp_shoot.set(shoot)
     
def set_physical_object(o, ix, data):
    ix = ix * data_vector_n
    if my_player_nr == data[ix]:
        o.set(data[ix+1], data[ix+2], data[ix+3], my_rotation, my_engine_visible, data[ix+6], data[ix+7], main_batch);

    else:
        o.set(data[ix+1], data[ix+2], data[ix+3], data[ix+4], data[ix+5], data[ix+6],  data[ix+7], main_batch);

def update(dt):
    global total_array_n, data_vector_n, nro_objects

    keyboard_input(dt)
    exp.send()

    imp.receive()
    state_bits, nro_objects = imp_nro_objects.get_ext()

    total_array_n = imp_object_data.get_attribute("n")
    data_vector_n = imp_object_data.get_attribute("ncolumns")

    if (state_bits & 2) and data_vector_n != None and total_array_n != None:
        max_objects = total_array_n // data_vector_n
        if nro_objects > max_objects:
            nro_objects = max_objects
    
        data = imp_object_data.get(nro_values = nro_objects * data_vector_n)

        for ix in range(len(game_objects), nro_objects):
            o = physicalobject.PhysicalObject(batch=main_batch)
            game_objects.append(o)

        for ix in range(nro_objects):
            set_physical_object(game_objects[ix], ix, data)

        while len(game_objects) > nro_objects:
            game_objects[-1].delete()
            del game_objects[-1]

if __name__ == "__main__":
    # Update the game 60 times per second
    pyglet.clock.schedule_interval(update, 1 / 60.0)
    pyglet.app.run()

