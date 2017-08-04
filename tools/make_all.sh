#!/bin/bash

cd $(dirname $0)/../build
cmake ..
make
