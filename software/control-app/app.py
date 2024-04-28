
import tkinter as tk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import numpy as np
import subprocess

# leg setup
femur_start_deg = 5
knee_start_deg = 15

# angle calculations
disk_holes = 38
res = 360 / (38*4)

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

def update_angle(val):
    draw_line(float(180-slider1.get()), float(-(180-slider2.get())))
    
    femur = 0
    knee = 0
    
    for i in range(4):
        if graph_states[i].get():
            femur = round(slider1.get()/res)
            knee = round(slider2.get()/res)
        else:
            femur = 0
            knee = 0
        # Update the text inputs with the new slider values
        text_inputs[i*2].delete(0, tk.END)
        text_inputs[i*2].insert(0, str(femur))
        text_inputs[i*2+1].delete(0, tk.END)
        text_inputs[i*2+1].insert(0, str(knee))      

root = tk.Tk()
root.wm_title("Angle Slider")

fig = Figure(figsize=(7, 6), dpi=100)
canvas = FigureCanvasTkAgg(fig, master=root)
canvas.draw()
canvas.get_tk_widget().grid(row=0, column=0)

graph_states = [tk.BooleanVar() for _ in range(4)]
all_graphs_state = tk.BooleanVar(value=True)
check_button_frame = tk.Frame(root)
check_button_frame.grid(row=0, column=3, rowspan=2)

def update_all_graphs_state():
    state = all_graphs_state.get()
    for graph_state in graph_states:
        graph_state.set(state)

for i in range(4):
    check_button = tk.Checkbutton(check_button_frame, text=str(i+1), variable=graph_states[i])
    check_button.pack(side=tk.TOP)

all_graphs_check_button = tk.Checkbutton(check_button_frame, text="All", variable=all_graphs_state, command=update_all_graphs_state)
all_graphs_check_button.pack(side=tk.TOP)

slider1 = tk.Scale(root, from_=0, to=180, orient=tk.VERTICAL, length=300, command=update_angle)
slider1.set(femur_start_deg)
slider1.grid(row=0, column=1)

slider2 = tk.Scale(root, from_=0, to=180, orient=tk.VERTICAL, length=300, command=update_angle)
slider2.set(knee_start_deg)
slider2.grid(row=0, column=2)

# Create a new frame for the new UI elements
new_frame = tk.Frame(root)
new_frame.grid(row=2, column=0, columnspan=4)  # Adjust the row and column as needed

# Create a title
title = tk.Label(new_frame, text="MESSAGE GENERATOR", font=("Arial", 20))
title.pack()

dropdowns = []
text_inputs = []
disabled_inputs = []

# Create a new frame for the inputs
input_frame = tk.Frame(new_frame)
input_frame.pack()

for j in range(2):  # Loop twice to create two sets of inputs
    entry = tk.Entry(input_frame, width=1)
    entry.insert(0, str(j))
    entry.config(state='disabled')
    entry.pack(side=tk.LEFT)
    disabled_inputs.append(entry)

    for i in range(4):
        var = tk.StringVar(input_frame)
        var.set('0')
        dropdown = tk.OptionMenu(input_frame, var, '0', '1')
        dropdown.pack(side=tk.LEFT)
        dropdowns.append(var)

        text_input = tk.Entry(input_frame, width=2)
        text_input.pack(side=tk.LEFT)
        text_inputs.append(text_input)


def generate():
    combined_text = disabled_inputs[0].get()
    combined_text += ''.join([dropdowns[i].get() + text_inputs[i].get().zfill(2) for i in range(0*4, 0*4+4)])
    combined_text += disabled_inputs[1].get()
    combined_text += ''.join([dropdowns[i].get() + text_inputs[i].get().zfill(2) for i in range(1*4, 1*4+4)])

    large_entry.delete(0, tk.END)
    large_entry.insert(0, combined_text)

generate_button = tk.Button(input_frame, text="Generate", command=generate)
generate_button.pack(side=tk.LEFT)

large_input_frame = tk.Frame(new_frame)
large_input_frame.pack()

large_entry = tk.Entry(large_input_frame, width=40)
large_entry.pack(side=tk.LEFT)

def send():
    # Use subprocess.run() to execute the command
    result = subprocess.run("./udp-send-command.sh " + str(large_entry.get()), shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    # You can get the output of the command from result.stdout
    # Be sure to decode it from bytes to a string
    output = result.stdout.decode()

    print("Output:")
    print(output)

send_button = tk.Button(large_input_frame, text="Send", command=send)
send_button.pack(side=tk.LEFT)

draw_line(180, 0)
tk.mainloop()

