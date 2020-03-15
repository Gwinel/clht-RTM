#!/bin/sh

threads="1 2 4 8 16"
#percent is the update rate
percents="10 20 50 90"
#range=32768
#initial=10000000

for thread in $threads; do
for percent in $percents;do
#    line="$thread"
    ./clht_lb_res -n "$thread" -u "$percent" -i 100000000 -d 5000 >>./data/"$thread"."$percent".lb.csv 
done;done;
