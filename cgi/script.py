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
import urllib.parse
import json
import cgi #useful?
import cgitb # delete after i think
from datetime import datetime, timezone




#os.environ is a mapping object that represents the env variables
#returns a dictionary with key:value elements
#syntax for returning a specific value:
# var = os.environ['VAR'] which throws a KeyError if the variable is missing which is unsafe
# var = os.environ.get("SOMETHING", "") which returns "" if the variable is missing

#Required HTTP header things
# this should come from the server: print("HTTP/1.1 200 OK")

def print_all(method, statuscode, date_h, contenttype, response)
    newline = ord('\r\n')

    print(f"HTTP1.1 {statuscode}{newline}")
    print(f"Server: Hardly_know_er/1.0{newline}")
    print(f"Date: {date_h} GMT{newline}")
    if method == POST:
        print(f"Content-Type: {contenttype}{newline}")
        print(f"Content-Length: {len(response)}{newline}")

    print({newline})
    print({response})

def handle_get():
    newline = ord('\r\n')
    query = os.environ.get("QUERY_STRING", "")
    response = f"<!DOCTYPE html>{newline}<html lang=""EN">{newline}<head><title>GREETINGS FROM THE SERVER!</title></head>{newline}	<body>{newline}     <h1>{query}?</h1>{newline}     <p>HARDLY KNOW 'ER!</p>{newline}	</body>{newline}</html>{newline}{newline}"
    return response


def handle_post():
    newline = ord('\r\n')
    ctype = os.environ.get("CONTENT_TYPE", "text/plain")
    clength = int(os.environ.get("CONTENT_LENGTH", "0"))

    if ctype == 'application/x-www-form-urlencoded':
        data = sys.stdin.read(clength)
        params = urllib.parse.parse_qs(data)
        name = params.get("name", [""])[0] #since the value is a list of strings the [0] returns the first and usually only item in the list
        email = params.get("email", [""])[0]
    elif ctype == 'application/json':
        data = sys.stdin.read(clength)
        params = json.loads(data)
        name = params.get("name", "unknown")
        email = params.get("email", "unknown@gmail.com")
    elif ctype == 'application/octet-stream':
        data = sys.stdin.buffer.read(clength)
        return data
    else:
        raw = sys.stdin.read(clength)
        data = {}
        for line in raw.strip().splitlines():
            if '=' in line:
                key, value = line.strip().split('='. 1)
                data[key] = value
        name = data.get("name", "unknown")
        email = data.get("email", "unknown")


    response = f"<!DOCTYPE html>{newline}<html lang=""EN">{newline}<head><title>HEHE!</title></head>{newline}	<body>{newline}     <h1>Thanks for the email, {name}.</h1>{newline}     <p>Spam incoming to email address: {email}</p>{newline}	</body>{newline}</html>{newline}{newline}"
    return response

#main block

if __name__ == "__main__":
    try:
        cgitb.enable() #enables debugging info in the browser, delete after?

        today = datetime.datetime.today()
        now = datetime.now(timzeone.utc) #get current utc time
        date_h = now.strftime("%a, %d %b %Y %H:%M:%S")

        method = os.environ.get("REQUEST_METHOD", "GET")

        if method == 'POST':
            response = handle_post()
        else:
            response = handle_get()

        statuscode = '200 OK'
        contenttype = text/html
    except:
        statuscode = '500 Internal Server Error'
        contenttype = 'text/html'
        response = f"<!DOCTYPE html>{newline}<html lang="EN">{newline}<head><title>500 Internal Server Error</title></head>{newline}<body>{newline}<p>Something went wrong.</p>{newline}</body>{newline}</html>{newline}{newline}"
    
    print_all(method, statuscode, date_h, contenttype, response)

