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
    b_wheel_d = 7
    b_wheel_w = 1.5
    b_wheel_x = 10
    f_wheel_d = 4
    f_wheel_w = 1.3
    f_wheel_x = 9
    f_wheel_y = 20

    coeff = -2.0 * 3.1415 / 360

    # for steering in range(-91,91,2):
    for steering in myrange:
        plt.cla()
        plt.text(0, -0.1, str(int(steering)), horizontalalignment='center',verticalalignment='top')

        a = coeff * steering

        center_x = f_wheel_y / math.tan(a)

        # center_x = 20

        x_space = 6

        plot_line(plt, f_wheel_x, f_wheel_y, center_x, 0)
        ar = math.atan((f_wheel_y)/(center_x - f_wheel_x))
        al = math.atan((f_wheel_y)/(center_x + f_wheel_x))
        plot_line(plt, -f_wheel_x, f_wheel_y, center_x, 0)

        plot_wheel(plt, b_wheel_x, 0, b_wheel_d, b_wheel_w, 0.0)
        plot_wheel(plt, -b_wheel_x, 0, b_wheel_d, b_wheel_w, 0.0)
        plot_line(plt, -b_wheel_x * x_space, 0,  b_wheel_x * x_space, 0)
        plot_wheel(plt, f_wheel_x, f_wheel_y, f_wheel_d, f_wheel_w, ar)
        plot_wheel(plt, -f_wheel_x, f_wheel_y, f_wheel_d, f_wheel_w, al)

        plt.text(f_wheel_x, f_wheel_y - f_wheel_d - 0.1, str(int(ar/coeff)), horizontalalignment='center',verticalalignment='top')
        plt.text(-f_wheel_x, f_wheel_y - f_wheel_d - 0.1, str(int(al/coeff)), horizontalalignment='center',verticalalignment='top')

        if center_x > b_wheel_x:
            sl = 100
            sr = sl * (center_x - b_wheel_x) /  (center_x + b_wheel_x)
        elif center_x < -b_wheel_x:
            sr = 100
            sl = sr * (-center_x - b_wheel_x) /  (-center_x + b_wheel_x)
        elif center_x > 0:
            sl = 100 * (center_x + b_wheel_x) / (2 * b_wheel_x)
            sr = -sl * (b_wheel_x - center_x) /(b_wheel_x + center_x)
        else:
            sr = 100 * (-center_x + b_wheel_x) / (2 * b_wheel_x)
            sl = -sr * (b_wheel_x + center_x) /(b_wheel_x - center_x)

        plt.text(b_wheel_x, -b_wheel_d - 0.1, str(int(sr)) + "%", horizontalalignment='center',verticalalignment='top')
        plt.text(-b_wheel_x, -b_wheel_d - 0.1, str(int(sl)) + "%", horizontalalignment='center',verticalalignment='top')

        plt.ylabel('some numbers')
        plt.gca().set_aspect('equal', adjustable='box')
        plt.xlim([-50, 50])
        plt.pause(0.05)


simulate(plt, range(1,45,2))
simulate(plt, range(47,-89, -2))
simulate(plt, range(-87,89, 2))
simulate(plt, range(87, -21, -2))
simulate(plt, range(-19, 15, 2))

plt.show()



