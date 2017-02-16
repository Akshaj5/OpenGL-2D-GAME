### Abstract
A laser and bricks game where the objective is to score as many points as possible by breaking black bricks and collecting red and green bricks. Destroying black bricks gives you points and if you collect a black brick or destroy 10 red or green bricks then game will end.There are mirrors in between to reflect the laser beam which can be used to shoot at critical angles.

###Controls:

#Mouse:
* Scroll up/down to zoom in and zoom out
* Right click and drag to pan the scene
* Left click anywhere on the screen to get the laser ready, and release to launch a beam.
* Click on laser,red container or greencontainer and drag to move.

#Keyboard:
* 'Q' to quit the game
* 'UP' to zoom in
* 'DOWN' to zoom out
* 'RIGHT' to pan right
* 'LEFT' to pan left
* 'N' to increase up bricks speed
* 'M' to decrease bricks speed
* 'SPACE' to launch the laser
* 'S' to move up the laser
* 'F' to move down the laser
* 'A' to increase the launch angle
* 'D' to decrease the launch angle
* alt + left to move green box left
* alt + right to move green box right
* ctrl + left to move red box left
* ctrl + right to move red box right

##About the game:

* Get a score as high as possible.
* There are bricks of red,green and black color.
* Collect red and green bricks in respective color boxes to get 2 points each.
* Hit black brick with laser to get 3 points.
* Breaking wrong bricks will give -2 points.
* Collecting black brick will end the game.
* Also,if 10 or more non-black bricks are destroyed then game is over.


##Features:

* No images used anywhere in the game. Everything is create by using shapes in openGL. This ensures the loading of the game is quick and efficient.
* Rendered text/numbers without the help of any libraries (only using shapes).
* Collision using boxes(not circles), this is a lot more effective when blocks are of uneven size.


##Note:

All objects are sorted into different layers. Each layer is drawn one at a time. Some layers are more prefered and will be drawn last  whereas others will be drawn earlier (Like the background layer). Within a layer objects are drawn in a lexicographical order. These two together give you the ability to draw complex objects with ease.

### Dependencies:
##### Linux/Windows/ Mac OSX - Dependencies: (Recommended)
* GLFW
* GLAD
* GLM

##### Linux - Dependencies: (alternative)
* FreeGLUT
* GLEW
* GLM
