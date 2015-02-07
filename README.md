#The Kartering (working title)
==============================

tldr; connect to the server at localhost over port 31337. You receive game
state in JSON when you ask for it (send "1337\n"), send back a single byte to
control it.

The byte is used as a bitmap, so set a bit to do that action (press the key in
parenthesis in a local game).  This part of the protocol is going to be
simplified a bit (switch to sending specific bytes instead of bitmaps).

Action      | Player 1 | Player 2 | Bit in map |
------------|----------|----------|------------
Accelerate  | Up       | W        | 0
Decelerate  | Down     | S        | 1
Turn left   | Left     | A        | 2
Turn right  | Right    | D        | 3
Drift       | Comma    | C        | 4
Use powerup | Period   | V        | 5

Some pseudo C code to use it:
```C
    #define NET_INPUT_UP 1<<0
    #define NET_INPUT_DOWN 1<<1
    #define NET_INPUT_LEFT 1<<2
    #define NET_INPUT_RIGHT 1<<3
    #define NET_INPUT_SPACE 1<<4
    #define NET_INPUT_RETURN 1<<5

    char action = NET_INPUT_UP;
    if (want_to_turn_left) {
        action |= NET_INPUT_LEFT;
    }
    send_byte(action);
```

CREDITS:
track graphics from here: http://opengameart.org/content/track-tiles
It is licensed by CC-BY TRBRY


Some compatibility code for windows is licensed under the ISC license:
Copyright (c) 1996,1999 by Internet Software Consortium.

A header for windows compatibility is licensed under the GPLv2+:
 By Gerald Combs <gerald@wireshark.org>
 Copyright 1998 Gerald Combs


The rest of the code is under an MIT license:
Copyright (c) 2014, 2015 Lars-Ivar Hesselberg Simonsen
Copyright (c) 2014, 2015 Martin Tobias Holmedahl Sandsmark

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.




# Build instructions
====================
Install the SDL2 development headers, and run "make".
