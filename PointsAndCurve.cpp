// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
/////////////
using namespace std;
#include <string.h>
#include <vector>
#include <iostream>

typedef struct Vertex {
	float XYZW[4];
	float RGBA[4];
	void SetCoords(float *coords) {
		XYZW[0] = coords[0];
		XYZW[1] = coords[1];
		XYZW[2] = coords[2];
		XYZW[3] = coords[3];
	}
	void SetCoords(float x, float y, float z, float w) {
		XYZW[0] = x;
		XYZW[1] = y;
		XYZW[2] = z;
		XYZW[3] = w;
	}
	void SetColor(float *color) {
		RGBA[0] = color[0];
		RGBA[1] = color[1];
		RGBA[2] = color[2];
		RGBA[3] = color[3];
	}
	void SetColor(float R, float G, float B, float A) {
		RGBA[0] = R;
		RGBA[1] = G;
		RGBA[2] = B;
		RGBA[3] = A;
	}
};

typedef struct point {
	float x, y, z;
	point(const float x = 0, const float y = 0, const float z = 0) : x(x), y(y), z(z) {};
	point(float *coords) : x(coords[0]), y(coords[1]), z(coords[2]) {};
	point operator -(const point& a)const {
		return point(x - a.x, y - a.y, z - a.z);
	}
	point operator +(const point& a)const {
		return point(x + a.x, y + a.y, z + a.z);
	}
	point operator *(const float& a)const {
		return point(x*a, y*a, z*a);
	}
	point operator /(const float& a)const {
		return point(x / a, y / a, z / a);
	}
	float* toXYZW() {
		float array[] = { x, y, z, 1.0f };
		return array;
	}
};

bool shouldDrawCRCurve = true;
bool shouldDrawCasteljau = true;

vector<unsigned short> shortList;
vector<Vertex> vertexList;

Vertex CRCurve[40];
unsigned short CRI[40];
Vertex deCastel[150];
unsigned short DeCastelI[150];
vector<Vertex> curveVertexList;
vector<unsigned short> curveShortList;
vector<Vertex> lineVertexList;
vector<unsigned short> lineShortList;

int startWindow(void);
void prepareOpenGL(void);
void createVAOs(Vertex[], unsigned short[], size_t, size_t, int);
void clearMemory(void);
void drawScene(Vertex[]);
static void mouseCallback(GLFWwindow*, int, int, int);
static void keyboardCallback(GLFWwindow*, int, int, int, int);
float randomNumZeroToOne(void);
void CRCurves(void);
void deCasteljau(void);
void CRCurvesAfterTenPoints(void);
void deCasteljauAfterTenPoints(void);

GLFWwindow* window;
const GLuint window_width = 1024, window_height = 768;

glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

GLuint programID;
GLuint pickingProgramID;

// ATTN: INCREASE THIS NUMBER AS YOU CREATE NEW OBJECTS
const GLuint NumObjects = 1;	// number of different "objects" to be drawn
GLuint VertexArrayId[NumObjects] = { 0 };
GLuint VertexBufferId[NumObjects] = { 0 };
GLuint IndexBufferId[NumObjects] = { 0 };
size_t NumVert[NumObjects] = { 0 };

GLuint MatrixID;
GLuint ViewMatrixID;
GLuint ModelMatrixID;
GLuint PickingMatrixID;
GLuint pickingColorArrayID;
GLuint pickingColorID;
GLuint LightID;

int main( void )
{
	startWindow();
	prepareOpenGL();

	for (int i = 0; i < 40; i++)
	{
		CRI[i] = i;
		DeCastelI[i] = i;
	}
	for (int i = 40; i < 150; i++)
	{
		DeCastelI[i] = i;
	}

	do {
		glClear(GL_COLOR_BUFFER_BIT);

		Vertex* v = &vertexList[0]; //Funciton is expecting an Array
		drawScene(v);
		
	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	clearMemory();

	return 0;
}

static void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		shouldDrawCRCurve = !shouldDrawCRCurve;
	}
	else if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		shouldDrawCasteljau = !shouldDrawCasteljau;
	}
}

static void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		Vertex newVertex;
		newVertex.SetCoords(-(float)xpos, (float)ypos, 0.0f, 1.0f);
		newVertex.SetColor(randomNumZeroToOne(), randomNumZeroToOne(), randomNumZeroToOne(), 1.0f);
		vertexList.push_back(newVertex);
		shortList.push_back((unsigned short)shortList.size());
		
		Vertex* v = &vertexList[0];
		unsigned short* i = &shortList[0];
		createVAOs(v, i, sizeof(Vertex)*vertexList.size(), sizeof(unsigned short)*shortList.size(), 0);

		if (vertexList.size() == 10) {
			CRCurves();
			deCasteljau();
			createVAOs(CRCurve, CRI, sizeof(CRCurve), sizeof(CRI), 1);
			createVAOs(deCastel, DeCastelI, sizeof(deCastel), sizeof(DeCastelI), 2);

			for (int i = 0; i < 40; i++) {
				curveVertexList.push_back(CRCurve[i]);
				curveShortList.push_back(CRI[i]);
			}
			for (int i = 0; i < 150; i++) {
				lineVertexList.push_back(deCastel[i]);
				lineShortList.push_back(DeCastelI[i]);
			}
		}
		else if (vertexList.size() > 10) {
			
			CRCurvesAfterTenPoints();

			Vertex* v2 = &curveVertexList[0];
			unsigned short* i2 = &curveShortList[0];
			createVAOs(v2, i2, sizeof(Vertex)*curveVertexList.size(), sizeof(unsigned short)*curveShortList.size(), 1);

			deCasteljauAfterTenPoints();

			Vertex* v3 = &lineVertexList[0];
			unsigned short* i3 = &lineShortList[0];
			createVAOs(v3, i3, sizeof(Vertex)*lineVertexList.size(), sizeof(unsigned short)*lineShortList.size(), 2);
		}
		
	}
	else if (action == GLFW_RELEASE) { // Actions 1 == mouse up
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
	}
}

void CRCurvesAfterTenPoints(void) { // recalculate end points and new points and not all the center points as that isnt nessicary

	point p0; // p_i
	point p1; // p_i+1 
	point p2; // final position
	point p3; // p_i-1
	point p4; // p_i+2
	float red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };

	Vertex newSpaceVer;
	for (int i = 0; i < 4; i++) {
		curveVertexList.push_back(newSpaceVer);
		curveShortList.push_back(curveShortList.size());
	}

	Vertex* v = &vertexList[0];
	int vSize = vertexList.size();
	
	//For index 0
	p0 = v[0].XYZW;
	p1 = v[1].XYZW;
	p3 = v[vSize - 1].XYZW;
	p4 = v[2].XYZW;

	p2 = p0 + (p1 - p3)*.2;
	curveVertexList[1].SetCoords(p2.toXYZW());
	curveVertexList[1].SetColor(red);

	p2 = p1 - (p4 - p0)*.2;
	curveVertexList[2].SetCoords(p2.toXYZW());
	curveVertexList[2].SetColor(red);

	p2 = v[1].XYZW;
	curveVertexList[3].SetCoords(p2.toXYZW());
	curveVertexList[3].SetColor(red);

	p2 = v[0].XYZW;
	curveVertexList[0].SetCoords(p2.toXYZW());
	curveVertexList[0].SetColor(red);
	
	for (int i = vSize - 3; i < vSize; i++) { // for end point (old ones since a new point has been added) and new end points.
		p0 = v[i].XYZW;

		if (i == vSize-1)
			p1 = v[0].XYZW;
		else
			p1 = v[i + 1].XYZW;

		p3 = v[i - 1].XYZW;

		if (i == vSize-2)
			p4 = v[0].XYZW;
		else if (i == vSize-1)
			p4 = v[1].XYZW;
		else
			p4 = v[i + 2].XYZW;

		p2 = v[i].XYZW;
		curveVertexList[i * 4].SetCoords(p2.toXYZW());
		curveVertexList[i * 4].SetColor(red);

		p2 = p0 + (p1 - p3)*.2;
		curveVertexList[i * 4 + 1].SetCoords(p2.toXYZW());
		curveVertexList[i * 4 + 1].SetColor(red);

		p2 = p1 - (p4 - p0)*.2;
		curveVertexList[i * 4 + 2].SetCoords(p2.toXYZW());
		curveVertexList[i * 4 + 2].SetColor(red);

		if (i != vSize-1)
			p2 = v[i + 1].XYZW;
		else
			p2 = v[0].XYZW;
		curveVertexList[i * 4 + 3].SetCoords(p2.toXYZW());
		curveVertexList[i * 4 + 3].SetColor(red);
	}

}

void deCasteljauAfterTenPoints(void) { // recalculate end points and new points and not all the center points as that isnt nessicary

	point p00, p01, p02, p03;
	point p10, p11, p12;
	point p20, p21;
	point p30;
	float green[4] = { 0.0f, 1.0f, 0.0f, 1.0f };

	Vertex newSpaceVer;
	for (int i = 1; i < 16; i++) {
		lineVertexList.push_back(newSpaceVer);
		lineShortList.push_back(lineShortList.size());
	}

	int vSize = curveVertexList.size();
	p00 = curveVertexList[0].XYZW;
	p01 = curveVertexList[1].XYZW;
	p02 = curveVertexList[2].XYZW;
	p03 = curveVertexList[3].XYZW;
	
	for (int j = 1; j < 16; j++) // recaluate first point
	{
		float t = float(j) / 15.0f;

		p10 = p00 * (1 - t) + p01 * t;
		p11 = p01 * (1 - t) + p02 * t;
		p12 = p02 * (1 - t) + p03 * t;
		
		p20 = p10 * (1 - t) + p11 * t;
		p21 = p11 * (1 - t) + p12 * t;

		p30 = p20 * (1 - t) + p21 * t;

		lineVertexList[(j - 1)].SetCoords(p30.toXYZW());
		lineVertexList[(j - 1)].SetColor(green);
	}

	for (int i = vSize-8; i < vSize; i += 4) // recaluate end point and new end point
	{
		p00 = curveVertexList[i].XYZW;
		p01 = curveVertexList[i + 1].XYZW;
		p02 = curveVertexList[i + 2].XYZW;
		p03 = curveVertexList[i + 3].XYZW;
		
		for (int j = 1; j < 16; j++)
		{
			float t = float(j) / 15.0f;

			p10 = p00 * (1 - t) + p01 * t;
			p11 = p01 * (1 - t) + p02 * t;
			p12 = p02 * (1 - t) + p03 * t;

			p20 = p10 * (1 - t) + p11 * t;
			p21 = p11 * (1 - t) + p12 * t;

			p30 = p20 * (1 - t) + p21 * t;

			lineVertexList[15 * (i / 4) + (j - 1)].SetCoords(p30.toXYZW());
			lineVertexList[15 * (i / 4) + (j - 1)].SetColor(green);
		}
		
	}
	
}

void CRCurves(void) // generate the points for the first 10 points
{
	point p0; // p_i
	point p1; // p_i+1 
	point p2; // final position
	point p3; // p_i-1
	point p4; // p_i+2
	float red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	Vertex* v = &vertexList[0];

	for (int i = 0; i < 10; i++) {
		
		p0 = v[i].XYZW;

		if (i == 9)
			p1 = v[0].XYZW;
		else
			p1 = v[i + 1].XYZW;

		if (i == 0)
			p3 = v[9].XYZW;
		else
			p3 = v[i - 1].XYZW;

		if (i == 8)
			p4 = v[0].XYZW;
		else if (i == 9)
			p4 = v[1].XYZW;
		else
			p4 = v[i + 2].XYZW;

		p2 = p0 + (p1 - p3)*.2;
		CRCurve[i * 4 + 1].SetCoords(p2.toXYZW());
		CRCurve[i * 4 + 1].SetColor(red);

		p2 = p1 - (p4 - p0)*.2;
		CRCurve[i * 4 + 2].SetCoords(p2.toXYZW());
		CRCurve[i * 4 + 2].SetColor(red);

		p2 = v[i].XYZW;
		CRCurve[i * 4].SetCoords(p2.toXYZW());
		CRCurve[i * 4].SetColor(red);

		if (i != 9)
			p2 = v[i + 1].XYZW;
		else
			p2 = v[0].XYZW;
		CRCurve[i * 4 + 3].SetCoords(p2.toXYZW());
		CRCurve[i * 4 + 3].SetColor(red);
	}
}

void deCasteljau(void) // generate the points for the first 10 points
{
	point p00, p01, p02, p03;
	point p10, p11, p12;
	point p20, p21;
	point p30;
	float green[4] = { 0.0f, 1.0f, 0.0f, 1.0f };

	for (int i = 0; i < 40; i += 4)
	{
		p00 = CRCurve[i].XYZW;
		p01 = CRCurve[i + 1].XYZW;
		p02 = CRCurve[i + 2].XYZW;
		p03 = CRCurve[i + 3].XYZW;

		for (int j = 1; j < 16; j++)
		{
			float t = float(j) / 15.0f;

			p10 = p00 * (1 - t) + p01 * t;
			p11 = p01 * (1 - t) + p02 * t;
			p12 = p02 * (1 - t) + p03 * t;

			p20 = p10 * (1 - t) + p11 * t;
			p21 = p11 * (1 - t) + p12 * t;

			p30 = p20 * (1 - t) + p21 * t;

			deCastel[15 * (i / 4) + (j - 1)].SetCoords(p30.toXYZW());
			deCastel[15 * (i / 4) + (j - 1)].SetColor(green);
		}
	}
}

float randomNumZeroToOne(void)
{
	return (float)rand() / (float)((unsigned)RAND_MAX + 1);
}

int startWindow(void) {

	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL 

																   // Open a window and create its OpenGL context
	window = glfwCreateWindow(window_width, window_height, "Points and Bezier Curves", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_FALSE);
	glfwSetCursorPos(window, window_width / 2, window_height / 2);
	glfwSetMouseButtonCallback(window, mouseCallback);
	glfwSetKeyCallback(window, keyboardCallback);
}

void prepareOpenGL(void) {
	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	//glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	gProjectionMatrix = glm::ortho(0.0f, 1024.0f, 768.0f, 0.0f, 0.0f, 100.0f); // In world coordinates

																			// Camera matrix
	gViewMatrix = glm::lookAt(
		glm::vec3(0, 0, -5), // Camera is at (0,0,10), in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	pickingProgramID = LoadShaders("Picking.vertexshader", "Picking.fragmentshader");

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ModelMatrixID = glGetUniformLocation(programID, "M");
	PickingMatrixID = glGetUniformLocation(pickingProgramID, "MVP");
	// Get a handle for our "pickingColorID" uniform
	pickingColorArrayID = glGetUniformLocation(pickingProgramID, "PickingColorArray");
	pickingColorID = glGetUniformLocation(pickingProgramID, "PickingColor");
	// Get a handle for our "LightPosition" uniform
	LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
}

void createVAOs(Vertex Vertices[], unsigned short Indices[], size_t BufferSize, size_t IdxBufferSize, int ObjectId) {

	NumVert[ObjectId] = IdxBufferSize / (sizeof GLubyte);

	GLenum ErrorCheckValue = glGetError();
	size_t VertexSize = sizeof(Vertices[0]);
	size_t RgbOffset = sizeof(Vertices[0].XYZW);

	// Create Vertex Array Object
	glGenVertexArrays(1, &VertexArrayId[ObjectId]);
	glBindVertexArray(VertexArrayId[ObjectId]);

	// Create Buffer for vertex data
	glGenBuffers(1, &VertexBufferId[ObjectId]);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
	glBufferData(GL_ARRAY_BUFFER, BufferSize, Vertices, GL_STATIC_DRAW);

	// Create Buffer for indices
	glGenBuffers(1, &IndexBufferId[ObjectId]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, IdxBufferSize, Indices, GL_STATIC_DRAW);

	// Assign vertex attributes
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset);

	glEnableVertexAttribArray(0);	// position
	glEnableVertexAttribArray(1);	// color

									// Disable our Vertex Buffer Object 
	glBindVertexArray(0);

	
	ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(
			stderr,
			"ERROR: Could not create a VBO: %s \n",
			gluErrorString(ErrorCheckValue)
		);
	}
	
}

void drawScene(Vertex v[])
{
	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	// Re-clear the screen for real rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);
	{
		glm::mat4 ModelMatrix = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
		glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		glEnable(GL_PROGRAM_POINT_SIZE);


		glBindVertexArray(VertexArrayId[0]);	// draw Vertices
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[0]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(v), v);				// update buffer data
																						//glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
		glDrawElements(GL_POINTS, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
		// ATTN: OTHER BINDING AND DRAWING COMMANDS GO HERE, one set per object:
		//glBindVertexArray(VertexArrayId[<x>]); etc etc
		glBindVertexArray(0);

		
		if (vertexList.size() >= 10)
		{
			if (shouldDrawCRCurve) {
				glBindVertexArray(VertexArrayId[1]);	// draw Vertices
				glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[1]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(v), v);				// update buffer data
				glDrawElements(GL_LINE_LOOP, NumVert[1], GL_UNSIGNED_SHORT, (void*)0);
				glDrawElements(GL_POINTS, NumVert[1], GL_UNSIGNED_SHORT, (void*)0);
			}

			if (shouldDrawCasteljau) {
				glBindVertexArray(VertexArrayId[2]);	// draw Vertices
				glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[2]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(v), v);				// update buffer data
				glDrawElements(GL_LINE_LOOP, NumVert[2], GL_UNSIGNED_SHORT, (void*)0);
			}
		}
		

	}
	glUseProgram(0);

	// Swap buffers
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void clearMemory(void) {
	// Cleanup VBO and shader
	for (int i = 0; i < NumObjects; i++) {
		glDeleteBuffers(1, &VertexBufferId[i]);
		glDeleteBuffers(1, &IndexBufferId[i]);
		glDeleteVertexArrays(1, &VertexArrayId[i]);
	}
	glDeleteProgram(programID);
	glDeleteProgram(pickingProgramID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
}