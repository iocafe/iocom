# Module: asteroidplayer.py
from iocompython import Root, Signal
import re, math, random

class AsteroidPlayer(object):
    # Set up IO signals needed for the player and initialize the space ship position and state.
    def __init__(self, root, player_name, network_name):
        self.root = root
        self.player_name = player_name
        tmp = re.findall(r'\d+', player_name)
        self.player_nr = int(tmp[0])
    
        # Setup Python access to exported IO signals. These pass user keyboard control to the asteroid service.
        self.device_path = player_name + "." + network_name
        dev_select = ".*." + self.device_path
        print("force_y" + dev_select)
        self.exp_force_x = Signal(root, "force_x" + dev_select)
        self.exp_force_y = Signal(root, "force_y" + dev_select)
        self.exp_rotation = Signal(root, "rotation" + dev_select)
        self.exp_engine_visible = Signal(root, "engine_visible" + dev_select)
        self.exp_shoot = Signal(root, "shoot" + dev_select)

        # Imported IO signals contain the object matrix to draw
        self.imp_nro_objects = Signal(root, "nro_objects" + dev_select)
        self.imp_object_data = Signal(root, "object_data" + dev_select)

        self.velocity_x = (random.random() - 0.5) * 5.0
        self.velocity_y = (random.random() - 0.5) * 5.0
        self.x = 100 + random.random() * 600.0
        self.y = 100 + random.random() * 400.0
        self.rotation = 0
        self.opacity = 0;
        self.shoot =  0;

        self.prev_fire_button = 0;
        self.fire_now = False;

    # Get user's keyboard control state, move the space ship and return the
    # data vector for the space ship.
    def run(self, dt):
        self.root.receive(self.device_path)
        force_x = self.exp_force_x.get0()
        force_y = self.exp_force_y.get0()
        self.rotation = self.exp_rotation.get0()
        engine_visible = self.exp_engine_visible.get0()
        state_bits, self.shoot = self.exp_shoot.get()
        if (state_bits & 2) == 0:
            self.opacity = 30;
            return

        self.velocity_x += force_x * dt
        self.velocity_y += force_y * dt
        self.x += self.velocity_x * dt
        self.y += self.velocity_y * dt

        if self.x < 0:
            self.x += 800
        if self.x > 800:
            self.x -= 800
        if self.y < 0:
            self.y += 600
        if self.y > 600:
            self.y -= 600
        
        self.opacity += dt * 930.0;
        if self.opacity > 255:
                self.opacity = 255

        return (self.player_nr, 100, self.x, self.y, self.rotation, engine_visible, 1, self.opacity)

    # Check if user is pressing fire key. If so return space ship position, rotation, and velocity
    # for setting bullet's starting point and speed.
    def fires(self):
        if self.shoot:
            self.shoot = False
            return (self.x, self.y, self.rotation, self.velocity_x, self.velocity_y)

        return None

    # Send object position and outlook data to the client application.
    # Send data synchronously to device "player name"
    def set_object_data(self, x):
        total_array_n = self.imp_object_data.get_attribute("n")
        player_ncols = self.imp_object_data.get_attribute("ncolumns")
        if total_array_n == None or player_ncols == None:
            return
        player_nrows = total_array_n // player_ncols

        world_nrows = len(x)
        if world_nrows < 1:
            return

        world_ncols = len(x[0])

        # if we are good and no need to reformat the data, just set it
        if world_nrows <= player_nrows and world_ncols == player_ncols:
            self.imp_nro_objects.set(world_nrows)
            self.imp_object_data.set(x)
            self.root.send(self.device_path)
            return

        # --- Rest of this function is resizing data matrix for possible
        #     future changes in data matrix format - NON ESSENTAL FOR EXAMPLE ---
        if player_nrows > world_nrows:
             player_nrows = world_nrows

        min_cols = world_ncols
        if player_ncols < min_cols:
            min_cols = player_ncols

        new_x = []
        for y in range(player_nrows):
            row = x[y]
            new_row = []
            for i in range(min_cols):
                new_row.append(row[i])

            for i in range(min_cols, player_ncols):
                new_row.append(0)

            new_x.append(new_row)

        self.imp_nro_objects.set(player_nrows)
        self.imp_object_data.set(new_x)
        self.root.send(self.device_path)
