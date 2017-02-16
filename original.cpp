#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct COLOR {
    float r;
    float g;
    float b;
};
typedef struct COLOR COLOR;


struct Sprite {
    string name;
    COLOR color;
    float x,y;
    VAO* object;
    int status;
    float height,width;
    float x_speed,y_speed;
    float angle; //Current Angle (Actual rotated angle of the object)
    int inAir;
    float radius;
    int fixed;
    float flag ; //Value from 0 to 1
    int health;
    int isRotating;
    int direction; //0 for clockwise and 1 for anticlockwise for animation
    float remAngle; //the remaining angle to finish animation
    int isMovingAnim;
    int dx;
    int dy;
    float weight;
};
typedef struct Sprite Sprite;


struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

map <string, Sprite> laser;
map <string, Sprite> bricks;
map <string, Sprite> containers;
map <string, Sprite> mirrors;
map <string, Sprite> background;
map <string, Sprite> point1;
map <string, Sprite> point2;
map <string, Sprite> point3;
map <string, Sprite> neg;
map <string, Sprite> endlabel;



int player_score=0;
float x_change = 0; //For the camera pan
float y_change = 0; //For the camera pan
float zoom_camera = 1;
float game_over=0;


/*pair<float,float> moveObject(string name, float dx, float dy) {
    objects[name].x+=dx;
    objects[name].y+=dy;
    return make_pair(objects[name].x,objects[name].y);
}*/

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;

double launch_angle=0;
int keyboard_pressed=0;

/* Executed when a regular key is pressed/released/held-down 
  Prefered for Keyboard events */
int key_pressed_S=0;
int key_pressed_F=0;
int key_pressed_left_arrow=0;
int key_pressed_right_arrow=0;
int key_pressed_right_alt=0;
int key_pressed_right_control=0;

void check_pan(){
    if(x_change-400.0f/zoom_camera<-400)
        x_change=-400+400.0f/zoom_camera;
    else if(x_change+400.0f/zoom_camera>400)
        x_change=400-400.0f/zoom_camera;
    if(y_change-300.0f/zoom_camera<-300)
        y_change=-300+300.0f/zoom_camera;
    else if(y_change+300.0f/zoom_camera>300)
        y_change=300-300.0f/zoom_camera;
}

void mousescroll(GLFWwindow* window, double xoffset, double yoffset)
{
    if (yoffset==-1) { 
        zoom_camera /= 1.1; //make it bigger than current size
    }
    else if(yoffset==1){
        zoom_camera *= 1.1; //make it bigger than current size
    }
    if (zoom_camera<=1) {
        zoom_camera = 1;
    }
    if (zoom_camera>=4) {
        zoom_camera=4;
    }
    if(x_change-400.0f/zoom_camera<-400)
        x_change=-400+400.0f/zoom_camera;
    else if(x_change+400.0f/zoom_camera>400)
        x_change=400-400.0f/zoom_camera;
    if(y_change-300.0f/zoom_camera<-300)
        y_change=-300+300.0f/zoom_camera;
    else if(y_change+300.0f/zoom_camera>300)
        y_change=300-300.0f/zoom_camera;
    Matrices.projection = glm::ortho((float)(-400.0f/zoom_camera+x_change), (float)(400.0f/zoom_camera+x_change), (float)(-300.0f/zoom_camera+y_change), (float)(300.0f/zoom_camera+y_change), 0.1f, 500.0f);
}

float downfall = 0.4;


void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods){
    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_N:
                downfall += 0.2;
                if(downfall>=0.8)
                  downfall=0.8;
                break;
            case GLFW_KEY_M:
                downfall -= 0.2;
                if(downfall<=0.2)
                  downfall=0.2;
                break;
            case GLFW_KEY_UP:
                mousescroll(window,0,+1);
                check_pan();
                break;
            case GLFW_KEY_DOWN:
                mousescroll(window,0,-1);
                check_pan();
                break;
            case GLFW_KEY_RIGHT:
                key_pressed_right_arrow = 0;
                x_change+=10;
                check_pan();
                break;
            case GLFW_KEY_LEFT:
                key_pressed_left_arrow = 0;
                x_change-=10;
                check_pan();
                break;
            case GLFW_KEY_S:
                key_pressed_S = 0;
                break;
            case GLFW_KEY_F:
                key_pressed_F = 0;
                break;
            case GLFW_KEY_RIGHT_ALT:
                key_pressed_right_alt=0;
                break;
            case GLFW_KEY_RIGHT_CONTROL:
                key_pressed_right_control = 0;
                break;
            case GLFW_KEY_A:
               // initKeyboard();
                if(launch_angle<90-10)
                    launch_angle+=10;
                else
                    launch_angle=90;
                break;
            case GLFW_KEY_D:
                //initKeyboard();
                if(launch_angle>-80)
                    launch_angle-=10;
                else
                    launch_angle=-90;
                break;
            case GLFW_KEY_SPACE:
                if(laser["beam"].status==0){
                  laser["beam"].x=laser["laser1"].x;
                  laser["beam"].y=laser["laser1"].y;
                  laser["beam"].x_speed=10*cos(launch_angle*(M_PI/180));
                  laser["beam"].y_speed=10*sin(launch_angle*(M_PI/180));
                  laser["beam"].angle=launch_angle;
                  laser["beam"].status=1;
                }
                break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_S:
                key_pressed_S = 1;
                break;
            case GLFW_KEY_F:
                key_pressed_F = 1;
                break;
            case GLFW_KEY_RIGHT_ALT:
                key_pressed_right_alt=1;
                break;
            case GLFW_KEY_RIGHT_CONTROL:
                key_pressed_right_control = 1;
                break;
            case GLFW_KEY_RIGHT:
                key_pressed_right_arrow = 1;
                break;
            case GLFW_KEY_LEFT:
                key_pressed_left_arrow = 1;
                break;    
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}
int mouse_clicked=0;
int right_mouse_clicked=0;

void mouse_click(){
    mouse_clicked=1;
    keyboard_pressed=0;
}
int flag=0; 

int red =0;
int green =0;
/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_PRESS) {
                mouse_click();
            }
            if (action == GLFW_RELEASE) {
              mouse_clicked=0;
              red=0;
              green=0;
              if(laser["beam"].status==0 && flag ==1){
                  laser["beam"].x=laser["laser1"].x;
                  laser["beam"].y=laser["laser1"].y;
                  laser["beam"].x_speed=10*cos(launch_angle*(M_PI/180));
                  laser["beam"].y_speed=10*sin(launch_angle*(M_PI/180));
                  laser["beam"].angle=launch_angle;
                  laser["beam"].status=1;
                }
            }
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_PRESS) {
                right_mouse_clicked=1;
            }
            if (action == GLFW_RELEASE) {
                right_mouse_clicked=0;
            }
            break;
        default:
            break;
    }
}



/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    Matrices.projection = glm::ortho(-400.0f, 400.0f, -300.0f, 300.0f, 0.1f, 500.0f);
}

VAO *triangle, *rectangle;

// Creates the triangle object used in this sample code

void createTriangle1(string name, float weight, COLOR color, float x[], float y[], string component, int fill)
{
    /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

    /* Define vertex array as used in glBegin (GL_TRIANGLES) */
    float xc=(x[0]+x[1]+x[2])/3;
    float yc=(y[0]+y[1]+y[2])/3;
    GLfloat vertex_buffer_data [] = {
        x[0]-xc,y[0]-yc,0, // vertex 0
        x[1]-xc,y[1]-yc,0, // vertex 1
        x[2]-xc,y[2]-yc,0 // vertex 2
    };

    GLfloat color_buffer_data [] = {
        color.r,color.g,color.b, // color 1
        color.r,color.g,color.b, // color 2
        color.r,color.g,color.b // color 3
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    VAO *triangle;
    if(fill==1)
        triangle=create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
    else
        triangle=create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
    Sprite vishsprite = {};
    vishsprite.color = color;
    vishsprite.name = name;
    vishsprite.object = triangle;
    vishsprite.x=(x[0]+x[1]+x[2])/3; //Position of the sprite is the position of the centroid
    vishsprite.y=(y[0]+y[1]+y[2])/3;
    vishsprite.height=-1; //Height of the sprite is undefined
    vishsprite.width=-1; //Width of the sprite is undefined
    vishsprite.status=1;
    vishsprite.inAir=0;
    vishsprite.x_speed=0;
    vishsprite.y_speed=0;
    vishsprite.radius=-1; //The bounding circle radius is not defined.
    vishsprite.fixed=0;
    vishsprite.health=100;
    vishsprite.weight=weight;

    if(component=="laser")
        laser[name]=vishsprite;
    else if(component=="background");
        background[name]=vishsprite;
}

// Creates the rectangle object used in this sample code


void createRectangle1(string name, float weight, COLOR colorA, COLOR colorB, COLOR colorC, COLOR colorD, float x, float y, float height, float width, string component)
{
    // GL3 accepts only Triangles. Quads are not supported
    float w=width/2,h=height/2;
    GLfloat vertex_buffer_data [] = {
        -w,-h,0, // vertex 1
        -w,h,0, // vertex 2
        w,h,0, // vertex 3

        w,h,0, // vertex 3
        w,-h,0, // vertex 4
        -w,-h,0  // vertex 1
    };

    GLfloat color_buffer_data [] = {
        colorA.r,colorA.g,colorA.b, // color 1
        colorB.r,colorB.g,colorB.b, // color 2
        colorC.r,colorC.g,colorC.b, // color 3

        colorC.r,colorC.g,colorC.b, // color 4
        colorD.r,colorD.g,colorD.b, // color 5
        colorA.r,colorA.g,colorA.b // color 6
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    VAO *rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
    Sprite vishsprite = {};
    vishsprite.color = colorA;
    vishsprite.name = name;
    vishsprite.object = rectangle;
    vishsprite.x=x;
    vishsprite.y=y;
    vishsprite.height=height;
    vishsprite.width=width;
    vishsprite.status=1;
    vishsprite.inAir=0;
    vishsprite.x_speed=0;
    vishsprite.y_speed=0;
    vishsprite.fixed=0;
    vishsprite.angle=launch_angle;
    vishsprite.radius=(sqrt(height*height+width*width))/2;
    vishsprite.flag=0;
    vishsprite.health=100;

    if(component=="bricks")
        bricks[name]=vishsprite;
    else if(component=="containers")
        containers[name]=vishsprite;
    else if(component=="mirror")
        mirrors[name]=vishsprite;
    else if(component=="laser")
        laser[name]=vishsprite;
    else if(component=="mirrors")
        mirrors[name]=vishsprite;
    else if(component=="point1")
        point1[name]=vishsprite;
    else if(component=="point2")
        point2[name]=vishsprite;
    else if(component=="point3")
        point3[name]=vishsprite;
    else if(component=="neg")
        neg[name]=vishsprite;
    else if(component=="end")
        endlabel[name]=vishsprite;
    }

void createCircle (string name, float weight, COLOR color, float x, float y, float r, int NoOfParts, string component, int fill)
{
    int parts = NoOfParts;
    float radius = r;
    GLfloat vertex_buffer_data[parts*9];
    GLfloat color_buffer_data[parts*9];
    int i,j;
    float angle=(2*M_PI/parts);
    float current_angle = 0;
    for(i=0;i<parts;i++){
        for(j=0;j<3;j++){
            color_buffer_data[i*9+j*3]=color.r;
            color_buffer_data[i*9+j*3+1]=color.g;
            color_buffer_data[i*9+j*3+2]=color.b;
        }
        vertex_buffer_data[i*9]=0;
        vertex_buffer_data[i*9+1]=0;
        vertex_buffer_data[i*9+2]=0;
        vertex_buffer_data[i*9+3]=radius*cos(current_angle);
        vertex_buffer_data[i*9+4]=radius*sin(current_angle);
        vertex_buffer_data[i*9+5]=0;
        vertex_buffer_data[i*9+6]=radius*cos(current_angle+angle);
        vertex_buffer_data[i*9+7]=radius*sin(current_angle+angle);
        vertex_buffer_data[i*9+8]=0;
        current_angle+=angle;
    }
    VAO* circle;
    if(fill==1)
        circle = create3DObject(GL_TRIANGLES, (parts*9)/3, vertex_buffer_data, color_buffer_data, GL_FILL);
    else
        circle = create3DObject(GL_TRIANGLES, (parts*9)/3, vertex_buffer_data, color_buffer_data, GL_LINE);
    Sprite vishsprite = {};
    vishsprite.color = color;
    vishsprite.name = name;
    vishsprite.object = circle;
    vishsprite.x=x;
    vishsprite.y=y;
    vishsprite.height=2*r; //Height of the sprite is 2*r
    vishsprite.width=2*r; //Width of the sprite is 2*r
    vishsprite.status=1;
    vishsprite.inAir=0;
    vishsprite.x_speed=0;
    vishsprite.y_speed=0;
    vishsprite.radius=r;
    vishsprite.fixed=0;
    vishsprite.health=100;
    vishsprite.weight=weight;
    if(component=="laser")
        laser[name]=vishsprite;
}



float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;


int checkCollisionRight(Sprite col_object, Sprite my_object){
    if(col_object.x>my_object.x && col_object.y+col_object.height/2>my_object.y-my_object.height/2 && col_object.y-col_object.height/2<my_object.y+my_object.height/2 && col_object.x-col_object.width/2<my_object.x+my_object.width/2 && col_object.x+col_object.width/2>my_object.x-my_object.width/2){
        return 1;
    }
    return 0;
}

int checkCollisionLeft(Sprite col_object, Sprite my_object){
    if(col_object.x<my_object.x && col_object.y+col_object.height/2>my_object.y-my_object.height/2 && col_object.y-col_object.height/2<my_object.y+my_object.height/2 && col_object.x+col_object.width/2>my_object.x-my_object.width/2 && col_object.x-col_object.width/2<my_object.x+my_object.width/2){
        return 1;
    }
    return 0;
}

int checkCollisionTop(Sprite col_object, Sprite my_object){
    if(col_object.y>my_object.y && col_object.x+col_object.width/2>my_object.x-my_object.width/2 && col_object.x-col_object.width/2<my_object.x+my_object.width/2 && col_object.y-col_object.height/2<my_object.y+my_object.height/2 && col_object.y+col_object.height/2>my_object.y-my_object.height/2){
        return 1;
    }
    return 0;
}

int checkCollisionBottom(Sprite col_object, Sprite my_object){
    if(col_object.y<my_object.y && col_object.x+col_object.width/2>my_object.x-my_object.width/2 && col_object.x-col_object.width/2<my_object.x+my_object.width/2 && col_object.y+col_object.height/2>my_object.y-my_object.height/2 && col_object.y-col_object.height/2<my_object.y+my_object.height/2){
        return 1;
    }
    return 0;
}
int checkBrickCollision(Sprite col_object, Sprite my_object){
    if(col_object.y<my_object.y && col_object.x+col_object.width/2>=my_object.x+my_object.width/2 && col_object.x-col_object.width/2<=my_object.x-my_object.width/2 && 0<=((col_object.y+col_object.height/2)-(my_object.y-my_object.height/2)) && ((col_object.y+col_object.height/2)-(my_object.y-my_object.height/2))<=1){
        return 1;
    }
    return 0;
}

/* Render the scene with openGL */
/* Edit this function according to your assignment */
double mouse_pos_x, mouse_pos_y;
double new_mouse_pos_x, new_mouse_pos_y;
float angle=0;
float xpos;
float ypos = 2720;
int num = 1;
int score =0;
int gameover=0;
int count=0;


void draw (GLFWwindow* window)
{

  if(count>=10)
    gameover=1;


  glClearColor(1.0f,1.0f,1.0f,1.0f);//set background color
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram (programID);
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  glm::vec3 target (0, 0, 0);
  glm::vec3 up (0, 1, 0);

  


  if(key_pressed_S==1){
    laser["laser1"].y+=1;
    laser["laser2"].y+=1;
    laser["laser3"].y+=1;
    laser["laser4"].y+=1;
    laser["laser5"].y+=1;
    if(laser["laser1"].y>=260){
      laser["laser1"].y=260;
      laser["laser2"].y=260;
      laser["laser3"].y=260;
      laser["laser4"].y=260;
      laser["laser5"].y=260;
    }
  }

  if(key_pressed_F==1){
    laser["laser1"].y-=1;
    laser["laser2"].y-=1;
    laser["laser3"].y-=1;
    laser["laser4"].y-=1;
    laser["laser5"].y-=1;
    if(laser["laser1"].y<=-260){
      laser["laser1"].y=-260;
      laser["laser2"].y=-260;
      laser["laser3"].y=-260;
      laser["laser4"].y=-260;
      laser["laser5"].y=-260;
    }
  }

  if(key_pressed_right_control==1 && key_pressed_left_arrow==1){
    containers["redcontainer"].x-=4;
    containers["zarkredcontainer"].x-=4;
    if(containers["redcontainer"].x<=-330){
      containers["redcontainer"].x=-330;
      containers["zarkredcontainer"].x=-330;
    }
  }

  if(key_pressed_right_control==1 && key_pressed_right_arrow==1){
    containers["redcontainer"].x+=4;
    containers["zarkredcontainer"].x+=4;
    if(containers["redcontainer"].x>=330){
      containers["redcontainer"].x=330;
      containers["zarkredcontainer"].x=330;
    }
  }

  if(key_pressed_right_alt==1 && key_pressed_left_arrow==1){
    containers["greencontainer"].x-=4;
    containers["zarkgreencontainer"].x-=4;
    if(containers["greencontainer"].x<=-330){
      containers["greencontainer"].x=-330;
      containers["zarkgreencontainer"].x=-330;
    }
  }

  if(key_pressed_right_alt==1 && key_pressed_right_arrow==1){
    containers["greencontainer"].x+=4;
    containers["zarkgreencontainer"].x+=4;
    if(containers["greencontainer"].x>=330){
      containers["greencontainer"].x=330;
      containers["zarkgreencontainer"].x=330;
    }
  }

  for(map<string,Sprite>::iterator it1=bricks.begin();it1!=bricks.end();it1++){
    string current = it1->first;
    if(checkBrickCollision(containers["redcontainer"],bricks[current]) && checkBrickCollision(containers["greencontainer"],bricks[current]))
      if(current[0]=='b')
        gameover=1;
      else{
        bricks[current].x=num*200;
          bricks[current].y=ypos;
          num++;
          ypos+=80;
          if(num==2)
            num-=3;
        continue;
      }
    if(checkBrickCollision(containers["redcontainer"],bricks[current])){
      bricks[current].x=num*200;
          bricks[current].y=ypos;
          num++;
          ypos+=80;
          if(num==2)
            num-=3;
      if(current[0]=='r')
        score+=2;
      else if(current[0]=='b')
        gameover=1;

      cout << score << endl;
    }
    if(checkBrickCollision(containers["greencontainer"],bricks[current])){
      bricks[current].x=num*200;
          bricks[current].y=ypos;
          num++;
          ypos+=80;
          if(num==2)
            num-=3;
     if(current[0]=='g')
        score+=2;
      else if(current[0]=='b')
        gameover=1; 
      
      cout << score << endl;
    }
    if(bricks[current].y<=-285){
      bricks[current].x=num*200;
          bricks[current].y=ypos;
          num++;
          ypos+=80;
          if(num==2)
            num-=3;
    }
  }

  for(map<string,Sprite>::iterator it1=point1.begin();it1!=point1.end();it1++){
    point1[it1->first].status=0;
  }
  for(map<string,Sprite>::iterator it1=point2.begin();it1!=point2.end();it1++){
    point2[it1->first].status=0;
  }
  for(map<string,Sprite>::iterator it1=point3.begin();it1!=point3.end();it1++){
    point3[it1->first].status=0;
  }

  int curr_score=score;

  if(curr_score<0)
    neg["neg"].status=1;
  else
    neg["neg"].status=0;

  int poi = abs(curr_score);

  int one;
  int two;
  int three;

  one = poi%10;
  if(one == 0 || one == 2 ||one == 3 ||one == 5 ||one == 6 ||one == 7 ||one == 8 ||one == 9){
    point3["seg1"].status=1;
  }
  if(one == 0 || one == 1 ||one == 2 ||one == 3 ||one == 4 ||one == 7 ||one == 8 ||one == 9){
    point3["seg2"].status=1;
  }
  if(one == 0 || one == 1 ||one == 3 ||one == 4 ||one == 5  ||one == 6 ||one == 7 ||one == 8 ||one == 9){
    point3["seg3"].status=1;
  }
  if(one == 0 || one == 2 ||one == 3 ||one == 5 ||one == 6 ||one == 8 ||one == 9){
    point3["seg4"].status=1;
  }
  if(one == 0 || one == 2 ||one == 6 ||one == 8){
    point3["seg5"].status=1;
  }
  if(one == 0 || one == 4 ||one == 5 ||one == 6 ||one == 8 ||one == 9){
    point3["seg6"].status=1;
  }
  if(one == 2 ||one == 3  ||one == 4 ||one == 5 ||one == 6 ||one == 8 ||one == 9){
    point3["seg7"].status=1;
  }

  poi =poi/10;
  one =poi%10;
  if(one == 0 || one == 2 ||one == 3 ||one == 5 ||one == 6 ||one == 7 ||one == 8 ||one == 9){
    point2["seg1"].status=1;
  }
  if(one == 0 || one == 1 ||one == 2 ||one == 3 ||one == 4 ||one == 7 ||one == 8 ||one == 9){
    point2["seg2"].status=1;
  }
  if(one == 0 || one == 1 ||one == 3 ||one == 4 ||one == 5  ||one == 6 ||one == 7 ||one == 8 ||one == 9){
    point2["seg3"].status=1;
  }
  if(one == 0 || one == 2 ||one == 3 ||one == 5 ||one == 6 ||one == 8 ||one == 9){
    point2["seg4"].status=1;
  }
  if(one == 0 || one == 2 ||one == 6 ||one == 8){
    point2["seg5"].status=1;
  }
  if(one == 0 || one == 4 ||one == 5 ||one == 6 ||one == 8 ||one == 9){
    point2["seg6"].status=1;
  }
  if(one == 2 || one == 3 ||one == 4 ||one == 5 ||one == 6 ||one == 8 ||one == 9){
    point2["seg7"].status=1;
  }


  poi =poi/10;
  one =poi%10;
  if(one == 0 || one == 2 ||one == 3 ||one == 5 ||one == 6 ||one == 7 ||one == 8 ||one == 9){
    point1["seg1"].status=1;
  }
  if(one == 0 || one == 1 ||one == 2 ||one == 3 ||one == 4 ||one == 7 ||one == 8 ||one == 9){
    point1["seg2"].status=1;
  }
  if(one == 0 || one == 1 ||one == 3 ||one == 4 ||one == 5  ||one == 6 ||one == 7 ||one == 8 ||one == 9){
    point1["seg3"].status=1;
  }
  if(one == 0 || one == 2 ||one == 3 ||one == 5 ||one == 6 ||one == 8 ||one == 9){
    point1["seg4"].status=1;
  }
  if(one == 0 || one == 2 ||one == 6 ||one == 8){
    point1["seg5"].status=1;
  }
  if(one == 0 || one == 4 ||one == 5 ||one == 6 ||one == 8 ||one == 9){
    point1["seg6"].status=1;
  }
  if(one == 2 || one == 3 ||one == 4 ||one == 5 ||one == 6 ||one == 8 ||one == 9){
    point1["seg7"].status=1;
  }


  glfwGetCursorPos(window, &new_mouse_pos_x, &new_mouse_pos_y);
    if(right_mouse_clicked==1){
        x_change+=new_mouse_pos_x-mouse_pos_x;
        y_change-=new_mouse_pos_y-mouse_pos_y;
        check_pan();
    }
  Matrices.projection = glm::ortho((float)(-400.0f/zoom_camera+x_change), (float)(400.0f/zoom_camera+x_change), (float)(-300.0f/zoom_camera+y_change), (float)(300.0f/zoom_camera+y_change), 0.1f, 500.0f);
  glfwGetCursorPos(window, &mouse_pos_x, &mouse_pos_y);

  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);

  if(mouse_clicked==1) {
          double mouse_x_cur;
          double mouse_y_cur;
          glfwGetCursorPos(window,&mouse_x_cur,&mouse_y_cur);
          //cout << mouse_x_cur << mouse_y_cur << endl;
      
          if(mouse_y_cur>=540){
            if(containers["redcontainer"].x-containers["redcontainer"].width/2<=(mouse_x_cur-400) && containers["redcontainer"].x+containers["redcontainer"].width/2>=(mouse_x_cur-400) && green==0){
              containers["redcontainer"].x = mouse_x_cur-400;
              containers["zarkredcontainer"].x = mouse_x_cur-400;
              red=1;
            }     
            else if(containers["greencontainer"].x-containers["greencontainer"].width/2<=(mouse_x_cur-400) && containers["greencontainer"].x+containers["greencontainer"].width/2>=(mouse_x_cur-400) && red==0){
              containers["greencontainer"].x = mouse_x_cur-400;
              containers["zarkgreencontainer"].x = mouse_x_cur-400;
              green=1;
            }
            flag =0;
          }  
          else if(mouse_x_cur<=40 && 300-mouse_y_cur>=laser["laser1"].y - 40 && 300-mouse_y_cur<=laser["laser1"].y + 40){
            laser["laser1"].y = 300 - mouse_y_cur;
            laser["laser2"].y = 300 - mouse_y_cur;
            laser["laser3"].y = 300 - mouse_y_cur;
            laser["laser4"].y = 300 - mouse_y_cur;
            laser["laser5"].y = 300 - mouse_y_cur;
            flag =0; 
          }
          else{
              angle=atan((300-mouse_y_cur-laser["laser2"].y)/(mouse_x_cur)); 
              angle*=180/M_PI;
              flag =1;
          }
          laser["laser2"].angle=angle;  
          launch_angle=angle;
      }

  if(laser["beam"].status==1){
    laser["beam"].x+=laser["beam"].x_speed;
    laser["beam"].y+=laser["beam"].y_speed;
    if(laser["beam"].x>=399 || laser["beam"].y>=299 || laser["beam"].x<=-399 || laser["beam"].y<=-299){
      laser["beam"].status=0;
      laser["beam"].x=laser["laser1"].x;
      laser["beam"].y=laser["laser1"].y;
      laser["beam"].flag =0;
    }
  }

  if(gameover==1){
      for(map<string,Sprite>::iterator it1=endlabel.begin();it1!=endlabel.end();it1++){
        string current = it1->first; 
        glm::mat4 MVP;  // MVP = Projection * View * Model

        Matrices.model = glm::mat4(1.0f);
        glm::mat4 ObjectTransform;
        glm::mat4 rotatev;

        rotatev = glm::rotate((float)((endlabel[current].angle)*M_PI/180.0f), glm::vec3(0,0,1));
        /* Render your scene */
        glm::mat4 translateObject = glm::translate (glm::vec3(endlabel[current].x, endlabel[current].y, 0.0f)); // glTranslatef
        if(current[0]=='v')
          ObjectTransform=translateObject*rotatev;
        else
          ObjectTransform=translateObject;
        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(endlabel[current].object);
      }    
        if(neg["neg"].status!=0) {
        Matrices.model = glm::mat4(1.0f);

        /* Render your scene */
        glm::mat4 ObjectTransform;
        glm::mat4 translateObject = glm::translate (glm::vec3(neg["neg"].x, neg["neg"].y, 0.0f)); // glTranslatef
        ObjectTransform=translateObject;


        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(neg["neg"].object);
      }

      for(map<string,Sprite>::iterator it1=point1.begin();it1!=point1.end();it1++){
          string current = it1->first; 
          glm::mat4 MVP;  // MVP = Projection * View * Model

          if(point1[current].status==0)
            continue;

          Matrices.model = glm::mat4(1.0f);

          /* Render your scene */
          glm::mat4 ObjectTransform;
          glm::mat4 translateObject = glm::translate (glm::vec3(point1[current].x,point1[current].y, 0.0f)); // glTranslatef
          ObjectTransform=translateObject;
          Matrices.model *= ObjectTransform;
          MVP = VP * Matrices.model; // MVP = p * V * M
          glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
          draw3DObject(point1[current].object);
      }

      for(map<string,Sprite>::iterator it1=point2.begin();it1!=point2.end();it1++){
          string current = it1->first; 
          glm::mat4 MVP;  // MVP = Projection * View * Model

          if(point2[current].status==0)
            continue;
          Matrices.model = glm::mat4(1.0f);

          /* Render your scene */
          glm::mat4 ObjectTransform;
          glm::mat4 translateObject = glm::translate (glm::vec3(point2[current].x,point2[current].y, 0.0f)); // glTranslatef
          ObjectTransform=translateObject;
          Matrices.model *= ObjectTransform;
          MVP = VP * Matrices.model; // MVP = p * V * M
          glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
          draw3DObject(point2[current].object);
      }

      for(map<string,Sprite>::iterator it1=point3.begin();it1!=point3.end();it1++){
          string current = it1->first; 
          glm::mat4 MVP;  // MVP = Projection * View * Model

          if(point3[current].status==0)
            continue;
          Matrices.model = glm::mat4(1.0f);

          /* Render your scene */
          glm::mat4 ObjectTransform;
          glm::mat4 translateObject = glm::translate (glm::vec3(point3[current].x,point3[current].y, 0.0f)); // glTranslatef
          ObjectTransform=translateObject;
          Matrices.model *= ObjectTransform;
          MVP = VP * Matrices.model; // MVP = p * V * M
          glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
          draw3DObject(point3[current].object);
      }
      return ;
    }


      
  /* Render your scene */
  for(map<string,Sprite>::iterator it1=laser.begin();it1!=laser.end();it1++){
      string current = it1->first; 
      glm::mat4 MVP;  // MVP = Projection * View * Model

      Matrices.model = glm::mat4(1.0f);
      if(laser[current].status==0)
        continue;
      /* Render your scene */
   
      glm::mat4 ObjectTransform;
      glm::mat4 translateObject;
      glm::mat4 rotatecannon;
      glm::mat4 translateObject1 = glm::translate (glm::vec3(50,0, 0.0f)); 
      translateObject = glm::translate (glm::vec3(laser[current].x, laser[current].y, 0.0f)); // glTranslatef
      if(current=="beam")
        rotatecannon = glm::rotate((float)((laser["beam"].angle)*M_PI/180.0f), glm::vec3(0,0,1));
      else
        rotatecannon = glm::rotate((float)(launch_angle*M_PI/180.0f), glm::vec3(0,0,1));

      if(current=="laser2" || current=="laser4")
      {
        ObjectTransform=translateObject*rotatecannon*translateObject1;
      }
      else{
        ObjectTransform=translateObject*rotatecannon;
      }
      Matrices.model *= ObjectTransform;
      MVP = VP * Matrices.model; // MVP = p * V * M
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(laser[current].object);
    
  }

  for(map<string,Sprite>::iterator it=bricks.begin();it!=bricks.end();it++){
        string current = it->first;
        Sprite my_object=laser["beam"];
        Sprite col_object=bricks[current];
        if(checkCollisionRight(col_object,my_object) || checkCollisionLeft(col_object,my_object) || checkCollisionTop(col_object,my_object) || checkCollisionBottom(col_object,my_object)){
          laser["beam"].status=0;
          bricks[current].x=num*200;
          bricks[current].y=ypos;
          num++;
          ypos+=80;
          if(num==2)
            num-=3;
          laser["beam"].x=laser["laser1"].x;
          laser["beam"].y=laser["laser1"].y;
          laser["beam"].flag =0;
          if(current[0]=='b'){
            score+=3;
          }
          else
          {
            count++;
            score-=2;
          }

          cout<< score << endl;
        }

        if(bricks[current].status==0)
            continue;
        glm::mat4 MVP;  // MVP = Projection * View * Model

        Matrices.model = glm::mat4(1.0f);

        /* Render your scene */
        glm::mat4 ObjectTransform;
        glm::mat4 translateObject = glm::translate (glm::vec3(bricks[current].x, bricks[current].y, 0.0f)); // glTranslatef
        ObjectTransform=translateObject;
        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M
        bricks[current].y = bricks[current].y - downfall; 

        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

        draw3DObject(bricks[current].object);
    }

    ypos -= downfall;

    float X=laser["beam"].x;
    float Y=laser["beam"].y;

    if(((34<=X && X<=69 && -208<=Y && Y<=-186) || (66<=X && X<=102 && -188<=Y && Y<=-169) || (100<=X && X<=133 && -171<=Y && Y<=-150) || (131<=X && X<=166 && -152<=Y && Y<=-133)) && laser["beam"].flag != 1){
            laser["beam"].angle= 60 - laser["beam"].angle;
            laser["beam"].x_speed=10*cos(laser["beam"].angle*(M_PI/180));
            laser["beam"].y_speed=10*sin(laser["beam"].angle*(M_PI/180));
            laser["beam"].flag = 1;
    }
    if((-175<=X && X<=-25 && 265<=Y && Y<=275) && laser["beam"].flag !=2 ){
            laser["beam"].angle= - laser["beam"].angle;
            laser["beam"].x_speed=10*cos(laser["beam"].angle*(M_PI/180));
            laser["beam"].y_speed=10*sin(laser["beam"].angle*(M_PI/180));
            laser["beam"].flag = 2;
          }
    if(((247<=X && X<=275 && 165<=Y && Y<=194) || (273<=X && X<=301 && 139<=Y && Y<=167) || (299<=X && X<=327 && 112<=Y && Y<=141) || (325<=X && X<=354 && 86<=Y && Y<=114)) && laser["beam"].flag != 3){
            laser["beam"].angle=  2*135 - laser["beam"].angle;
            laser["beam"].x_speed=10*cos(laser["beam"].angle*(M_PI/180));
            laser["beam"].y_speed=10*sin(laser["beam"].angle*(M_PI/180));
            laser["beam"].flag = 3;
    }
 
  if(neg["neg"].status!=0) {
    Matrices.model = glm::mat4(1.0f);

    /* Render your scene */
    glm::mat4 ObjectTransform;
    glm::mat4 translateObject = glm::translate (glm::vec3(neg["neg"].x, neg["neg"].y, 0.0f)); // glTranslatef
    ObjectTransform=translateObject;


    Matrices.model *= ObjectTransform;
    MVP = VP * Matrices.model; // MVP = p * V * M
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(neg["neg"].object);
  }

  for(map<string,Sprite>::iterator it1=point1.begin();it1!=point1.end();it1++){
      string current = it1->first; 
      glm::mat4 MVP;  // MVP = Projection * View * Model

      if(point1[current].status==0)
        continue;

      Matrices.model = glm::mat4(1.0f);

      /* Render your scene */
      glm::mat4 ObjectTransform;
      glm::mat4 translateObject = glm::translate (glm::vec3(point1[current].x,point1[current].y, 0.0f)); // glTranslatef
      ObjectTransform=translateObject;
      Matrices.model *= ObjectTransform;
      MVP = VP * Matrices.model; // MVP = p * V * M
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(point1[current].object);
  }

  for(map<string,Sprite>::iterator it1=point2.begin();it1!=point2.end();it1++){
      string current = it1->first; 
      glm::mat4 MVP;  // MVP = Projection * View * Model

      if(point2[current].status==0)
        continue;
      Matrices.model = glm::mat4(1.0f);

      /* Render your scene */
      glm::mat4 ObjectTransform;
      glm::mat4 translateObject = glm::translate (glm::vec3(point2[current].x,point2[current].y, 0.0f)); // glTranslatef
      ObjectTransform=translateObject;
      Matrices.model *= ObjectTransform;
      MVP = VP * Matrices.model; // MVP = p * V * M
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(point2[current].object);
  }

  for(map<string,Sprite>::iterator it1=point3.begin();it1!=point3.end();it1++){
      string current = it1->first; 
      glm::mat4 MVP;  // MVP = Projection * View * Model

      if(point3[current].status==0)
        continue;
      Matrices.model = glm::mat4(1.0f);

      /* Render your scene */
      glm::mat4 ObjectTransform;
      glm::mat4 translateObject = glm::translate (glm::vec3(point3[current].x,point3[current].y, 0.0f)); // glTranslatef
      ObjectTransform=translateObject;
      Matrices.model *= ObjectTransform;
      MVP = VP * Matrices.model; // MVP = p * V * M
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(point3[current].object);
  }

  for(map<string,Sprite>::iterator it1=mirrors.begin();it1!=mirrors.end();it1++){
        string current = it1->first; 
        Sprite my_object=laser["beam"];
        Sprite col_object=mirrors[current];
       
        glm::mat4 MVP;  // MVP = Projection * View * Model

        Matrices.model = glm::mat4(1.0f);

        /* Render your scene */
        glm::mat4 ObjectTransform;
        glm::mat4 rotatemirror;
        if(current=="mirror2")
          rotatemirror = glm::rotate((float)(30*M_PI/180.0f), glm::vec3(0,0,1));
        else if(current=="mirror3")
          rotatemirror = glm::rotate((float)(135*M_PI/180.0f), glm::vec3(0,0,1));

        glm::mat4 translateObject = glm::translate (glm::vec3(mirrors[current].x, mirrors[current].y, 0.0f)); // glTranslatef
        if(current=="mirror1")
          ObjectTransform=translateObject;
        else
          ObjectTransform=translateObject*rotatemirror;

        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(mirrors[current].object);
    }

   for(map<string,Sprite>::iterator it1=containers.begin();it1!=containers.end();it1++){
            string current = it1->first; 
            glm::mat4 MVP;  // MVP = Projection * View * Model

            Matrices.model = glm::mat4(1.0f);

            /* Render your scene */
            glm::mat4 ObjectTransform;
            glm::mat4 translateObject = glm::translate (glm::vec3(containers[current].x, containers[current].y, 0.0f)); // glTranslatef
            ObjectTransform=translateObject;
            Matrices.model *= ObjectTransform;
            MVP = VP * Matrices.model; // MVP = p * V * M
            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
            draw3DObject(containers[current].object);
        }



      
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
      if (!glfwInit()) {
          exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width ,height, "My openGL game", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    glfwSetScrollCallback(window, mousescroll); // mouse scroll

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height){

    /* Objects should be created before any other gl function and shaders */
	// Create the models
  COLOR green = {0,1,0};
  COLOR grey = {168.0/255.0,168.0/255.0,168.0/255.0};
  COLOR gold = {218.0/255.0,165.0/255.0,32.0/255.0};
  COLOR coingold = {255.0/255.0,223.0/255.0,0.0/255.0};
  COLOR red = {255.0/255.0,51.0/255.0,51.0/255.0};
  COLOR lightgreen = {57/255.0,230/255.0,0/255.0};
  COLOR darkgreen = {51/255.0,102/255.0,0/255.0};
  COLOR black = {30/255.0,30/255.0,21/255.0};
  COLOR blue = {0,0,1};
  COLOR darkbrown = {46/255.0,46/255.0,31/255.0};
  COLOR lightbrown = {95/255.0,63/255.0,32/255.0};
  COLOR darkred = {124/255.0,29/255.0,14/255.0};
  COLOR cratebrown = {153/255.0,102/255.0,0/255.0};
  COLOR cratebrown1 = {121/255.0,85/255.0,0/255.0};
  COLOR cratebrown2 = {102/255.0,68/255.0,0/255.0};
  COLOR skyblue2 = {113/255.0,185/255.0,209/255.0};
  COLOR skyblue1 = {123/255.0,201/255.0,227/255.0};
  COLOR skyblue = {132/255.0,217/255.0,245/255.0};
  COLOR cloudwhite = {229/255.0,255/255.0,255/255.0};
  COLOR cloudwhite1 = {204/255.0,255/255.0,255/255.0};
  COLOR lightpink = {255/255.0,122/255.0,173/255.0};
  COLOR darkpink = {255/255.0,51/255.0,119/255.0};
  COLOR white = {255/255.0,255/255.0,255/255.0};
  COLOR score = {117/255.0,78/255.0,40/255.0};
	// Create and compile our GLSL program from the shaders

  createCircle("laser1",10000,blue,-400,0,40,200,"laser",1);
  createCircle("laser3",10000,black,-400,0,30,200,"laser",1);
  createCircle("laser5",10000,white,-400,0,15,200,"laser",1);
  createRectangle1("laser2",1000,blue,blue,blue,blue,-400,0,24,100,"laser");
  createRectangle1("laser4",1000,black,black,black,black,-400,0,12,90,"laser");
  createRectangle1("beam",10000,darkpink,darkpink,darkpink,darkpink,-400,0,10,40,"laser");
  laser["beam"].status=0;

  createRectangle1("greenbrick1",10000,green,green,green,green,-200,400,30,20,"bricks");
  createRectangle1("redbrick1",10000,red,red,red,red,0,480,30,20,"bricks");
  createRectangle1("blackbrick1",10000,black,black,black,black,200,560,30,20,"bricks");
  createRectangle1("greenbrick2",10000,green,green,green,green,-200,640,30,20,"bricks");
  createRectangle1("redbrick2",10000,red,red,red,red,0,720,30,20,"bricks");
  createRectangle1("redbrick3",10000,red,red,red,red,200,800,30,20,"bricks");
  createRectangle1("blackbrick2",10000,black,black,black,black,-200,880,30,20,"bricks");
  createRectangle1("redbrick4",10000,red,red,red,red,0,960,30,20,"bricks");
  createRectangle1("blackbrick3",10000,black,black,black,black,200,1040,30,20,"bricks");
  createRectangle1("greenbrick3",10000,green,green,green,green,-200,1120,30,20,"bricks");
  createRectangle1("redbrick5",10000,red,red,red,red,0,1200,30,20,"bricks");
  createRectangle1("blackbrick4",10000,black,black,black,black,200,1280,30,20,"bricks");
  createRectangle1("greenbrick4",10000,green,green,green,green,-200,1360,30,20,"bricks");
  createRectangle1("redbrick6",10000,red,red,red,red,0,1440,30,20,"bricks");
  createRectangle1("blackbrick5",10000,black,black,black,black,200,1520,30,20,"bricks");
  createRectangle1("greenbrick5",10000,green,green,green,green,-200,1600,30,20,"bricks");
  createRectangle1("redbrick7",10000,red,red,red,red,0,1680,30,20,"bricks");
  createRectangle1("greenbrick6",10000,green,green,green,green,200,1760,30,20,"bricks");
  createRectangle1("blackbrick6",10000,black,black,black,black,-200,1840,30,20,"bricks");
  createRectangle1("greenbrick7",10000,green,green,green,green,0,1920,30,20,"bricks");
  createRectangle1("redbrick8",10000,red,red,red,red,200,2000,30,20,"bricks");
  createRectangle1("blackbrick7",10000,black,black,black,black,-200,2080,30,20,"bricks");
  createRectangle1("greenbrick8",10000,green,green,green,green,0,2160,30,20,"bricks");
  createRectangle1("greenbrick9",10000,green,green,green,green,200,2240,30,20,"bricks");
  createRectangle1("redbrick9",10000,red,red,red,red,-200,2320,30,20,"bricks");
  createRectangle1("blackbrick8",10000,black,black,black,black,0,2400,30,20,"bricks");
  createRectangle1("redbrick10",10000,red,red,red,red,200,2480,30,20,"bricks");
  createRectangle1("greenbrick10",10000,green,green,green,green,-200,2560,30,20,"bricks");
  createRectangle1("blackbrick9",10000,black,black,black,black,0,2640,30,20,"bricks");


  createRectangle1("redcontainer",10000,darkred,darkred,darkred,darkred,-90,-270,60,140,"containers");
  createRectangle1("zarkredcontainer",10000,red,red,red,red,-90,-270,40,120,"containers");
  createRectangle1("greencontainer",10000,darkgreen,darkgreen,darkgreen,darkgreen,220,-270,60,140,"containers");
  createRectangle1("zarkgreencontainer",10000,green,green,green,green,220,-270,40,120,"containers");


  createRectangle1("mirror1",10000,skyblue,skyblue,skyblue,skyblue,-100,270,10,150,"mirrors");
  createRectangle1("mirror2",10000,skyblue,skyblue,skyblue,skyblue,100,-170,10,150,"mirrors");
  createRectangle1("mirror3",10000,skyblue,skyblue,skyblue,skyblue,300,140,10,150,"mirrors");

  createRectangle1("neg",10000,score,score,score,score,312,275,2,10,"neg");
  neg["neg"].status=0;
  createRectangle1("seg1",10000,score,score,score,score,325,285,2,10,"point1");
  createRectangle1("seg2",10000,score,score,score,score,330,280,10,2,"point1");
  createRectangle1("seg3",10000,score,score,score,score,330,270,10,2,"point1");
  createRectangle1("seg4",10000,score,score,score,score,325,265,2,10,"point1");
  createRectangle1("seg5",10000,score,score,score,score,320,270,10,2,"point1");
  createRectangle1("seg6",10000,score,score,score,score,320,280,10,2,"point1");
  createRectangle1("seg7",10000,score,score,score,score,325,275,2,10,"point1");
  point1["seg7"].status=0;
 
  createRectangle1("seg1",10000,score,score,score,score,340,285,2,10,"point2");
  createRectangle1("seg2",10000,score,score,score,score,345,280,10,2,"point2");
  createRectangle1("seg3",10000,score,score,score,score,345,270,10,2,"point2");
  createRectangle1("seg4",10000,score,score,score,score,340,265,2,10,"point2");
  createRectangle1("seg5",10000,score,score,score,score,335,270,10,2,"point2");
  createRectangle1("seg6",10000,score,score,score,score,335,280,10,2,"point2");
  createRectangle1("seg7",10000,score,score,score,score,340,275,2,10,"point2");
  point2["seg7"].status=0;
 
  createRectangle1("seg1",10000,score,score,score,score,355,285,2,10,"point3");
  createRectangle1("seg2",10000,score,score,score,score,360,280,10,2,"point3");
  createRectangle1("seg3",10000,score,score,score,score,360,270,10,2,"point3");
  createRectangle1("seg4",10000,score,score,score,score,355,265,2,10,"point3");
  createRectangle1("seg5",10000,score,score,score,score,350,270,10,2,"point3");
  createRectangle1("seg6",10000,score,score,score,score,350,280,10,2,"point3");
  createRectangle1("seg7",10000,score,score,score,score,355,275,2,10,"point3");
  point3["seg7"].status=0;

  createRectangle1("g1",10000,score,score,score,score,-110,0,20,4,"end");
  createRectangle1("g2",10000,score,score,score,score,-90,0,20,4,"end");
  createRectangle1("g3",10000,score,score,score,score,-90,-20,20,4,"end");
  createRectangle1("g4",10000,score,score,score,score,-100,-10,4,20,"end");
  createRectangle1("g5",10000,score,score,score,score,-100,10,4,20,"end");
  createRectangle1("g6",10000,score,score,score,score,-100,-30,4,20,"end");
  createRectangle1("a1",10000,score,score,score,score,-80,0,20,4,"end");
  createRectangle1("a2",10000,score,score,score,score,-70,10,4,20,"end");
  createRectangle1("a3",10000,score,score,score,score,-70,-10,4,20,"end");
  createRectangle1("a4",10000,score,score,score,score,-62,0,20,4,"end");
  createRectangle1("a5",10000,score,score,score,score,-57.5,-10,4,5,"end");
  createRectangle1("m1",10000,score,score,score,score,-50,-1,22,4,"end");
  createRectangle1("m2",10000,score,score,score,score,-40,-1,22,4,"end");
  createRectangle1("m3",10000,score,score,score,score,-30,-1,22,4,"end");
  createRectangle1("m4",10000,score,score,score,score,-45,10,4,10,"end");
  createRectangle1("m5",10000,score,score,score,score,-35,10,4,10,"end");
  createRectangle1("e1",10000,score,score,score,score,-10,10,4,20,"end");
  createRectangle1("e2",10000,score,score,score,score,-20,0,20,4,"end");
  createRectangle1("e3",10000,score,score,score,score,-10,0,4,20,"end");
  createRectangle1("e4",10000,score,score,score,score,0,5,10,4,"end");
  createRectangle1("e5",10000,score,score,score,score,-10,-10,4,20,"end");
  createRectangle1("o1",10000,score,score,score,score,10,0,20,4,"end");
  createRectangle1("o2",10000,score,score,score,score,20,10,4,20,"end");
  createRectangle1("o3",10000,score,score,score,score,30,0,20,4,"end");
  createRectangle1("o4",10000,score,score,score,score,20,-10,4,20,"end");
  createRectangle1("v1",10000,score,score,score,score,40,0,25,4,"end");
  createRectangle1("v2",10000,score,score,score,score,50,0,25,4,"end");
  createRectangle1("e6",10000,score,score,score,score,70,10,4,20,"end");
  createRectangle1("e7",10000,score,score,score,score,60,0,20,4,"end");
  createRectangle1("e8",10000,score,score,score,score,70,0,4,20,"end");
  createRectangle1("e9",10000,score,score,score,score,80,5,10,4,"end");
  createRectangle1("e10",10000,score,score,score,score,70,-10,4,20,"end");
  createRectangle1("r1",10000,score,score,score,score,90,0,22,4,"end");
  createRectangle1("r2",10000,score,score,score,score,97,7,4,14,"end");
  endlabel["v1"].angle=16;
  endlabel["v2"].angle=-16;





  
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 800;
	int height = 600;

    GLFWwindow* window = initGLFW(width, height);

	  initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    glfwGetCursorPos(window, &mouse_pos_x, &mouse_pos_y);

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw(window);

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
