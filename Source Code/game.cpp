/*
Abhishek Ashwanikumar Sharma 2017A7PS0150P
Pulkit Agarwal 2016A7PS0060P
*/

#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>
#include <ao/ao.h>
#include <mpg123.h>

#include <include/glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define BITS 8
using namespace std;

typedef struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
}VAO;

typedef struct COLOUR {
    float r;
    float g;
    float b;
}COLOUR;


typedef struct Sprite {
    string name;
    COLOUR colour;
    float x,y,z;
    VAO* object;
    int status;
    float height,width,depth;
    float x_change,y_change,z_change;
    float angle; //Current Angle 
    float radius;
    int fixed;
    float flag ; //Value from 0 to 1
    int direction; //0 for clockwise and 1 for anticlockwise for animation
}Sprite;


struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

struct GLMatrices Matrices1;

map <string, Sprite> tiles;
map <string, Sprite> weakTiles;
map <string, Sprite> block;
map <string, Sprite> switches;
map <string, Sprite> point1;
map <string, Sprite> point2;
map <string, Sprite> point3;
map <string, Sprite> s1;
map <string, Sprite> s2;
map <string, Sprite> m1;
map <string, Sprite> m2;
map <string, Sprite> label;
map <string, Sprite> endlabel;


glm::mat4 rotateblock = glm::mat4(1.0f);


int seconds=0;
float zoomCamera = 1;
float gameOver=0;


GLuint programID;

//load Shaders
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
	printf("Vertex Shader: %s\n", vertex_file_path);
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
	printf("Fragment Shader: %s\n", fragment_file_path);
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
	fprintf(stdout, "Creating Program Link\n");
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

static void errorHandling(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quitGame(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertexBufferData, const GLfloat* colorBufferData, GLenum fill_mode=GL_FILL)
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
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertexBufferData, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), colorBufferData, GL_STATIC_DRAW);  // Copy the vertex colors
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
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertexBufferData, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* colorBufferData = new GLfloat [3*numVertices];
    for(int i=0; i<numVertices;i++){
        colorBufferData [3*i] = red;
        colorBufferData [3*i + 1] = green;
        colorBufferData [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertexBufferData, colorBufferData, fill_mode);
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



int level =0;
int moves =0;

double launch_angle=0;
int keyboard_pressed=0;

bool upKeyPressed=0;
bool downKeyPressed=0;
bool leftKeyPressed=0;
bool rightKeyPressed=0;

void mousescroll(GLFWwindow* window, double xoffset, double yoffset){
    Matrices.projection = glm::perspective(glm::radians(45.0f),(float)1000/(float)800, 0.1f, 5000.0f);
}

bool tKeyPressed =0;
//int hKeyPressed =0;
bool fKeyPressed =0;
bool bKeyPressed =0;

float xEye = -300;
float yEye = 1000;
float zEye = 600;
float xTarget = -200; 
float yTarget = -100;
float zTarget = 0;


mpg123_handle *mh;
unsigned char *buffer;
size_t bufferSize;
size_t done;
int err;

int driver;
ao_device *dev;

ao_sample_format format;
int channels, encoding;
long rate;

void initializeAudio() {
    ao_initialize();
    driver = ao_default_driver_id();
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    bufferSize = 3500;
    buffer = (unsigned char*) malloc(bufferSize * sizeof(unsigned char));

    mpg123_open(mh, "./background.mp3");
    mpg123_getformat(mh, &rate, &channels, &encoding);

    format.bits = mpg123_encsize(encoding) * BITS;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;
    dev = ao_open_live(driver, &format, NULL);
}

void playAudio() {
    if (mpg123_read(mh, buffer, bufferSize, &done) == MPG123_OK)
        ao_play(dev, (char*) buffer, done);
    else mpg123_seek(mh, 0, SEEK_SET);
}

void clearAudio() {
    free(buffer);
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_shutdown();
}


void keyPress (GLFWwindow* window, int key, int scancode, int action, int mods){
    if (action == GLFW_PRESS) 
    {
        switch (key) {
            case GLFW_KEY_F: //front view
                fKeyPressed =1;
                tKeyPressed =0;
                bKeyPressed =0;
                break;
            case GLFW_KEY_B: //back view
                bKeyPressed =1;
                tKeyPressed =0;
                fKeyPressed =0; 
                break;
            case GLFW_KEY_T: //top view
                tKeyPressed = 1;
                bKeyPressed =0;
                fKeyPressed =0;
                break;
            case GLFW_KEY_H: //Helicopter view
                xEye = -300;
                yEye = 1000;
                zEye = 600;
                xTarget = -200; 
                yTarget = -100;
                zTarget = 0;
                tKeyPressed =0;
                fKeyPressed =0;
                bKeyPressed =0;
                break;
            case GLFW_KEY_DOWN: // go back
                downKeyPressed =1;
                moves++;
                break;
            case GLFW_KEY_UP: //go ahead
                upKeyPressed = 1;
                moves++;
                break;
            case GLFW_KEY_LEFT: //go left
                leftKeyPressed =1;
                moves++;
                break;
            case GLFW_KEY_RIGHT: //go right
                rightKeyPressed =1;
                moves++;
                break;
            case GLFW_KEY_ESCAPE: //quit game
                quitGame(window);
                break;
            default:
                break;
        }
    }
}


void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    Matrices.projection = glm::perspective(fov,(float)fbwidth/(float)fbheight, 0.1f, 5000.0f);
}

//Creating the Cubes 
void makeCube(string name,COLOUR top,COLOUR bottom,COLOUR right,COLOUR left,COLOUR away,COLOUR close,float x, float y ,float z,float width,float height,float depth,string part)
{

    GLfloat vertexBufferData []=
    {
        //Face1
       -(width/2),-(height/2),-(depth/2), //vertex1
       -(width/2),(height/2),-(depth/2), //vertex2
       (width/2),(height/2),-(depth/2), //vertex3
       (width/2),(height/2),-(depth/2), //vertex3
       (width/2),-(height/2),-(depth/2), //vertex4
       -(width/2),-(height/2),-(depth/2), //vertex1
       //Face2
       -(width/2),-(height/2),(depth/2), //vertex5
       -(width/2),(height/2),(depth/2), //vertex6
       (width/2),(height/2),(depth/2), //vertex7
       (width/2),(height/2),(depth/2), //vertex7
       (width/2),-(height/2),(depth/2), //vertex8
       -(width/2),-(height/2),(depth/2), //vertex5
       //Face3
       -(width/2),(height/2),(depth/2), //vertex6
       -(width/2),(height/2),-(depth/2), //vertex2
       -(width/2),-(height/2),(depth/2), //vertex5
       -(width/2),-(height/2),(depth/2), //vertex5
       -(width/2),-(height/2),-(depth/2), //vertex1
       -(width/2),(height/2),-(depth/2), //vertex2
       //Face4
       (width/2),(height/2),(depth/2), //vertex7
       (width/2),-(height/2),(depth/2), //vertex8
       (width/2),(height/2),-(depth/2), //vertex3
       (width/2),(height/2),-(depth/2), //vertex3
       (width/2),-(height/2),-(depth/2), //vertex4
       (width/2),-(height/2),(depth/2), //vertex8
       //Face5
       -(width/2),(height/2),(depth/2), //vertex6
       -(width/2),(height/2),-(depth/2), //vertex2
       (width/2),(height/2),(depth/2), //vertex7
       (width/2),(height/2),(depth/2), //vertex7
       (width/2),(height/2),-(depth/2), //vertex3
       -(width/2),(height/2),-(depth/2), //vertex2
       //Face6
       -(width/2),-(height/2),(depth/2), //vertex5
       -(width/2),-(height/2),-(depth/2), //vertex1
       (width/2),-(height/2),(depth/2), //vertex8
       (width/2),-(height/2),(depth/2), //vertex8
       (width/2),-(height/2),-(depth/2), //vertex4
       -(width/2),-(height/2),-(depth/2)  //vertex1    
    };


    GLfloat colorBufferData [] = 
    {
        //Face 1
        away.r,away.g,away.b, //vertex1
        away.r,away.g,away.b, //vertex2
        away.r,away.g,away.b, //vertex3
        away.r,away.g,away.b, //vertex3
        away.r,away.g,away.b, //vertex4
        away.r,away.g,away.b, //vertex1
         //Face 2
        close.r,close.g,close.b, //vertex5
        close.r,close.g,close.b, //vertex6
        close.r,close.g,close.b, //vertex7
        close.r,close.g,close.b, //vertex7
        close.r,close.g,close.b, //vertex8
        close.r,close.g,close.b, //vertex5
        //Face3
        left.r,left.g,left.b, //vertex6
        left.r,left.g,left.b, //vertex2
        left.r,left.g,left.b, //vertex5
        left.r,left.g,left.b, //vertex5
        left.r,left.g,left.b, //vertex1
        left.r,left.g,left.b, //vertex2
        //Face4
        right.r,right.g,right.b, //vertex7
        right.r,right.g,right.b, //vertex8
        right.r,right.g,right.b, //vertex3
        right.r,right.g,right.b, //vertex3
        right.r,right.g,right.b, //vertex4
        right.r,right.g,right.b, //vertex8
        //Face5
        top.r,top.g,top.b, //vertex6
        top.r,top.g,top.b, //vertex2
        top.r,top.g,top.b, //vertex7
        top.r,top.g,top.b, //vertex7
        top.r,top.g,top.b, //vertex3
        top.r,top.g,top.b, //vertex2
        //Face6
        bottom.r,bottom.g,bottom.b, //vertex5
        bottom.r,bottom.g,bottom.b, //vertex1
        bottom.r,bottom.g,bottom.b, //vertex8
        bottom.r,bottom.g,bottom.b, //vertex8
        bottom.r,bottom.g,bottom.b, //vertex4
        bottom.r,bottom.g,bottom.b //vertex1
    };

    VAO *cube = create3DObject(GL_TRIANGLES,36,vertexBufferData,colorBufferData,GL_FILL);
    Sprite sprite = {};
    sprite.colour = top;
    sprite.name = name;
    sprite.object = cube;
    sprite.x=x;
    sprite.y=y;
    sprite.z=z;
    sprite.height=height;
    sprite.width=width;
    sprite.depth=depth;
    sprite.status=1;
    sprite.x_change=x;
    sprite.y_change=y;
    sprite.z_change=z;
    sprite.fixed=0;
    sprite.flag=0;

    if(part=="tiles")
        tiles[name]=sprite;
    else if(part=="weakTiles")
        weakTiles[name]=sprite;
    else if(part=="block")
        block[name]=sprite;
}

void makeRectangle(string name, COLOUR c1, COLOUR c2, COLOUR c3, COLOUR c4, float x, float y, float height, float width, string part){
    // GL3 accepts only Triangles. Quads are not supported
    float w=width/2,h=height/2;
    GLfloat vertexBufferData [] = {
        -w,-h,0, // vertex1
        -w,h,0, // vertex2
        w,h,0, // vertex3
        w,h,0, // vertex3
        w,-h,0, // vertex4
        -w,-h,0  // vertex1
    };

    GLfloat colorBufferData [] = {
        c1.r,c1.g,c1.b, // colour1
        c2.r,c2.g,c2.b, // colour2
        c3.r,c3.g,c3.b, // colour3
        c3.r,c3.g,c3.b, // colour4
        c4.r,c4.g,c4.b, // colour5
        c1.r,c1.g,c1.b // colour6
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    VAO *rectangle = create3DObject(GL_TRIANGLES, 6, vertexBufferData, colorBufferData, GL_FILL);
    Sprite sprite = {};
    sprite.colour = c1;
    sprite.name = name;
    sprite.object = rectangle;
    sprite.x=x;
    sprite.y=y;
    sprite.height=height;
    sprite.width=width;
    sprite.status=1;
    sprite.fixed=0;
    sprite.angle=launch_angle;
    sprite.radius=(sqrt(height*height+width*width))/2;
    sprite.flag=0;

    if(part=="block")
        block[name]=sprite;
    else if(part=="point1")
        point1[name]=sprite;
    else if(part=="point2")
        point2[name]=sprite;
    else if(part=="point3")
        point3[name]=sprite;
    else if(part=="s1")
        s1[name]=sprite;
    else if(part=="s2")
        s2[name]=sprite;
    else if(part=="m1")
        m1[name]=sprite;
    else if(part=="m2")
        m2[name]=sprite;
    else if(part=="label")
        label[name]=sprite;
}

void createCircle (string name, COLOUR colour, float x,float y,float z, float r, int NoOfSegments, string part, int fill){
    int segments = NoOfSegments;
    float radius = r;
    GLfloat vertexBufferData[segments*9];
    GLfloat colorBufferData[segments*9];
    int i,j;
    float angle=(2*M_PI/segments);
    float current_angle = 0;
    for(i=0;i<segments;i++){
        for(j=0;j<3;j++){
            colorBufferData[i*9+j*3]=colour.r;
            colorBufferData[i*9+j*3+1]=colour.g;
            colorBufferData[i*9+j*3+2]=colour.b;
        }
        vertexBufferData[i*9]=0;
        vertexBufferData[i*9+1]=1;
        vertexBufferData[i*9+2]=0;
        vertexBufferData[i*9+3]=radius*cos(current_angle);
        vertexBufferData[i*9+4]=1;
        vertexBufferData[i*9+5]=radius*sin(current_angle);
        vertexBufferData[i*9+6]=radius*cos(current_angle+angle);
        vertexBufferData[i*9+7]=1;
        vertexBufferData[i*9+8]=radius*sin(current_angle+angle);
        current_angle+=angle;
    }
    VAO* circle;
    if(fill==1)
        circle = create3DObject(GL_TRIANGLES, (segments*9)/3, vertexBufferData, colorBufferData, GL_FILL);
    else
        circle = create3DObject(GL_TRIANGLES, (segments*9)/3, vertexBufferData, colorBufferData, GL_LINE);
    Sprite sprite = {};
    sprite.colour = colour;
    sprite.name = name;
    sprite.object = circle;
    sprite.x=x;
    sprite.y=y;
    sprite.z=z;
    sprite.height=2*r; //Height of the sprite is 2*r
    sprite.width=2*r; //Width of the sprite is 2*r
    sprite.status=1;
    sprite.radius=r;
    sprite.fixed=0;

    if(part=="switch")
        switches[name]=sprite;
}


// Render the scene with openGL 

float angle=0;
float xpos;
float ypos = 2720;
int num = 1;
int flag =0;
int gir1 =0;
int gir2 =0;
int score =0;
int gameover=0;
int count=0;
float downfall =.1;
float downtile = 0;
int tileflag =0;
float tileX;
float tileZ;
int switch1 =0;
int switch2 =0;
int sig=0;


void draw (GLFWwindow* window, int width, int height)
{

    if(gameover ==1)
        return;

    if(tKeyPressed == 1){
        xEye = block["block"].x_change;
        yEye = 1300;
        zEye = block["block"].z_change;
        xTarget = block["block"].x_change;
        yTarget = 0;
        zTarget = block["block"].z_change - 10;
    }
    else if(fKeyPressed ==1){
        xEye = block["block"].x_change ;
        yEye = block["block"].y_change +300;
        zEye = block["block"].z_change +300;
        xTarget = block["block"].x_change;
        yTarget = block["block"].y_change;
        zTarget = block["block"].z_change;
    }
    else if(bKeyPressed ==1){
        xEye = block["block"].x_change;
        yEye = block["block"].y_change + 300; 
        zEye = block["block"].z_change + 50;
        xTarget = block["block"].x_change;
        yTarget = block["block"].y_change;
        zTarget = block["block"].z_change - 200;   
    }


    glClearColor(184.0f/255.0f, 213.0f/255.0f, 238.0f/255.0f, 1.0f);//set background colour
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram (programID);
      
    glm::vec3 eye ( xEye ,yEye, zEye );
    glm::vec3 target (xTarget,yTarget, zTarget);
    glm::vec3 up (0, 1, 0);
    Matrices.view = glm::lookAt(eye, target, up);


    for(map<string,Sprite>::iterator it=point1.begin();it!=point1.end();it++){
        point1[it->first].status=0;
    }
    for(map<string,Sprite>::iterator it=point2.begin();it!=point2.end();it++){
        point2[it->first].status=0;
    }
    for(map<string,Sprite>::iterator it=point3.begin();it!=point3.end();it++){
        point3[it->first].status=0;
    }
    for(map<string,Sprite>::iterator it=s1.begin();it!=s1.end();it++){
        s1[it->first].status=0;
    }
    for(map<string,Sprite>::iterator it=s2.begin();it!=s2.end();it++){
        s2[it->first].status=0;
    }
    for(map<string,Sprite>::iterator it=m1.begin();it!=m1.end();it++){
        m1[it->first].status=0;
    }
    for(map<string,Sprite>::iterator it=m2.begin();it!=m2.end();it++){
        m2[it->first].status=0;
    }


    int time = abs(seconds % 60);

    int t;

    t = time%10;
    if(t == 0 || t == 2 ||t == 3 ||t == 5 ||t == 6 ||t == 7 ||t == 8 ||t == 9){
    s1["seg1"].status=1;
    }
    if(t == 0 || t == 1 ||t == 2 ||t == 3 ||t == 4 ||t == 7 ||t == 8 ||t == 9){
    s1["seg2"].status=1;
    }
    if(t == 0 || t == 1 ||t == 3 ||t == 4 ||t == 5  ||t == 6 ||t == 7 ||t == 8 ||t == 9){
    s1["seg3"].status=1;
    }
    if(t == 0 || t == 2 ||t == 3 ||t == 5 ||t == 6 ||t == 8 ||t == 9){
    s1["seg4"].status=1;
    }
    if(t == 0 || t == 2 ||t == 6 ||t == 8){
    s1["seg5"].status=1;
    }
    if(t == 0 || t == 4 ||t == 5 ||t == 6 ||t == 8 ||t == 9){
    s1["seg6"].status=1;
    }
    if(t == 2 ||t == 3  ||t == 4 ||t == 5 ||t == 6 ||t == 8 ||t == 9){
    s1["seg7"].status=1;
    }


    time = time/10;
    t = time%10;

    if(t == 0 || t == 2 ||t == 3 ||t == 5 ||t == 6 ||t == 7 ||t == 8 ||t == 9){
    s2["seg1"].status=1;
    }
    if(t == 0 || t == 1 ||t == 2 ||t == 3 ||t == 4 ||t == 7 ||t == 8 ||t == 9){
    s2["seg2"].status=1;
    }
    if(t == 0 || t == 1 ||t == 3 ||t == 4 ||t == 5  ||t == 6 ||t == 7 ||t == 8 ||t == 9){
    s2["seg3"].status=1;
    }
    if(t == 0 || t == 2 ||t == 3 ||t == 5 ||t == 6 ||t == 8 ||t == 9){
    s2["seg4"].status=1;
    }
    if(t == 0 || t == 2 ||t == 6 ||t == 8){
    s2["seg5"].status=1;
    }
    if(t == 0 || t == 4 ||t == 5 ||t == 6 ||t == 8 ||t == 9){
    s2["seg6"].status=1;
    }
    if(t == 2 ||t == 3  ||t == 4 ||t == 5 ||t == 6 ||t == 8 ||t == 9){
    s2["seg7"].status=1;
    }


    for(map<string,Sprite>::iterator it=s1.begin();it!=s1.end();it++){
        string current = it->first; 

        glm::vec3 eye ( 0 ,0, 5 );
        glm::vec3 target (0,0, 0);
        glm::vec3 up (0, 1, 0);
        Matrices1.view = glm::lookAt(eye, target, up);

        Matrices1.projection = glm::ortho((float)(-400.0f), (float)(400.0f), (float)(-300.0f), (float)(300.0f), 0.1f, 500.0f);

        glm::mat4 VP = Matrices1.projection * Matrices1.view;

        glm::mat4 MVP; 

        if(s1[current].status==0)
        continue;

        Matrices.model = glm::mat4(1.0f);

        // Render your scene 
        glm::mat4 ObjectTransform;
        glm::mat4 translateObject = glm::translate (glm::vec3(s1[current].x,s1[current].y, 0.0f));
        ObjectTransform=translateObject;
        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(s1[current].object);
    }


    for(map<string,Sprite>::iterator it=s2.begin();it!=s2.end();it++){
        string current = it->first; 

        glm::vec3 eye ( 0 ,0, 5 );
        glm::vec3 target (0,0, 0);
        glm::vec3 up (0, 1, 0);
        Matrices1.view = glm::lookAt(eye, target, up);

        Matrices1.projection = glm::ortho((float)(-400.0f), (float)(400.0f), (float)(-300.0f), (float)(300.0f), 0.1f, 500.0f);

        glm::mat4 VP = Matrices1.projection * Matrices1.view;

        glm::mat4 MVP;  

        if(s2[current].status==0)
        continue;

        Matrices.model = glm::mat4(1.0f);

        // Render your scene 
        glm::mat4 ObjectTransform;
        glm::mat4 translateObject = glm::translate (glm::vec3(s2[current].x,s2[current].y, 0.0f)); 
        ObjectTransform=translateObject;
        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(s2[current].object);
    }


    int time1 = abs(seconds/60);

    int t1 = time1%10;

    if(t1 == 0 || t1 == 2 ||t1 == 3 ||t1 == 5 ||t1 == 6 ||t1 == 7 ||t1 == 8 ||t1 == 9){
    
    m1["seg1"].status=1;
    }
    if(t1 == 0 || t1 == 1 ||t1 == 2 ||t1 == 3 ||t1 == 4 ||t1 == 7 ||t1 == 8 ||t1 == 9){
    
    m1["seg2"].status=1;
    }
    if(t1 == 0 || t1 == 1 ||t1 == 3 ||t1 == 4 ||t1 == 5  ||t1 == 6 ||t1 == 7 ||t1 == 8 ||t1 == 9){
    
    m1["seg3"].status=1;
    }
    if(t1 == 0 || t1 == 2 ||t1 == 3 ||t1 == 5 ||t1 == 6 ||t1 == 8 ||t1 == 9){
    
    m1["seg4"].status=1;
    }
    if(t1 == 0 || t1 == 2 ||t1 == 6 ||t1 == 8){
    
    m1["seg5"].status=1;
    }
    if(t1 == 0 || t1 == 4 ||t1 == 5 ||t1 == 6 ||t1 == 8 ||t1 == 9){
    
    m1["seg6"].status=1;
    }
    if(t1 == 2 ||t1 == 3  ||t1 == 4 ||t1 == 5 ||t1 == 6 ||t1 == 8 ||t1 == 9){
    
    m1["seg7"].status=1;
    }

    time1 = time1/10;
    t1=time1%10;

    if(t1 == 0 || t1 == 2 ||t1 == 3 ||t1 == 5 ||t1 == 6 ||t1 == 7 ||t1 == 8 ||t1 == 9){
    m2["seg1"].status=1;
    }
    if(t1 == 0 || t1 == 1 ||t1 == 2 ||t1 == 3 ||t1 == 4 ||t1 == 7 ||t1 == 8 ||t1 == 9){
    m2["seg2"].status=1;
    }
    if(t1 == 0 || t1 == 1 ||t1 == 3 ||t1 == 4 ||t1 == 5  ||t1 == 6 ||t1 == 7 ||t1 == 8 ||t1 == 9){
    m2["seg3"].status=1;
    }
    if(t1 == 0 || t1 == 2 ||t1 == 3 ||t1 == 5 ||t1 == 6 ||t1 == 8 ||t1 == 9){
    m2["seg4"].status=1;
    }
    if(t1 == 0 || t1 == 2 ||t1 == 6 ||t1 == 8){
    m2["seg5"].status=1;
    }
    if(t1 == 0 || t1 == 4 ||t1 == 5 ||t1 == 6 ||t1 == 8 ||t1 == 9){
    m2["seg6"].status=1;
    }
    if(t1 == 2 ||t1 == 3  ||t1 == 4 ||t1 == 5 ||t1 == 6 ||t1 == 8 ||t1 == 9){
    m2["seg7"].status=1;
    }


    for(map<string,Sprite>::iterator it=m1.begin();it!=m1.end();it++){
        string current = it->first; 

        glm::vec3 eye ( 0 ,0, 5 );
        glm::vec3 target (0,0, 0);
        glm::vec3 up (0, 1, 0);
        Matrices1.view = glm::lookAt(eye, target, up);

        Matrices1.projection = glm::ortho((float)(-400.0f), (float)(400.0f), (float)(-300.0f), (float)(300.0f), 0.1f, 500.0f);

        glm::mat4 VP = Matrices1.projection * Matrices1.view;

        glm::mat4 MVP;  

        if(m1[current].status==0)
        continue;

        Matrices.model = glm::mat4(1.0f);

        // Render your scene 
        glm::mat4 ObjectTransform;
        glm::mat4 translateObject = glm::translate (glm::vec3(m1[current].x,m1[current].y, 0.0f)); // glTranslatef
        ObjectTransform=translateObject;
        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(m1[current].object);
    }

    for(map<string,Sprite>::iterator it=m2.begin();it!=m2.end();it++){
        string current = it->first; 

        glm::vec3 eye ( 0 ,0, 5 );
        glm::vec3 target (0,0, 0);
        glm::vec3 up (0, 1, 0);
        Matrices1.view = glm::lookAt(eye, target, up);

        Matrices1.projection = glm::ortho((float)(-400.0f), (float)(400.0f), (float)(-300.0f), (float)(300.0f), 0.1f, 500.0f);

        glm::mat4 VP = Matrices1.projection * Matrices1.view;

        glm::mat4 MVP;  

        if(m2[current].status==0)
        continue;

        Matrices.model = glm::mat4(1.0f);

        /* Render your scene */
        glm::mat4 ObjectTransform;
        glm::mat4 translateObject = glm::translate (glm::vec3(m2[current].x,m2[current].y, 0.0f)); // glTranslatef
        ObjectTransform=translateObject;
        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(m2[current].object);
    }


    for(map<string,Sprite>::iterator it=label.begin();it!=label.end();it++){
        string current = it->first; 

        glm::vec3 eye ( 0 ,0, 5 );
        glm::vec3 target (0,0, 0);
        glm::vec3 up (0, 1, 0);
        Matrices1.view = glm::lookAt(eye, target, up);

        Matrices1.projection = glm::ortho((float)(-400.0f), (float)(400.0f), (float)(-300.0f), (float)(300.0f), 0.1f, 500.0f);

        glm::mat4 VP = Matrices1.projection * Matrices1.view;

        glm::mat4 MVP; 

        Matrices.model = glm::mat4(1.0f);

        // Render your scene 
        glm::mat4 ObjectTransform;
        glm::mat4 translateObject = glm::translate (glm::vec3(label[current].x,label[current].y, 0.0f)); // glTranslatef
        ObjectTransform=translateObject;
        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(label[current].object);
    }

    int point = abs(moves);

    int p;

    p = point%10;
    if(p == 0 || p == 2 ||p == 3 ||p == 5 ||p == 6 ||p == 7 ||p == 8 ||p == 9){
    point3["seg1"].status=1;
    }
    if(p == 0 || p == 1 ||p == 2 ||p == 3 ||p == 4 ||p == 7 ||p == 8 ||p == 9){
    point3["seg2"].status=1;
    }
    if(p == 0 || p == 1 ||p == 3 ||p == 4 ||p == 5  ||p == 6 ||p == 7 ||p == 8 ||p == 9){
    point3["seg3"].status=1;
    }
    if(p == 0 || p == 2 ||p == 3 ||p == 5 ||p == 6 ||p == 8 ||p == 9){
    point3["seg4"].status=1;
    }
    if(p == 0 || p == 2 ||p == 6 ||p == 8){
    point3["seg5"].status=1;
    }
    if(p == 0 || p == 4 ||p == 5 ||p == 6 ||p == 8 ||p == 9){
    point3["seg6"].status=1;
    }
    if(p == 2 ||p == 3  ||p == 4 ||p == 5 ||p == 6 ||p == 8 ||p == 9){
    point3["seg7"].status=1;
    }

    point =point/10;
    p =point%10;
    if(p == 0 || p == 2 ||p == 3 ||p == 5 ||p == 6 ||p == 7 ||p == 8 ||p == 9){
    point2["seg1"].status=1;
    }
    if(p == 0 || p == 1 ||p == 2 ||p == 3 ||p == 4 ||p == 7 ||p == 8 ||p == 9){
    point2["seg2"].status=1;
    }
    if(p == 0 || p == 1 ||p == 3 ||p == 4 ||p == 5  ||p == 6 ||p == 7 ||p == 8 ||p == 9){
    point2["seg3"].status=1;
    }
    if(p == 0 || p == 2 ||p == 3 ||p == 5 ||p == 6 ||p == 8 ||p == 9){
    point2["seg4"].status=1;
    }
    if(p == 0 || p == 2 ||p == 6 ||p == 8){
    point2["seg5"].status=1;
    }
    if(p == 0 || p == 4 ||p == 5 ||p == 6 ||p == 8 ||p == 9){
    point2["seg6"].status=1;
    }
    if(p == 2 || p == 3 ||p == 4 ||p == 5 ||p == 6 ||p == 8 ||p == 9){
    point2["seg7"].status=1;
    }


    point =point/10;
    p =point%10;
    if(p == 0 || p == 2 ||p == 3 ||p == 5 ||p == 6 ||p == 7 ||p == 8 ||p == 9){
    point1["seg1"].status=1;
    }
    if(p == 0 || p == 1 ||p == 2 ||p == 3 ||p == 4 ||p == 7 ||p == 8 ||p == 9){
    point1["seg2"].status=1;
    }
    if(p == 0 || p == 1 ||p == 3 ||p == 4 ||p == 5  ||p == 6 ||p == 7 ||p == 8 ||p == 9){
    point1["seg3"].status=1;
    }
    if(p == 0 || p == 2 ||p == 3 ||p == 5 ||p == 6 ||p == 8 ||p == 9){
    point1["seg4"].status=1;
    }
    if(p == 0 || p == 2 ||p == 6 ||p == 8){
    point1["seg5"].status=1;
    }
    if(p == 0 || p == 4 ||p == 5 ||p == 6 ||p == 8 ||p == 9){
    point1["seg6"].status=1;
    }
    if(p == 2 || p == 3 ||p == 4 ||p == 5 ||p == 6 ||p == 8 ||p == 9){
    point1["seg7"].status=1;
    }


    for(map<string,Sprite>::iterator it=point1.begin();it!=point1.end();it++){
        string current = it->first; 

        glm::vec3 eye ( 0 ,0, 5 );
        glm::vec3 target (0,0, 0);
        glm::vec3 up (0, 1, 0);
        Matrices1.view = glm::lookAt(eye, target, up);

        Matrices1.projection = glm::ortho((float)(-400.0f), (float)(400.0f), (float)(-300.0f), (float)(300.0f), 0.1f, 500.0f);

        glm::mat4 VP = Matrices1.projection * Matrices1.view;

        glm::mat4 MVP;  

        if(point1[current].status==0)
        continue;

        Matrices.model = glm::mat4(1.0f);

        // Render your scene 
        glm::mat4 ObjectTransform;
        glm::mat4 translateObject = glm::translate (glm::vec3(point1[current].x,point1[current].y, 0.0f)); // glTranslatef
        ObjectTransform=translateObject;
        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(point1[current].object);
    }

    for(map<string,Sprite>::iterator it=point2.begin();it!=point2.end();it++){
        string current = it->first; 
        glm::mat4 MVP; 

        glm::vec3 eye ( 0 ,0, 5 );
        glm::vec3 target (0,0, 0);
        glm::vec3 up (0, 1, 0);
        Matrices1.view = glm::lookAt(eye, target, up);

        Matrices1.projection = glm::ortho((float)(-400.0f), (float)(400.0f), (float)(-300.0f), (float)(300.0f), 0.1f, 500.0f);

        glm::mat4 VP = Matrices1.projection * Matrices1.view;

        if(point2[current].status==0)
        continue;
        Matrices.model = glm::mat4(1.0f);

        // Render your scene 
        glm::mat4 ObjectTransform;
        glm::mat4 translateObject = glm::translate (glm::vec3(point2[current].x,point2[current].y, 0.0f)); // glTranslatef
        ObjectTransform=translateObject;
        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(point2[current].object);
    }

    for(map<string,Sprite>::iterator it=point3.begin();it!=point3.end();it++){
        string current = it->first; 
        glm::mat4 MVP;  

        glm::vec3 eye ( 0 ,0, 5 );
        glm::vec3 target (0,0, 0);
        glm::vec3 up (0, 1, 0);
        Matrices1.view = glm::lookAt(eye, target, up);

        Matrices1.projection = glm::ortho((float)(-400.0f), (float)(400.0f), (float)(-300.0f), (float)(300.0f), 0.1f, 500.0f);

        glm::mat4 VP = Matrices1.projection * Matrices1.view;

        if(point3[current].status==0)
        continue;
        Matrices.model = glm::mat4(1.0f);

        // Render your scene 
        glm::mat4 ObjectTransform;
        glm::mat4 translateObject = glm::translate (glm::vec3(point3[current].x,point3[current].y, 0.0f)); // glTranslatef
        ObjectTransform=translateObject;
        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(point3[current].object);
    }

    GLfloat fov = M_PI/4;
    Matrices.projection = glm::perspective(fov, (GLfloat) width / (GLfloat) height, 0.1f, 50000.0f);


    glm::mat4 VP = Matrices.projection * Matrices.view;
    

    glm::mat4 MVP;	// MVP = Projection * View * Model

    // Load identity to model matrix
    Matrices.model = glm::mat4(1.0f);


    if(upKeyPressed ==1 && flag ==0)//handling the translation of the object
    {
        if(block["block"].direction ==0){
             glm::mat4 translateObject = glm::translate (glm::vec3(0,0,-(block["block"].z_change - 30)));
             glm::mat4 rotate = glm::rotate((float)((-90.0)*M_PI/180.0f), glm::vec3(1,0,0));
             glm::mat4 translateObject1 = glm::translate (glm::vec3(0,0,block["block"].z_change -30));

             rotateblock = translateObject1*rotate*translateObject*rotateblock;
             block["block"].x_change += 0;
             block["block"].z_change -= 90.0;
             block["block"].y_change -= 30.0;
             block["block"].direction =2;
        }
        else if(block["block"].direction==1){
             glm::mat4 translateObject = glm::translate (glm::vec3(0,0,-(block["block"].z_change - 30)));
             glm::mat4 rotate = glm::rotate((float)((-90.0)*M_PI/180.0f), glm::vec3(1,0,0));
             glm::mat4 translateObject1 = glm::translate (glm::vec3(0,0,block["block"].z_change -30));

             rotateblock = translateObject1 * rotate * translateObject * rotateblock;
             block["block"].z_change -= 60.0;
        } 
        else if(block["block"].direction==2){
             glm::mat4 translateObject = glm::translate (glm::vec3(0,0,-(block["block"].z_change - 60)));
             glm::mat4 rotate = glm::rotate((float)((-90.0)*M_PI/180.0f), glm::vec3(1,0,0));
             glm::mat4 translateObject1 = glm::translate (glm::vec3(0,0,block["block"].z_change -60));

             rotateblock = translateObject1 * rotate * translateObject * rotateblock;
             block["block"].y_change += 30.0;
             block["block"].z_change -= 90.0;
             block["block"].direction =0;
        }
        if(switches["switch1"].x == block["block"].x_change && switches["switch1"].z == block["block"].z_change){
            if(switch1==1)
                switch1=0;
            else
                switch1 =1;
        }
        if(switches["switch3"].x == block["block"].x_change && switches["switch3"].z == block["block"].z_change){
            if(switch2==1)
                switch2=0;
            else 
                switch2 =1;
        }
        upKeyPressed =0;
    }

    if(downKeyPressed ==1 && flag ==0)//handling the translation of the object
    {
        if(block["block"].direction ==0){
             glm::mat4 translateObject = glm::translate (glm::vec3(0,0,-(block["block"].z_change + 30)));
             glm::mat4 rotate = glm::rotate((float)((90.0)*M_PI/180.0f), glm::vec3(1,0,0));
             glm::mat4 translateObject1 = glm::translate (glm::vec3(0,0,block["block"].z_change +30));

             rotateblock = translateObject1*rotate*translateObject*rotateblock;
             block["block"].x_change += 0;
             block["block"].z_change += 90.0;
             block["block"].y_change -= 30.0;
             block["block"].direction =2;
        }
        else if(block["block"].direction==1){
             glm::mat4 translateObject = glm::translate (glm::vec3(0,0,-(block["block"].z_change + 30)));
             glm::mat4 rotate = glm::rotate((float)((90.0)*M_PI/180.0f), glm::vec3(1,0,0));
             glm::mat4 translateObject1 = glm::translate (glm::vec3(0,0,block["block"].z_change +30));

             rotateblock = translateObject1 * rotate * translateObject * rotateblock;
             block["block"].z_change += 60.0;
        } 
        else if(block["block"].direction==2){
             glm::mat4 translateObject = glm::translate (glm::vec3(0,0,-(block["block"].z_change + 60)));
             glm::mat4 rotate = glm::rotate((float)((+90.0)*M_PI/180.0f), glm::vec3(1,0,0));
             glm::mat4 translateObject1 = glm::translate (glm::vec3(0,0,block["block"].z_change +60));

             rotateblock = translateObject1 * rotate * translateObject * rotateblock;
             block["block"].y_change += 30.0;
             block["block"].z_change += 90.0;
             block["block"].direction =0;
        }
     downKeyPressed =0;       
    }
    
    if(rightKeyPressed ==1 && flag==0)//handling the translation of the object
    {
        if(block["block"].direction ==0){
             glm::mat4 translateObject = glm::translate (glm::vec3(-(block["block"].x_change + 30.0),0,0));
             glm::mat4 rotate = glm::rotate((float)((-90.0)*M_PI/180.0f), glm::vec3(0,0,1));
             glm::mat4 translateObject1 = glm::translate (glm::vec3(block["block"].x_change + 30.0,0,0));

             rotateblock = translateObject1*rotate*translateObject*rotateblock;
             block["block"].x_change += 90.0;
             block["block"].y_change -= 30.0;
             block["block"].direction =1;
        }
        else if(block["block"].direction==1){
             glm::mat4 translateObject = glm::translate (glm::vec3(-(block["block"].x_change + 60),0,0));
             glm::mat4 rotate = glm::rotate((float)((-90.0)*M_PI/180.0f), glm::vec3(0,0,1));
             glm::mat4 translateObject1 = glm::translate (glm::vec3(block["block"].x_change + 60,0,0));

             rotateblock = translateObject1 * rotate * translateObject * rotateblock;
             block["block"].x_change += 90.0;
             block["block"].y_change += 30.0;
             block["block"].direction = 0;
        } 
        else if(block["block"].direction==2){
             glm::mat4 translateObject = glm::translate (glm::vec3(-(block["block"].x_change + 30.0),0,0));
             glm::mat4 rotate = glm::rotate((float)((-90.0)*M_PI/180.0f), glm::vec3(0,0,1));
             glm::mat4 translateObject1 = glm::translate (glm::vec3(block["block"].x_change + 30.0,0,0));

             rotateblock = translateObject1 * rotate * translateObject * rotateblock;
             block["block"].x_change += 60.0;
             block["block"].direction = 2;
        }
        rightKeyPressed =0;
    }

    if(leftKeyPressed ==1 && flag==0)//handling the translation of the object
    {
        if(block["block"].direction ==0){
             glm::mat4 translateObject = glm::translate (glm::vec3(-(block["block"].x_change - 30.0),0,0));
             glm::mat4 rotate = glm::rotate((float)((90.0)*M_PI/180.0f), glm::vec3(0,0,1));
             glm::mat4 translateObject1 = glm::translate (glm::vec3(block["block"].x_change - 30.0,0,0));

             rotateblock = translateObject1*rotate*translateObject*rotateblock;
             block["block"].x_change -= 90.0;
             block["block"].y_change -= 30.0;
             block["block"].direction =1;
        }
        else if(block["block"].direction==1){
             glm::mat4 translateObject = glm::translate (glm::vec3(-(block["block"].x_change - 60),0,0));
             glm::mat4 rotate = glm::rotate((float)((90.0)*M_PI/180.0f), glm::vec3(0,0,1));
             glm::mat4 translateObject1 = glm::translate (glm::vec3(block["block"].x_change - 60,0,0));

             rotateblock = translateObject1 * rotate * translateObject * rotateblock;
             block["block"].x_change -= 90.0;
             block["block"].y_change += 30.0;
             block["block"].direction = 0;
        } 
        else if(block["block"].direction==2){
             glm::mat4 translateObject = glm::translate (glm::vec3(-(block["block"].x_change - 30.0),0,0));
             glm::mat4 rotate = glm::rotate((float)((90.0)*M_PI/180.0f), glm::vec3(0,0,1));
             glm::mat4 translateObject1 = glm::translate (glm::vec3(block["block"].x_change - 30.0,0,0));

             rotateblock = translateObject1 * rotate * translateObject * rotateblock;
             block["block"].x_change -= 60.0;
             block["block"].direction = 2;
        }  
        leftKeyPressed =0;
    }
        
    if(level==0)//second level
    {
        flag =1;
        gir1=0;
        gir2=0;
        for(map<string,Sprite>::iterator it=weakTiles.begin();it!=weakTiles.end();it++){
            string current = it->first;
            float XX = block["block"].x_change;
            float ZZ = block["block"].z_change;
            
            if(block["block"].direction == 0){
                if(weakTiles[current].x == XX && weakTiles[current].z == ZZ){
                        flag =0 ;
                        break;
                }
                if(XX == -380 && ZZ == -120){
                   sig=1;
                   break;
                }
            }    
            if(block["block"].direction == 1){
                if(weakTiles[current].z == ZZ){
                    if(weakTiles[current].x + 30.0 == XX || weakTiles[current].x - 30.0 == XX ||( XX<=-320 && XX>= -440 && ZZ ==-120)){
                        if(gir1 ==1)
                            gir2 =1;
                        else 
                            gir1 =1;
                    }
                    if(gir1 == 1 && gir2 ==1){
                        flag =0 ; 
                        break;
                    }
                }
            }
            if(block["block"].direction == 2){
                if(weakTiles[current].x == XX){
                    if(weakTiles[current].z + 30.0 == ZZ || weakTiles[current].z - 30.0 == ZZ || ( ZZ<= -60 && ZZ>= -180 && XX ==-380)){
                        if(gir1 ==1)
                            gir2 =1;
                        else 
                            gir1 =1;
                    }
                    if(gir1 == 1 && gir2 ==1){
                        flag =0 ; 
                        break;
                    }
                }
            }
        }
    }

    if(level==1)//first level
    {
        flag =1;
        gir1=0;
        gir2=0;
        for(map<string,Sprite>::iterator it=tiles.begin();it!=tiles.end();it++){
            string current = it->first;
            float XX = block["block"].x_change;
            float ZZ = block["block"].z_change;
            
            if(block["block"].direction == 0){
                if(tiles[current].x == XX && tiles[current].z == ZZ){
                    if(current[0] == 'o'){
                        tileflag =1;
                        tileX = tiles[current].x;
                        tileZ = tiles[current].z;
                        break;
                    }
                    else{
                        flag =0 ;
                        break;
                     }
                }
                if(XX == -380 && ZZ == -120){
                    moves=0;
                    seconds=0;
                }
            }    
            if(block["block"].direction == 1){
                if(tiles[current].z == ZZ){
                    if(tiles[current].x + 30.0 == XX || tiles[current].x - 30.0 == XX ||( XX<=-320 && XX>= -440 && ZZ ==-120)){
                        if(gir1 ==1)
                            gir2 =1;
                        else 
                            gir1 =1;
                    }
                    if(gir1 == 1 && gir2 ==1){
                        flag =0 ; 
                        break;
                    }
                }
            }
            if(block["block"].direction == 2){
                if(tiles[current].x == XX){
                    if(tiles[current].z + 30.0 == ZZ || tiles[current].z - 30.0 == ZZ || ( ZZ<= -60 && ZZ>= -180 && XX ==-380)){
                        if(gir1 ==1)
                            gir2 =1;
                        else 
                            gir1 =1;
                    }
                    if(gir1 == 1 && gir2 ==1){
                        flag =0 ; 
                        break;
                    }
                }
            }
        }
        if(switch1==0)//handling switches
        {
            float XX = block["block"].x_change;
            float ZZ = block["block"].z_change;
            if( XX >= -80 && XX <= -50 && ZZ <=210 && ZZ >= 180)
                flag =1;
        }
        if(switch2==0)//handling switches
        {
            if(block["block"].z_change ==0 && block["block"].x_change >= -50 && block["block"].x_change <= 70)
                flag =1;
        }
    }

    glm::mat4 rotatetile1 = glm::mat4(1.0f);
    glm::mat4 rotatetile2 = glm::mat4(1.0f);
    glm::mat4 rotatetile3 = glm::mat4(1.0f);

    if(switch1==0)//Handle switches
    {
        glm::mat4 translatetile = glm::translate (glm::vec3(-(tiles["tile31"].x_change - 30.0),12,0));
        glm::mat4 rotate = glm::rotate((float)((-90.0)*M_PI/180.0f), glm::vec3(0,0,1));
        glm::mat4 translatetile1 = glm::translate (glm::vec3(tiles["tile31"].x_change - 30.0,-12,0));
        rotatetile3 = translatetile1 * rotate * translatetile;
    }   
    if(switch2==0)//Handle switches
    {
        glm::mat4 translatetile = glm::translate (glm::vec3(-(tiles["tile5"].x_change - 30.0),12,0));
        glm::mat4 rotate = glm::rotate((float)((-90.0)*M_PI/180.0f), glm::vec3(0,0,1));
        glm::mat4 translatetile1 = glm::translate (glm::vec3(tiles["tile5"].x_change - 30.0,-12,0));
        rotatetile1 = translatetile1 * rotate * translatetile;
    }   
    if(switch2==0)//Handle switches
    {
        glm::mat4 translatetile = glm::translate (glm::vec3(-(tiles["tile6"].x_change + 30.0),12,0));
        glm::mat4 rotate = glm::rotate((float)((90.0)*M_PI/180.0f), glm::vec3(0,0,1));
        glm::mat4 translatetile1 = glm::translate (glm::vec3(tiles["tile6"].x_change + 30.0,-12,0));
        rotatetile2 = translatetile1 * rotate * translatetile;
    }
    // Render your scene
    if(level==1){
        glm::mat4 ObjectTransform;
        glm::mat4 translateObject = glm::translate (glm::vec3(block["block"].x, block["block"].y,block["block"].z));
        
        glm::mat4 translateblock = glm::translate (glm::vec3(0,-downfall,0));
        if(flag ==1){
            ObjectTransform=  translateblock * rotateblock * translateObject ;
            block["block"].y_change -= 3;
            downfall += 3.0;
        }
        else 
            ObjectTransform= rotateblock * translateObject ;

        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(block["block"].object);
        
        if(block["block"].y_change <= -200){
            block["block"].x_change =block["block"].x;
            block["block"].y_change =block["block"].y;
            block["block"].z_change =block["block"].z;
            rotateblock = glm::mat4(1.0f);
            downfall =0;
            flag =0;
            block["block"].direction =0;
            tileflag =0;
            downtile =0;
            switch1=0;
            switch2=0;  
        }

    }

    if(level==1){
        for(map<string,Sprite>::iterator it=tiles.begin();it!=tiles.end();it++){
            string current = it->first; 
            glm::mat4 MVP; 
            Matrices.model = glm::mat4(1.0f);

                /* Render your scene */
            glm::mat4 ObjectTransform;
            glm::mat4 translateObject = glm::translate (glm::vec3(tiles[current].x, tiles[current].y,tiles[current].z)); // glTranslatef
            glm::mat4 translatetile = glm::translate (glm::vec3(0,-downtile,0)); // glTranslatef
         
            if(tileflag ==1 && tiles[current].x==tileX && tiles[current].z==tileZ){
                    ObjectTransform = translatetile * translateObject;
                    downtile += 5;
            }
            else{
                if(current=="tile5"){
                    ObjectTransform = rotatetile1 * translateObject;
                }
                else if(current=="tile6"){
                    ObjectTransform = rotatetile2 * translateObject;
                }
                else if(current =="tile31"){
                    ObjectTransform = rotatetile3 * translateObject;
                }
                else 
                    ObjectTransform=translateObject;
            }
            Matrices.model *= ObjectTransform;
            MVP = VP * Matrices.model; // MVP = p * V * M
            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
            draw3DObject(tiles[current].object);
        }
          
        for(map<string,Sprite>::iterator it=switches.begin();it!=switches.end();it++){
            string current = it->first; 
            glm::mat4 MVP; 
            Matrices.model = glm::mat4(1.0f);

            // Render your scene 
            glm::mat4 ObjectTransform;
            glm::mat4 translateObject = glm::translate (glm::vec3(switches[current].x,switches[current].y,switches[current].z)); // glTranslatef
         
            ObjectTransform=translateObject;
            Matrices.model *= ObjectTransform;
            MVP = VP * Matrices.model; // MVP = p * V * M
            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
            draw3DObject(switches[current].object);
        }
    }
   
    if(level==0){
        for(map<string,Sprite>::iterator it=weakTiles.begin();it!=weakTiles.end();it++){
            string current = it->first; 
            glm::mat4 MVP; 
            Matrices.model = glm::mat4(1.0f);

            // Render your scene 
            glm::mat4 ObjectTransform;
            glm::mat4 translateObject = glm::translate (glm::vec3(weakTiles[current].x, weakTiles[current].y,weakTiles[current].z)); // glTranslatef
         
            ObjectTransform=translateObject;
            
            Matrices.model *= ObjectTransform;
            MVP = VP * Matrices.model; // MVP = p * V * M
            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
            draw3DObject(weakTiles[current].object);
        }

    

        glm::mat4 ObjectTransform;
        glm::mat4 translateObject = glm::translate (glm::vec3(160.0,60.0,300.0)); // glTranslatef
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateblock = glm::translate (glm::vec3(0,-downfall,0)); // glTranslatef
        if(flag ==1){
            ObjectTransform=  translateblock * rotateblock * translateObject ;
            block["block"].y_change -= 3;
            downfall += 3.0;
        }
        else 
            ObjectTransform= rotateblock * translateObject ;

        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(block["block"].object);


        if(block["block"].y_change <= -200){
            if(sig==1)
                level=1;
            else{
                block["block"].x_change =160;
                block["block"].y_change =60;
                block["block"].z_change =300;
            }
            rotateblock = glm::mat4(1.0f);
            downfall =0;
            flag =0;
            block["block"].direction =0; 
        }
    }

}

// Initialise glfw window, I/O callbacks and the renderer 
GLFWwindow* initGLFW (int width, int height) {
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(errorHandling);
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

    //registering callbacks with GLFW 

    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    glfwSetWindowCloseCallback(window, quitGame);

    glfwSetKeyCallback(window, keyPress);      // general keyboard input

    glfwSetScrollCallback(window, mousescroll); // mouse scroll

    return window;
}

//models created
void initGL (GLFWwindow* window, int width, int height)
{

	// Create the models
    COLOUR green = {0/255.0,255/255.0,0/255.0};
    COLOUR ForestGreen = { 35/255.0,142/255.0,35/255.0};
    COLOUR LimeGreen = {50/255.0,204/255.0, 50/255.0};
    COLOUR orange = {204/255.0,50/255.0,50/255.0};
    COLOUR DarkOrchid  = {153/255.0,50/255.0,204/255.0};
    COLOUR DimGrey = {84/255.0,84/255.0,84/255.0};
    COLOUR Gold = {204/255.0,127/255.0,50/255.0};
    COLOUR Goldenrod = {219/255.0,219/255.0,112/255.0};
    COLOUR red = {255.0/255.0,2.0/255.0,0.0/255.0};
    COLOUR MediumForestGreen = {107/255.0,142/255.0,35/255.0};
    COLOUR black = {0/255.0,0/255.0,0/255.0};
    COLOUR blue = { 0/255.0,0/255.0,255/255.0};
    COLOUR Maroon = {142/255.0,35/255.0,107/255.0};
    COLOUR lightbrown = {95/255.0,63/255.0,32/255.0};
    COLOUR IndianRed = {79/255.0,47/255.0,47/255.0};
    COLOUR cratebrown = {153/255.0,102/255.0,0/255.0};
    COLOUR cratebrown1 = {121/255.0,85/255.0,0/255.0};
    COLOUR cratebrown2 = {102/255.0,68/255.0,0/255.0};
    COLOUR MidnightBlue = {47/255.0,47/255.0,79/255.0};
    COLOUR NavyBlue = {35/255.0,35/255.0,142/255.0};
    COLOUR SkyBlue = {50/255.0,153/255.0,204/255.0};
    COLOUR Violet = {79/255.0,47/255.0,79/255.0};
    COLOUR BlueViolet = {159/255.0,95/255.0,159/255.0};
    COLOUR Pink = {188/255.0,143/255.0,143/255.0};
    COLOUR darkpink = {255/255.0,51/255.0,119/255.0};
    COLOUR White = {252/255.0,252/255.0,252/255.0};
    COLOUR points = {117/255.0,78/255.0,40/255.0};




    makeCube("block",green,green,ForestGreen,ForestGreen,LimeGreen,LimeGreen,-500,60,60,60.0,120.0,60.0,"block");
    block["block"].direction = 0;

    makeCube("otile1",orange,Gold,points,DimGrey,Goldenrod,black,-200,-6,0,60.0,12.0,60.0,"tiles");
    makeCube("tile2",SkyBlue,Gold,points,DimGrey,Goldenrod,black,-140,-6,0,60.0,12.0,60.0,"tiles");
    makeCube("tile3",SkyBlue,Gold,points,DimGrey,Goldenrod,black,-200,-6,60,60.0,12.0,60.0,"tiles");
    makeCube("tile4",blue,Gold,points,DimGrey,Goldenrod,black,-80,-6,0,60.0,12.0,60.0,"tiles");
    makeCube("tile5",SkyBlue,Gold,points,DimGrey,Goldenrod,black,-20,-6,0,60.0,12.0,60.0,"tiles");
    makeCube("tile6",blue,Gold,points,DimGrey,Goldenrod,black,40,-6,0,60.0,12.0,60.0,"tiles");
    makeCube("tile7",SkyBlue,Gold,points,DimGrey,Goldenrod,black,100,-6,0,60.0,12.0,60.0,"tiles");
    makeCube("tile8",blue,Gold,points,DimGrey,Goldenrod,black,160,-6,0,60.0,12.0,60.0,"tiles");
    makeCube("tile10",blue,Gold,points,DimGrey,Goldenrod,black,100,-6,-60,60.0,12.0,60.0,"tiles");
    makeCube("tile11",SkyBlue,Gold,points,DimGrey,Goldenrod,black,160,-6,-60,60.0,12.0,60.0,"tiles");
    makeCube("tile17",blue,Gold,points,DimGrey,Goldenrod,black,160,-6,240,60.0,12.0,60.0,"tiles");
    makeCube("tile18",SkyBlue,Gold,points,DimGrey,Goldenrod,black,160,-6,300,60.0,12.0,60.0,"tiles");
    makeCube("tile19",SkyBlue,Gold,points,DimGrey,Goldenrod,black,100,-6,240,60.0,12.0,60.0,"tiles");
    createCircle("switch3",Goldenrod,100,0,240,25,200,"switch",1);
    createCircle("switch4",IndianRed,100,1,240,12,200,"switch",1);
    makeCube("tile20",blue,Gold,points,DimGrey,Goldenrod,black,100,-6,300,60.0,12.0,60.0,"tiles");
    makeCube("tile21",SkyBlue,Gold,points,DimGrey,Goldenrod,black,100,-6,360,60.0,12.0,60.0,"tiles");
    makeCube("tile22",SkyBlue,Gold,points,DimGrey,Goldenrod,black,40,-6,300,60.0,12.0,60.0,"tiles");
    makeCube("tile23",blue,Gold,points,DimGrey,Goldenrod,black,40,-6,360,60.0,12.0,60.0,"tiles");
    makeCube("tile24",blue,Gold,points,DimGrey,Goldenrod,black,-20,-6,300,60.0,12.0,60.0,"tiles");
    makeCube("tile25",SkyBlue,Gold,points,DimGrey,Goldenrod,black,-20,-6,360,60.0,12.0,60.0,"tiles");
    makeCube("tile26",SkyBlue,Gold,points,DimGrey,Goldenrod,black,-80,-6,300,60.0,12.0,60.0,"tiles");
    makeCube("tile27",blue,Gold,points,DimGrey,Goldenrod,black,-80,-6,360,60.0,12.0,60.0,"tiles");
    makeCube("tile28",blue,Gold,points,DimGrey,Goldenrod,black,-140,-6,300,60.0,12.0,60.0,"tiles");
    makeCube("tile29",SkyBlue,Gold,points,DimGrey,Goldenrod,black,-140,-6,360,60.0,12.0,60.0,"tiles");
	makeCube("tile30",SkyBlue,Gold,points,DimGrey,Goldenrod,black,-140,-6,240,60.0,12.0,60.0,"tiles");
    makeCube("otile31",red,Gold,points,DimGrey,Goldenrod,black,-80,-6,240,60.0,12.0,60.0,"tiles");
    makeCube("tile31",SkyBlue,Gold,points,DimGrey,Goldenrod,black,-80,-6,180,60.0,12.0,60.0,"tiles");
    makeCube("otile32",orange,Gold,points,DimGrey,Goldenrod,DarkOrchid,-200,-6,240,60.0,12.0,60.0,"tiles");
    makeCube("otile33",red,Gold,points,DimGrey,Goldenrod,IndianRed,-200,-6,300,60.0,12.0,60.0,"tiles");
    makeCube("otile34",orange,Gold,points,DimGrey,Goldenrod,DarkOrchid,-260,-6,300,60.0,12.0,60.0,"tiles");
    makeCube("otile35",red,Gold,points,DimGrey,Goldenrod,IndianRed,-260,-6,240,60.0,12.0,60.0,"tiles");
    makeCube("tile36",SkyBlue,Gold,points,DimGrey,Goldenrod,DarkOrchid,-320,-6,240,60.0,12.0,60.0,"tiles");
    makeCube("otile37",red,Gold,points,DimGrey,Goldenrod,IndianRed,-320,-6,300,60.0,12.0,60.0,"tiles");
    makeCube("otile38",orange,Gold,points,DimGrey,Goldenrod,DarkOrchid,-380,-6,300,60.0,12.0,60.0,"tiles");
    makeCube("otile39",red,Gold,points,DimGrey,Goldenrod,IndianRed,-380,-6,240,60.0,12.0,60.0,"tiles");
    makeCube("tile40",SkyBlue,Gold,points,DimGrey,Goldenrod,DarkOrchid,-440,-6,240,60.0,12.0,60.0,"tiles");
    makeCube("otile41",red,Gold,points,DimGrey,Goldenrod,IndianRed,-440,-6,300,60.0,12.0,60.0,"tiles");
    makeCube("otile42",orange,Gold,points,DimGrey,Goldenrod,DarkOrchid,-500,-6,300,60.0,12.0,60.0,"tiles");
    makeCube("otile43",red,Gold,points,DimGrey,Goldenrod,IndianRed,-500,-6,240,60.0,12.0,60.0,"tiles");
    makeCube("otile44",orange,Gold,points,DimGrey,Goldenrod,DarkOrchid,-560,-6,240,60.0,12.0,60.0,"tiles");
    makeCube("otile45",red,Gold,points,DimGrey,Goldenrod,IndianRed,-560,-6,300,60.0,12.0,60.0,"tiles");
    makeCube("tile46",SkyBlue,Gold,points,DimGrey,Goldenrod,black,-620,-6,300,60.0,12.0,60.0,"tiles");
    makeCube("otile47",red,Gold,points,DimGrey,Goldenrod,IndianRed,-620,-6,240,60.0,12.0,60.0,"tiles");
    makeCube("otile48",orange,Gold,points,DimGrey,Goldenrod,DarkOrchid,-560,-6,360,60.0,12.0,60.0,"tiles");
    makeCube("tile49",SkyBlue,Gold,points,DimGrey,Goldenrod,black,-500,-6,360,60.0,12.0,60.0,"tiles");
    makeCube("otile50",orange,Gold,points,DimGrey,Goldenrod,DarkOrchid,-440,-6,360,60.0,12.0,60.0,"tiles");
    makeCube("otile51",red,Gold,points,DimGrey,Goldenrod,IndianRed,-380,-6,360,60.0,12.0,60.0,"tiles");
    makeCube("otile52",orange,Gold,points,DimGrey,Goldenrod,DarkOrchid,-320,-6,360,60.0,12.0,60.0,"tiles");
    makeCube("otile53",orange,Gold,points,DimGrey,Goldenrod,IndianRed,-620,-6,180,60.0,12.0,60.0,"tiles");
    makeCube("otile54",red,Gold,points,DimGrey,Goldenrod,IndianRed,-560,-6,180,60.0,12.0,60.0,"tiles");
    makeCube("otile55",orange,Gold,points,DimGrey,Goldenrod,IndianRed,-500,-6,180,60.0,12.0,60.0,"tiles");
    makeCube("otile56",red,Gold,points,DimGrey,Goldenrod,IndianRed,-440,-6,180,60.0,12.0,60.0,"tiles");
    makeCube("tile57",SkyBlue,Gold,points,DimGrey,Goldenrod,IndianRed,-560,-6,120,60.0,12.0,60.0,"tiles");
    createCircle("switch1",Goldenrod,-560,0,120,25,200,"switch",1);
    createCircle("switch2",IndianRed,-560,1,120,12,200,"switch",1);
    makeCube("tile58",blue,Gold,points,DimGrey,Goldenrod,IndianRed,-500,-6,120,60.0,12.0,60.0,"tiles");
    makeCube("tile59",SkyBlue,Gold,points,DimGrey,Goldenrod,IndianRed,-440,-6,120,60.0,12.0,60.0,"tiles");
    makeCube("tile60",SkyBlue,Gold,points,DimGrey,Goldenrod,IndianRed,-500,-6,60,60.0,12.0,60.0,"tiles");
    makeCube("tile61",blue,Gold,points,DimGrey,Goldenrod,black,-200,-6,120,60.0,12.0,60.0,"tiles");
    makeCube("otile62",red,Gold,points,DimGrey,Goldenrod,black,-200,-6,180,60.0,12.0,60.0,"tiles");
    makeCube("otile63",orange,Gold,points,DimGrey,Goldenrod,black,-140,-6,60,60.0,12.0,60.0,"tiles");
    makeCube("otile64",red,Gold,points,DimGrey,Goldenrod,black,-140,-6,120,60.0,12.0,60.0,"tiles");
    makeCube("tile65",blue,Gold,points,DimGrey,Goldenrod,black,-140,-6,180,60.0,12.0,60.0,"tiles");
    makeCube("tile66",SkyBlue,Gold,points,DimGrey,Goldenrod,black,100,-6,-120,60.0,12.0,60.0,"tiles");
    makeCube("tile67",blue,Gold,points,DimGrey,Goldenrod,black,160,-6,-120,60.0,12.0,60.0,"tiles");
    makeCube("tile68",blue,Gold,points,DimGrey,Goldenrod,black,100,-6,-180,60.0,12.0,60.0,"tiles");
    makeCube("tile69",SkyBlue,Gold,points,DimGrey,Goldenrod,black,160,-6,-180,60.0,12.0,60.0,"tiles");
    makeCube("tile70",SkyBlue,Gold,points,DimGrey,Goldenrod,black,40,-6,-180,60.0,12.0,60.0,"tiles");
    makeCube("tile71",blue,Gold,points,DimGrey,Goldenrod,black,-20,-6,-180,60.0,12.0,60.0,"tiles");
    makeCube("otile72",red,Gold,points,DimGrey,Goldenrod,black,-80,-6,-180,60.0,12.0,60.0,"tiles");
    makeCube("otile73",orange,Gold,points,DimGrey,Goldenrod,black,-140,-6,-180,60.0,12.0,60.0,"tiles");
    makeCube("tile74",SkyBlue,Gold,points,DimGrey,Goldenrod,black,-200,-6,-180,60.0,12.0,60.0,"tiles");
    makeCube("tile75",blue,Gold,points,DimGrey,Goldenrod,black,-260,-6,-180,60.0,12.0,60.0,"tiles"); 
    makeCube("tile76",SkyBlue,Gold,points,DimGrey,Goldenrod,black,-260,-6,-120,60.0,12.0,60.0,"tiles");
    makeCube("tile77",SkyBlue,Gold,points,DimGrey,Goldenrod,black,-320,-6,-180,60.0,12.0,60.0,"tiles");
    makeCube("tile78",blue,Gold,points,DimGrey,Goldenrod,black,-320,-6,-120,60.0,12.0,60.0,"tiles");
    makeCube("tile79",SkyBlue,Gold,points,DimGrey,Goldenrod,black,-320,-6,-60,60.0,12.0,60.0,"tiles");
    makeCube("tile80",blue,Gold,points,DimGrey,Goldenrod,black,-380,-6,-180,60.0,12.0,60.0,"tiles");
    makeCube("tile81",blue,Gold,points,DimGrey,Goldenrod,black,-380,-6,-60,60.0,12.0,60.0,"tiles");
    makeCube("tile82",SkyBlue,Gold,points,DimGrey,Goldenrod,black,-440,-6,-180,60.0,12.0,60.0,"tiles");
    makeCube("tile83",blue,Gold,points,DimGrey,Goldenrod,black,-440,-6,-120,60.0,12.0,60.0,"tiles");
    makeCube("tile84",SkyBlue,Gold,points,DimGrey,Goldenrod,black,-440,-6,-60,60.0,12.0,60.0,"tiles");


    makeRectangle("seg1",points,points,points,points,325,285,2,10,"point1");
    makeRectangle("seg2",points,points,points,points,330,280,10,2,"point1");
    makeRectangle("seg3",points,points,points,points,330,270,10,2,"point1");
    makeRectangle("seg4",points,points,points,points,325,265,2,10,"point1");
    makeRectangle("seg5",points,points,points,points,320,270,10,2,"point1");
    makeRectangle("seg6",points,points,points,points,320,280,10,2,"point1");
    makeRectangle("seg7",points,points,points,points,325,275,2,10,"point1");
    point1["seg7"].status=0;

    makeRectangle("seg1",points,points,points,points,340,285,2,10,"point2");
    makeRectangle("seg2",points,points,points,points,345,280,10,2,"point2");
    makeRectangle("seg3",points,points,points,points,345,270,10,2,"point2");
    makeRectangle("seg4",points,points,points,points,340,265,2,10,"point2");
    makeRectangle("seg5",points,points,points,points,335,270,10,2,"point2");
    makeRectangle("seg6",points,points,points,points,335,280,10,2,"point2");
    makeRectangle("seg7",points,points,points,points,340,275,2,10,"point2");
    point2["seg7"].status=0;

    makeRectangle("seg1",points,points,points,points,355,285,2,10,"point3");
    makeRectangle("seg2",points,points,points,points,360,280,10,2,"point3");
    makeRectangle("seg3",points,points,points,points,360,270,10,2,"point3");
    makeRectangle("seg4",points,points,points,points,355,265,2,10,"point3");
    makeRectangle("seg5",points,points,points,points,350,270,10,2,"point3");
    makeRectangle("seg6",points,points,points,points,350,280,10,2,"point3");
    makeRectangle("seg7",points,points,points,points,355,275,2,10,"point3");
    point3["seg7"].status=0;

    makeRectangle("seg1",points,points,points,points,355,255,2,10,"s1");
    makeRectangle("seg2",points,points,points,points,360,250,10,2,"s1");
    makeRectangle("seg3",points,points,points,points,360,240,10,2,"s1");
    makeRectangle("seg4",points,points,points,points,355,235,2,10,"s1");
    makeRectangle("seg5",points,points,points,points,350,240,10,2,"s1");
    makeRectangle("seg6",points,points,points,points,350,250,10,2,"s1");
    makeRectangle("seg7",points,points,points,points,355,245,2,10,"s1");
    s1["seg7"].status=0;


    makeRectangle("seg1",points,points,points,points,340,255,2,10,"s2");
    makeRectangle("seg2",points,points,points,points,345,250,10,2,"s2");
    makeRectangle("seg3",points,points,points,points,345,240,10,2,"s2");
    makeRectangle("seg4",points,points,points,points,340,235,2,10,"s2");
    makeRectangle("seg5",points,points,points,points,335,240,10,2,"s2");
    makeRectangle("seg6",points,points,points,points,335,250,10,2,"s2");
    makeRectangle("seg7",points,points,points,points,340,245,2,10,"s2");
    s2["seg7"].status=0;
    
    makeRectangle("l1",points,points,points,points,330,250,3,3,"label");
    makeRectangle("l2",points,points,points,points,330,240,3,3,"label");

    makeRectangle("seg1",points,points,points,points,320,255,2,10,"m1");
    makeRectangle("seg2",points,points,points,points,325,250,10,2,"m1");
    makeRectangle("seg3",points,points,points,points,325,240,10,2,"m1");
    makeRectangle("seg4",points,points,points,points,320,235,2,10,"m1");
    makeRectangle("seg5",points,points,points,points,315,240,10,2,"m1");
    makeRectangle("seg6",points,points,points,points,315,250,10,2,"m1");
    makeRectangle("seg7",points,points,points,points,320,245,2,10,"m1");
    m1["seg7"].status=0;


    makeRectangle("seg1",points,points,points,points,305,255,2,10,"m2");
    makeRectangle("seg2",points,points,points,points,310,250,10,2,"m2");
    makeRectangle("seg3",points,points,points,points,310,240,10,2,"m2");
    makeRectangle("seg4",points,points,points,points,305,235,2,10,"m2");
    makeRectangle("seg5",points,points,points,points,300,240,10,2,"m2");
    makeRectangle("seg6",points,points,points,points,300,250,10,2,"m2");
    makeRectangle("seg7",points,points,points,points,305,245,2,10,"m2");
    m2["seg7"].status=0;
    
    /*level 1*/

    makeCube("tile7",SkyBlue,Gold,points,DimGrey,Goldenrod,black,100,-6,0,60.0,12.0,60.0,"weakTiles");
    makeCube("tile8",blue,Gold,points,DimGrey,Goldenrod,black,160,-6,0,60.0,12.0,60.0,"weakTiles");
    makeCube("tile9",SkyBlue,Gold,points,DimGrey,Goldenrod,black,220,-6,0,60.0,12.0,60.0,"weakTiles");
    makeCube("tile10",blue,Gold,points,DimGrey,Goldenrod,black,100,-6,-60,60.0,12.0,60.0,"weakTiles");
    makeCube("tile11",SkyBlue,Gold,points,DimGrey,Goldenrod,black,160,-6,-60,60.0,12.0,60.0,"weakTiles");
    makeCube("tile12",blue,Gold,points,DimGrey,Goldenrod,black,220,-6,-60,60.0,12.0,60.0,"weakTiles");
    makeCube("tile13",blue,Gold,points,DimGrey,Goldenrod,black,220,-6,60,60.0,12.0,60.0,"weakTiles");
    makeCube("tile14",SkyBlue,Gold,points,DimGrey,Goldenrod,black,220,-6,120,60.0,12.0,60.0,"weakTiles");
    makeCube("tile15",blue,Gold,points,DimGrey,Goldenrod,black,220,-6,180,60.0,12.0,60.0,"weakTiles");
    makeCube("tile16",SkyBlue,Gold,points,DimGrey,Goldenrod,black,160,-6,180,60.0,12.0,60.0,"weakTiles");
    makeCube("tile17",blue,Gold,points,DimGrey,Goldenrod,black,160,-6,240,60.0,12.0,60.0,"weakTiles");
    makeCube("tile18",SkyBlue,Gold,points,DimGrey,Goldenrod,black,160,-6,300,60.0,12.0,60.0,"weakTiles");
    makeCube("tile19",SkyBlue,Gold,points,DimGrey,Goldenrod,black,220,-6,240,60.0,12.0,60.0,"weakTiles");
    makeCube("tile66",SkyBlue,Gold,points,DimGrey,Goldenrod,black,100,-6,-120,60.0,12.0,60.0,"weakTiles");
    makeCube("tile67",blue,Gold,points,DimGrey,Goldenrod,black,160,-6,-120,60.0,12.0,60.0,"weakTiles");
    makeCube("tile68",blue,Gold,points,DimGrey,Goldenrod,black,100,-6,-180,60.0,12.0,60.0,"weakTiles");
    makeCube("tile69",SkyBlue,Gold,points,DimGrey,Goldenrod,black,160,-6,-180,60.0,12.0,60.0,"weakTiles");
    makeCube("tile70",SkyBlue,Gold,points,DimGrey,Goldenrod,black,40,-6,-180,60.0,12.0,60.0,"weakTiles");
    makeCube("tile71",blue,Gold,points,DimGrey,Goldenrod,black,-20,-6,-180,60.0,12.0,60.0,"weakTiles");
    makeCube("tile72",SkyBlue,Gold,points,DimGrey,Goldenrod,black,-80,-6,-180,60.0,12.0,60.0,"weakTiles");
    makeCube("tile73",blue,Gold,points,DimGrey,Goldenrod,black,-140,-6,-180,60.0,12.0,60.0,"weakTiles");
    makeCube("tile74",SkyBlue,Gold,points,DimGrey,Goldenrod,black,-200,-6,-180,60.0,12.0,60.0,"weakTiles");
    makeCube("tile75",blue,Gold,points,DimGrey,Goldenrod,black,-260,-6,-180,60.0,12.0,60.0,"weakTiles"); 
    makeCube("tile76",SkyBlue,Gold,points,DimGrey,Goldenrod,black,-260,-6,-120,60.0,12.0,60.0,"weakTiles");
    makeCube("tile77",SkyBlue,Gold,points,DimGrey,Goldenrod,black,-320,-6,-180,60.0,12.0,60.0,"weakTiles");
    makeCube("tile78",blue,Gold,points,DimGrey,Goldenrod,black,-320,-6,-120,60.0,12.0,60.0,"weakTiles");
    makeCube("tile79",SkyBlue,Gold,points,DimGrey,Goldenrod,black,-320,-6,-60,60.0,12.0,60.0,"weakTiles");
    makeCube("tile80",blue,Gold,points,DimGrey,Goldenrod,black,-380,-6,-180,60.0,12.0,60.0,"weakTiles");
    makeCube("tile81",blue,Gold,points,DimGrey,Goldenrod,black,-380,-6,-60,60.0,12.0,60.0,"weakTiles");
    makeCube("tile82",SkyBlue,Gold,points,DimGrey,Goldenrod,black,-440,-6,-180,60.0,12.0,60.0,"weakTiles");
    makeCube("tile83",blue,Gold,points,DimGrey,Goldenrod,black,-440,-6,-120,60.0,12.0,60.0,"weakTiles");
    makeCube("tile84",SkyBlue,Gold,points,DimGrey,Goldenrod,black,-440,-6,-60,60.0,12.0,60.0,"weakTiles");
    block["block"].x_change =160.0;
    block["block"].y_change= 60.0;
    block["block"].z_change= 300.0;



    programID = LoadShaders( "vertexShader.vert", "fragmentShader.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background colour of the scene
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
	int width = 1200;
	int height = 900;

    GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    initializeAudio();
    double last_update_time = glfwGetTime(), current_time;

    // Draw in loop 
    while (!glfwWindowShouldClose(window)) 
    { 
        playAudio();
        // OpenGL Draw commands
        draw(window, width, height);

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time 
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 1){ // atleast 0.5s elapsed since last frame
            seconds ++;
            last_update_time = current_time;
        }
    }

    clearAudio();
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
