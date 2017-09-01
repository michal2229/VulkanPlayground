#!/bin/bash

# glslc way (from LunarSDK) - these spvs are somewhat bigger in size

for type in vert frag; do
    for i in $(ls -d *$type); do
        cmd="glslc $i -o $i.spv"
        printf "\n    >>> $cmd\n"
        eval $cmd
    done
done

# pyshaderc way (installable by python's PIP)
#../../../tools/compile_shaders_glsl_to_spv_here.py vert ./*vert
#../../../tools/compile_shaders_glsl_to_spv_here.py frag ./*frag
