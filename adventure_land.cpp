#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <cstdlib>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <FTGL/ftgl.h>
#include <SOIL/SOIL.h>

using namespace std;
float LEFT_BOUND = -72.0f;
float RIGHT_BOUND = 72.0f;
float TOP_BOUND = 34.0f;
float BOTTOM_BOUND = -34.0f;
float ZOOM_FACTOR = 2.0f;
float WINDOW_WIDTH = 1300;
float WINDOW_HEIGHT = 600;
const int NUM_TILES_ROW = 10;
const int NUM_TILES_COL = 10;
const float TILE_WIDTH = 5.0f;
const float TILE_HEIGHT = 5.0f;
const float TILE_LENGTH = 5.0f;

struct VAO {
  GLuint VertexArrayID;
  GLuint VertexBuffer;
  GLuint ColorBuffer;
  GLuint TextureBuffer;
  GLuint TextureID;

  GLenum PrimitiveMode; // GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES, GL_LINE_STRIP_ADJACENCY, GL_LINES_ADJACENCY, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLES, GL_TRIANGLE_STRIP_ADJACENCY and GL_TRIANGLES_ADJACENCY
  GLenum FillMode; // GL_FILL, GL_LINE
  int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
  glm::mat4 projection;
  glm::mat4 model;
  glm::mat4 view;
  GLuint MatrixID; // For use with normal shader
  GLuint TexMatrixID; // For use with texture shader
};
typedef struct GLMatrices GLMatrices;

GLMatrices Matrices;
GLuint programID, fontProgramID, textureProgramID;
GLint fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform;

//forward declarations
class Villain;
class Bonus;

class Cuboid{
public:
	Cuboid(GLMatrices *mtx, GLuint textureID, float *color, float x, float y, float z, float length, float width, float height, int type);
  ~Cuboid();
  void draw();
  void setPosition(float x, float y, float z);
  float getPosX();
  float getPosY();
  float getPosZ();
  float getMinX();
  float getMinY();
  float getMinZ();
  float getMaxX();
  float getMaxY();
  float getMaxZ();
  float getWidth();
  float getLength();
  float getHeight();
  void setVisible(bool value);
  void setSliding(bool value);
  bool isVisible();
  bool isSliding();
  bool checkCollision(Cuboid &cb);
  friend void undergoSliding();
private:
  GLfloat *vertex_buffer_data;
  GLfloat *color_buffer_data;
  GLfloat *texture_buffer_data;
  GLuint textureID;
  GLMatrices *mtx;
  VAO *vaobj;
  float x;
  float y;
  float z;
  float length;
  float width;
  float height;
  bool visible;
  bool sliding;
  static const float UPPER_LIMIT = 10.0f;
  static const float LOWER_LIMIT = -10.0f;
};

class Player{
public:
  Player(GLMatrices *mtx, float x, float y, float z);
  void setPosition(float x, float y, float z);
  void draw();
  void applyForces();
  void setDynamic(bool value);
  void jump();
  void applyForces(float timeInstance);
  void enableMoveLeft();
  void enableMoveRight();
  void enableMoveUp();
  void enableMoveDown();
  float getPosX();
  float getPosY();
  float getPosZ();
  friend bool checkCollisionVillain(Villain &v);
  friend void simulateCollisionVillain();
  friend void handleCollisionVillain();

  friend bool checkCollisionBonus(Bonus &b);
  friend void simulateCollisionBonus();
  friend void handleCollisionBonus();
private:
  Cuboid *cb;
  Cuboid *barrel;
  float speedX;
  float speedY;
  float jumpTime;
  float groundY;
  bool move_left;
  bool move_right;
  bool move_up;
  bool move_down;
  bool dynamic;
  bool inAir;
  static const float GRAVITY = 20.0f;
};

class Villain{
public:
  Villain(GLMatrices *mtx, float x, float y, float z);
  void draw();
  float getPosX();
  float getPosY();
  float getPosZ();
  friend bool checkCollisionVillain(Villain &v);
  friend void simulateCollisionVillain();
  friend void handleCollisionVillain();
private:
  Cuboid *cb;
};

class Bonus{
public:
  Bonus(GLMatrices *mtx, float x, float y, float z);
  void draw();
  float getPosX();
  float getPosY();
  float getPosZ();
  bool isVisible();
  void setVisible(bool value);
  friend bool checkCollisionBonus(Bonus &b);
  friend void simulateCollisionBonus();
  friend void handleCollisionBonus();
private:
  Cuboid *cb;
  bool visible;
};

Cuboid *cb;
Player *p;
vector<Cuboid*> tilesList;
vector<Cuboid*> waterList;
vector<Villain*> villainList;
vector<Bonus*> bonusList;

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

struct VAO* create3DTexturedObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* texture_buffer_data, GLuint textureID, GLenum fill_mode=GL_FILL)
{
  struct VAO* vao = new struct VAO;
  vao->PrimitiveMode = primitive_mode;
  vao->NumVertices = numVertices;
  vao->FillMode = fill_mode;
  vao->TextureID = textureID;

  // Create Vertex Array Object
  // Should be done after CreateWindow and before any other GL calls
  glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
  glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
  glGenBuffers (1, &(vao->TextureBuffer));  // VBO - textures

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

  glBindBuffer (GL_ARRAY_BUFFER, vao->TextureBuffer); // Bind the VBO textures
  glBufferData (GL_ARRAY_BUFFER, 2*numVertices*sizeof(GLfloat), texture_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
  glVertexAttribPointer(
              2,                  // attribute 2. Textures
              2,                  // size (s,t)
              GL_FLOAT,           // type
              GL_FALSE,           // normalized?
              0,                  // stride
              (void*)0            // array buffer offset
              );

  return vao;
}

void draw3DTexturedObject (struct VAO* vao)
{
  // Change the Fill Mode for this object
  glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

  // Bind the VAO to use
  glBindVertexArray (vao->VertexArrayID);

  // Enable Vertex Attribute 0 - 3d Vertices
  glEnableVertexAttribArray(0);
  // Bind the VBO to use
  glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

  // Bind Textures using texture units
  glBindTexture(GL_TEXTURE_2D, vao->TextureID);

  // Enable Vertex Attribute 2 - Texture
  glEnableVertexAttribArray(2);
  // Bind the VBO to use
  glBindBuffer(GL_ARRAY_BUFFER, vao->TextureBuffer);

  // Draw the geometry !
  glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle

  // Unbind Textures to be safe
  glBindTexture(GL_TEXTURE_2D, 0);
}

/* Create an OpenGL Texture from an image */
GLuint createTexture (const char* filename)
{
  GLuint TextureID;
  // Generate Texture Buffer
  glGenTextures(1, &TextureID);
  // All upcoming GL_TEXTURE_2D operations now have effect on our texture buffer
  glBindTexture(GL_TEXTURE_2D, TextureID);
  // Set our texture parameters
  // Set texture wrapping to GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // Set texture filtering (interpolation)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // Load image and create OpenGL texture
  int twidth, theight;
  unsigned char* image = SOIL_load_image(filename, &twidth, &theight, 0, SOIL_LOAD_RGB);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, twidth, theight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
  glGenerateMipmap(GL_TEXTURE_2D); // Generate MipMaps to use
  SOIL_free_image_data(image); // Free the data read from file after creating opengl texture
  glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess it up

  return TextureID;
}

float calculateDistance(float x1, float y1, float z1, float x2, float y2, float z2){
  float dist = (x1-x2) * (x1-x2) + (y1-y2)*(y1-y2) + (z1-z1)*(z1-z2);
  return sqrt(dist);
}


Cuboid::Cuboid(GLMatrices *mtx, GLuint textureID, float *color, float x, float y, float z, float length, float width, float height, int type)
{
  vertex_buffer_data = new GLfloat[36*3];
  color_buffer_data = new GLfloat[36*3];
  texture_buffer_data = new GLfloat[36 * 2];
  this->textureID = textureID;
  this->mtx  = mtx;
  this->x = x;
  this->y = y;
  this->z = z;
  this->length = length;
  this->width = width;
  this->height = height;
  float tvert[8*3];

  tvert[0] = width/2.0f;	//1
  tvert[1] = height/2.0f;
  tvert[2] = -1.0f * length/2.0f;

  tvert[3] = width/2.0f;	//2
  tvert[4] = -1.0f * height/2.0f;
  tvert[5] = -1.0f * length/2.0f;

  tvert[6] = width/2.0f;	//3
  tvert[7] = -1.0f * height/2.0f;
  tvert[8] = length/2.0f;

  tvert[9] = width/2.0f;	//4
  tvert[10] = height/2.0f;
  tvert[11] = length/2.0f;

  tvert[12] = -1.0f * width/2.0f;	//5
  tvert[13] = height/2.0f;
  tvert[14] = -1.0f * length/2.0f;

  tvert[15] = -1.0f * width/2.0f;	//6
  tvert[16] = -1.0f * height/2.0f;
  tvert[17] = -1.0f * length/2.0f;

  tvert[18] = -1.0f * width/2.0f;	//7
  tvert[19] = -1.0f * height/2.0f;
  tvert[20] = length/2.0f;

  tvert[21] = -1.0f * width/2.0f;	//8
  tvert[22] = height/2.0f;
  tvert[23] = length/2.0f;

//*********

  //Triangle 1
  vertex_buffer_data[0] = tvert[0];   //1
  vertex_buffer_data[1] = tvert[1];
  vertex_buffer_data[2] = tvert[2];	

  vertex_buffer_data[3] = tvert[3];   //2
  vertex_buffer_data[4] = tvert[4];
  vertex_buffer_data[5] = tvert[5];	

  vertex_buffer_data[6] = tvert[6];   //3
  vertex_buffer_data[7] = tvert[7];
  vertex_buffer_data[8] = tvert[8];	

  //Triangle 2
  vertex_buffer_data[9] = tvert[0];   //1
  vertex_buffer_data[10] = tvert[1];
  vertex_buffer_data[11] = tvert[2];	

  vertex_buffer_data[12] = tvert[9];   //4
  vertex_buffer_data[13] = tvert[10];
  vertex_buffer_data[14] = tvert[11];	

  vertex_buffer_data[15] = tvert[6];   //3
  vertex_buffer_data[16] = tvert[7];
  vertex_buffer_data[17] = tvert[8];

  //Triangle 3
  vertex_buffer_data[18] = tvert[0];   //1
  vertex_buffer_data[19] = tvert[1];
  vertex_buffer_data[20] = tvert[2];	

  vertex_buffer_data[21] = tvert[9];   //4
  vertex_buffer_data[22] = tvert[10];
  vertex_buffer_data[23] = tvert[11];	

  vertex_buffer_data[24] = tvert[21];   //8
  vertex_buffer_data[25] = tvert[22];
  vertex_buffer_data[26] = tvert[23];	

   //Triangle 4
  vertex_buffer_data[27] = tvert[0];   //1
  vertex_buffer_data[28] = tvert[1];
  vertex_buffer_data[29] = tvert[2];	

  vertex_buffer_data[30] = tvert[12];   //5
  vertex_buffer_data[31] = tvert[13];
  vertex_buffer_data[32] = tvert[14];	

  vertex_buffer_data[33] = tvert[21];   //8
  vertex_buffer_data[34] = tvert[22];
  vertex_buffer_data[35] = tvert[23];

     //Triangle 5
  vertex_buffer_data[36] = tvert[0];   //1
  vertex_buffer_data[37] = tvert[1];
  vertex_buffer_data[38] = tvert[2];	

  vertex_buffer_data[39] = tvert[3];   //2
  vertex_buffer_data[40] = tvert[4];
  vertex_buffer_data[41] = tvert[5];	

  vertex_buffer_data[42] = tvert[15];   //6
  vertex_buffer_data[43] = tvert[16];
  vertex_buffer_data[44] = tvert[17];

  //Triangle 6
  vertex_buffer_data[45] = tvert[0];   //1
  vertex_buffer_data[46] = tvert[1];
  vertex_buffer_data[47] = tvert[2];	

  vertex_buffer_data[48] = tvert[12];   //5
  vertex_buffer_data[49] = tvert[13];
  vertex_buffer_data[50] = tvert[14];	

  vertex_buffer_data[51] = tvert[15];   //6
  vertex_buffer_data[52] = tvert[16];
  vertex_buffer_data[53] = tvert[17];

    //Triangle 7
  vertex_buffer_data[54] = tvert[3];   //2
  vertex_buffer_data[55] = tvert[4];
  vertex_buffer_data[56] = tvert[5];	

  vertex_buffer_data[57] = tvert[6];   //3
  vertex_buffer_data[58] = tvert[7];
  vertex_buffer_data[59] = tvert[8];	

  vertex_buffer_data[60] = tvert[18];   //7
  vertex_buffer_data[61] = tvert[19];
  vertex_buffer_data[62] = tvert[20];

      //Triangle 8
  vertex_buffer_data[63] = tvert[3];   //2
  vertex_buffer_data[64] = tvert[4];
  vertex_buffer_data[65] = tvert[5];	

  vertex_buffer_data[66] = tvert[15];   //6
  vertex_buffer_data[67] = tvert[16];
  vertex_buffer_data[68] = tvert[17];	

  vertex_buffer_data[69] = tvert[18];   //7
  vertex_buffer_data[70] = tvert[19];
  vertex_buffer_data[71] = tvert[20];

        //Triangle 9
  vertex_buffer_data[72] = tvert[9];   //4
  vertex_buffer_data[73] = tvert[10];
  vertex_buffer_data[74] = tvert[11];	

  vertex_buffer_data[75] = tvert[6];   //3
  vertex_buffer_data[76] = tvert[7];
  vertex_buffer_data[77] = tvert[8];	

  vertex_buffer_data[78] = tvert[18];   //7
  vertex_buffer_data[79] = tvert[19];
  vertex_buffer_data[80] = tvert[20];

  		//Triangle 10
  vertex_buffer_data[81] = tvert[9];   //4
  vertex_buffer_data[82] = tvert[10];
  vertex_buffer_data[83] = tvert[11];	

  vertex_buffer_data[84] = tvert[21];   //8
  vertex_buffer_data[85] = tvert[22];   
  vertex_buffer_data[86] = tvert[23];   

  vertex_buffer_data[87] = tvert[18];   //7
  vertex_buffer_data[88] = tvert[19];
  vertex_buffer_data[89] = tvert[20];

  //Triangle 11
  vertex_buffer_data[90] = tvert[12];   //5
  vertex_buffer_data[91] = tvert[13];
  vertex_buffer_data[92] = tvert[14];	

  vertex_buffer_data[93] = tvert[15];   //6
  vertex_buffer_data[94] = tvert[16];
  vertex_buffer_data[95] = tvert[17];	

  vertex_buffer_data[96] = tvert[18];   //7
  vertex_buffer_data[97] = tvert[19];
  vertex_buffer_data[98] = tvert[20];

  //Triangle 12
  vertex_buffer_data[99] = tvert[12];   //5
  vertex_buffer_data[100] = tvert[13];
  vertex_buffer_data[101] = tvert[14];	

  vertex_buffer_data[102] = tvert[21];   //8
  vertex_buffer_data[103] = tvert[22];
  vertex_buffer_data[104] = tvert[23];	

  vertex_buffer_data[105] = tvert[18];   //7
  vertex_buffer_data[106] = tvert[19];
  vertex_buffer_data[107] = tvert[20];

  for(int i = 0; i < 36 * 3; i+=3){
 	color_buffer_data[i] = color[0];
 	color_buffer_data[i + 1] = color[1];
 	color_buffer_data[i + 2] = color[2];
 }

 for(int i = 0; i < 36 * 2; i+=2){
  texture_buffer_data[i] = 0;
  texture_buffer_data[i+1] = 0;
 }
                                //Cube coords   //vertex buffer coords
 texture_buffer_data[12] = 1;   //1             
 texture_buffer_data[13] = 0;

 texture_buffer_data[14] = 1;   //4
 texture_buffer_data[15] = 1;

 texture_buffer_data[16] = 0;   //8
 texture_buffer_data[17] = 1;

 texture_buffer_data[18] = 1;   //1
 texture_buffer_data[19] = 0;

 texture_buffer_data[20] = 0;   //5
 texture_buffer_data[21] = 0;

 texture_buffer_data[22] = 0;   //8
 texture_buffer_data[23] = 1;

 if(type == 1){
  //side 1
  //tri 1
  texture_buffer_data[0] = 1;
  texture_buffer_data[1] = 0;

  texture_buffer_data[2] = 1;
  texture_buffer_data[3] = 1;

  texture_buffer_data[4] = 0;
  texture_buffer_data[5] = 1;

  //tri 2
  texture_buffer_data[6] = 1;
  texture_buffer_data[7] = 0;

  texture_buffer_data[8] = 0;
  texture_buffer_data[9] = 0;

  texture_buffer_data[10] = 0;
  texture_buffer_data[11] = 1;

  //side2
  //tri 5
  texture_buffer_data[24] = 1;
  texture_buffer_data[25] = 0;

  texture_buffer_data[26] = 1;
  texture_buffer_data[27] = 1;

  texture_buffer_data[28] = 0;
  texture_buffer_data[29] = 1;

  //tri 6
  texture_buffer_data[30] = 1;
  texture_buffer_data[31] = 0;

  texture_buffer_data[32] = 0;
  texture_buffer_data[33] = 0;

  texture_buffer_data[34] = 0;
  texture_buffer_data[35] = 1;

  //side 3
  //tri 9
  texture_buffer_data[48] = 1;
  texture_buffer_data[49] = 0;

  texture_buffer_data[50] = 1;
  texture_buffer_data[51] = 1;

  texture_buffer_data[52] = 0;
  texture_buffer_data[53] = 1;

  //tri 10
  texture_buffer_data[54] = 1;
  texture_buffer_data[55] = 0;

  texture_buffer_data[56] = 0;
  texture_buffer_data[57] = 0;

  texture_buffer_data[58] = 0;
  texture_buffer_data[59] = 1;

    //side 4
  //tri 11
  texture_buffer_data[60] = 1;
  texture_buffer_data[61] = 0;

  texture_buffer_data[62] = 1;
  texture_buffer_data[63] = 1;

  texture_buffer_data[64] = 0;
  texture_buffer_data[65] = 1;

  //tri 12
  texture_buffer_data[66] = 1;
  texture_buffer_data[67] = 0;

  texture_buffer_data[68] = 0;
  texture_buffer_data[69] = 0;

  texture_buffer_data[70] = 0;
  texture_buffer_data[71] = 1;

  


 }

 visible = true;
 sliding = false;

 vaobj = create3DTexturedObject(GL_TRIANGLES, 36, vertex_buffer_data, texture_buffer_data, textureID, GL_FILL);

}

Cuboid::~Cuboid(){
  delete[] vertex_buffer_data;
  delete[] color_buffer_data;
  delete[] texture_buffer_data;
}

void undergoSliding(){
  static float factor = 0.5f;
  for(int i = 0; i < NUM_TILES_ROW * NUM_TILES_COL; i++){
    if(tilesList[i]->isSliding()){
      tilesList[i]->y += factor;
      if(tilesList[i]->y > Cuboid::UPPER_LIMIT || tilesList[i]->y < Cuboid::LOWER_LIMIT)factor *= -1.0f;
    }
  }
}

void Cuboid::draw(){
  glm::mat4 MVP;
  mtx->model = glm::mat4(1.0f);
  glm::mat4 translateCube = glm::translate(glm::vec3(x, y, z));  
  //glm::mat4 rotateCube = glm::rotate((float)(cube_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  mtx->model *= (translateCube);
  MVP =  mtx->projection * mtx->view * mtx->model; // MVP = p * V * M
  //  Don't change unless you are sure!!
  glUniformMatrix4fv(mtx->TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);
  draw3DTexturedObject(vaobj);
}

void Cuboid::setVisible(bool value){
  this->visible = value;
}


void Cuboid::setSliding(bool value){
  this->sliding = value;
}

bool Cuboid::isVisible(){
  return this->visible;
}

bool Cuboid::isSliding(){
  return this->sliding;
}

void Cuboid::setPosition(float x, float y, float z){
  this->x = x;
  this->y = y;
  this->z = z;
}

float Cuboid::getPosX(){
  return x;
}

float Cuboid::getPosY(){
  return y;
}

float Cuboid::getPosZ(){
  return z;
}

float Cuboid::getMinX(){
  return x - width/2.0f;
}

float Cuboid::getMinY(){
  return y - height/2.0f;
}

float Cuboid::getMinZ(){
  return z - length/2.0f;
}

float Cuboid::getMaxX(){
  return x + width/2.0f;
}

float Cuboid::getMaxY(){
  return y + height/2.0f;
}

float Cuboid::getMaxZ(){
  return z + length/2.0f;
}


float Cuboid::getWidth(){
  return width;
}

float Cuboid::getLength(){
  return length;
}

float Cuboid::getHeight(){
  return height;
}

bool Cuboid::checkCollision(Cuboid &cb){
  if(getMinX() <= cb.getMaxX() && getMaxX() >= cb.getMinX() &&
     getMinY() <= cb.getMaxY() && getMaxY() >= cb.getMinY() &&
     getMinZ() <= cb.getMaxZ() && getMaxZ() >= cb.getMinZ()
    )return true;
  else return false;
}

Player::Player(GLMatrices *mtx, float x, float y, float z){
  float *colorCube = new float[3];
  colorCube[0] = 0;
  colorCube[1] = 1;//0.412;
  colorCube[2] = 1;//0.270;
  GLuint textureId = createTexture("box.png");
  // check for an error during the load process
  if(textureId == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;
  cb = new Cuboid(mtx, textureId, colorCube, x, y, z, 4.0f, 4.0f, 4.0f, 1);
  barrel = new Cuboid(mtx, textureId, colorCube, x, y, z, 1.0f, 7.0f, 1.0f, 1);
  groundY = y;
  speedX = 1.0f;
  speedY = 6.0f;
  jumpTime = 0.0f;
  dynamic = false;
  move_down = false;
  move_up = false;
  move_left = false;
  move_right = false;
  inAir = false;
  delete[] colorCube;

}

void Player::jump(){
  if(!inAir){
    inAir = true;
    jumpTime = 0.0f;
  }
}
  
void Player::draw(){
  cb->draw();
  barrel->draw();
}

void Player::setPosition(float x, float y, float z){
  cb->setPosition(x, y, z);
  barrel->setPosition(x, y, z);
}

void Player::applyForces(float timeInstance){
  float tx = getPosX();
  float ty = getPosY();
  float tz = getPosZ();
  float width = TILE_WIDTH;
  float length = TILE_LENGTH;
  int tileRowNum = (int)floor(tx/width);
  int tileColNum = (int)floor(tz/length);
  int tileIndex = tileColNum * NUM_TILES_COL + tileRowNum;
  if(tileIndex>=0 && tileIndex < NUM_TILES_ROW*NUM_TILES_COL && !tilesList[tileIndex]->isVisible() && !inAir){
    cout<<"Standing on an empty tile"<<endl;
    setPosition(0.0f,groundY,0.0f);
    return;
  }
  cout<<"Row Num: "<<tileRowNum<<endl;
  cout<<"Col Num: "<<tileColNum<<endl;
  cout<<"Index: "<<tileIndex<<endl;
  if(dynamic){
    //cout<<"Force applied"<<endl;
    if(move_up)
      tz -= speedX;
    if(move_down)
      tz += speedX;
    if(move_right)
      tx += speedX;
    if(move_left)
      tx -= speedX;
    setPosition(tx, ty, tz);
  }
  if(inAir){
    jumpTime += timeInstance;
    ty += speedY * jumpTime - (0.5 * GRAVITY * jumpTime *jumpTime);
    setPosition(tx, ty, tz);
    if(ty <= groundY){
      ty = groundY;
      setPosition(tx, ty, tz);
      jumpTime = 0.0f;
      inAir = false;
    }
  }

}

float Player::getPosX(){
  return cb->getPosX();
}

float Player::getPosY(){
  return cb->getPosY();
}

float Player::getPosZ(){
  return cb->getPosZ();
}

void Player::setDynamic(bool value){
  dynamic = value;
  if(!value){
    move_left = move_right = move_down = move_up = false;
  }
}

void Player::enableMoveLeft(){
  move_left = true;
}

void Player::enableMoveRight(){
  move_right = true;
}

void Player::enableMoveUp(){
  move_up = true;
}

void Player::enableMoveDown(){
  move_down = true;
}

Villain::Villain(GLMatrices *mtx, float x, float y, float z){
  float *colorCube = new float[3];
  colorCube[0] = 0;
  colorCube[1] = 1;//0.412;
  colorCube[2] = 1;//0.270;
  GLuint textureId = createTexture("oandb.png");
  // check for an error during the load process
  if(textureId == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;
  cb = new Cuboid(mtx, textureId, colorCube, x, y, z, 2.0f, 2.0f, 2.0f, 1);
  delete[] colorCube;
}

void Villain::draw(){
  cb->draw();
}
float Villain::getPosX(){
  return cb->getPosX();
}

float Villain::getPosY(){
  return cb->getPosY();

}
float Villain::getPosZ(){
  return cb->getPosZ();
}

Bonus::Bonus(GLMatrices *mtx, float x, float y, float z){
  float *colorCube = new float[3];
  colorCube[0] = 0;
  colorCube[1] = 1;//0.412;
  colorCube[2] = 1;//0.270;
  GLuint textureId = createTexture("gold.png");
  // check for an error during the load process
  if(textureId == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;
  cb = new Cuboid(mtx, textureId, colorCube, x, y, z, 2.0f, 2.0f, 2.0f, 1);
  delete[] colorCube;
}

void Bonus::draw(){
  if(visible)
    cb->draw();
}

float Bonus::getPosX(){
  cb->getPosX();
}

float Bonus::getPosY(){
  cb->getPosY();
}

float Bonus::getPosZ(){
  cb->getPosZ();
}

bool Bonus::isVisible(){
  return visible;
}

void Bonus::setVisible(bool value){
  this->visible = value;
}

bool checkCollisionVillain(Villain &v){
  return p->cb->checkCollision(*(v.cb));
}

void simulateCollisionVillain(){
  p->setPosition(0.0f,6.0f,0.0f);
}

void handleCollisionVillain(){
  for(int i = 0; i < villainList.size(); i++){
    if(checkCollisionVillain(*villainList[i]))
      {
        cout<<"Collision happened:Villain"<<endl;
        simulateCollisionVillain();
      }
  }
}

bool checkCollisionBonus(Bonus &b){
   if(b.visible)
    return p->cb->checkCollision(*(b.cb));
  else return false;
}

void simulateCollisionBonus(Bonus &b){
  b.setVisible(false);
}

void handleCollisionBonus(){
  for(int i = 0; i < bonusList.size(); i++){
    if(checkCollisionBonus(*bonusList[i]))
      {
        cout<<"Collision happened: Bonus"<<endl;
        simulateCollisionBonus(*bonusList[i]);
      }
  }

}



/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
          cout<<"Key release"<<endl;
          case GLFW_KEY_LEFT:
                p->setDynamic(false);
                break;
          case GLFW_KEY_RIGHT:
              p->setDynamic(false);
              break;
          case GLFW_KEY_UP:
              p->setDynamic(false);
              break;
          case GLFW_KEY_DOWN:
              p->setDynamic(false);
              break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
          cout<<"Key pressed"<<endl;
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_LEFT:
                p->setDynamic(true);
                p->enableMoveLeft();
                break;
            case GLFW_KEY_RIGHT:
                p->setDynamic(true);
                p->enableMoveRight();
                break;
            case GLFW_KEY_UP:
                p->setDynamic(true);
                p->enableMoveUp();
                break;
            case GLFW_KEY_DOWN:
                p->setDynamic(true);
                p->enableMoveDown();
                break;
            case GLFW_KEY_SPACE:
                p->jump();
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

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
                triangle_rot_dir *= -1;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                rectangle_rot_dir *= -1;
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

	// set the projection matrix as perspective
	//glMatrixMode (GL_PROJECTION);
	 //  glLoadIdentity ();
	  // gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); 
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
     Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
  // Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *triangle, *rectangle, *cube;

// Creates the triangle object used in this sample code
void createTriangle ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    0, 1,0, // vertex 0
    -1,-1,0, // vertex 1
    1,-1,0, // vertex 2
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 0
    0,1,0, // color 1
    0,0,1, // color 2
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

// Creates the rectangle object used in this sample code
void createRectangle ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -1.2,-1,0, // vertex 1
    1.2,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -1.2,-1,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createCube()
{
	static const GLfloat vertex_buffer_data[] = {

     -1.0f,-1.0f,-1.0f, // triangle 1 : begin

     -1.0f,-1.0f, 1.0f,

     -1.0f, 1.0f, 1.0f, // triangle 1 : end

     1.0f, 1.0f,-1.0f, // triangle 2 : begin

     -1.0f,-1.0f,-1.0f,

     -1.0f, 1.0f,-1.0f, // triangle 2 : end

     1.0f,-1.0f, 1.0f,

     -1.0f,-1.0f,-1.0f,

     1.0f,-1.0f,-1.0f,

     1.0f, 1.0f,-1.0f,

     1.0f,-1.0f,-1.0f,

     -1.0f,-1.0f,-1.0f,

     -1.0f,-1.0f,-1.0f,

     -1.0f, 1.0f, 1.0f,

     -1.0f, 1.0f,-1.0f,

     1.0f,-1.0f, 1.0f,

     -1.0f,-1.0f, 1.0f,

     -1.0f,-1.0f,-1.0f,

     -1.0f, 1.0f, 1.0f,

     -1.0f,-1.0f, 1.0f,

     1.0f,-1.0f, 1.0f,

     1.0f, 1.0f, 1.0f,

     1.0f,-1.0f,-1.0f,

     1.0f, 1.0f,-1.0f,

     1.0f,-1.0f,-1.0f,

     1.0f, 1.0f, 1.0f,

     1.0f,-1.0f, 1.0f,

     1.0f, 1.0f, 1.0f,

     1.0f, 1.0f,-1.0f,

     -1.0f, 1.0f,-1.0f,

     1.0f, 1.0f, 1.0f,

     -1.0f, 1.0f,-1.0f,

     -1.0f, 1.0f, 1.0f,

     1.0f, 1.0f, 1.0f,

     -1.0f, 1.0f, 1.0f,

     1.0f,-1.0f, 1.0f

 };

  static const GLfloat color_buffer_data[] = {

     0.583f,  0.771f,  0.014f,

     0.609f,  0.115f,  0.436f,

     0.327f,  0.483f,  0.844f,

     0.822f,  0.569f,  0.201f,

     0.435f,  0.602f,  0.223f,

     0.310f,  0.747f,  0.185f,

     0.597f,  0.770f,  0.761f,

     0.559f,  0.436f,  0.730f,

     0.359f,  0.583f,  0.152f,

     0.483f,  0.596f,  0.789f,

     0.559f,  0.861f,  0.639f,

     0.195f,  0.548f,  0.859f,

     0.014f,  0.184f,  0.576f,

     0.771f,  0.328f,  0.970f,

     0.406f,  0.615f,  0.116f,

     0.676f,  0.977f,  0.133f,

     0.971f,  0.572f,  0.833f,

     0.140f,  0.616f,  0.489f,

     0.997f,  0.513f,  0.064f,

     0.945f,  0.719f,  0.592f,

     0.543f,  0.021f,  0.978f,

     0.279f,  0.317f,  0.505f,

     0.167f,  0.620f,  0.077f,

     0.347f,  0.857f,  0.137f,

     0.055f,  0.953f,  0.042f,

     0.714f,  0.505f,  0.345f,

     0.783f,  0.290f,  0.734f,

     0.722f,  0.645f,  0.174f,

     0.302f,  0.455f,  0.848f,

     0.225f,  0.587f,  0.040f,

     0.517f,  0.713f,  0.338f,

     0.053f,  0.959f,  0.120f,

     0.393f,  0.621f,  0.362f,

     0.673f,  0.211f,  0.457f,

     0.820f,  0.883f,  0.371f,

     0.982f,  0.099f,  0.879f

 };

 static GLfloat color_buffer_data2[36*3];
 for(int i = 0; i < 36 * 3; i+=3){
 	color_buffer_data2[i] = 1.0f;
 	color_buffer_data2[i + 1] = 153/255.0f;
 	color_buffer_data2[i + 2] = 51/255.0f;
 }

 cube = create3DObject(GL_TRIANGLES, 12 * 3, vertex_buffer_data, color_buffer_data, GL_FILL);

}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;

void createScene(){
  int i,j;
  GLuint textureId = createTexture("tile1.png");
  // check for an error during the load process
  if(textureId == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;
  time_t t;
  srand((unsigned) time(&t));

  GLuint textureWaterId = createTexture("water2.png");
  // check for an error during the load process
  if(textureWaterId == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;

  float *colorCube = new float[3];
  colorCube[0] = 0;
  colorCube[1] = 1;//0.412;
  colorCube[2] = 1;//0.270;
  float posX , posZ , width = TILE_WIDTH, height = TILE_HEIGHT, length = TILE_LENGTH;
  Cuboid *temp_cuboid;
  posX = posZ = 0.0f;
  for(i = 0; i < NUM_TILES_ROW; i++){
    for(j = 0; j < NUM_TILES_COL; j++){
      temp_cuboid = new Cuboid(&Matrices, textureId, colorCube, posX, 0.0f, posZ, length, width, height, 0);
      tilesList.push_back(temp_cuboid);
      posX += width;
      if(rand() % 5 ==0 && tilesList.size()!=1)temp_cuboid->setVisible(false);
      //else if(rand() % 10 == 4)temp_cuboid->setSliding(true);
    }
    posX = 0.0f;
    posZ += length;
  }



  //Water area bottom
  posX = 0.0f - 3*width;
  posZ = 0.0f - length;
  for(i = 0; i < NUM_TILES_ROW; i++){
    for(j = 0; j < NUM_TILES_COL + 6; j++){
      temp_cuboid = new Cuboid(&Matrices, textureWaterId, colorCube, posX, 0.0f, posZ, length, width, height, 0);
      waterList.push_back(temp_cuboid);
      posX += width;
    }
    posX = 0.0f - 3*width;
    posZ -= length;
  }

  //Water area top
  posX = 0.0f - 3*width;
  posZ = 0.0f + length * (NUM_TILES_ROW+1);
  for(i = 0; i < NUM_TILES_ROW; i++){
    for(j = 0; j < NUM_TILES_COL + 6; j++){
      temp_cuboid = new Cuboid(&Matrices, textureWaterId, colorCube, posX, 0.0f, posZ, length, width, height, 0);
      waterList.push_back(temp_cuboid);
      posX += width;
    }
    posX = 0.0f - 3*width;
    posZ += length;
  }

  //Water area left
  posX = 0.0f - width;
  posZ = 0.0f;
  for(i = 0; i < 3; i++){
    for(j = 0; j < NUM_TILES_ROW + 1; j++){
      temp_cuboid = new Cuboid(&Matrices, textureWaterId, colorCube, posX, 0.0f, posZ, length, width, height, 0);
      waterList.push_back(temp_cuboid);
      posZ += length;
    }
    posZ = 0.0f;
    posX -= width;
  }

  //Water area right
  posX = 0.0f + width * (NUM_TILES_COL);
  posZ = 0.0f;
  for(i = 0; i < 3; i++){
    for(j = 0; j < NUM_TILES_ROW + 1; j++){
      temp_cuboid = new Cuboid(&Matrices, textureWaterId, colorCube, posX, 0.0f, posZ, length, width, height, 0);
      waterList.push_back(temp_cuboid);
      posZ += length;
    }
    posZ = 0.0f;
    posX += width;
  }



  delete[] colorCube;
}

void drawScene(){
  int i;
  for(i = 0; i < NUM_TILES_ROW * NUM_TILES_COL; i++){
    if(tilesList[i]->isVisible())tilesList[i]->draw();
  }
  for(i = 0; i < waterList.size(); i++){
    waterList[i]->draw();
  }
  for(i = 0; i < villainList.size(); i++){
    villainList[i]->draw();
  }
  for(i = 0; i < bonusList.size(); i++){
    bonusList[i]->draw();
  }

}

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  //glUseProgram (programID);
    glUseProgram(textureProgramID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 9, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  //glm::vec3 eye ( p->getPosX() + 6.0f, p->getPosY() + 6.0f, p->getPosZ() + 6.0f);
    //glm::vec3 eye ( p->getPosX() , p->getPosY() + 10, p->getPosZ());
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  //glm::vec3 target (0, 0, 0);
  glm::vec3 target ( p->getPosX(), p->getPosY(), p->getPosZ());
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  //Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model


  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);

  /* Render your scene */


/*
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateCube = glm::translate (glm::vec3(2, 0, 0));  
  //glm::mat4 rotateCube = glm::rotate((float)(cube_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateCube);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(cube);
*/

  //cb->draw();
  drawScene();
  p->draw();



  // Increment angles
  float increments = 1;

  camera_rotation_angle++; // Simulating camera rotation
  triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
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

    window = glfwCreateWindow(width, height, "The Box Machine", NULL, NULL);

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

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
  // Load Textures
  // Enable Texture0 as current texture memory
  glActiveTexture(GL_TEXTURE0);
  // load an image file directly as a new OpenGL texture
  // GLuint texID = SOIL_load_OGL_texture ("beach.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_TEXTURE_REPEATS); // Buggy for OpenGL3
  GLuint textureIdTile = createTexture("tile1.png");
  // check for an error during the load process
  if(textureIdTile == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;

  GLuint textureIdWater = createTexture("water1.png");
  // check for an error during the load process
  if(textureIdWater == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;



  // Create and compile our GLSL program from the texture shaders
  textureProgramID = LoadShaders( "TextureRender.vert", "TextureRender.frag" );
  // Get a handle for our "MVP" uniform
  Matrices.TexMatrixID = glGetUniformLocation(textureProgramID, "MVP");
    /* Objects should be created before any other gl function and shaders */
	// Create the models
	//createCube();
  float *colorCube = new float[3];
  colorCube[0] = 0;
  colorCube[1] = 1;//0.412;
  colorCube[2] = 1;//0.270;
  //Cuboid(GLMatrices *mtx, GLuint textureID, float *color, float x, float y, float z, float length, float width, float height);
  //cb = new Cuboid(&Matrices, textureIdTile, colorCube, 0.0f, 0.0f, 0.0f, 5.0f, 5.0f, 8.0f, 0);
  createScene();

  Villain *v1 = new Villain(&Matrices, 10.0f, 6.0f, 10.0f);
  villainList.push_back(v1);

  Bonus *b1 = new Bonus(&Matrices, 20.0f, 6.0f, 30.0f);
  bonusList.push_back(b1);

  p = new Player(&Matrices, 0.0f, 6.0f, 0.0f);

//Player::Player(GLMatrices *mtx, 
  //float x, float y, float z){

	
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	//glDepthFunc (GL_LEQUAL);
	glDepthFunc(GL_LESS);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 600;
	int height = 600;

    GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw();

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.05f) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
            undergoSliding();
            p->applyForces(0.05f);
            handleCollisionVillain();
            handleCollisionBonus();
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
