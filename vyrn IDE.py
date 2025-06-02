import subprocess
import tkinter as tk
from tkinter import scrolledtext, messagebox

class VyrnIDE(tk.Tk) :
    def __init__(self):
        super().__init__()
        self.title('Vyrn IDE')
        self.geometry("800x600")

        tk.Label(self, text = 'code Vyrn :').pack()
        self.editor = scrolledtext.ScrolledText(self, height = 20)
        self.editor.pack(fill = tk.BOTH, expand = True)

        self.compile_btn = tk.Button(self, text="Compiler et Exécuter", command=self.compile_and_run)
        self.compile_btn.pack()

        tk.Label(self, text="Sortie :").pack()
        self.output = scrolledtext.ScrolledText(self, height=10, state='disabled')
        self.output.pack(fill=tk.BOTH, expand=True)

    def compile_and_run(self):
        code = self.editor.get('1.0', tk.END)
        with open('input.vyn', 'w') as f:
            f.write(code)
        
        try:
            subprocess.run(['./vyrn', 'input.vyrn'], check=True)
            subprocess.run(['gcc', 'output.c', '-o', 'a.out', '-mconsole'], check=True)
            result = subprocess.run(['./a.out'], capture_output=True, text=True)
            self.output.config(state='normal')
            self.output.delete("1.0", tk.END)
            self.output.insert(tk.END, result.stdout)
            self.output.config(state='disabled')
        except subprocess.CalledProcessError as e:
            messagebox.showerror("Erreur", f"Erreur de compilation ou d'exécution:\n{e}")


if __name__ == "__main__":
    app = VyrnIDE()
    app.mainloop()