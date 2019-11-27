import pyglet, random, math
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
           '{"name": "ver", "type": "short"},'
           '{"name": "hor"}'
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
           '{"name": "coords", "type": "short", "array": 8}'
         ']'
      '},'
      '{'
        '"name": "me",'
        '"signals": ['
          '{"name": "x", "type": "short"},'
          '{"name": "y"}'
        ']'
      '}'
    ']'
  '}'
  ']'
'}')


root = Root('mygame', device_nr=4, network_name='pekkanet')
exp = MemoryBlock(root, 'source,auto', 'exp', nbytes=256)
imp = MemoryBlock(root, 'target,auto', 'imp', nbytes=256)
data = json2bin(signal_conf)
info = MemoryBlock(root, 'source,auto', 'info', nbytes=len(data))
info.publish(data)
connection = Connection(root, "192.168.1.220", "socket")
myworld = Signal(root, "coords", "pekkanet")

# Set up a window
game_window = pyglet.window.Window(1200, 800)

main_batch = pyglet.graphics.Batch()

# Set up the two top labels
score_label = pyglet.text.Label(text="Score: 0", x=10, y=575, batch=main_batch)
level_label = pyglet.text.Label(text="Version 3: Basic Collision",
                                x=400, y=575, anchor_x='center', batch=main_batch)

# Initialize the player sprite
player_ship = player.Player(myworld=myworld, x=400, y=300, batch=main_batch)

# Make three sprites to represent remaining lives
player_lives = load.player_lives(2, main_batch)

# Make three asteroids so we have something to shoot at 
asteroids = load.asteroids(myworld, 3, player_ship.position, main_batch)

# Store all objects that update each frame in a list
game_objects = [player_ship] + asteroids

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
    global key_handler

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

def update(dt):
    keyboard_input(dt)

    state_bits, d = myworld.get()
    print (state_bits)
    print (d)

    player_ship.mysetplayer(True, 100, 100, my_rotation, my_engine_visible);

    for obj in game_objects:
        obj.update(dt)

    # To avoid handling collisions twice, we employ nested loops of ranges.
    # This method also avoids the problem of colliding an object with itself.
    for i in range(len(game_objects)):
        for j in range(i + 1, len(game_objects)):

            obj_1 = game_objects[i]
            obj_2 = game_objects[j]

            # Make sure the objects haven't already been killed
            if not obj_1.dead and not obj_2.dead:
                if obj_1.collides_with(obj_2):
                    obj_1.handle_collision_with(obj_2)
                    obj_2.handle_collision_with(obj_1)

    # Get rid of dead objects
    for to_remove in [obj for obj in game_objects if obj.dead]:
        # Remove the object from any batches it is a member of
        to_remove.delete()

        # Remove the object from our list
        game_objects.remove(to_remove)


if __name__ == "__main__":
    # Update the game 60 times per second
    pyglet.clock.schedule_interval(update, 1 / 60.0)

    # Tell pyglet to do its thing
    pyglet.app.run()

