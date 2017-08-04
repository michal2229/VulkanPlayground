#!/bin/bash

cd $(dirname $0)/../bin

for i in $(ls); do
    ./$i
done
