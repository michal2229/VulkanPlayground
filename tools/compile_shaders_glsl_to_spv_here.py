#!/usr/bin/env python3

import pyshaderc
import sys

scriptname = str(sys.argv[0])
shadertype = str(sys.argv[1])  # vert, frag...
filelist   = [str(i) for i in sys.argv[2:]]

print("\nIn script:\n    {}".format(scriptname));
print("Computing shaders:\n    {}".format(filelist));
print("of type:\n    {}".format(shadertype))

for fil in filelist:
    print("\n-> Working on {}...".format(fil))
    spirv = pyshaderc.compile_file_into_spirv(fil, shadertype)
    
    with open("{}.spv".format(fil), "wb") as file:
        file.write(spirv)
    print("-> {} - done.".format(fil))
    
print("\nDone.\n")
