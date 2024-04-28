import tkinter as tk
from tkinter import ttk, scrolledtext

class Application(tk.Frame):
    def __init__(self, master=None):
        super().__init__(master)
        self.master = master
        self.pack()
        self.create_widgets()

    def create_widgets(self):
        self.text_display = scrolledtext.ScrolledText(self, width=50, height=10)
        self.text_display.pack(side="top")

        self.text_input = tk.Entry(self)
        self.text_input.pack(side="left")

        self.add_button = tk.Button(self)
        self.add_button["text"] = "Add"
        self.add_button["command"] = self.add_text
        self.add_button.pack(side="left")

        self.delay_input = tk.Entry(self, width=5)
        self.delay_input.insert(0, "500")  # Set default value
        self.delay_input.pack(side="left")

    def add_text(self):
        text = self.text_input.get()
        print(text)  # Print text to console
        self.text_display.insert('end', text + '\n')
        self.text_input.delete(0, 'end')

root = tk.Tk()
app = Application(master=root)
app.mainloop()

