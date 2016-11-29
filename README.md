# PTGame
A game with procedurally generated terrain.

This was a fun little project I've been playing with. You get to fly around in a seemingly infinate world of green bumpy terrain.

The terrain is implemented as a grid rectangle that follows you everywhere, where its height is calculated in the vertex shader. I'm using the noise function I found [here](https://github.com/hughsk/glsl-noise/blob/master/simplex/2d.glsl).
