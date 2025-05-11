import os
import sys
import cgi
import cgitb
import json
import tkinter as tk
from tkinter import messagebox
import requests

cgitb.enable()

# Check if the script is being run as CGI or tkinter client
IS_CGI = 'GATEWAY_INTERFACE' in os.environ

if IS_CGI:
    # ---- CGI PART (Server) ----
    
    # Handle form submission
    def handle_submission():
        form = cgi.FieldStorage()
        name = form.getvalue('name', '')
        email = form.getvalue('email', '')

        # Return a JSON response
        print("Content-Type: application/json\r\n\r\n")
        print(json.dumps({
            "status": "success",
            "message": f"Received name: {name}, email: {email}"
        }))
    
    # Respond with the HTML form if no form data is received
    def handle_form():
        print("Content-Type: text/html\r\n\r\n")
        print("""
        <html>
<head>
    <script src="https://cdn.jsdelivr.net/npm/@tailwindcss/browser@4"></script>
    <title>Submit Your Information</title>
</head>
<body class="bg-gradient-to-br text-white flex from-black via-[#02274a] to-[#021636] h-[100vh] w-screen justify-center items-center">
    <main class="flex flex-col items-center gap-8 w-[500px]">
        <h1 class="text-3xl font-bold text-center">Enter Your Information</h1>

        <!-- The Form with Tailwind Styling -->
        <form method="POST" class="border rounded-3xl p-8 grid gap-4 w-full">
            <label for="name" class="text-white">Name:</label>
            <input type="text" id="name" name="name" class="bg-white/10 backdrop-blur-2xl rounded-3xl p-4 text-white w-full" required><br><br>
            
            <label for="email" class="text-white">Email:</label>
            <input type="text" id="email" name="email" class="bg-white/10 backdrop-blur-2xl rounded-3xl p-4 text-white w-full" required><br><br>
            
            <input type="submit" value="Submit" class="bg-white/10 backdrop-blur-2xl rounded-3xl p-4 cursor-pointer w-full">
        </form>
    </main>
</body>
</html>

        """)

    # Main handler based on request type (GET or POST)
    def main():
        if os.environ.get('REQUEST_METHOD') == 'POST':
            handle_submission()
        else:
            handle_form()

    main()

else:
    # ---- CLIENT PART (tkinter Form) ----

    # Function to send form data to server
    def submit_form():
        name = name_entry.get()
        email = email_entry.get()

        data = {
            "name": name,
            "email": email
        }

        try:
            # Replace this URL with your actual CGI script URL
            url = "http://localhost:9090/form.py"
            response = requests.post(url, data=data)
            if response.ok:
                result = response.json()
                messagebox.showinfo("Response", result.get("message", "No message returned."))
            else:
                messagebox.showerror("Error", f"Status code: {response.status_code}")
        except Exception as e:
            messagebox.showerror("Error", str(e))

    # Create the tkinter window and form
    root = tk.Tk()
    root.title("Submit to CGI Server")

    tk.Label(root, text="Name").grid(row=0, column=0)
    name_entry = tk.Entry(root)
    name_entry.grid(row=0, column=1)

    tk.Label(root, text="Email").grid(row=1, column=0)
    email_entry = tk.Entry(root)
    email_entry.grid(row=1, column=1)

    submit_btn = tk.Button(root, text="Submit", command=submit_form)
    submit_btn.grid(row=2, column=0, columnspan=2)

    root.mainloop()
