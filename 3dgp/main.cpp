#include <iostream>
#include "include/3dgl.h"
#include "include/GLee.h"
#include "include/glut.h"
#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;
using namespace _3dgl;

//Fire Flicker Variables
float currentFlickerTime;
float flickerInterval = 0.2;

//Ground Snow Opacity
float snowOpacity = 0;
float transitionTime;
float currentTime;

//Player control variables
bool isSnowing = false;

// 3D Models
C3dglTerrain terrain, water;

//Skybox
C3dglSkyBox skybox;

//cube map id
GLuint idTexCube;

// texture ids
GLuint idTexGrass;		// Grass Texture
GLuint idTexPebbles;	// Pebbles Texture
GLuint idTexSnow;		// Snow Texture
GLuint idTexPeak;		// Peak Texture
GLuint idTexNone;

// particle texture ids
GLuint idTexSnowParticle;
GLuint idTexFireParticle;
GLuint idTexSmokeParticle;

// GLSL Objects (Shader Program)
C3dglProgram Program;
C3dglProgram WaterProgram;
C3dglProgram TerrainProgram;
C3dglProgram SnowProgram;
C3dglProgram FireProgram;
C3dglProgram SmokeProgram;

//Snow Particle System Parameters
const float SNOWPERIOD = 0.000004f;
const float SNOWLIFETIME = 26.6;
const int NSNOWFLAKES = SNOWLIFETIME / SNOWPERIOD;
const int precipitationBoxSize = 160;

//Snow Particle Buffer ids
GLuint idBufferSnowInitialPos;
GLuint idBufferSnowVelocity;
GLuint idBufferSnowStartTime;

//Fire Particle System Parameters
const float FIREPERIOD = 0.001f;
const float FIRELIFETIME = 1.2;
const int NFIREP = FIRELIFETIME / FIREPERIOD;

//Fire Particle Buffer ids
GLuint idBufferFireInitialPos;
GLuint idBufferFireVelocity;
GLuint idBufferFireStartTime;

//Smoke Particle System Parameters
const float SMOKEPERIOD = 0.0025;
const float SMOKELIFETIME = 15.0;
const int NSMOKEP = SMOKELIFETIME / SMOKEPERIOD;

//Smoke Particle Buffer ids
GLuint idBufferVelocity;
GLuint idBufferStartTime;

// Multitexturing specific variables
float waterLevel = 28.2;
float grassLevel = 33;
float snowLevel = 60;

// camera position (for first person type camera navigation)
float matrixView[16];		// The View Matrix
float angleTilt = 0;		// Tilt Angle
float deltaX = 0, deltaY = 0, deltaZ = 0;	// Camera movement values

void SendUniform(string name, GLint val, bool basic, bool water, bool terrain)
{
	if(basic)
		Program.SendUniform(name, val);
	if(water)
		WaterProgram.SendUniform(name, val);
	if(terrain)
		TerrainProgram.SendUniform(name, val);
}

void SendUniform(string name, double val, bool basic, bool water, bool terrain)
{
	if(basic)
		Program.SendUniform(name, val);
	if(water)
		WaterProgram.SendUniform(name, val);
	if(terrain)
		TerrainProgram.SendUniform(name, val);
}

void SendUniform(string name, double val1, double val2, double val3, bool basic, bool water, bool terrain)
{
	if(basic)
		Program.SendUniform(name, val1, val2, val3);
	if(water)
		WaterProgram.SendUniform(name, val1, val2, val3);
	if(terrain)
		TerrainProgram.SendUniform(name, val1, val2, val3);
}

bool initShaders()
{
	// Initialise Shaders
	C3dglShader VertexShader;
	C3dglShader FragmentShader;

	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/basic.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/basic.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!Program.Create()) return false;
	if (!Program.Attach(VertexShader)) return false;
	if (!Program.Attach(FragmentShader)) return false;
	if (!Program.Link()) return false;
	if (!Program.Use(true)) return false;

	C3dglShader WaterVertexShader;
	C3dglShader WaterFragmentShader;

	if (!WaterVertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!WaterVertexShader.LoadFromFile("shaders/water.vert")) return false;
	if (!WaterVertexShader.Compile()) return false;

	if (!WaterFragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!WaterFragmentShader.LoadFromFile("shaders/water.frag")) return false;
	if (!WaterFragmentShader.Compile()) return false;

	if (!WaterProgram.Create()) return false;
	if (!WaterProgram.Attach(WaterVertexShader)) return false;
	if (!WaterProgram.Attach(WaterFragmentShader)) return false;
	if (!WaterProgram.Link()) return false;
	if (!WaterProgram.Use(true)) return false;

    C3dglShader TerrainVertexShader;
	C3dglShader TerrainFragmentShader;

	if (!TerrainVertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!TerrainVertexShader.LoadFromFile("shaders/terrain.vert")) return false;
	if (!TerrainVertexShader.Compile()) return false;

	if (!TerrainFragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!TerrainFragmentShader.LoadFromFile("shaders/terrain.frag")) return false;
	if (!TerrainFragmentShader.Compile()) return false;

	if (!TerrainProgram.Create()) return false;
	if (!TerrainProgram.Attach(TerrainVertexShader)) return false;
	if (!TerrainProgram.Attach(TerrainFragmentShader)) return false;
	if (!TerrainProgram.Link()) return false;
	if (!TerrainProgram.Use(true)) return false;

	C3dglShader SnowParticleVertexShader;
	C3dglShader SnowParticleFragmentShader;

	// Initialise Shader - Particle
	if (!SnowParticleVertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!SnowParticleVertexShader.LoadFromFile("shaders/snow.vert")) return false;
	if (!SnowParticleVertexShader.Compile()) return false;

	if (!SnowParticleFragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!SnowParticleFragmentShader.LoadFromFile("shaders/snow.frag")) return false;
	if (!SnowParticleFragmentShader.Compile()) return false;

	if (!SnowProgram.Create()) return false;
	if (!SnowProgram.Attach(SnowParticleVertexShader)) return false;
	if (!SnowProgram.Attach(SnowParticleFragmentShader)) return false;
	if (!SnowProgram.Link()) return false;
	if (!SnowProgram.Use(true)) return false;

	C3dglShader FireParticleVertexShader;
	C3dglShader FireParticleFragmentShader;

	// Initialise Shader - Particle
	if (!FireParticleVertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!FireParticleVertexShader.LoadFromFile("shaders/fire.vert")) return false;
	if (!FireParticleVertexShader.Compile()) return false;

	if (!FireParticleFragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FireParticleFragmentShader.LoadFromFile("shaders/fire.frag")) return false;
	if (!FireParticleFragmentShader.Compile()) return false;

	if (!FireProgram.Create()) return false;
	if (!FireProgram.Attach(FireParticleVertexShader)) return false;
	if (!FireProgram.Attach(FireParticleFragmentShader)) return false;
	if (!FireProgram.Link()) return false;
	if (!FireProgram.Use(true)) return false;

	C3dglShader SmokeParticleVertexShader;
	C3dglShader SmokeParticleFragmentShader;

	// Initialise Shader - Particle
	if (!SmokeParticleVertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!SmokeParticleVertexShader.LoadFromFile("shaders/smoke.vert")) return false;
	if (!SmokeParticleVertexShader.Compile()) return false;

	if (!SmokeParticleFragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!SmokeParticleFragmentShader.LoadFromFile("shaders/smoke.frag")) return false;
	if (!SmokeParticleFragmentShader.Compile()) return false;

	if (!SmokeProgram.Create()) return false;
	if (!SmokeProgram.Attach(SmokeParticleVertexShader)) return false;
	if (!SmokeProgram.Attach(SmokeParticleFragmentShader)) return false;
	if (!SmokeProgram.Link()) return false;
	if (!SmokeProgram.Use(true)) return false;
}

void prepareSnowBuffers()
{
	cout << "PREPARING SNOW PARTICLE BUFFERS - PLEASE WAIT" << endl;
	// Prepare the particle buffers
	std::vector<float> bufferVelocity;
	std::vector<float> bufferStartTime;
	std::vector<float> bufferInitialPos;
	float time = 0;
	for (int i = 0; i < NSNOWFLAKES; i++)
	{
		float v = 2.5 + 0.2f * (float)rand()/(float)RAND_MAX;

		bufferVelocity.push_back(-0.5 * v);
		bufferVelocity.push_back(-v);
		bufferVelocity.push_back(0.25 * v);  
		bufferStartTime.push_back(time);
		time += SNOWPERIOD;

		bufferInitialPos.push_back(-precipitationBoxSize + (precipitationBoxSize - -precipitationBoxSize) * (float)rand()/(float)RAND_MAX);
		bufferInitialPos.push_back(100);
		bufferInitialPos.push_back(-precipitationBoxSize + (precipitationBoxSize - -precipitationBoxSize) * (float)rand()/(float)RAND_MAX);
	}

	glGenBuffers(1, &idBufferSnowInitialPos);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferSnowInitialPos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferInitialPos.size(), &bufferInitialPos[0], GL_STATIC_DRAW);

	glGenBuffers(1, &idBufferSnowVelocity);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferSnowVelocity);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferVelocity.size(), &bufferVelocity[0], GL_STATIC_DRAW);

	glGenBuffers(1, &idBufferSnowStartTime);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferSnowStartTime);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferStartTime.size(), &bufferStartTime[0], GL_STATIC_DRAW);

	// Setup the particle system
	SnowProgram.SendUniform("gravity",	 0.0, 0.0, 0.0);
	SnowProgram.SendUniform("particleLifetime", SNOWLIFETIME);
}

void prepareFireBuffers()
{
	cout << "PREPARING FIRE PARTICLE BUFFERS - PLEASE WAIT" << endl;
	// Prepare the particle buffers
	std::vector<float> bufferVelocity;
	std::vector<float> bufferStartTime;
	std::vector<float> bufferInitialPos;
	float time = 0;
	for (int i = 0; i < NFIREP; i++)
	{
		float v = 0.1 + 0.4f * (float)rand()/(float)RAND_MAX;

		bufferVelocity.push_back(0);
		bufferVelocity.push_back(v);
		bufferVelocity.push_back(0);

		bufferStartTime.push_back(time);
		time += FIREPERIOD;

		float fireMinX = 17; float fireMaxX = 17.3; float fireMinZ = -17.5; float FireMaxZ = -17.2;

		bufferInitialPos.push_back(fireMinX + (fireMaxX - fireMinX) * (float)rand()/(float)RAND_MAX);
		bufferInitialPos.push_back(29.72f);
		bufferInitialPos.push_back(fireMinZ + (FireMaxZ - fireMinZ) * (float)rand()/(float)RAND_MAX);
	}

	glGenBuffers(1, &idBufferFireInitialPos);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferFireInitialPos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferInitialPos.size(), &bufferInitialPos[0], GL_STATIC_DRAW);

	glGenBuffers(1, &idBufferFireVelocity);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferFireVelocity);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferVelocity.size(), &bufferVelocity[0], GL_STATIC_DRAW);

	glGenBuffers(1, &idBufferFireStartTime);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferFireStartTime);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferStartTime.size(), &bufferStartTime[0], GL_STATIC_DRAW);

	// Setup the particle system
	FireProgram.SendUniform("gravity", -0.05, 0.1, 0.05);
	FireProgram.SendUniform("particleLifetime", FIRELIFETIME);
}

void prepareSmokeBuffers()
{
	cout << "PREPARING SMOKE PARTICLE BUFFERS - PLEASE WAIT" << endl;
	// Prepare the particle buffers
	std::vector<float> bufferVelocity;
	std::vector<float> bufferStartTime;
	float time = 0;
	for (int i = 0; i < NSMOKEP; i++)
	{
		float theta = (float)M_PI / 1.5f * (float)rand()/(float)RAND_MAX;
		float phi = (float)M_PI * 2.f * (float)rand()/(float)RAND_MAX;
		float x = sin(theta) * cos(phi);
		float y = cos(theta);
		float z = sin(theta) * sin(phi);
		float v = 0.1 + 0.1f * (float)rand()/(float)RAND_MAX;

		bufferVelocity.push_back(x * v);
		bufferVelocity.push_back(y * v);
		bufferVelocity.push_back(z * v);

		bufferStartTime.push_back(time);
		time += SMOKEPERIOD;
	}
	glGenBuffers(1, &idBufferVelocity);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocity);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferVelocity.size(), &bufferVelocity[0], GL_STATIC_DRAW);

	glGenBuffers(1, &idBufferStartTime);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTime);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferStartTime.size(), &bufferStartTime[0], GL_STATIC_DRAW);

	// Setup the particle system
	SmokeProgram.SendUniform("initialPos",  17.15, 29.9, -17.35);
	SmokeProgram.SendUniform("gravity",	 -0.01, 0.1, 0.01);
	SmokeProgram.SendUniform("particleLifetime", SMOKELIFETIME);
}

// called before window opened or resized - to setup the Projection Matrix
void reshape(int w, int h)
{
	// find screen aspect ratio
	float ratio =  w * 1.0 / h;      // we hope that h is not zero

	// setup the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);
	gluPerspective(60.0, ratio, 0.02, 1000.0);

	float matrix[16];
	glGetFloatv(GL_PROJECTION_MATRIX, matrix);
	Program.SendUniform("matrixProjection", matrix);
	WaterProgram.SendUniform("matrixProjection", matrix);
	TerrainProgram.SendUniform("matrixProjection", matrix);
	SnowProgram.SendUniform("matrixProjection", matrix);
	FireProgram.SendUniform("matrixProjection", matrix);
	SmokeProgram.SendUniform("matrixProjection", matrix);
}

void renderObjects()
{
	float matrix[16];

	Program.Use();

	SendUniform("materialAmbient", 1.0, 1.0, 1.0, true, false, true);
	SendUniform("materialDiffuse", 0.0, 0.0, 0.0, true, false, true);
	//Render Skybox
	Program.SendUniform("lightEmissive.on", 1);
	Program.SendUniform("lightEmissive.color", 0.3, 0.3, 0.3);
	skybox.render();
	Program.SendUniform("lightEmissive.on", 0);
	Program.SendUniform("lightEmissive.color", 1.0, 1.0, 1.0);
	//End Skybox Render

	TerrainProgram.Use();

	SendUniform("materialDiffuse", 1.0, 1.0, 1.0, true, false, true);

	// render the terrain
	glPushMatrix();
	float modelviewMatrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelviewMatrix);
	TerrainProgram.SendUniform("matrixModelView", modelviewMatrix);
	terrain.render();
	glPopMatrix();

	if(isSnowing == true)
	{
		/////////////////////////////////////
		// RENDER THE SNOW PARTICLE SYSTEM //
		/////////////////////////////////////

		// setup the rain drop texture
		glBindTexture(GL_TEXTURE_2D, idTexSnowParticle);

		glPointSize(5);

		SnowProgram.Use();
		glDepthMask(GL_FALSE);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, idTexSnowParticle);

		glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
		SnowProgram.SendUniform("matrixModelView", matrix);

		SnowProgram.SendUniform("time", glutGet(GLUT_ELAPSED_TIME) / 1000.f - 2);

		// render the buffer
		glEnableVertexAttribArray(0);	// initial position
		glEnableVertexAttribArray(1);	// velocity
		glEnableVertexAttribArray(2);	// start time
		glBindBuffer(GL_ARRAY_BUFFER, idBufferSnowInitialPos);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, idBufferSnowVelocity);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, idBufferSnowStartTime);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
		glDrawArrays(GL_POINTS, 0, NSNOWFLAKES);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// revert to normal
		glDepthMask(GL_TRUE);

		///////////////////////////////////////////
		// END OF SNOW PARTICLE SYSTEM RENDERING //
		///////////////////////////////////////////

		if(snowOpacity < 1)
		{
			//Handles layering snow over the grass when it's snowing
			snowOpacity += (float)((currentTime - transitionTime) / 10.f);
			transitionTime = currentTime;
		}
	}
	else
	{
		if(snowOpacity > 0)
		{
			//Handles fading away the snow over the grass when it's not snowing
			snowOpacity -= (float)((currentTime - transitionTime) / 10.f);
			transitionTime = currentTime;
		}
	}

	//Sends snow opacity to shader
	TerrainProgram.SendUniform("snowOpacity", snowOpacity);

	//////////////////////////////////////
	// RENDER THE SMOKE PARTICLE SYSTEM //
	//////////////////////////////////////

	SmokeProgram.Use();
	glDepthMask(GL_FALSE);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexSmokeParticle);

	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	SmokeProgram.SendUniform("matrixModelView", matrix);

	SmokeProgram.SendUniform("time", glutGet(GLUT_ELAPSED_TIME) / 1000.f - 2);

	// render the buffer
	glEnableVertexAttribArray(0);	// velocity
	glEnableVertexAttribArray(1);	// start time
	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocity);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTime);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_POINTS, 0, NSMOKEP);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	// revert to normal
	glDepthMask(GL_TRUE);
	 
	////////////////////////////////////////////
	// END OF SMOKE PARTICLE SYSTEM RENDERING //
	////////////////////////////////////////////

	/////////////////////////////////////
	// RENDER THE FIRE PARTICLE SYSTEM //
	/////////////////////////////////////

	FireProgram.Use();
	glDepthMask(GL_FALSE);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexFireParticle);

	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	FireProgram.SendUniform("matrixModelView", matrix);

	FireProgram.SendUniform("time", glutGet(GLUT_ELAPSED_TIME) / 1000.f - 2);

	// render the buffer
	glEnableVertexAttribArray(0);	// initial position
	glEnableVertexAttribArray(1);	// velocity
	glEnableVertexAttribArray(2);	// start time
	glBindBuffer(GL_ARRAY_BUFFER, idBufferFireInitialPos);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferFireVelocity);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferFireStartTime);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_POINTS, 0, NFIREP);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	// revert to normal
	glDepthMask(GL_TRUE);

	///////////////////////////////////////////
	// END OF FIRE PARTICLE SYSTEM RENDERING //
	///////////////////////////////////////////
}

void prepareCubeMap(float x, float y, float z)
{
		float matrix[16];

		// Store the current viewport in a safe place
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		int w = viewport[2];
		int h = viewport[3];

		// setup the viewport to 256x256, 90 degrees FoV (Field of View)
		glViewport(0, 0, 256, 256);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(90, 1, 0.02f, 1000.0f);

		// send the projection matrix to the shader
		glGetFloatv(GL_PROJECTION_MATRIX, matrix);
		WaterProgram.SendUniform("matrixProjection", matrix);

		// render environment 6 times
		glMatrixMode(GL_MODELVIEW);
		WaterProgram.SendUniform("reflectionPower", 0.0);
		for (int i = 0; i < 6; ++i)
		{
			// clear background
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// setup the camera
			const GLfloat ROTATION[6][6] =
			{	// at              up
				{ 1.0, 0.0, 0.0,   0.0, -1.0, 0.0 },  // pos x
				{ -1.0, 0.0, 0.0,  0.0, -1.0, 0.0 },  // neg x
				{ 0.0, 1.0, 0.0,   0.0, 0.0, 1.0 },   // pos y
				{ 0.0, -1.0, 0.0,  0.0, 0.0, -1.0 },  // neg y
				{ 0.0, 0.0, 1.0,   0.0, -1.0, 0.0 },  // poz z
				{ 0.0, 0.0, -1.0,  0.0, -1.0, 0.0 }   // neg z
			};
			glLoadIdentity();
			gluLookAt(x, y, z, 	x + ROTATION[i][0], y + ROTATION[i][1], z + ROTATION[i][2],
				ROTATION[i][3], ROTATION[i][4], ROTATION[i][5]);
	
			// send the View Matrix
			glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
			//WaterProgram.SendUniform("matrixView", matrix);

			// render scene objects - all but the reflective one
			glActiveTexture(GL_TEXTURE0);
			renderObjects();

			// send the image to the cube texture
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_CUBE_MAP, idTexCube);
			glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8, 0, 0, 256, 256, 0);
		}
		// restore the viewport and projection
		reshape(w, h);
}

bool init()
{
	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE!

	//Switch on transparency/blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_POINT_SPRITE);

	glEnable(0x8642);

	//Initialise Shaders
	if (!initShaders()) return false;

	// load your 3D models here!
	if (!terrain.loadHeightmap("models\\heightmap.bmp", 80)) return false;
	if (!water.loadHeightmap("models\\watermap.bmp", 10)) return false;

	//Load Skybox
	if (!skybox.load("models\\Skybox\\snowy_s1.bmp", "models\\Skybox\\snowy_s2.bmp", "models\\Skybox\\snowy_s3.bmp",
		"models\\Skybox\\snowy_s4.bmp", "models\\Skybox\\snowy_s6.bmp", "models\\Skybox\\snowy_s5.bmp")) return false;

	//Prepare Particle Buffers
	prepareSnowBuffers();
	prepareFireBuffers();
	prepareSmokeBuffers();

	// create & load textures
	C3dglBitmap bm;
    glActiveTexture(GL_TEXTURE0);
	
	// Grass texture
	bm.Load("models/grass.jpg", GL_RGBA);
	glGenTextures(1, &idTexGrass);
	glBindTexture(GL_TEXTURE_2D, idTexGrass);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	bm.Load("models/pebbles.jpg", GL_RGBA);
	glGenTextures(1, &idTexPebbles);
	glBindTexture(GL_TEXTURE_2D, idTexPebbles);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	bm.Load("models/snow.bmp", GL_RGBA);
	glGenTextures(1, &idTexSnow);
	glBindTexture(GL_TEXTURE_2D, idTexSnow);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	bm.Load("models/peak.jpg", GL_RGBA);
	glGenTextures(1, &idTexPeak);
	glBindTexture(GL_TEXTURE_2D, idTexPeak);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	bm.Load("models/snowdrop.bmp", GL_RGBA);
    glGenTextures(1, &idTexSnowParticle);
    glBindTexture(GL_TEXTURE_2D, idTexSnowParticle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	bm.Load("models/fire.bmp", GL_RGBA);
    glGenTextures(1, &idTexFireParticle);
    glBindTexture(GL_TEXTURE_2D, idTexFireParticle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	bm.Load("models/smoke.bmp", GL_RGBA);
    glGenTextures(1, &idTexSmokeParticle);
    glBindTexture(GL_TEXTURE_2D, idTexSmokeParticle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	//None Texture
	glGenTextures(1, &idTexNone);
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	BYTE bytes[] = {255, 255, 255};
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &bytes);

	// Send the texture info to the shaders
	SendUniform("texture0", 0, true, false, false);
	SnowProgram.SendUniform("texture0", 0);
	FireProgram.SendUniform("texture0", 0);
	SmokeProgram.SendUniform("texture0", 0);

	// setup lights:
	SendUniform("lightAmbient.on", 1, true, false, true); 
	SendUniform("lightAmbient.color", 0.2, 0.2, 0.2, true, false, true);

	SendUniform("lightEmissive.color", 1.0, 1.0, 1.0, true, false, true);

	SendUniform("lightDir.on", 1, true, false, true);	
	SendUniform("lightDir.direction", 1.0, 0.5, 0.5, true, false, true);	
	SendUniform("lightDir.diffuse", 0.2, 0.2, 0.2, true, false, true);	

	SendUniform("lightPoint1.on", 1, true, false, true);	
	SendUniform("lightPoint1.position", 17.15, 31.0, -17.35, true, false, true);	
	SendUniform("lightPoint1.diffuse", 0.2, 0.1, 0.0, true, false, true);	 
	SendUniform("lightPoint1.specular", 0.2, 0.1, 0.0, true, false, true);		
	SendUniform("lightPoint1.att_quadratic", 0.05f, true, false, true);		

	// setup materials
	SendUniform("materialAmbient", 1.0, 1.0, 1.0, true, false, true);		// full power (note: ambient light is extremely dim)	
	SendUniform("materialDiffuse", 1.0, 1.0, 1.0, true, false, true);	
	SendUniform("materialSpecular", 0.0, 0.0, 0.0, true, false, true);	
	SendUniform("shininess", 0.0, true, false, true);

	// setup fog
	SendUniform("fogColour", 0.3f, 0.3f, 0.3f, true, true, true);	
	SendUniform("fogDensity", 0.0, true, true, true);

	// setup the water colours and level
	WaterProgram.SendUniform("waterColor", 0.0f, 0.2f, 0.3f);
	TerrainProgram.SendUniform("waterColor", 0.0f, 0.2f, 0.3f);
	WaterProgram.SendUniform("skyColor", 0.0f, 0.0f, 0.2f);
	TerrainProgram.SendUniform("waterLevel", waterLevel);
	TerrainProgram.SendUniform("grassLevel", grassLevel);
	TerrainProgram.SendUniform("snowLevel", snowLevel);

	//Setup terrain textures
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, idTexPebbles);
	TerrainProgram.SendUniform("textureBed", 4);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, idTexGrass);
	TerrainProgram.SendUniform("textureShore", 3);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, idTexSnow);
	TerrainProgram.SendUniform("textureSnow", 2);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, idTexPeak);
	TerrainProgram.SendUniform("texturePeak", 1);

	// load Cube Map
	glActiveTexture(GL_TEXTURE5);
	glGenTextures(1, &idTexCube);
	glBindTexture(GL_TEXTURE_CUBE_MAP, idTexCube);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	WaterProgram.SendUniform("textureCubeMap", 5);

	// Initialise the View Matrix (initial position for the first-person camera)
	glMatrixMode(GL_MODELVIEW);
	angleTilt = 15;
	glLoadIdentity();
	glRotatef(angleTilt, 1, 0, 0);
	gluLookAt(4.0, 0.4, 30.0, 
	          4.0, 0.4, 0.0,
	          0.0, 1.0, 0.0);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrixView);

	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  Space/Shift+space to set the camera height over the ground" << endl;
	cout << "  Use the mouse with the left button down to look around" << endl;
	cout << endl;

	currentFlickerTime = glutGet(GLUT_ELAPSED_TIME) / 1000.f;

	return true;
}

void done()
{
}

void render()
{
	prepareCubeMap(0.0, 30.0, 0.0);

	currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.f;

	if ((glutGet(GLUT_ELAPSED_TIME) / 1000.f) - currentFlickerTime > flickerInterval)
	{
		//Flicker fire point light
		float redAmount = (float)(((rand() % 11) + 10) / 100.f);
		float greenAmount = redAmount / 2;

		SendUniform("lightPoint1.diffuse", redAmount, greenAmount, 0.0, true, false, true);	 
		SendUniform("lightPoint1.specular", redAmount,	greenAmount, 0.0, true, false, true);	

		flickerInterval = (((rand() % 61)) + 40) / 1000.f;

		currentFlickerTime = glutGet(GLUT_ELAPSED_TIME) / 1000.f;
	}

	//Send time to water shader for animation
	WaterProgram.SendUniform("time", glutGet(GLUT_ELAPSED_TIME) / 1000.f);

	float matrix[16];

	// clear screen and buffers
	glClearColor(0.0f, 0.0f, 0.05f, 1.0f);   // blue sky colour
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// setup the View Matrix (camera)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(angleTilt, 1, 0, 0);					// switch tilt off
	glTranslatef(deltaX, deltaY, deltaZ);			// animate camera motion (controlled by WASD keys)
	glRotatef(-angleTilt, 1, 0, 0);					// switch tilt on
	glMultMatrixf(matrixView);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrixView);

	// set the camera height above the ground
	gluInvertMatrix(matrixView, matrix);
	glTranslatef(0, -max(terrain.getInterpolatedHeight(matrix[12], matrix[14]), waterLevel),  0);

	// setup View Matrix
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	Program.SendUniform("matrixView", matrix);
	//WaterProgram.SendUniform("matrixView", matrix);
	TerrainProgram.SendUniform("matrixView", matrix);

	// send the Inverted View Matrix
	gluInvertMatrix(matrix, matrix);
	WaterProgram.SendUniform("matrixInvertedView", matrix);

	glActiveTexture(GL_TEXTURE0);
	WaterProgram.SendUniform("reflectionPower", 0.0);

	//Render non reflective objects
	renderObjects();

	WaterProgram.Use();

	// render the water

	float modelviewMatrix[16];

	glActiveTexture(GL_TEXTURE5);
	WaterProgram.SendUniform("reflectionPower", 0.6);
	glBindTexture(GL_TEXTURE_CUBE_MAP, idTexCube);

	glPushMatrix();
	glTranslatef(0, waterLevel, 0);
	glScalef(0.5f, 1.0f, 0.5f);
	glGetFloatv(GL_MODELVIEW_MATRIX, modelviewMatrix);
	WaterProgram.SendUniform("matrixModelView", modelviewMatrix);
	water.render();
	glPopMatrix();

	glActiveTexture(GL_TEXTURE0);
	WaterProgram.SendUniform("reflectionPower", 0.0);
	glBindTexture(GL_TEXTURE_2D, idTexNone);

	Program.Use();

	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();
}

// Handle WASD keys and space
void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': deltaZ = max(deltaZ * 1.05,  0.01); break;
	case 's': deltaZ = min(deltaZ * 1.05, -0.01); break;
	case 'a': deltaX = max(deltaX * 1.05,  0.01); break;
	case 'd': deltaX = min(deltaX * 1.05, -0.01); break;
	case '1': if(isSnowing)
			  {
				  transitionTime = glutGet(GLUT_ELAPSED_TIME) / 1000.f;
				  isSnowing = false;
				  SendUniform("fogDensity", 0.0, true, true, true);
				  break;	 
			  }
			  else if (!isSnowing)
			  {
				  transitionTime = glutGet(GLUT_ELAPSED_TIME) / 1000.f;
				  isSnowing = true;		 
				  SendUniform("fogDensity", 0.03, true, true, true);
				  break;
			  }
			  break;
	case ' ': if ((glutGetModifiers() & GLUT_ACTIVE_SHIFT) == 0)
				  deltaY = -0.02; 
			  else
				  deltaY = 0.02; 
			  break;
	}
}

// Handle WASD keys and space (key up)
void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w':
	case 's': deltaZ = 0; break;
	case 'a':
	case 'd': deltaX = 0; break;
	case ' ': deltaY = 0; break;
	}
}

// Handle arrow keys and Alt+F4
void onSpecDown(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F4: if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:	onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:	onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:	onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:onKeyDown('d', x, y); break;
	}
}

// Handle arrow keys (key up)
void onSpecUp(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:	onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:	onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:	onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:onKeyUp('d', x, y); break;
	}
}

// Handle mouse click
void onMouse(int button, int state, int x, int y)
{
	int cx = glutGet(GLUT_WINDOW_WIDTH) / 2;
	int cy = glutGet(GLUT_WINDOW_HEIGHT) / 2;

	if (state == GLUT_DOWN)
	{
		glutSetCursor(GLUT_CURSOR_CROSSHAIR);
		glutWarpPointer(cx, cy);
	}
	else
		glutSetCursor(GLUT_CURSOR_INHERIT);
}

// handle mouse move
void onMotion(int x, int y)
{
	int cx = glutGet(GLUT_WINDOW_WIDTH) / 2;
	int cy = glutGet(GLUT_WINDOW_HEIGHT) / 2;
	if (x == cx && y == cy) 
		return;	// caused by glutWarpPointer

	float amp = 0.25;
	float deltaTilt = amp * (y - cy);
	float deltaPan  = amp * (x - cx);

	glutWarpPointer(cx, cy);

	// handle camera tilt (mouse move up & down)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(deltaTilt, 1, 0, 0);
	glMultMatrixf(matrixView);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrixView);

	angleTilt += deltaTilt;

	// handle camera pan (mouse move left & right)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(angleTilt, 1, 0, 0);
	glRotatef(deltaPan, 0, 1, 0);
	glRotatef(-angleTilt, 1, 0, 0);
	glMultMatrixf(matrixView);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrixView);
}

int main(int argc, char **argv)
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1280, 720);
	glutCreateWindow("CI5520 3D Graphics Programming");

	// register callbacks
	glutDisplayFunc(render);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);

	cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
	cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Version: " << glGetString(GL_VERSION) << endl;

	// init light and everything – not a GLUT or callback function!
	if (!init())
	{
		cerr << "Application failed to initialise" << endl;
		return 0;
	}

	// enter GLUT event processing cycle
	glutMainLoop();

	done();

	return 1;
}

