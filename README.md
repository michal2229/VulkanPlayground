# README

My Vulkan API playground, written in C++, based on [Sascha Willems's work](https://github.com/SaschaWillems/Vulkan).
<br>
<br>
I like tinkering with programming and 3D graphics, so I chose to learn Vulkan, and this repository is intended to help me with it. 
Probably I wouldn't dare to start doing this without examples and tools written by Sascha Willems, also articles/twitter threads from Stephanie Hurlburt helped me a lot.
I am not an artist, so please pardon lack of proper scene design, etc.
<br>
I try to keep track of my progress, make goals and tasks, mark bugs etc., what can be seen under Issues/Milestones tab.
<br>
More info in individual example's folder's readme, on other branches and on wiki page. 
<br>
There is a [separate branch](https://github.com/michal2229/VulkanPlayground/tree/resources_branch) dedicated to working on assets and other resources.
<br>
Any suggestions and comments are very welcome!

### My examples
* [my static baked scene](src/my_new_scene1) - static scene with baked shadows, indirect lighting, reflections, ambient occlusion and normal maps
* [instancing-229](src/instancing-229) - based on [instancing](https://github.com/SaschaWillems/Vulkan/tree/master/instancing) example by Sascha Willems

## Info about Vulkan API

### Info from [Khronos](https://www.khronos.org/vulkan/)

> Vulkan is a new generation graphics and compute API that provides high-efficiency, cross-platform access to modern GPUs used in a wide variety of devices from PCs and consoles to mobile phones and embedded platforms.

### Info from [Wikipedia](https://en.wikipedia.org/wiki/Vulkan_(API))

> Vulkan is a low-overhead, cross-platform 3D graphics and compute API. Vulkan targets high-performance realtime 3D graphics applications such as video games and interactive media across all platforms. Compared with OpenGL and Direct3D 11 and like Direct3D 12 and Mantle, Vulkan is intended to offer higher performance and more balanced CPU/GPU usage. Other major differences from Direct3D 11 (and prior) and OpenGL are Vulkan being a considerably lower level API and offering parallel tasking. Vulkan also has the ability to render 2D graphics applications, however it is generally suited for 3D. In addition to its lower CPU usage, Vulkan is also able to better distribute work amongst multiple CPU cores.
> 
> Vulkan was first announced by the Khronos Group at GDC 2015. The Vulkan API was initially referred to as the "next generation OpenGL initiative," or "OpenGL next" by Khronos, but use of those names was discontinued once the Vulkan name was announced. Vulkan is derived from and built upon components of AMD's Mantle API, which was donated by AMD to Khronos with the intent of giving Khronos a foundation on which to begin developing a low-level API that they could standardize across the industry, much like OpenGL.

### Info from [Nvidia](https://developer.nvidia.com/Vulkan)

> Vulkan is a modern cross-platform graphics and compute API currently in development by the Khronos consortium. The Khronos members span the computing industry and are jointly creating an explicit and predictable API that satisfies the needs of software vendors in fields as varied as game, mobile and workstation development. Vulkan's conscious API design enables efficient implementations on platforms that span a wide range of mobile and desktop hardware as well as across operating systems.

### Info from [AMD](http://www.amd.com/en-us/innovations/software-technologies/technologies-gaming/vulkan)

> Vulkan™ supports close-to-metal control enabling faster performance and better image quality across Windows® 7, Windows® 8.1, Windows® 10, and Linux®. No other graphics API offers the same powerful combination of OS compatibility, rendering features, and hardware efficiency.

> Developed by the Khronos Group, the same consortium that developed OpenGL®, Vulkan™ is a descendant of AMD’s Mantle, inheriting a powerful low-overhead architecture that gives software developers complete access to the performance, efficiency, and capabilities of Radeon™ GPUs and multi-core CPUs.

> Compared to OpenGL, Vulkan™ substantially reduces “API overhead” – the background work a CPU does to interpret what a game asks of the hardware – to deliver meaningful features, performance, and image quality and expose GPU hardware features that wouldn’t ordinarily be accessible through OpenGL.

### Info from [Intel](https://software.intel.com/en-us/articles/api-without-secrets-introduction-to-vulkan-preface)

> Vulkan is seen as an OpenGL’s successor. It is a multiplatform API that allows developers to prepare high-performance graphics applications likes games, CAD tools, benchmarks, and so forth. It can be used on different operating systems like Windows*, Linux*, or Android*. The Khronos consortium created and maintains Vulkan. Vulkan also shares some other similarities with OpenGL, including graphics pipeline stages, GLSL shaders (sort of) or nomenclature.
> 
> (...)
> 
> Vulkan was based on Mantle*—the first in a series of new low-level graphics APIs. Mantle was developed by AMD and designed only for the architecture of Radeon cards. And despite it being the first publicly available API, games and benchmarks that used Mantle saw some impressive performance gains. Then other low-level APIs started appearing, such as Microsoft’s DirectX* 12, Apple’s Metal* and now Vulkan.
> 
> (...)
> 
> In low-level APIs the developer is the one who must take care of most things. They are required to adhere to strict programming and usage rules and also must write much more code. But this approach is reasonable. The developer knows what they want to do and what they want to achieve. The driver does not, so with traditional APIs the driver has to make additional effort for the program to work properly. With APIs like Vulkan this additional effort can be avoided. That’s why DirectX 12, Metal, or Vulkan are called thin-drivers/thin-APIs. Mostly they only communicate user requests to the hardware, providing only a thin abstraction layer of the hardware itself. The driver does as little as possible for the sake of much higher performance.
> 
> (...)

## Resources

* [Vulkan Overview - The Khronos Group](https://www.khronos.org/vulkan/)
* [Vulkan API Examples - Sascha Willems](https://github.com/SaschaWillems/Vulkan)
* [Vulkan tutorial](https://vulkan-tutorial.com/Introduction)
* [I Am Graphics And So Can You - Dustin H Land](https://www.fasterthan.life/blog/2017/7/11/i-am-graphics-and-so-can-you-part-1)
* [Understanding Vulkan Objects - Adam Sawicki](https://gpuopen.com/understanding-vulkan-objects/)
* [Beginner-Friendly Vulkan Tutorials - Stephanie Hurlburt](http://stephaniehurlburt.com/blog/2017/7/14/beginner-friendly-vulkan-tutorials)
* [API without Secrets: Introduction to Vulkan - Intel](https://software.intel.com/en-us/articles/api-without-secrets-introduction-to-vulkan-preface)
* [LunarG® Vulkan™ SDK](https://www.lunarg.com/vulkan-sdk/)
* [RenderDoc graphics debugger](https://renderdoc.org/)
