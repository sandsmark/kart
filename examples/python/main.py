#!/usr/bin/env python2
import json
import random
import socket


ip = "localhost"
port = 31337

movements = {"up": 1<<0,
             "down": 1<<1,
             "left": 1<<2,
             "right": 1<<3,
             "space": 1<<4,
             "return": 1<<5}


def write(sock, msg):
    sock.sendall(str(msg)+"\n")

def read(sock):
    try:
        msg = sock.makefile().readline()
        return msg
    except:
        pass

def main():
    sock = socket.socket()
    sock.connect((ip, port))

    write(sock, "PyBot")
    msg = read(sock)
    data = json.loads(msg)
    write(sock, 1<<5) # Tell the server we are ready

    sock.setblocking(0)
    while True:
        move = random.choice(movements.values())
        write(sock, move)
        msg = read(sock)


if __name__ == "__main__":
    main()

