#include <iostream>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
#include "common/utils.hpp"
#include <common/controls.hpp>

using namespace std;

// Global Variables
GLFWwindow* window;
static const int window_width = 800;
static const int window_height = 600;
static const int n_points = 200; // Minimum 2
static const float m_scale = 5;
unsigned int nIndices;
bool glPolygonModeState = false; // State for wireframe mode

// VAO
GLuint VertexArrayID;
// Buffers for VAO
GLuint vertexbuffer;
GLuint uvbuffer;
GLuint normalbuffer;
GLuint elementbuffer;
// Texture IDs
GLuint heightmapID;
GLuint rockDiffuseID;
GLuint rockSpecularID;
GLuint rockNormalID;
GLuint grassDiffuseID;
GLuint grassSpecularID;
GLuint grassNormalID;
GLuint snowDiffuseID;
GLuint snowSpecularID;
GLuint snowNormalID;
// Shader program ID
GLuint programID;

// Light direction and height map scale - Global
glm::vec3 lightDir = glm::normalize(glm::vec3(0, -0.15, 1)); // Light source direction
float heightMapScaleValue = 0.000002f; // Heightmap scaling

// Functions for cleaning resources
void UnloadShaders();
void UnloadTextures();
void UnloadModel();

// Function prototypes for shader and model loading
void LoadShaders(GLuint& program, const char* vertex_file_path, const char* fragment_file_path, const char* tcsPath = nullptr, const char* tesPath = nullptr);
void LoadTextures();
void LoadModel();

// Additional function prototypes
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void RotateLightDirection(int key);
void AdjustHeightMapScaling(int key);

//Clean shader program
void UnloadShaders()
{
	//Responsible for deleting previously created shader programs
	glDeleteProgram(programID);
}

//Clean up loaded texture resources
void UnloadTextures()
{
	//Delete the texture object and release the GPU resources associated with it
	glDeleteTextures(1, &snowNormalID);
	glDeleteTextures(1, &snowSpecularID);
	glDeleteTextures(1, &snowDiffuseID);
	glDeleteTextures(1, &grassNormalID);
	glDeleteTextures(1, &grassSpecularID);
	glDeleteTextures(1, &grassDiffuseID);
	glDeleteTextures(1, &rockNormalID);
	glDeleteTextures(1, &rockSpecularID);
	glDeleteTextures(1, &rockDiffuseID);
	glDeleteTextures(1, &heightmapID);
}

//Used to clean up model-related resources
void UnloadModel()
{
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
}

//Read the shader code from the specified path, compile it, and check whether the compilation was successful
bool readAndCompileShader(const char* shader_path, const GLuint& id)
{
	//Read shader code
	string shaderCode;
	ifstream shaderStream(shader_path, std::ios::in);
	if (shaderStream.is_open()) {
		stringstream sstr;
		sstr << shaderStream.rdbuf();
		shaderCode = sstr.str();
		shaderStream.close();
	}
	else
	{
		cout << "Impossible to open " << shader_path << ". Are you in the right directory ? " << endl;
		return false;
	}

	//Compile shader
	cout << "Compiling shader :" << shader_path << endl;
	char const* sourcePointer = shaderCode.c_str();
	glShaderSource(id, 1, &sourcePointer, NULL);
	glCompileShader(id);

	//Check the compilation results
	GLint Result = GL_FALSE;
	int InfoLogLength;
	glGetShaderiv(id, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		vector<char> shaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(id, InfoLogLength, NULL, &shaderErrorMessage[0]);
		cout << &shaderErrorMessage[0] << endl;
	}

	//Returns whether compilation is successful
	cout << "Compilation of Shader: " << shader_path << " " << (Result == GL_TRUE ? "Success" :
		"Failed!") << endl;
	return Result == 1;
}

//Used to load, compile and link vertex and fragment shaders
void LoadShaders(GLuint& program, const char* vertex_file_path, const char* fragment_file_path, const char* tcsPath, const char* tesPath)
{
	// Create the shaders - tasks 1 and 2
	//Create shader object
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	//Reading and compiling shaders
	bool vok = readAndCompileShader(vertex_file_path, VertexShaderID);
	bool fok = readAndCompileShader(fragment_file_path, FragmentShaderID);

	bool tcok = false, teok = false;
	GLuint TCShaderID, TEShaderID;
	if (tcsPath && tesPath) { // if we use tessellation shader
		TCShaderID = glCreateShader(GL_TESS_CONTROL_SHADER); 
		TEShaderID = glCreateShader(GL_TESS_EVALUATION_SHADER); 
		tcok = readAndCompileShader(tcsPath, TCShaderID); 
		teok = readAndCompileShader(tesPath, TEShaderID); 
	}


	//linker
	if (vok && fok && (!tcsPath || tcok) && (!tesPath || teok)) {
		GLint Result = GL_FALSE;
		int InfoLogLength;
		cout << "Linking program" << endl;
		program = glCreateProgram();
		glAttachShader(program, VertexShaderID);
		glAttachShader(program, FragmentShaderID);
		if (tcok && teok) { 
			// if we use tessellation shader and no compilling issue, we link them to the program
			glAttachShader(program, TCShaderID); 
			glAttachShader(program, TEShaderID); 
		}
		glLinkProgram(program);
		glGetProgramiv(program, GL_LINK_STATUS, &Result);
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0) {
			std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
			glGetProgramInfoLog(program, InfoLogLength, NULL, &ProgramErrorMessage[0]);
			cout << &ProgramErrorMessage[0];
		}
		std::cout << "Linking program: " << (Result == GL_TRUE ? "Success" : "Failed!") <<
			std::endl;
	}
	else
	{
		std::cout << "Program will not be linked: one of the shaders has an error" <<
			std::endl;
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);
	
	if(tcok) glDeleteShader(TCShaderID);
	if(teok) glDeleteShader(TEShaderID);
	
}


void LoadTextures()
{
	// height map loads BMP images
	int width, height;
	unsigned char* data = nullptr;
	loadBMP_custom("mountains_height.bmp", width, height, data);

	//Create and bind textures
	glGenTextures(1, &heightmapID);
	glBindTexture(GL_TEXTURE_2D, heightmapID);

	//Set texture and upload data
    //Set the pixel storage mode to ensure that the image data is correctly aligned
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	//Clear previously allocated image data memory
	delete[] data;

	//Set texture parameters
    //Wrap mode refers to how the texture handles texture coordinates outside the range [0, 1]
    //The wrapping mode in the horizontal direction is: use the edge pixels of the texture to fill out-of-range coordinates
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//The wrapping mode in the vertical direction is: use the edge pixels of the texture to fill out-of-range coordinates
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//Set the texture's magnification filter to: Select the color of the nearest texture element as the output color
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//Set the texture reduction filter to: trilinear filtering = mipmap linear interpolation
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	// generate Mipmap
	glGenerateMipmap(GL_TEXTURE_2D);

	//Unbind texture
	glBindTexture(GL_TEXTURE_2D, -1);

	// rocks diffuse loads and sets the "rock" diffuse texture in OpenGL
    //Load BMP pictures
	data = nullptr;
	loadBMP_custom("rocks.bmp", width, height, data);

	//Create and bind textures
	glGenTextures(1, &rockDiffuseID);
	glBindTexture(GL_TEXTURE_2D, rockDiffuseID);

	//Set up textures and upload data
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	//Clean image data memory
	delete[] data;

	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	//Generate mipmap
	glGenerateMipmap(GL_TEXTURE_2D);

	//Unbind texture
	glBindTexture(GL_TEXTURE_2D, -1);

	// rocks specular
	data = nullptr;
	loadBMP_custom("rocks-r.bmp", width, height, data);

	glGenTextures(1, &rockSpecularID);
	glBindTexture(GL_TEXTURE_2D, rockSpecularID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	//The function before delete transfers data from cpu to gpu
	delete[] data;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, -1);

	// rocks normal
	data = nullptr;
	loadBMP_custom("rocks-n.bmp", width, height, data);

	glGenTextures(1, &rockNormalID);
	glBindTexture(GL_TEXTURE_2D, rockNormalID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, -1);

	// grass diffuse
	data = nullptr;
	loadBMP_custom("grass.bmp", width, height, data);

	glGenTextures(1, &grassDiffuseID);
	glBindTexture(GL_TEXTURE_2D, grassDiffuseID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, -1);

	// grass specular
	data = nullptr;
	loadBMP_custom("grass-r.bmp", width, height, data);

	glGenTextures(1, &grassSpecularID);
	glBindTexture(GL_TEXTURE_2D, grassSpecularID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, -1);

	// grass normal
	data = nullptr;
	loadBMP_custom("grass-n.bmp", width, height, data);

	glGenTextures(1, &grassNormalID);
	glBindTexture(GL_TEXTURE_2D, grassNormalID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, -1);

	// snow diffuse
	data = nullptr;
	loadBMP_custom("snow.bmp", width, height, data);

	glGenTextures(1, &snowDiffuseID);
	glBindTexture(GL_TEXTURE_2D, snowDiffuseID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, -1);

	// snow specular
	data = nullptr;
	loadBMP_custom("snow-r.bmp", width, height, data);

	glGenTextures(1, &snowSpecularID);
	glBindTexture(GL_TEXTURE_2D, snowSpecularID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, -1);

	// snow normal
	data = nullptr;
	loadBMP_custom("snow-n.bmp", width, height, data);

	glGenTextures(1, &snowNormalID);
	glBindTexture(GL_TEXTURE_2D, snowNormalID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, -1);
}

//Loading models using vertex buffer objects and element buffer objects
void LoadModel()
{
	//Creation of vertex, UV and index data
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<unsigned int> indices;
	//Calculate the x and z coordinates of each point through nested loops, set the y coordinate to 0, and assume it is a plane
	for (int i = 0; i < n_points; i++)
	{
		float x = (m_scale) * ((i / float(n_points - 1)) - 0.5f) * 2.0f;
		for (int j = 0; j < n_points; j++)
		{
			float z = (m_scale) * ((j / float(n_points - 1)) - 0.5f) * 2.0f;
			vertices.push_back(vec3(x, 0, z));
			uvs.push_back(vec2(float(i + 0.5f) / float(n_points - 1),
				float(j + 0.5f) / float(n_points - 1)));
		}
	}
	//Enable primitive restart: a mechanism that allows restarting the primitive drawing sequence when drawing
    // Because it allows you to use the same index array to draw multiple separate primitives in a single draw call. This reduces the number of draw calls, thus improving performance.
	glEnable(GL_PRIMITIVE_RESTART);
	constexpr unsigned int restartIndex = std::numeric_limits<std::uint32_t>::max();
 	//When OpenGL encounters this value when drawing the index array, it will end the current primitive drawing and start drawing a new primitive from the next index.
    //Then the function of the above function is to run the function after you reach the specified index, allowing you to restart the above process from the first point of the third line.
	glPrimitiveRestartIndex(restartIndex);

	//Index array used to generate the grid
	int n = 0;
	// row
	for (int i = 0; i < n_points - 1; i++)
	{
		// line
		for (int j = 0; j < n_points; j++)
		{
			//Calculate and add index
			unsigned int topLeft = n;
			unsigned int bottomLeft = topLeft + n_points;
			indices.push_back(bottomLeft);
			indices.push_back(topLeft);
			n++;
		}
		//Tells OpenGL that the current primitive ends and the next primitive is about to begin
		indices.push_back(restartIndex);
	}

	//Create and set up vertex array objects and vertex buffer objects to store and manage vertex data
    //Create and bind VAO
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	//Enable vertex attribute array
	glEnableVertexAttribArray(0);

	//Create and bind VBO
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

	//Uploading vertex data to the GPU The data will be stored in the vertex buffer The size of the data transferred to the buffer will not change during draw operations
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0],
		GL_STATIC_DRAW);

	//Specify the data format and location of the vertex attribute array
	glVertexAttribPointer(
		0, // attribute
		3, // size (we have x,y,z)
		GL_FLOAT, // type of each individual element
		GL_FALSE, // normalized?
		0, // stride
		(void*)0 // array buffer offset
	);

	//Set vertex attributes and buffers for rendering
    //Enable vertex attribute array
	glEnableVertexAttribArray(1);
	//Generate a new buffer object
	glGenBuffers(1, &uvbuffer);
	//Bind buffer to specified type
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	//Fill buffer data
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
	//Set vertex attribute pointer
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	// Generate a buffer for the indices as well
	glGenBuffers(1, &elementbuffer);
	//bind index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	//Fill index data into buffer
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0],
		GL_STATIC_DRAW);
	//Set the number of indexes
	nIndices = indices.size();
}

//Steps to set up the OpenGL environment and create a rendering window
bool initializeGL()
{
	// Initialise GLFW
	if (!glfwInit())
	{
		cerr << "Failed to initialize GLFW" << endl;
		return false;
	}

	//Set window properties
	glfwWindowHint(GLFW_SAMPLES, 1); //no anti-aliasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy;forward compatible
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//core mode

	//Create window
	window = glfwCreateWindow(window_width, window_height, "OpenGLRenderer", NULL, NULL);

	if (window == NULL)
	{
		cerr << "Failed to open GLFW window. If you have an Intel GPU, they may not be 4.5 compatible." << endl;
		glfwTerminate();
		return false;
	}

	//Set the context of the current window
	glfwMakeContextCurrent(window);


	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK)
	{
		cerr << "Failed to initialize GLEW" << endl;
		glfwTerminate();
		return false;
	}

	//Check OpenGL debug output support
	if (!GLEW_ARB_debug_output)
		return false;

	//Set input mode
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);//Enable Sticky Keys so that key presses are detected even after they are released
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);//Used to hide and lock the mouse cursor to the GLFW window
	//Update window state and set initial cursor position
	glfwPollEvents();//Handle window events such as keyboard input and mouse movement
	glfwSetCursorPos(window, window_width / 2, window_height / 2);//Sets the cursor position to the center of the window.

	return true;
}

// Key callback function
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// Handle both press and repeat actions
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		switch (key) {
		case GLFW_KEY_SPACE:
			if (action == GLFW_PRESS) { // Only toggle on a fresh press
				glPolygonMode(GL_FRONT_AND_BACK, glPolygonModeState ? GL_FILL : GL_LINE);
				glPolygonModeState = !glPolygonModeState;
			}
			break;

		case GLFW_KEY_R:
			// Reload shaders - maybe only on press to avoid too frequent reloads
			if (action == GLFW_PRESS) {
				LoadShaders(programID, "Basic.vert", "Texture.frag", "dLod.tesc", "dLod.tese");
			}
			break;

		case GLFW_KEY_W:
		case GLFW_KEY_S:
		case GLFW_KEY_A:
		case GLFW_KEY_D:
			RotateLightDirection(key);
			break;

		case GLFW_KEY_T:
		case GLFW_KEY_G:
			AdjustHeightMapScaling(key);
			break;

		case GLFW_KEY_ESCAPE:
			if (action == GLFW_PRESS) {
				glfwSetWindowShouldClose(window, GLFW_TRUE);
			}
			break;

		default:
			break;
		}
	}
}

// Function to rotate light direction based on key input
void RotateLightDirection(int key) {
	float speed = 0.6f;
	glm::vec3 rotationAxis;

	switch (key) {
	case GLFW_KEY_W:
		rotationAxis = glm::vec3(1, 0, 0);
		break;
	case GLFW_KEY_S:
		rotationAxis = glm::vec3(-1, 0, 0);
		break;
	case GLFW_KEY_A:
		rotationAxis = glm::vec3(0, 1, 0);
		break;
	case GLFW_KEY_D:
		rotationAxis = glm::vec3(0, -1, 0);
		break;
	default:
		return;
	}

	lightDir = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(speed), rotationAxis) * glm::vec4(lightDir, 0.0f));
}

// Function to adjust height map scaling based on key input
void AdjustHeightMapScaling(int key) {
	float heightTransitionSpeed = 0.00000006f;

	if (key == GLFW_KEY_T) {
		heightMapScaleValue += heightTransitionSpeed;
	}
	else if (key == GLFW_KEY_G) {
		heightMapScaleValue -= heightTransitionSpeed;
	}

	heightMapScaleValue = std::min(0.000006f, std::max(0.0f, heightMapScaleValue));
}

//Render and control 3D graphics
int main()
{
	// Initialize the OpenGL environment
	if (!initializeGL())
		return -1;

	// Load models and textures
	LoadModel();
	LoadTextures();

	// Create and load shader programs
	programID = glCreateProgram();
	LoadShaders(programID, "Basic.vert", "Texture.frag", "dLod.tesc", "dLod.tese");

	// Register key callback
	glfwSetKeyCallback(window, KeyCallback);

	//Set rendering state
	glClearColor(0.7f, 0.8f, 1.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	// Set rendering state
	do {
		// Clear the screen and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();

		// Set and compute MVP matrix
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// we will add more code here
		// First pass: Base mesh
		// Use shader program
		glUseProgram(programID);
		// Get a handle for our uniforms and set MVP matrix uniform
		GLuint MatrixID = glGetUniformLocation(programID, "MVP");
		// Pass the previously calculated MVP matrix to the shader
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// Bind model matrix and set model matrix uniform
		GLuint modelMatrix = glGetUniformLocation(programID, "Model");
		// Transform object from model space to world space
		glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, &ModelMatrix[0][0]);

		// Bind height map texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, heightmapID);
		GLuint heightMapSampler = glGetUniformLocation(programID, "heightMapSampler");
		glUniform1i(heightMapSampler, 0);

		// Uniform: height map scale, retrieve the uniform location of heightMapScale, and pass the value of heightMapScaleValue to the shader
		GLuint heightMapScale = glGetUniformLocation(programID, "heightMapScale");
		glUniform1f(heightMapScale, heightMapScaleValue);

		// Uniform: number of vertices
		GLuint numOfVertices = glGetUniformLocation(programID, "numOfVertices");
		glUniform1i(numOfVertices, n_points);

		// Uniform: light direction (WCS)
		GLuint lightDir_wcs = glGetUniformLocation(programID, "lightDir_wcs");
		glUniform3f(lightDir_wcs, lightDir.x, lightDir.y, lightDir.z);

		// Uniform: view position (WCS)
		auto camPos = getCameraPosition();
		GLuint viewPos_wcs = glGetUniformLocation(programID, "viewPos_wcs");
		glUniform3f(viewPos_wcs, camPos.x, camPos.y, camPos.z);

		// Bind rock diffuse texture
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, rockDiffuseID);
		GLuint rockDiffSampler = glGetUniformLocation(programID, "rockDiffSampler");
		glUniform1i(rockDiffSampler, 1);

		// Bind rock specular texture
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, rockSpecularID);
		GLuint rockSpecSampler = glGetUniformLocation(programID, "rockSpecSampler");
		glUniform1i(rockSpecSampler, 2);

		// Bind rock normal texture
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, rockNormalID);
		GLuint rockNormSampler = glGetUniformLocation(programID, "rockNormSampler");
		glUniform1i(rockNormSampler, 3);

		// Bind grass diffuse texture
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, grassDiffuseID);
		GLuint grassDiffSampler = glGetUniformLocation(programID, "grassDiffSampler");
		glUniform1i(grassDiffSampler, 4);

		// Bind grass specular texture
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, grassSpecularID);
		GLuint grassSpecSampler = glGetUniformLocation(programID, "grassSpecSampler");
		glUniform1i(grassSpecSampler, 5);

		// Bind grass normal texture
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, grassNormalID);
		GLuint grassNormSampler = glGetUniformLocation(programID, "grassNormSampler");
		glUniform1i(grassNormSampler, 6);

		// Bind snow diffuse texture
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, snowDiffuseID);
		GLuint snowDiffSampler = glGetUniformLocation(programID, "snowDiffSampler");
		glUniform1i(snowDiffSampler, 7);

		// Bind snow specular texture
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, snowSpecularID);
		GLuint snowSpecSampler = glGetUniformLocation(programID, "snowSpecSampler");
		glUniform1i(snowSpecSampler, 8);

		// Bind snow normal texture
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, snowNormalID);
		GLuint snowNormSampler = glGetUniformLocation(programID, "snowNormSampler");
		glUniform1i(snowNormSampler, 9);

		// Render vertex data
		glDrawElements(
			GL_TRIANGLE_STRIP, // mode
			(GLsizei)nIndices, // count
			GL_UNSIGNED_INT, // type
			(void*)0 // element array buffer offset
		);
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents(); // Ensure the OpenGL application can respond to user interaction
	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0); // Check if ESC key is not pressed and there are no requests to close the window, continue looping

	UnloadModel();
	UnloadShaders();
	UnloadTextures();
	glfwTerminate(); // Release model, shader, and texture resources

	return 0;
}
