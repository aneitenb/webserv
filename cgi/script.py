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
from urllib.parse import parse_qs
import json
# import cgi #useful?
# import cgitb # delete after i think
from datetime import datetime, timezone


# cgitb.enable() #comment out later

NEWLINE = "\r\n"
BNEWLINE = b"\r\n" #for the binary stream

#os.environ is a mapping object that represents the env variables
#returns a dictionary with key:value elements
#syntax for returning a specific value:
# var = os.environ['VAR'] which throws a KeyError if the variable is missing which is unsafe
# var = os.environ.get("SOMETHING", "") which returns "" if the variable is missing

#Required HTTP header things
# this should come from the server: print("HTTP/1.1 200 OK")

def build_response_binary(response, status_code="200 OK", content_type="application/octet-stream"):
    out = sys.stdout.buffer #the binary mode stdout
    #headers in ascii
    out.write(f"HTTP/1.1 {status_code}".encode("ascii") + BNEWLINE)
    out.write(f"Server: Hardly_know_er/1.0".encode("ascii") + BNEWLINE)
    now = datetime.now(timezone.utc)
    date_h = now.strftime("%a, %d %b %Y %H:%M:%S GMT")
    out.write(f"Date: {date_h}".encode("ascii") + BNEWLINE)
    out.write(f"Content-Type: {content_type}".encode("ascii") + BNEWLINE)
    out.write(f"Content-Length: {len(response)}".encode("ascii") + BNEWLINE)
    out.write(BNEWLINE)
    out.write(response)
    out.write(BNEWLINE + BNEWLINE) #check
    out.flush()

def build_response(method, response, status_code="200 OK", content_type="text/html"):
    #CGI headers
    print(f"HTTP/1.1 {status_code}", end=NEWLINE)
    print(f"Server: Hardly_know_er/1.0", end=NEWLINE)
    now = datetime.now(timezone.utc)
    print(f"Date: {now.strftime('%a, %d %b %Y %H:%M:%S')} GMT", end=NEWLINE)
    print(f"Content-Type: {content_type}", end=NEWLINE)
    print(f"Content-Length: {len(response)}", end=NEWLINE)
    print(NEWLINE, end="") #empty line
    print(response, end="") #response ie body

def handle_get():
    query = os.environ.get("QUERY_STRING", "")
    response = (
        "<!DOCTYPE html>" + NEWLINE +
        "<html lang=\"EN\">" + NEWLINE +
        "<head><title>GREETINGS FROM THE SERVER!</title></head>" + NEWLINE +
        "<body>" + NEWLINE +
        f"  <h1>{query}?</h1>" + NEWLINE +
        "       <p>HARDLY KNOW 'ER!</p>" + NEWLINE +
        "</body>" + NEWLINE +
        "</html>" + NEWLINE + NEWLINE
    )
    return response


def handle_post():
    ctype = os.environ.get("CONTENT_TYPE", "text/plain")
    try:
        clength = int(os.environ.get("CONTENT_LENGTH", "0"))
    except ValueError:
        clength = 0

    #sys.stdin is a text-mode file handle, as in, you get strings back
    #but .buffer attribute gives a binary stream, so gives back a bytes object instead of string
    data = sys.stdin.buffer.read(clength) if clength else b"" #if clength is 0, give empty byte-string
    name = email = "unknown"

    #have to use .decode because everything used expects a string not bytes
    #default is decode("utf-8")
    if ctype == 'application/x-www-form-urlencoded':
        body = data.decode('utf-8')
        print(repr(body))
        params = parse_qs(data.decode())
        name = params.get("name", [""])[0] #since the value is a list of strings the [0] returns the first and usually only item in the list
        email = params.get("email", [""])[0]

    elif ctype == 'application/json':
        params = json.loads(data.decode() or "{}")
        name = params.get("name", "unknown")
        email = params.get("email", "unknown")

    #bytes need to be handled differently since print expects text
    elif ctype == 'application/octet-stream':
        return data

    else:
        for line in data.decode().splitlines():
            if '=' in line:
                key, value = line.split('=', 1)
                if key == "name": name = value
                if key == "email": email = value
                # data[key] = value
        # name = data.get("name", "unknown")
        # email = data.get("email", "unknown")
    response = (
        "<!DOCTYPE html>" + NEWLINE +
        "<html lang=\"EN\">" + NEWLINE +
        "<head><title>HEHE!</title></head>" + NEWLINE +
        "<body>" + NEWLINE +
        f"  <h1>Thanks for the email, {name}.</h1>" + NEWLINE +
        f"       <p>Spam incoming to email address: {email}</p>" + NEWLINE +
        "</body>" + NEWLINE +
        "</html>" + NEWLINE + NEWLINE
    )
    return response

#main block

if __name__ == "__main__":
    try:
        method = os.environ.get("REQUEST_METHOD", "GET")

        if method == "POST":
            response = handle_post()
        else:
            response = handle_get()

        if isinstance(response, bytes):
            build_response_binary(response)
        else:
            build_response(method, response)
    except Exception:
        #send 500 error
        response = (
            "<!DOCTYPE html>" + NEWLINE +
            "<html lang=\"EN\">" + NEWLINE +
            "<head><title>500 Internal Server Error</title></head>" + NEWLINE +
            "<body><p>Something went wrong.</p></body>" + NEWLINE +
            "</html>" + NEWLINE + NEWLINE
        )
        build_response("GET", response, status_code="500 Internal Server Error")

