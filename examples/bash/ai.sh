#!/bin/bash
set -e

exec 3<>/dev/tcp/localhost/31337

echo -en "BashBot" >&3

while true
do
    let i=RANDOM%5 
    echo -e "$((1<<$i))" >&3
done
