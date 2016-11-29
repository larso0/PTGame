# PTGame
A game with procedurally generated terrain.

This was a fun little project I've been playing with. You get to fly around in a seemingly infinate world of green bumpy terrain.

The terrain is implemented as a grid rectangle that follows you everywhere, where its height is calculated in the vertex shader. I'm using the noise function I found [here](https://github.com/hughsk/glsl-noise/blob/master/simplex/2d.glsl).

To control the camera, use wasd to move forward, back and sideways. Use e and q to move up and down. Press escape to toggle mouse control to look around and f11 to toggle fullscreen.
