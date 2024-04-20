import tkinter as tk

# Create a new window
window = tk.Tk()

# Create lists to store the dropdowns, text inputs, and disabled inputs
dropdowns = []
text_inputs = []
disabled_inputs = []

for j in range(2):  # Loop twice to create two sets of inputs
    # Create a disabled text input
    entry = tk.Entry(window, width=1)
    entry.insert(0, str(j))  # Hardcode the first input to 0 for the first set and 1 for the second set
    entry.config(state='disabled')  # Disable the input
    entry.grid(row=0, column=j*9)  # Adjust the grid position based on the set
    disabled_inputs.append(entry)

    # Create the dropdowns and text inputs
    for i in range(4):
        # Create a dropdown
        var = tk.StringVar(window)
        var.set('0')  # set the default option
        dropdown = tk.OptionMenu(window, var, '0', '1')
        dropdown.grid(row=0, column=1+i*2+j*9, padx=(0, 20))  # Adjust the grid position based on the set and input
        dropdowns.append(var)

        # Create a text input
        text_input = tk.Entry(window, width=2)
        text_input.grid(row=0, column=2+i*2+j*9)  # Adjust the grid position based on the set and input
        text_inputs.append(text_input)

# Create the large text input
large_entry = tk.Entry(window, width=40)
large_entry.grid(row=1, column=0, columnspan=24)

# Define the function to be called when the button is clicked
def generate():
    # Get the text from the text inputs
    combined_text = disabled_inputs[0].get()
    combined_text += ''.join([dropdowns[i].get() + text_inputs[i].get().zfill(2) for i in range(0*4, 0*4+4)] )  # Include all inputs
    combined_text += disabled_inputs[1].get()
    combined_text += ''.join([dropdowns[i].get() + text_inputs[i].get().zfill(2) for i in range(1*4, 1*4+4)] )  # Include all inputs

    # Combine the text and set it in the large text input
    large_entry.delete(0, tk.END)
    large_entry.insert(0, combined_text)

# Define an empty function to be called when the send button is clicked
def send():
    pass

# Create the generate button
generate_button = tk.Button(window, text="Generate", command=generate)
generate_button.grid(row=0, column=18)

# Create the send button
send_button = tk.Button(window, text="Send", command=send)
send_button.grid(row=1, column=18)

# Start the main loop
window.mainloop()

