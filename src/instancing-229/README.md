## Example - instancing-229

<img src="../../results/instancing-229/Zrzut ekranu z 2017-06-04 18-22-43 - playing with number of rocks, planet and rock size, rings thickness.png" width="450px" align="right" vspace="4">

<img src="../../results/instancing-229/Zrzut ekranu z 2017-06-19 22-11-05 - made point light instead of light from camera, some fake AO, matricies behave differently.png" width="450px" align="right" vspace="4">

<img src="../../results/instancing-229/Zrzut ekranu z 2017-06-20 23-14-34 - added planet shadow.png" width="450px" align="right" vspace="4">

<img src="../../results/instancing-229/Zrzut ekranu z 2017-08-16 15-03-31 - more rings, animated light (simple gravity sim on CPU) with object.png" width="450px" align="right" vspace="4">

<img src="../../results/instancing-229/Zrzut ekranu z 2017-08-17 19-58-34 - changed textures.png" width="450px" align="right" vspace="4">

<img src="../../results/instancing-229/Zrzut ekranu z 2017-08-18 00-57-38 - construct, soft shadows.png" width="450px" align="right" vspace="4">

Based on *instancing* example

* changed rocks number
* changed rocks size, planet size
* changed rings thickness
* made point light instead of light from the camera
* reorganized matricies (real camera pos, no multiplication by view in vert shader for vectors computation)
* disabled starfield
* IN PROGRESS: enable gravitational interactions computed in real time, but it should still make rings
* included cage model (as system;s boundary) and light model orbiting main planet
* changed planet model + texture
* TODO: camera orbiting the planet on elliptical orbit? (like Juno)
* IN PROGRESS: rocks and planet should cast shadow on the planet and other rocks (this could be very computationally expensive)
* TODO: enable multisampling
