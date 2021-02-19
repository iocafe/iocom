import matplotlib.pyplot as plt
import math
import numpy as np


def rotate_origin_only(xy, radians):
    """Only rotate a point around the origin (0, 0)."""
    x, y = xy
    xx = x * math.cos(radians) + y * math.sin(radians)
    yy = -x * math.sin(radians) + y * math.cos(radians)

def plot_wheel(plt, x, y, d, w, a):
    X = np.array([-w/2, w/2, w/2, -w/2, -w/2])
    Y = np.array([-d/2, -d/2, d/2, d/2, -d/2])
    XX = X * math.cos(a) + Y * math.sin(a) + x
    YY = -X * math.sin(a) + Y * math.cos(a) + y
    plt.plot(XX, YY)

def plot_line(plt, x1, y1, x2, y2):
    plt.plot([x1, x2], [y1, y2])

def simulate(plt, myrange):
    b_wheel_d = 9.6
    b_wheel_w = 2.3
    b_wheel_x = 5.5 * 2.54
    f_wheel_d = 7.6
    f_wheel_w = 2.3
    f_wheel_x = 4.6 * 2.54
    f_wheel_y = (7 + 1/2) * 2.54

    # We use randians and degrees 
    coeff = 2.0 * 3.1415 / 360

    # for steering in range(-91,91,2):
    for steering in myrange:
        plt.cla()
        plt.text(0, -0.1, str(int(steering)), horizontalalignment='center',verticalalignment='top')

        # "steering" input from -90 degrees (right) to 90 degrees (left) is for direction. 0 = straight forward.
        # "speed" is movement speed from -100% (backwards) to 100% (forward).
        speed = 100

        # Calculate center_x on back wheel axis as rotation center.
        a = coeff * steering
        center_x = f_wheel_y / math.tan(a)

        # Just decoration
        dx = f_wheel_x - center_x;
        c = plt.Circle((center_x, 0), radius=math.sqrt(dx*dx + f_wheel_y*f_wheel_y), color='red', alpha=.05)
        plt.gca().add_artist(c)
        dx = f_wheel_x + center_x;
        c = plt.Circle((center_x, 0), radius=math.sqrt(dx*dx + f_wheel_y*f_wheel_y), color='red', alpha=.05)
        plt.gca().add_artist(c)

        # Calculate wheel directins in radians and convert to degrees
        ar = math.atan((f_wheel_y)/(center_x - f_wheel_x))
        al = math.atan((f_wheel_y)/(center_x + f_wheel_x))
        r_dir = ar/coeff
        l_dir = al/coeff

        # Plot wheels and lines
        plot_line(plt, f_wheel_x, f_wheel_y, center_x, 0)
        plot_line(plt, -f_wheel_x, f_wheel_y, center_x, 0)
        x_space = 6
        plot_wheel(plt, b_wheel_x, 0, b_wheel_d, b_wheel_w, 0.0)
        plot_wheel(plt, -b_wheel_x, 0, b_wheel_d, b_wheel_w, 0.0)
        plot_line(plt, -b_wheel_x * x_space, 0,  b_wheel_x * x_space, 0)
        plot_wheel(plt, f_wheel_x, f_wheel_y, f_wheel_d, f_wheel_w, ar)
        plot_wheel(plt, -f_wheel_x, f_wheel_y, f_wheel_d, f_wheel_w, al)

        # Show directions        
        plt.text(f_wheel_x, f_wheel_y - f_wheel_d - 0.1, str(int(r_dir)), horizontalalignment='center',verticalalignment='top')
        plt.text(-f_wheel_x, f_wheel_y - f_wheel_d - 0.1, str(int(l_dir)), horizontalalignment='center',verticalalignment='top')

        # Calculate motor speeds, sl is left motor speed and sr right motor. Positive values forward and negative back.
        if center_x > b_wheel_x:
            sl = speed
            sr = sl * (center_x - b_wheel_x) /  (center_x + b_wheel_x)
        elif center_x < -b_wheel_x:
            sr = speed
            sl = sr * (-center_x - b_wheel_x) /  (-center_x + b_wheel_x)
        elif center_x > 0:
            sl = speed * (center_x + b_wheel_x) / (2 * b_wheel_x)
            sr = -sl * (b_wheel_x - center_x) /(b_wheel_x + center_x)
        else:
            sr = speed * (-center_x + b_wheel_x) / (2 * b_wheel_x)
            sl = -sr * (b_wheel_x + center_x) /(b_wheel_x - center_x)

        # Show motor speeds
        plt.text(b_wheel_x, -b_wheel_d - 0.1, str(int(sr)) + "%", horizontalalignment='center',verticalalignment='top')
        plt.text(-b_wheel_x, -b_wheel_d - 0.1, str(int(sl)) + "%", horizontalalignment='center',verticalalignment='top')

        plt.ylabel('grumpy wheel direction and speed')
        plt.gca().set_aspect('equal', adjustable='box')
        plt.xlim([-50, 50])
        plt.ylim([-10, 30])
        plt.pause(0.05)


simulate(plt, range(1,45,2))
simulate(plt, range(47,-89, -2))
simulate(plt, range(-87,89, 2))
simulate(plt, range(87, -21, -2))
simulate(plt, range(-19, 15, 2))

plt.show()



