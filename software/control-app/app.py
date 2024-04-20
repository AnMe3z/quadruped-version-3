import tkinter as tk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import numpy as np

def draw_line(angle1, angle2):
    fig.clear()
    rad1 = np.deg2rad(angle1)  # Convert angle1 to radians
    rad2 = np.deg2rad(angle2)  # Convert angle2 to radians

    # Create a 2x2 grid of Axes objects
    axs = fig.subplots(2, 2)

    titles = ['FL', 'FR', 'BL', 'BR']
    for i, ax in enumerate(axs.flat):
        if graph_states[i].get():
            # First line
            x1 = np.linspace(0, np.cos(rad1), 100)
            y1 = np.linspace(0, np.sin(rad1), 100)
            ax.plot(x1, y1, 'r')  # 'r' for red color

            # Second line
            x2 = np.linspace(np.cos(rad1), np.cos(rad1) + np.cos(rad1 + rad2), 100)
            y2 = np.linspace(np.sin(rad1), np.sin(rad1) + np.sin(rad1 + rad2), 100)
            ax.plot(x2, y2, 'b')  # 'b' for blue color
            
            ax.set_title(titles[i]+f' FEMUR: {180-angle1}°, KNEE: {-(180-angle2)}°')

            ax.set_xlim(-2, 2)  # Set x limits
            ax.set_ylim(2, -2)  # Set y limits (reversed)

    # Adjust the spacing between the subplots
    fig.subplots_adjust(wspace=0.5, hspace=0.5)

    canvas.draw()
        
#        ax.set_title(f'Angle 1: {180-angle1}°, Angle 2: {angle2}°')
def update_angle(val):
    draw_line(float(180-slider1.get()), float(-(180-slider2.get())))

root = tk.Tk()
root.wm_title("Angle Slider")

fig = Figure(figsize=(7, 6), dpi=100)
canvas = FigureCanvasTkAgg(fig, master=root)
canvas.draw()
canvas.get_tk_widget().grid(row=0, column=0)  # Changed column to 0

# Create variables to hold the state of the check buttons
graph_states = [tk.BooleanVar() for _ in range(4)]
all_graphs_state = tk.BooleanVar(value=True)
check_button_frame = tk.Frame(root)
check_button_frame.grid(row=0, column=3, rowspan=2)  # Adjust the row and column as needed

# Create a function to update the state of all check buttons
def update_all_graphs_state():
    state = all_graphs_state.get()
    for graph_state in graph_states:
        graph_state.set(state)

# Create the check buttons
for i in range(4):
    check_button = tk.Checkbutton(check_button_frame, text=str(i+1), variable=graph_states[i])
    check_button.pack(side=tk.TOP)

all_graphs_check_button = tk.Checkbutton(check_button_frame, text="All", variable=all_graphs_state, command=update_all_graphs_state)
all_graphs_check_button.pack(side=tk.TOP)

slider1 = tk.Scale(root, from_=0, to=180, orient=tk.VERTICAL, length=300, command=update_angle)  # Changed HORIZONTAL to VERTICAL
slider1.set(45)  # Set initial value to 0
slider1.grid(row=0, column=1)  # Changed column to 1

slider2 = tk.Scale(root, from_=0, to=180, orient=tk.VERTICAL, length=300, command=update_angle)  # Changed HORIZONTAL to VERTICAL
slider2.set(45)  # Set initial value to 0
slider2.grid(row=0, column=2)  # No change

# Create a new frame for the new UI elements
new_frame = tk.Frame(root)
new_frame.grid(row=2, column=0, columnspan=4)  # Adjust the row and column as needed

# Create a title
title = tk.Label(new_frame, text="New Section", font=("Arial", 20))
title.pack()

# Create 8 2-digit prompts
entries = []
for i in range(10):
    entry = tk.Entry(new_frame, width=2)
    entry.pack(side=tk.LEFT)
    entries.append(entry)

# Function to update the large text prompt
def update_text_prompt():
    text_prompt.delete(1.0, tk.END)
    text_prompt.insert(tk.END, ''.join(entry.get() for entry in entries))

# Create a button to update the large text prompt
button = tk.Button(new_frame, text="Update", command=update_text_prompt)
button.pack()

# Create the large text prompt
text_prompt = tk.Text(new_frame, width=16, height=4)
text_prompt.pack()


draw_line(180, 0)  # Draw lines at initial angles
tk.mainloop()

