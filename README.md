# README

My C++ Vulkan playground, based on Sascha Willems work (https://github.com/SaschaWillems/Vulkan).

## Examples

### instancing-229

<img src="./results/instancing-229/Zrzut ekranu z 2017-06-04 18-22-43 - playing with number of rocks, planet and rock size, rings thickness.png" height="240px" align="right">

<img src="./results/instancing-229/Zrzut ekranu z 2017-06-19 22-11-05 - made point light instead of light from camera, some fake AO, matricies behave differently.png" height="240px" align="right">

<img src="./results/instancing-229/Zrzut ekranu z 2017-06-20 23-14-34 - added planet shadow.png" height="240px" align="right">

<img src="./results/instancing-229/Zrzut ekranu z 2017-08-16 15-03-31 - more rings, animated light (simple gravity sim on CPU) with object.png" height="240px" align="right">

<img src="./results/instancing-229/Zrzut ekranu z 2017-08-16 23-13-55 - MSAA WITH SAMPLE SHADING.png" height="240px" align="right">

Based on *instancing* example

* changed rocks number
* changed rocks size, planet size
* changed rings thickness
* made point light instead of light from the camera
* reorganized matricies (real camera pos, no multiplication by view in vert shader for vectors computation)
* disabled starfield
* fake AO in frag shader
* IN PROGRESS: enable gravitational interactions computed in real time, but it should still make rings
* TODO: include some other models
* TODO: change planet model + texture to some real planet
* TODO: camera orbiting the planet on elliptical orbit? (like Juno)
* IN PROGRESS: rocks and planet should cast shadow on the planet and other rocks (this could be very computationally expensive)
* IN PROGRESS: enabled multisampling (with per-sample shading) (BUG: text overlay not working as of this moment)
