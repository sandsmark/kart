#!/bin/bash
exec 3<>/dev/tcp/localhost/31337

set -e
echo -en "BashBot" >&3

while true
do
    let i=RANDOM%5 
#    echo $((1<<$i))
    echo -e "$((1<<$i))" >&3
done
