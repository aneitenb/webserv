#!/usr/bin/env python3
import os
import sys
import time

# Print the CGI header
print("Content-Type: text/html\r\n\r")

if os.environ.get("PATH_INFO") == "/timeout":
    while True:
        time.sleep(1)

# Generate a simple HTML page
print("<html><head><title>CGI Test</title></head><body>")
print("<h1>CGI Test Successful!</h1>")

# Print environment variables
print("<h2>Environment Variables:</h2>")
print("<ul>")
for key, value in os.environ.items():
    print(f"<li><strong>{key}</strong>: {value}</li>")
print("</ul>")

# If it's a POST request, read and display the input
if os.environ.get("REQUEST_METHOD") == "POST":
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))
    post_data = sys.stdin.read(content_length)
    
    print("<h2>POST Data:</h2>")
    print(f"<pre>{post_data}</pre>")

# Add back button
print("<p><a href=\"/index.html\">Back to Home</a></p>")

print("</body></html>")
