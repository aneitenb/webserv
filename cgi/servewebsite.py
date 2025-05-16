#!/usr/bin/env python3

# CGI in Python
#“Dynamic content” just means: The response is generated programmatically at request time

#needs to:
#   accept request data 
#       if POST -> via stdin
#       if GET -> via QUERY_STRING
#   read the environment variables for request metadata 


#   (method, content type, etc.)
#   write the http response into stdout

#get the libraries
import os #for the environmental variables
import sys #for reading from the stdin
import cgi #useful?

#os.environ is a mapping object that represents the env variables
#returns a dictionary with key:value elements
#syntax for returning a specific value:
# var = os.environ['VAR'] which throws a KeyError if the variable is missing which is unsafe
# var = os.environ.get("SOMETHING", "") which returns "" if the variable is missing

#Required HTTP header things
# this should come from the server: print("HTTP/1.1 200 OK")
def handle_get():



print("Content-Type: text/html") #new line yes or no

method = os.environ.get("REQUEST_METHOD", "GET")

if method == 'GET'
    query = os.environ.get("QUERY_STRING", "")
    response = f"<html><body><h1>GET query: {query}</h1></body></html>"

elif method == 'POST'
    length = int(os.environ.get("CONTENT_LENGTH", 0))
    body = sys.stdin.read(length)
    response = f"<html><body><h1>POST body: {body}</h1></body></html>"

print(f"Content-Length: {len(response)}")
print() #empty line to separate the headers from the body
print(response)

#print out the minimum requirements for the response