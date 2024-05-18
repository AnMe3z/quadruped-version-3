import tkinter as tk

def print_lines():
    text = text_input.get("1.0", "end-1c")
    lines = text.split("\n")
    for line in lines:
        print(line)

root = tk.Tk()
root.title("Text Input App")

text_input = tk.Text(root, height=10)
text_input.pack()

print_button = tk.Button(root, text="Print Lines", command=print_lines)
print_button.pack()

root.mainloop()
