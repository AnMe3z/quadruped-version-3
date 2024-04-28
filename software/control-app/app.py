
import tkinter as tk
from tkinter import ttk, scrolledtext
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import numpy as np
import subprocess
import time

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

def decode_message(message):
    # Subtract the direction from each half
    first_half = message[1:13]
    second_half = message[14:]

    # Split each half into 4 3-digit numbers
    first_half_numbers = [first_half[i:i+3] for i in range(0, len(first_half), 3)]
    second_half_numbers = [second_half[i:i+3] for i in range(0, len(second_half), 3)]

    # Decode each 3-digit number
    def decode_number(num):
        sign = '+' if num[0] == '0' else '-'
        value = int(num[1:])
        return (sign, value)

    first_half_decoded = [decode_number(num) for num in first_half_numbers]
    second_half_decoded = [decode_number(num) for num in second_half_numbers]

    return first_half_decoded, second_half_decoded

def compare_messages(message1, message2):
    # Decode the messages
    first_half1, second_half1 = decode_message(message1)
    first_half2, second_half2 = decode_message(message2)

    # Calculate the differences
    def calculate_difference(half1, half2):
        difference = []
        for num1, num2 in zip(half1, half2):
            sign1, value1 = num1
            sign2, value2 = num2
            diff_value = value2 - value1 if sign1 == sign2 else value2 + value1
            diff_sign = '0' if diff_value >= 0 else '1'
            difference.append(f'{diff_sign}{abs(diff_value):02d}')
        return difference

    first_half_diff = calculate_difference(first_half1, first_half2)
    second_half_diff = calculate_difference(second_half1, second_half2)

    # Construct the third message
    third_message = '0' + ''.join(first_half_diff) + '1' + ''.join(second_half_diff)
    return third_message

last_command = 0;

def add_text_from_gen():
    text = "Empty! ERROR!"
    global last_command
    if last_command == 0:
        last_command = large_entry.get()
        text = large_entry.get()
    else:
        text = compare_messages(last_command, large_entry.get())

    # Get the text from the entry
    
    # Add the text to the list
    texts.append(text)
    # Clear the entry
    text_entry.delete(0, tk.END)
    # Create a new frame for the text
    text_frame = tk.Frame(new_functionality_frame)
    text_frame.pack(side=tk.TOP)
    # Create a label with the text and pack it in the frame
    text_label = tk.Label(text_frame, text=text)
    text_label.pack()
    
add_send_button = tk.Button(large_input_frame, text="Add to Q", command=add_text_from_gen)
add_send_button.pack(side=tk.LEFT)

# ... (Your existing code remains unchanged here)

# New functionality
new_functionality_frame = tk.Frame(root)
new_functionality_frame.grid(row=0, column=5, rowspan=3, padx=20)  # Added some padding to create distance
entry_button_frame = tk.Frame(new_functionality_frame)
entry_button_frame.pack(side=tk.TOP)

# Create a list to store the texts
texts = []

def add_text():
    # Get the text from the entry
    text = text_entry.get()
    # Add the text to the list
    texts.append(text)
    # Clear the entry
    text_entry.delete(0, tk.END)
    # Create a new frame for the text
    text_frame = tk.Frame(new_functionality_frame)
    text_frame.pack(side=tk.TOP)
    # Create a label with the text and pack it in the frame
    text_label = tk.Label(text_frame, text=text)
    text_label.pack()

# Move the text entry and the "Add" button to this frame
text_entry = tk.Entry(entry_button_frame, width=20)
text_entry.pack(side=tk.LEFT)
add_button = tk.Button(entry_button_frame, text="Add to Q", command=add_text)
add_button.pack(side=tk.LEFT)

def delete_all():
    # Loop over all children of new_functionality_frame
    for widget in new_functionality_frame.winfo_children():
        # If the widget is a Frame and it's not one of the frames we want to keep
        if isinstance(widget, tk.Frame) and widget not in [entry_button_frame, delete_button_frame, delay_button_frame]:
            widget.destroy()
    # Clear the texts list
    texts.clear()
    
    last_command = 0;


# Create a frame for the "Delete All" button
delete_button_frame = tk.Frame(new_functionality_frame)
delete_button_frame.pack(side=tk.TOP)

# Move the "Delete All" button to this frame
delete_all_button = tk.Button(delete_button_frame, text="Delete All", command=delete_all)
delete_all_button.pack(side=tk.TOP)

def send_q():
    for command in texts:
        com_q.append([command, delay_entry.get()])
        
        result = subprocess.run("./udp-send-command.sh " + str(command), shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        output = result.stdout.decode()
        print("Output:")
        print(output)
        
        time.sleep(int(delay_entry.get())*0.001)

    
    print(com_q)
    
    
delay_button_frame = tk.Frame(new_functionality_frame)
delay_button_frame.pack(side=tk.TOP)
delay_entry = tk.Entry(delay_button_frame, width=20)
delay_entry.pack(side=tk.LEFT)
delay_entry.insert(0, "500")
add_button = tk.Button(delay_button_frame, text="Send Q", command=send_q)
add_button.pack(side=tk.LEFT)

com_q = []

# ... (Your existing code remains unchanged here)

draw_line(180, 0)
tk.mainloop()

