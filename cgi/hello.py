#!/usr/bin/env python3

import os
import sys
import datetime

method = os.environ.get("REQUEST_METHOD", "GET")
today = datetime.datetime.today()
newline = ord('\r\n')

if method == 'GET'
    query = os.environ.get("QUERY_STRING", "")
    response = f"<html>{newline}<body>{newline}<h1>{newline}Hello, {query}!{newline}Time of your visit: {today:%B %d, %Y}{newline}</h1>{newline}</body>{newline}</html>{newline}{newline}"

elif method == 'POST'
    length = int(os.environ.get("CONTENT_LENGTH", 0))
    body = sys.stdin.read(length)
    response = f"<html><body><h1>Hello, {body}!{newline}Time of your visit: {today:%B %d, %Y}</h1></body></html>"

# print(f"Content-Length: {len(response)}{newline}")
print() #empty line to separate the headers from the body
print(response)