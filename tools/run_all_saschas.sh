#!/bin/bash

cd $(dirname $0)/../EngineSW/bin

for i in $(ls); do
    ./$i
done
