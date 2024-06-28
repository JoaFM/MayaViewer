# MayaViewer

## !Warning old code from a naive young me SO tread lightly ! 
 This was made a long time ago and mostly for fun and quickly. So looking back its a bit of a mess. But maybe its helps some. Have fun

## What is it
Developed for Unreal Engine 4 and maya. The grand goal is that as a user you could have a unreal viewport for what you are doing in maya. This would allow for example in a games development environment a user to see there content with the right shaders and lighting, while not losing the ablility to make interactive edits.

This tool has essential 3 parts

* TCP/IP server. That Maya and unreal can connect to. The idea was also that you could have multiple clients ( For example 2 unreals lisning to 1 maya)
* Maya plugin: this tool would look at your maya scene and send the camera and mesh data to the server, that in turn would pass it to unreal.
* Unreal Plugin that would read the camera, mesh and material data and generate the content in Unreal. 
