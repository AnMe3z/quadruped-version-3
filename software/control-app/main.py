import tkinter as tk

# Create a new window
window = tk.Tk()

# Create the first text input
entry1 = tk.Entry(window, width=1)
entry1.insert(0, "0")  # Hardcode the first input to 0
entry1.config(state='disabled')  # Disable the first input
entry1.grid(row=0, column=0)

# Create lists to store the dropdowns and text inputs
dropdowns = []
text_inputs = []

# Create the dropdowns and text inputs
for i in range(6):
    # Create a dropdown for the second input
    var = tk.StringVar(window)
    var.set('0')  # set the default option
    dropdown = tk.OptionMenu(window, var, '0', '1')
    dropdown.grid(row=0, column=1+i*2, padx=(0, 20))  # Add padding to move the inputs closer
    dropdowns.append(var)

    # Create the text input
    text_input = tk.Entry(window, width=2)
    text_input.grid(row=0, column=2+i*2)
    text_inputs.append(text_input)

# Create the large text input
large_entry = tk.Entry(window, width=20)
large_entry.grid(row=1, column=0, columnspan=12)

# Define the function to be called when the button is clicked
def generate():
    # Get the text from the text inputs
    text1 = entry1.get()
    combined_text = ''.join([dropdowns[i].get() + text_inputs[i].get().zfill(2) for i in range(6)])

    # Combine the text and set it in the large text input
    large_entry.delete(0, tk.END)
    large_entry.insert(0, text1 + combined_text)

# Define an empty function to be called when the send button is clicked
def send():
    pass

# Create the generate button
generate_button = tk.Button(window, text="Generate", command=generate)
generate_button.grid(row=0, column=13)

# Create the send button
send_button = tk.Button(window, text="Send", command=send)
send_button.grid(row=1, column=13)

# Start the main loop
window.mainloop()

