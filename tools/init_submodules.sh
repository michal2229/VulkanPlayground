#!/bin/bash

## init submodule with Sascha Willems's repo
#git submodule add https://github.com/SaschaWillems/Vulkan.git "$(dirname $0)/../EngineSW"

cd "$(dirname $0)/../"
git submodule update --init --recursive

