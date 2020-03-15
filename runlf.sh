#!/bin/sh

threads="1 2 4 8 16 32"
#percent is the update rate
percents="10 20 50 90"
#range=32768
#initial=10000000

for thread in $threads; do
for percent in $percents;do
#    line="$thread"
    ./clht_lf_res -n "$thread" -u "$percent" -i 10000000 -d 1000>>./data/"$thread"."$percent".lb.csv 
done;done;
