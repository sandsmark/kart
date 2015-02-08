#The Kartering (working title)

**tldr**; connect to the server at localhost over port 31337. You send a single
bitmapped number to select what to do (for example "9\n" to accelerate and turn
right), and receive a JSON status update everytime you do so. The first AI to
complete a set number of laps wins!

The number is used as a bitmap, so set a bit to do that action:

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

On connect you will receive a JSON object describing the map:
```JSON
{
    "tile_width": 128,
    "tile_height": 128,
    "tiles": [
        [ ".", ".", ".", ".", ".", ".", ".", "." ],
        [ ".", "/", "-", "-", "-", "`", ".", "." ],
        [ ".", "|", ".", ".", ".", "|", ".", "." ],
        [ ".", "|", ".", ".", ".", "|", ".", "." ],
        [ ".", "\\", "-", "-", "-", ",", ".", "." ],
        [ ".", ".", ".", ".", ".", ".", ".", "." ]
    ],
    "modifiers": [
        {
            "type": "mud",
            "x": 168,
            "y": 218,
            "width": 64,
            "height": 64
        },
        {
            "type": "booster",
            "x": 418,
            "y": 538,
            "width": 32,
            "height": 32
        },
        {
            "type": "ice",
            "x": 638,
            "y": 188,
            "width": 64,
            "height": 64
        }
    ],
    "path": [
        { "tile_x": 3, "tile_y": 1 },
        { "tile_x": 4, "tile_y": 1 },
        { "tile_x": 5, "tile_y": 1 },
        { "tile_x": 5, "tile_y": 2 },
        { "tile_x": 5, "tile_y": 3 },
        { "tile_x": 5, "tile_y": 4 },
        { "tile_x": 4, "tile_y": 4 },
        { "tile_x": 3, "tile_y": 4 },
        { "tile_x": 2, "tile_y": 4 },
        { "tile_x": 1, "tile_y": 4 },
        { "tile_x": 1, "tile_y": 3 },
        { "tile_x": 1, "tile_y": 2 }
    ]
}
```

It should be self-explanatory (otherwise please file an issue here on
github or contact us otherwise so we can explain better here).

Then, each time you send a command (invalid or valid), you will get back a
status update JSON object like this:
```JSON
{
    "cars": [
        {
            "id": 0,
            "direction": {
                "x": 1,
                "y": 0
            },
            "velocity": {
                "x": 0,
                "y": 0
            },
            "pos": {
                "x": 448,
                "y": 153
            },
            "drift": 0,
            "width": 30,
            "height": 16
        }
    ],
    "shells": [
        {
            "type": "blue",
            "x": 342,
            "y": 233,
            "dx": 0.5,
            "dy": 0.3
        }
    ],
    "boxes": [
        { "x": 576, "y": 153, "width": 16, "height": 16 },
        { "x": 576, "y": 174, "width": 16, "height": 16 },
        { "x": 576, "y": 195, "width": 16, "height": 16 },
        { "x": 576, "y": 216, "width": 16, "height": 16 },
        { "x": 448, "y": 537, "width": 16, "height": 16 },
        { "x": 448, "y": 558, "width": 16, "height": 16 },
        { "x": 448, "y": 579, "width": 16, "height": 16 },
        { "x": 448, "y": 600, "width": 16, "height": 16 }
    ]
}
```


**NOTE: the actual JSON you receive is minified, and each initial or status update is a single
object on a single line, terminated by a newline:**

```JSON
{"cars":[{"id":0,"direction":{"x":1,"y":0},"velocity":{"x":0,"y":0},"pos":{"x":448,"y":153},"drift":0,"width":30,"height":16}],"shells":[]}
```

*All messages back and forth are terminated by a newline.*


### Build instructions
Install the SDL2 development headers, and run "make".

### CREDITS:
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
