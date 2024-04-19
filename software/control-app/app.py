import tkinter as tk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import numpy as np

def draw_line(angle1, angle2):
    fig.clear()
    ax = fig.add_subplot(111)
    rad1 = np.deg2rad(angle1)  # Convert angle1 to radians
    rad2 = np.deg2rad(angle2)  # Convert angle2 to radians

    # First line
    x1 = np.linspace(0, np.cos(rad1), 100)
    y1 = np.linspace(0, np.sin(rad1), 100)
    ax.plot(x1, y1, 'r')  # 'r' for red color

    # Second line
    x2 = np.linspace(np.cos(rad1), np.cos(rad1) + np.cos(rad1 + rad2), 100)
    y2 = np.linspace(np.sin(rad1), np.sin(rad1) + np.sin(rad1 + rad2), 100)
    ax.plot(x2, y2, 'b')  # 'b' for blue color

    ax.set_xlim(-2, 2)  # Set x limits
    ax.set_ylim(-2, 2)  # Set y limits
    canvas.draw()

def update_angle(val):
    draw_line(float(slider1.get()), float(slider2.get()))

root = tk.Tk()
root.wm_title("Angle Slider")

fig = Figure(figsize=(5, 4), dpi=100)
canvas = FigureCanvasTkAgg(fig, master=root)
canvas.draw()
canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=1)

slider1 = tk.Scale(root, from_=0, to=180, orient=tk.HORIZONTAL, command=update_angle)
slider1.pack(side=tk.BOTTOM, fill=tk.X)

slider2 = tk.Scale(root, from_=0, to=180, orient=tk.HORIZONTAL, command=update_angle)
slider2.pack(side=tk.BOTTOM, fill=tk.X)

draw_line(0, 0)  # Draw lines at initial angles
tk.mainloop()

