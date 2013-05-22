ECS 175: Computer Graphics
UC Davis, Spring 2013
Vase Mesh
Christopher Chen
tech@ucdavis.edu

Requirements:
GLUT
GLEW
OpenGL
C++
cmake 


Overview:
Draws a vase by revolving a Bezier curve around the y axis. Uses smooth shading
as well as texture mapping. The camera is a bit awkward, but it should be easy
to look inside the vase by zooming in far enough.


Commands to create makefile + compile + run:

cmake .
make
./vase.x

Controls:

Camera Zoom/Translation    W, S
Camera Rotation            Mouse Click and Drag
Quit                       ESC, Q
