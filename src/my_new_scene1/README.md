## Example - my_new_baked_scene1

<img src="../../results/my_new_scene1/Zrzut ekranu z 2017-09-02 11-01-59.png" width="450px" align="right" vspace="4">

<img src="../../results/my_new_scene1/Zrzut ekranu z 2017-09-02 12-00-36.png" width="450px" align="right" vspace="4">

<img src="../../results/my_new_scene1/Zrzut ekranu z 2017-09-02 12-26-20.png" width="450px" align="right" vspace="4">

<img src="../../results/my_new_scene1/Zrzut ekranu z 2017-09-02 20-39-18.png" width="450px" align="right" vspace="4">

<img src="../../results/my_new_scene1/Zrzut ekranu z 2017-09-02 20-59-15.png" width="450px" align="right" vspace="4">

<img src="../../results/my_new_scene1/Zrzut ekranu z 2017-09-02 22-08-20.png" width="450px" align="right" vspace="4">

This scene is made of several objects with number of texture types assinged.
There is one common vertex and fragment shader and one common set of textures, which define object's look.
Object's UV coordinates assign part of texture to given object.
So far there are:

* color map,
* diffuse direct + indirect map,
* AO map,
* emit map,
* normal map (not used yet)
* reflection map (which is not UV'd).

Texture maps were baked in Blender + Cycles (low quality so far), models were also created in Blender.
