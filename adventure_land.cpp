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

#include <SFML/Audio.hpp>

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


class FTGLFont{
public:
	FTGLFont(GLMatrices *mtx, float* color, char* fontfile, char *word, float size, float x, float y, float z, float scaleFactor);
	~FTGLFont();
	void draw();
	void setWord(char* word);
private:
	GLMatrices *mtx;
	FTFont* font;
	GLuint fontMatrixID;
	GLuint fontColorID;
	float scaleFactor;
	float x;
	float y;
	float z;
  	glm::vec3 fontColor; 
  	char* fontfile;
  	char* word;
};

class Cuboid{
public:
	Cuboid(GLMatrices *mtx, GLuint textureID, float *color, float x, float y, float z, float length, float width, float height, int type);
  ~Cuboid();
  void draw();
  void setPosition(float x, float y, float z);
  void setX(float value);
  void setY(float value);
  void setZ(float value);
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
  float getAngle();
  void setAngle(float angle);
  void setVisible(bool value);
  void setEmpty(bool value);
  void setSliding(bool value);
  void setLastKey(char value);
  bool isVisible();
  bool isSliding();
  bool isEmpty();
  bool checkCollision(Cuboid &cb);
  //friend bool checkCollisionMovingTile(Cuboid &cbd);
  friend void undergoSliding();
private:
  GLfloat *vertex_buffer_data;
  GLfloat *color_buffer_data;
  GLfloat *texture_buffer_data;
  GLuint textureID;
  GLMatrices *mtx;
  VAO *vaobj;
  float initX;
  float initY;
  float initZ;
  float x;
  float y;
  float z;
  float length;
  float width;
  float height;
  bool empty;
  bool visible;
  bool sliding;
  glm::vec3 axis;
  float angle;
  static const float UPPER_LIMIT = 15.0f;
  static const float LOWER_LIMIT = -20.0f;
};

class Player{
public:
  Player(GLMatrices *mtx, float x, float y, float z);
  void setPosition(float x, float y, float z);
  void setX(float value);
  void setY(float value);
  void setZ(float value);
  void draw();
  void applyForces();
  void setDynamic(bool value);
  void jump();
  void applyForces(float timeInstance);
  void enableMoveLeft();
  void enableMoveRight();
  void enableMoveUp();
  void enableMoveDown();
  void barrelLeft();
  void barrelRight();
  int getScore();
  void increaseSpeed();
  void decreaseSpeed();
  void incrementScore();
  void decrementLife();
  void setLastKey(char value);
  int getStandingTileIndex();
  float getAngle();
  float getPosX();
  float getPosY();
  float getPosZ();
  float getHeadX();
  float getHeadY();
  float getHeadZ();
  char getLastKey();
  float getHeight();
  float getWidth();
  float getLength();
  friend bool checkCollisionVillain(Villain &v);
  friend void simulateCollisionVillain();
  friend void handleCollisionVillain();

  friend bool checkCollisionBonus(Bonus &b);
  friend void simulateCollisionBonus();
  friend void handleCollisionBonus();

  friend bool checkCollisionMovingTile(Cuboid &cbd);
  friend void simulateCollisionMovingTile();
  friend void handleCollisionMovingTile();

  friend void checkWinCollision();
private:
  Cuboid *cb;
  Cuboid *barrel;
  float speedX;
  float speedY;
  float jumpTime;
  float groundY;
  float headX;
  float headY;
  float headZ;
  bool move_left;
  bool move_right;
  bool move_up;
  bool move_down;
  bool dynamic;
  bool inAir;
  bool falling;
  bool onSlider;
  float fallTime;
  int score;
  int life;
  char lastKey;
  Cuboid *sliderTile;
  static const float GRAVITY = 20.0f;
};

class Villain{
public:
  Villain(GLMatrices *mtx, float x, float y, float z, bool dynamic = false);
  void draw();
  float getPosX();
  float getPosY();
  float getPosZ();
  void applyForces(float timeInstance);
  bool getVisible();
  void setAlive(bool value);
  bool getAlive();
  friend bool checkCollisionVillain(Villain &v);
  friend void simulateCollisionVillain();
  friend void handleCollisionVillain();

  friend void handleCollisionBullet();
private:
  Cuboid *cb;
  bool visible;
  bool dynamic;
  bool alive;
  float time;
  float speed;
  float switchTime;
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

class Bullet{
public:
  Bullet(GLMatrices *mtx, float x, float y, float z, float ux, float uz);
  void applyForces(float timeInstance);
  void draw();
  void fire();

  friend void handleCollisionBullet();
private:
  Cuboid *cb;
  float ux;
  float uz;
  bool visible;
  float speed;
};

Cuboid *cb;
Cuboid *winBlock;
Player *p;
vector<Cuboid*> tilesList;
vector<Cuboid*> waterList;
vector<Villain*> villainList;
vector<Bonus*> bonusList;
int viewMode;
float camPosX,camPosY;
float zoomFactor;
float cameraRotationAngle;
bool panFlag;
float prevX;
float prevY;
float prevCamPosX;
float prevCamPosY;
bool looseFlag;
bool winFlag;
int lives;
Bullet *bt;
FTGLFont *f1;
int score;
sf::SoundBuffer bonusBuffer;
sf::SoundBuffer villainBuffer;
sf::Sound sound;

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
  this->initX = x;
  this->initZ = z;
  this->initY = y;
  this->x = x;
  this->y = y;
  this->z = z;
  this->length = length;
  this->width = width;
  this->height = height;
  this->axis = glm::vec3(0.0f,1.0f,0.0f);
  this->angle = 0.0f;
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

 empty = false;
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
  int tempIdx;
  for(int i = 0; i < NUM_TILES_ROW * NUM_TILES_COL; i++){
    if(tilesList[i]->isSliding()){
    	//cout<<"Cuboid:: Upper Limit"<<Cuboid::UPPER_LIMIT<<endl;
    	//cout<<"Cuboid:: Lower Limit"<<Cuboid::LOWER_LIMIT<<endl;
    	//cout<<"Sliding Factor "<<factor<<endl;
    	//cout<<"Y value = "<<tilesList[i]->y<<endl;
      tilesList[i]->y += factor;
      tempIdx = i;
    }
  }
  if(tilesList[tempIdx]->y >= Cuboid::UPPER_LIMIT)
	  factor = -0.5f;
  else if(tilesList[tempIdx]->y <= Cuboid::LOWER_LIMIT)
	  factor = 0.5f;
}

void Cuboid::setX(float value){
	x = value;
}
void Cuboid::setY(float value){
	y = value;
}
void Cuboid::setZ(float value){
	z = value;
}

void Cuboid::draw(){
  glm::mat4 MVP;
  mtx->model = glm::mat4(1.0f);
  glm::mat4 translateCube = glm::translate(glm::vec3(x, y, z)); 
  glm::mat4 t1 = glm::translate(glm::vec3(initX, initY, initZ)); 
  glm::mat4 rotateCube = glm::rotate((float)(angle*M_PI/180.0f), axis); // rotate about vector (-1,1,1)
  mtx->model *= ( translateCube * rotateCube );
  MVP =  mtx->projection * mtx->view * mtx->model; // MVP = p * V * M
  //  Don't change unless you are sure!!
  glUniformMatrix4fv(mtx->TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);
  draw3DTexturedObject(vaobj);
}

void Cuboid::setAngle(float angle){
  this->angle = angle;
}

float Cuboid::getAngle(){
  return angle;
}


void Cuboid::setVisible(bool value){
  this->visible = value;
}


void Cuboid::setSliding(bool value){
  this->sliding = value;

}

void Cuboid::setEmpty(bool value){
	this->empty = value;
}

bool Cuboid::isVisible(){
  return this->visible;
}

bool Cuboid::isSliding(){
  return this->sliding;
}

bool Cuboid::isEmpty(){
	return this->empty;
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

FTGLFont::FTGLFont(GLMatrices *mtx, float* color, char* fontfile, char* word,float size, float x, float y, float z, float scaleFactor)
{
	cout<<"Entered ftgl"<<endl;
	this->mtx = mtx;
	cout<<"mtx made"<<endl;
	fontColor = glm::vec3(color[0], color[1], color[2]);
	cout<<"vec color made"<<endl;
	this->fontfile = new char[20];
	strcpy(this->fontfile, fontfile);
	this->word = new char[100];
	strcpy(this->word, word);
	cout<<"str copied"<<endl;
	this->x = x;
	this->y = y;
	this->z = z;
	this->scaleFactor = scaleFactor;
	
	cout<<" Above this->font"<<endl;
	this->font = new FTExtrudeFont(fontfile); // 3D extrude style rendering
	if(this->font->Error())
	{
		cout << "Error: Could not load font `" << fontfile << "'" << endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// Create and compile our GLSL program from the font shaders
	
	this->fontMatrixID = glGetUniformLocation(fontProgramID, "MVP");
	this->fontColorID = glGetUniformLocation(fontProgramID, "fontColor");

	this->font->ShaderLocations(fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform);
	this->font->FaceSize(size);
	this->font->Depth(0);
	this->font->Outset(0, 0);
	this->font->CharMap(ft_encoding_unicode);
}

void FTGLFont::draw(){
  glm::mat4 MVP;

  Matrices.view = glm::lookAt(glm::vec3(0,25,25), glm::vec3(25,2,2), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
	// Transform the text
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText = glm::translate(glm::vec3(x,y,z));
	glm::mat4 scaleText = glm::scale(glm::vec3(scaleFactor,scaleFactor,scaleFactor));
	Matrices.model *= (translateText * scaleText);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(this->fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(this->fontColorID, 1, &fontColor[0]); 

	// Render font
	this->font->Render(word);
}

void FTGLFont::setWord(char* word){
	strcpy(this->word, word);
}

Player::Player(GLMatrices *mtx, float x, float y, float z){
  float *colorCube = new float[3];
  colorCube[0] = 0;
  colorCube[1] = 1;//0.412;
  colorCube[2] = 1;//0.270;
  GLuint textureId = createTexture("box.png");
  this->score = 0;
  this->life = 3;
  // check for an error during the load process
  if(textureId == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;
  cb = new Cuboid(mtx, textureId, colorCube, x, y, z, 4.0f, 4.0f, 4.0f, 1);
  barrel = new Cuboid(mtx, textureId, colorCube, x , y, z , 1.0f, 7.0f, 1.0f, 1);
  groundY = y;
  speedX = 1.0f;
  speedY = 6.0f;
  headX = getPosX() + getWidth()/2.0f;
  headY = getPosY() + getHeight()/2.0f;
  headZ = getPosZ();
  jumpTime = 0.0f;
  dynamic = false;
  move_down = false;
  move_up = false;
  move_left = false;
  move_right = false;
  inAir = false;
  falling = false;
  fallTime = 0.0f;
  onSlider = false;
  lastKey = 'T';
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
  barrel->setPosition(x , y, z);
}

float Player::getHeadX(){
	return headX;
}


float Player::getHeadY(){
	return headY;
}

char Player::getLastKey(){
	return lastKey;
}


float Player::getHeadZ(){
	return headZ;
}

void Player::setLastKey(char value){
	lastKey = value;
}

void Player::barrelLeft(){
  float currentAngle = barrel->getAngle();
  currentAngle += 2.0f;
  barrel->setAngle(currentAngle);
}

float Player::getAngle(){
	return barrel->getAngle();
}

void Player::barrelRight(){
  float currentAngle = barrel->getAngle();
  currentAngle -= 2.0f;
  barrel->setAngle(currentAngle);
}

int Player::getScore(){
  return score;
}

int Player::getStandingTileIndex(){
  float tx = getPosX();
  float ty = getPosY();
  float tz = getPosZ();
  float width = TILE_WIDTH;
  float length = TILE_LENGTH;
  int tileRowNum = (int)floor(tx/width);
  int tileColNum = (int)floor(tz/length);
  int tileIndex = tileColNum * NUM_TILES_COL + tileRowNum;
  if(tileIndex >= 0 && tileIndex < NUM_TILES_ROW*NUM_TILES_COL)
  	return tileIndex;
  else 
  	return -1;
}

void Player::applyForces(float timeInstance){
  float tileX,tileZ;
  float tx = getPosX();
  float ty = getPosY();
  float tz = getPosZ();

  static float initFallY,finalFallY;
  static bool fallFlag = false;

  static float initAirY,finalAirY;
  static bool airFlag = false;

  float edgeXLow = tx - cb->getWidth()/2.0f;
  float edgeXHigh = tx + cb->getWidth()/2.0f;
  float edgeZLow = tz - cb->getLength()/2.0f;
  float edgeZHigh = tz + cb->getLength()/2.0f;

  if(edgeXLow < -2.5f)tx = 0.0f;
  else if(edgeXHigh > 47.5f)tx = 47.5f - cb->getWidth()/2.0f ;

  if(edgeZLow < -2.5f)tz = 0.0f;
  else if(edgeZHigh > 47.5f)tz = 47.5f - cb->getLength()/2.0f;

  setPosition(tx, ty, tz);

  int tileIndex = getStandingTileIndex();
  //cout<<"Tile -> "<<tileIndex<<endl;
  if(tileIndex == -1)return;
  if(tileIndex != -1 && tilesList[tileIndex]->isEmpty() && !inAir && !falling && !onSlider){
    //cout<<"Standing on an empty tile"<<endl;
    //setPosition(0.0f,groundY,0.0f);
    falling = true;
    //return;
  }
  if(onSlider){
  	//cout<<" -- ty = "<<ty<<endl;
  	ty = sliderTile->getPosY() + sliderTile->getHeight()/2.0f + cb->getHeight()/2.0f; 
  	setY(ty);
  	//cout<<" ++ ty = "<<ty<<endl;
  	tileX = sliderTile->getPosX();
  	tileZ = sliderTile->getPosZ();
  	if(abs(tx - tileX) > sliderTile->getWidth()/2.0f || abs(tz - tileZ) > sliderTile->getLength()/2.0f){
  		onSlider = false;
  		if(ty > 0.0f){
  			inAir = true;
  			speedY = -1.0f * 5.0f;
  		}
  		else{
  			falling = true;
  		}
  		sliderTile = NULL;
  		//cout<<"Made jump"<<endl;
  	} 

  }
  if(dynamic){
    //cout<<"Force applied"<<endl;
    if(move_up){
      tz -= speedX;
    }
    if(move_down){
      tz += speedX;
    }

    if(move_right){
      tx += speedX;
    }
    if(move_left){
      tx -= speedX;
    }
    //cout<<"Position Set"<<endl;
    //cout<<"X --> "<<tx<<endl;
    //cout<<"Z --> "<<tz<<endl;
    setX(tx);
    setZ(tz);
  }
  //setPosition(tx, ty, tz);
  if(inAir){
  	if(airFlag == false){
  		airFlag = true;
  		initAirY = getPosY();
  		cout<<"Init air "<<initAirY<<endl;
  	}
  	//cout<<"Entered in air"<<endl;
    jumpTime += timeInstance;
    //cout<<" ** -- ty = "<<ty<<" airSpeed* t = "<<speedY<<" 0.5at^2 = "<<(0.5 * GRAVITY * jumpTime *jumpTime)<<endl;
    ty += speedY * jumpTime - (0.5 * GRAVITY * jumpTime *jumpTime);
    //cout<<"jumpTime = "<<jumpTime<<"  ** ++ ty = "<<ty<<endl;
    setPosition(tx, ty, tz);
    //Segmentation fault will take place for index = -1, rectify afterwards
    if(p->cb->checkCollision(*tilesList[tileIndex])){


      //ty = groundY;
      //cout<<"Collided with -> "<<tileIndex<<endl;	
      ty = tilesList[tileIndex]->getPosY() + tilesList[tileIndex]->getHeight()/2.0f + cb->getHeight()/2.0f;
      setPosition(tx, ty, tz);
      finalAirY = getPosY();
      airFlag = false;
      cout<<"Final air y"<<finalAirY<<endl;
      if(abs(initAirY - finalAirY) >= 13.0f)
      	looseFlag = true;
      finalAirY = initAirY = 0.0f;	
      if(tilesList[tileIndex]->isSliding()){
      	onSlider = true;
      	sliderTile = tilesList[tileIndex];
      	cout<<" ******************* Made on slider true "<<endl;

      }
      jumpTime = 0.0f;
      inAir = false; 
      speedY = 6.0f;
    }
  }
  if(falling){
  		if(fallFlag == false){
  			fallFlag = true;
  			initFallY = getPosY();
  		}
  		cout<<"Entered falling"<<endl;
  		cout<<" Y pos is = "<<getPosY()<<endl;
	  	if(onSlider){
	  		sliderTile = tilesList[tileIndex];
	  		fallTime = 0.0f;
	  		falling = false;
	  	}
	  	fallTime += timeInstance;
	  	ty -= (0.5 * GRAVITY * fallTime * fallTime);
	  	setPosition(tx, ty, tz);
	  	//cout<<" Fall time = "<< fallTime<<endl;
	  	//cout<<"on slider - "<<onSlider<<endl;
	  	if(fallTime >= 1.3f){
	  		finalFallY = getPosY();
	  		tx = 0.0f;
	       	ty = groundY;
	       	tz = 0.0f;
	       	setPosition(tx, ty, tz);
	  		fallTime = 0.0f;
	  		falling = false;
	  		cout<<"Fall finished timeout"<<endl;
	  		fallFlag = false;
	  		if(abs(finalFallY - initFallY) >= 11.0f)
	  			looseFlag = true;
	  		finalFallY = initFallY = 0.0f;
	  	}
		else if(tileIndex != -1 && p->cb->checkCollision(*tilesList[tileIndex]) && tilesList[tileIndex]->isSliding() ){
			ty = tilesList[tileIndex]->getPosY() + tilesList[tileIndex]->getHeight()/2.0f + cb->getHeight()/2.0f;
			onSlider = true;
			sliderTile = tilesList[tileIndex];
			setPosition(tx, ty, tz);
	  		fallTime = 0.0f;
	  		falling = false;
	  		cout<<"Fall finished"<<endl;
	  		cout<<" Y pos is = "<<getPosY()<<endl;
	  	    finalFallY = getPosY();
	  		fallFlag = false;
	  		if(abs(finalFallY - initFallY) >= 11.0f)
	  			looseFlag = true;
	  		finalFallY = initFallY = 0.0f;
		}
  	}

  	//Setting the head camera
  	headX = getPosX();
  	headY = getPosY();
  	headZ = getPosZ();
  	headY += getHeight()/2.0f;
  	if(lastKey == 'T'){
  		headX += getWidth()/2.0f;  		
  	}
  	else if(lastKey == 'B'){
  		headX -= getWidth()/2.0f; 		
  	}
  	else if(lastKey == 'L'){
  		headZ -= getLength()/2.0f;
  	}
  	else if(lastKey == 'R'){
  		headZ += getLength()/2.0f;
  	}
  	

}

void Player::setX(float value){
	cb->setX(value);
	barrel->setX(value);
}
void Player::setY(float value){
	cb->setY(value);
	barrel->setY(value);
}
void Player::setZ(float value){
	cb->setZ(value);
	barrel->setZ(value);
}

void Player::incrementScore(){
  score++;
}

void Player::decrementLife(){
  life--;
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

float Player::getHeight(){
	return cb->getHeight();
}

float Player::getWidth(){
	return cb->getWidth();
}

float Player::getLength(){
	return cb->getLength();
}


void Player::setDynamic(bool value){
  dynamic = value;
  if(!value){
    move_left = move_right = move_down = move_up = false;
  }
}

void Player::increaseSpeed(){
  speedX += 1.0f;
  if(speedX > 5.0f)
    speedX = 5.0f;
}

void Player::decreaseSpeed(){
  speedX -= 1.0f;
  if(speedX < 0.0f)
    speedX = 0.0f;
}

void Player::enableMoveLeft(){
  move_left = true;
  //lastKey = 'L';
}

void Player::enableMoveRight(){
  move_right = true;
  //lastKey = 'R';
}

void Player::enableMoveUp(){
  move_up = true;
  //lastKey = 'T';
}

void Player::enableMoveDown(){
  move_down = true;
  //lastKey = 'B';
}

Villain::Villain(GLMatrices *mtx, float x, float y, float z, bool dynamic){
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
  this->dynamic = dynamic;
  this->visible = true;
  this->time = 0.0f;
  this->switchTime = 0.0f;
  this->speed = 5.0f;
  this->alive = true;
}

void Villain::setAlive(bool value){
	alive = value;
	visible = false;
}

bool Villain::getAlive(){
	return alive;
}

void Villain::applyForces(float timeInstance){
	float tx = cb->getPosX();
	float ty = cb->getPosY();
	float tz = cb->getPosZ();
	if(alive){
		if(dynamic){
			time += timeInstance;
			//cout<<"Time - "<<time<<endl;
			switchTime += timeInstance;
			if(time >= 15.0f)
			{
				//cout<<"Switched visibility "<<endl;
				visible = !visible;
				time = 0.0f;
			}
			if(switchTime >= 5.0f){
				speed *= -1.0f;
				switchTime = 0.0f;
			}
			
			tx += speed*timeInstance;
			cb->setPosition(tx,ty,tz);
		}

	}
	
}

void Villain::draw(){
  if(getVisible() && alive){
  	  cb->draw();
  }
}

float Villain::getPosX(){
  return cb->getPosX();
}

bool Villain::getVisible(){
	return visible;
}

float Villain::getPosY(){
  return cb->getPosY();

}
float Villain::getPosZ(){
  return cb->getPosZ();
}

Bonus::Bonus(GLMatrices *mtx, float x, float y, float z){
  float *colorCube = new float[3];
  visible = true;
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

Bullet::Bullet(GLMatrices *mtx, float x, float y, float z, float ux, float uz){
  float *colorCube = new float[3];
  colorCube[0] = 0;
  colorCube[1] = 1;//0.412;
  colorCube[2] = 1;//0.270;
  this->ux = ux;
  this->uz = uz;
  GLuint textureId = createTexture("lava.png");
  // check for an error during the load process
  if(textureId == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;
  cb = new Cuboid(mtx, textureId, colorCube, x, y, z, 1.0f, 1.5f, 1.0f, 1);
  speed = 10.0f;
  visible = false;
  delete[] colorCube;
}

void Bullet::applyForces(float timeInstance){
	float  tx = cb->getPosX();
	float ty = cb->getPosY();
	float tz = cb->getPosZ();
	float angle = p->getAngle() + 90.0f;
	uz = speed * cos(angle * M_PI/180.0f);
	ux = speed * sin(angle * M_PI/180.0f);
	if(visible){
		tx += timeInstance * ux;
		tz += timeInstance * uz;
		cb->setPosition(tx, ty, tz);
		if(abs(tx) > 200.0f || abs(tz) > 200.0f)
			visible = false;
	}
}

void Bullet::draw(){
	if(visible)
		cb->draw();
}

void Bullet::fire(){
	float tx = p->getPosX();
	float ty = p->getPosY();
	float tz = p->getPosZ();
	cb->setPosition(tx, ty, tz);
	visible = true;
}





bool checkCollisionVillain(Villain &v){
  return  (v.visible && p->cb->checkCollision(*(v.cb)) );
}

void simulateCollisionVillain(){
  sound.setBuffer(villainBuffer);
  sound.play();
  lives--;
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

void checkWinCollision(){
	if(p->cb->checkCollision(*winBlock))
		winFlag = true; 
}

bool checkCollisionBonus(Bonus &b){
   if(b.visible)
    return p->cb->checkCollision(*(b.cb));
  else return false;
}

void simulateCollisionBonus(Bonus &b){
  score++;	
  sound.setBuffer(bonusBuffer);
  sound.play();
  b.setVisible(false);
}

void handleCollisionBonus(){
  for(int i = 0; i < bonusList.size(); i++){
    if(checkCollisionBonus(*bonusList[i]))
      {
        //cout<<"Collision happened: Bonus"<<endl;
        simulateCollisionBonus(*bonusList[i]);
      }
  }
}

void handleCollisionBullet(){
	for(int i = 0; i < villainList.size(); i++){
		if(bt->cb->checkCollision( *(villainList[i]->cb) ) && villainList[i]->alive && villainList[i]->visible ){
			villainList[i]->visible = false;
			villainList[i]->alive = false;
			cout<<"Bullet HIT!!"<<endl;
		}
	}
}

bool checkCollisionMovingTile(Cuboid &cbd){
	return p->cb->checkCollision(cbd);
}

void simulateCollisionMovingTile(){
	float tx,ty,tz;
	int tileIndex = p->getStandingTileIndex();
	if(tileIndex != -1){
		tx = tilesList[tileIndex]->getPosX();
		ty = tilesList[tileIndex]->getPosY() + tilesList[tileIndex]->getHeight()/2.0f + p->cb->getHeight()/2.0f;
		tz = tilesList[tileIndex]->getPosZ();	
		//cout<<"Ty is "<<ty<<endl;
		p->setPosition(tx, ty, tz);
	}
}

void handleCollisionMovingTile(){
	for(int i = 0; i < tilesList.size(); i++){
		if(tilesList[i]->isSliding() && checkCollisionMovingTile(*tilesList[i]) && p->getPosY() < tilesList[i]->getHeight()/2.0f + p->cb->getHeight() && p->getPosY() > 0.0f){
			//cout<<"Collision Happened"<<endl;
			simulateCollisionMovingTile();
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
            case GLFW_KEY_UP:
                p->setDynamic(true);
                p->enableMoveRight();
                p->setLastKey('T');
                break;
            case GLFW_KEY_DOWN:
                p->setDynamic(true);
                p->enableMoveLeft();
                p->setLastKey('B');
                break;
            case GLFW_KEY_RIGHT:
                p->setDynamic(true);
                p->enableMoveDown();
                p->setLastKey('R');
                break;
            case GLFW_KEY_LEFT:
                p->setDynamic(true);
                p->enableMoveUp();
                p->setLastKey('L');
                break;
            case GLFW_KEY_SPACE:
                p->jump();
                break;
            case GLFW_KEY_L:
              p->barrelLeft();
              break;
            case GLFW_KEY_R:
              p->barrelRight();
              break;
            case GLFW_KEY_F:
              p->increaseSpeed();
              break;
            case GLFW_KEY_S:
              p->decreaseSpeed();
              break;
            case GLFW_KEY_F1:
              viewMode = 0;
              break;
            case GLFW_KEY_F2:
              viewMode = 1;
              break;
            case GLFW_KEY_F3:
              viewMode = 2;
              break;
            case GLFW_KEY_F4:
              viewMode = 3;
              break;
            case GLFW_KEY_F5:
              viewMode = 4;
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

static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	if(!panFlag){
		camPosX = xpos/600*60;
		camPosY = ypos/600*60;
		//cout<<"Xpos = "<<camPosX<<endl;
		//cout<<"Ypos = "<<camPosY<<endl;

	}

}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if(yoffset > 0){
		zoomFactor--;
	}
	else if(yoffset < 0){
		zoomFactor++;
	}
}

void checkPan(GLFWwindow* window){
	double xpos, ypos;
	if(panFlag){
		glfwGetCursorPos(window, &xpos, &ypos);
		if((float)ypos > prevY)cameraRotationAngle += 3.5f;
		else if((float)ypos < prevY)cameraRotationAngle -= 3.5f;
		prevY = (float)ypos;
	}
}


/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE){
            	bt->fire();
               
           
            }
            break;

        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_PRESS) {
                panFlag = true;
                prevCamPosX = camPosX;
                prevCamPosY = camPosY;
            }
            else if(action == GLFW_RELEASE){
            	panFlag = false;
            	//prevY = 0.0f;
            	camPosX = prevCamPosX;
            	camPosY = prevCamPosY;
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
      //if(rand() % 5 ==0 && tilesList.size()!=1)temp_cuboid->setVisible(false);
      //else if(rand() % 10 == 4)temp_cuboid->setSliding(true);
    }
    posX = 0.0f;
    posZ += length;
  }

  //Create holes
  tilesList[11]->setVisible(false);
  tilesList[22]->setVisible(false);
  tilesList[29]->setVisible(false);
  tilesList[33]->setVisible(false);
  tilesList[43]->setVisible(false);
  tilesList[48]->setVisible(false);
  tilesList[50]->setVisible(false);
  tilesList[73]->setVisible(false);
  tilesList[85]->setVisible(false);
  tilesList[92]->setVisible(false);
  tilesList[98]->setVisible(false);

  tilesList[11]->setEmpty(true);
  tilesList[22]->setEmpty(true);
  tilesList[29]->setEmpty(true);
  tilesList[33]->setEmpty(true);
  tilesList[43]->setEmpty(true);
  tilesList[48]->setEmpty(true);
  tilesList[50]->setEmpty(true);
  tilesList[73]->setEmpty(true);
  tilesList[85]->setEmpty(true);
  tilesList[92]->setEmpty(true);
  tilesList[98]->setEmpty(true);

  //Create Sliding tiles
  tilesList[5]->setSliding(true);
  tilesList[18]->setSliding(true);
  tilesList[37]->setSliding(true);

  tilesList[5]->setEmpty(true);
  tilesList[18]->setEmpty(true);
  tilesList[37]->setEmpty(true);


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
  posZ = 0.0f + length * (NUM_TILES_ROW);
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
    //cout<<"Bonus drawn:-> "<<i<<endl;
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

  glm::vec3 eye;
  glm::vec3 target;
  glm::vec3 up( 0, 1, 0);

  float offsetX;
  float offsetY;
  float offsetZ;


  if(viewMode == 0){
    eye = glm::vec3( p->getPosX() - 4.0f, p->getPosY() + 6.0f, p->getPosZ() - 4.0f);
    target = glm::vec3(p->getPosX(), p->getPosY(), p->getPosZ());
  }
  else if(viewMode == 1){
    eye = glm::vec3( -5, 23, -5);
    target = glm::vec3(25,0,25);
  }
  else if(viewMode == 2){
    eye = glm::vec3( TILE_WIDTH * NUM_TILES_ROW/2.0f , 25, TILE_LENGTH * NUM_TILES_COL/2.0f);
    target = glm::vec3(TILE_WIDTH * NUM_TILES_ROW/2.0f -1, 0, TILE_LENGTH * NUM_TILES_COL/2.0f - 1);
  }
  else if(viewMode == 3){
  	char lastKey = p->getLastKey(); 
  	if(lastKey == 'T'){
  		offsetX = 3.0f;
  	}
  	else if(lastKey == 'B'){
  		offsetX = -3.0f;
  	}
  	else if(lastKey == 'L'){
  		offsetZ = -3.0f;
  	}
  	else if(lastKey == 'R'){
  		offsetZ = 3.0f;
  	}
  	eye = glm::vec3(p->getHeadX(), p->getHeadY(), p->getHeadZ());
  	target = glm::vec3(p->getHeadX() + offsetX, p->getHeadY(), p->getHeadZ() + offsetZ);
  }
  else if(viewMode == 4){
  	//  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 3, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  	//eye = glm::vec3(camPosX, zoomFactor, camPosY);
  	eye = glm::vec3(camPosX, zoomFactor, camPosY);
  	target = glm::vec3(camPosX + 3*sin(cameraRotationAngle * M_PI/180.0f) , zoomFactor - 3*cos(cameraRotationAngle * M_PI/180.0f) , camPosY - 3);
  }



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
  winBlock->draw();
  p->draw();
  bt->draw();

  glUseProgram(fontProgramID);
  f1->draw();



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

    glfwSetCursorPosCallback(window, cursorPosCallback);

    glfwSetScrollCallback(window, scrollCallback);

    return window;
}

void applyForcesVillains(float timeInstance){
	for(int i = 0; i < villainList.size(); i++){
		villainList[i]->applyForces(timeInstance);
	}

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

  GLuint textureIdWin = createTexture("gift.png");
  // check for an error during the load process
  if(textureIdWin == 0 )
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

  winBlock = new Cuboid(&Matrices, textureIdWin, colorCube, 47.5f, 4.5f, 47.5f, 4.0f, 4.0f, 4.0f, 1);

  Villain *v1 = new Villain(&Matrices, 10.0f, 6.0f, 10.0f,true);
  villainList.push_back(v1);

  Villain *v2 = new Villain(&Matrices, 20.0f, 6.0f, 23.0f);
  villainList.push_back(v2);

  Villain *v3 = new Villain(&Matrices, 5.0f, 6.0f, 30.0f,true);
  villainList.push_back(v3);

  Bonus *b1 = new Bonus(&Matrices, 10.0f, 6.0f, 40.0f);
  bonusList.push_back(b1);

  Bonus *b2 = new Bonus(&Matrices, 40.0f, 6.0f, 40.0f);
  bonusList.push_back(b2);

  Bonus *b3 = new Bonus(&Matrices, 30.0f, 6.0f, 10.0f);
  bonusList.push_back(b3);

  p = new Player(&Matrices, 0.0f, 4.5f, 0.0f);

  //Bullet(GLMatrices *mtx, float x, float y, float z, float ux, float uz);
  bt = new Bullet(&Matrices, 0.0f, 4.5f, 0.0f, 0.0f, 0.0f);

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

	fontProgramID = LoadShaders( "fontrender.vert", "fontrender.frag" );

	fontVertexCoordAttrib = glGetAttribLocation(fontProgramID, "vertexPosition");
	fontVertexNormalAttrib = glGetAttribLocation(fontProgramID, "vertexNormal");
	fontVertexOffsetUniform = glGetUniformLocation(fontProgramID, "pen");

	float colArrayFont[3];
	colArrayFont[0] = 0;
	colArrayFont[1] = 0;
	colArrayFont[2] = 0;

	char fileString[20]; 
	strcpy(fileString, "kimberly.ttf");

	char wordName[50];
	strcpy(wordName, "Score");

	f1 = new FTGLFont(&Matrices, colArrayFont, fileString, wordName, 20.0f, 10.0f, 35.0f, -30.0f, 1.0f); 



    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 600;
	int height = 600;
	score = 0;
	looseFlag = winFlag = false;
	lives = 3;
	camPosX = 25.0f;
	camPosY = 25.0f;
	zoomFactor = 30.0f;
    viewMode = 0;
    cameraRotationAngle = 0.0f;

    if (!bonusBuffer.loadFromFile("bonus.ogg"))
    {
    	cout<<"Bonus sound not loaded";
    	return -1;
    }

    if (!villainBuffer.loadFromFile("villain.ogg"))
    {
    	cout<<"Villain sound not loaded";
    	return -1;
    }

    GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    char str[50];
    char strB[50];
	

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

    	if(winFlag){
    		cout<<"You won !! :-) "<<endl;
			quit(window);
    	}

    	if(looseFlag){
    		cout<<"You Loose :-( "<<endl;
    			quit(window);
    	}

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
            strcpy(strB,"Score:");
            undergoSliding();
            p->applyForces(0.05f);
            bt->applyForces(0.05f);
            applyForcesVillains(0.05f);
            handleCollisionMovingTile();
            handleCollisionVillain();
            handleCollisionBonus();
            handleCollisionBullet();
            checkWinCollision();
            checkPan(window);
            sprintf(str, "%d", score);   
  			strcat(strB,str);
 			f1->setWord(strB);
 			if(lives == -1)looseFlag = true;
        }
    }
    //cout<<"Your Score "<<p->getScore()<<endl;

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
