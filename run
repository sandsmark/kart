#!/usr/bin/zsh
# or whereever you keep yours, when in doubt '$ where zsh'
#
# strictly licensed under the terms of WTFPL
# see http://www.wtfpl.net/txt/copying/

# in case of nuclear rain, dont leave them zombies
TRAPINT() {
    # do your dirty cleanup work here
    echo killing $KARTID
    kill $KARTID
    exit 0
}

# you get the idea
for map in TESTMAP_*.map ; do
    cp $map map1.map

    # unfortunately the port parameter isn't correctly used
    # by the server, i think i'll have to patch that too
    # sooner or later :)
    ./kart server 31337 >$map.log 2>&1 &
    export KARTID=$!

    # server is not fast, give it time to breath
    sleep 1

    # your command to launch goes here
    ../karthering/main.py &

    LAPS=3

    while [[ $(grep laptime $map.log | wc -l) -lt $LAPS ]]
    do
        sleep 1
    done

    # i'd recommend terminating peacefully when connection to
    # ./kart breaks, if you'd like to get a SIG to your process
    # before that happens - here'd be the place to do so :)
    kill $KARTID
done

