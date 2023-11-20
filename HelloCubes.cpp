/* Hello Cubes
code adapted from from OpenGL® ES 2.0 Programming Guide

Presented here for Block B as a basic Framework, Rendering method is sub optimal, please take note of that, You should impliment
your own render systems and shader managment systems when you understand the methods available, workshops will be run to discuss this

Initialises a Raspberry format EGL window for rendering and provides an example a simple shader system

Provides a skeletal framework for a processing loop 
Provides input systems for mouse and keyboard (requires init to work)
(note...Linux key reads are unpredictable, if you find keys are not working use event0 or event 1 or event2.. etc
This is a known issue, we could fix with a listener on all events but other methods are being investigated)

Provides a simple Gui demonstrated on start up which allows resolution changes and info display
Provides Graphic file loading capability via MyFiles.h a wrapper class for stb_image
GLM is used as a standard maths library, you may use your own if you wish but why???

Recommended course..
Review ObjectModel base class, provide the functions stated
Review CubeModel as a standard object type. 
Provide init update and draw routines

Review the Draw function here in HelloCubes which is the main processing loop...make it work

Project builds for Arm64 or Arm32
Rpi4 is considered an X11 system, see the info on CMGT regarding setup of systems, ensure you use X11 for render.. 

*/

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <sys/time.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <vector>

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <sys/time.h>
#include "CubeModel.h"
#include "ShipModel.h"
#ifdef RASPBERRY
#include "bcm_host.h"
#endif

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <vector>
// Include GLM
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "Headers/Graphics.h"
#include "Headers/ObjectModel.h"

#include "Input.h"

using namespace glm;

// keeping some things in global space...because....Brian does it this way :D you should consider better ways....AKA, you should consider better ways...do I have to spell it out?
std::vector<ObjectModel*> MyObjects;
// on the basis that every object is derived from ObjectModel, we keep a list of things to draw.
Graphics Graphics; // create a 1 time instance of a graphics class in Global space

// ugly throw back to the classic OpenGL demo... these can be removed if you want to smarten things up
Target_State state;
Target_State* p_state = &state;
Input* TheInput;
// nice pointer to a hardware key and mouse thread system... but..it can be flakey with some wireless keyboards


/*****************************************
Up date all objects shape stored in the MyObjects list

This is however not an optimal way to process objects
updating them, then doing their draw is inefficient
Consider ways to improve this
******************************************/

void Update(Target_State* p_state)
{
	// Setup the viewport
	glViewport(0, 0, p_state->width, p_state->height);


	// Clear the color draw buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	for (size_t i = 0; i < MyObjects.size(); i++)
	{
		//update the object, in this demo, we just up its position and transform data but you should consider logic
		MyObjects[i]->Update();
		// the draw routine is the responsbility of the object itself, thats not an ideal system, consider how to improve
		MyObjects[i]->Draw();
	}
}


// Our projects main  Game loop (could be put into a class, hint hint)
void MainLoop(Target_State* esContext)
{
	MyFiles* Handler = new MyFiles();

	TheInput = new Input();
	TheInput->Init();

	ObjectModel* T2; // so both types even though quite different use the same base to create them

	T2 = new CubeModel(Handler); // make a new cube
	glm::vec3 Pos = glm::vec3(10.0f, 0.0f, 0.0f); // set a position
	T2->Scales = glm::vec3(0.1f, 0.1f, 0.1f); // a cube is actually quite large (screen size) so shrink it down
	T2->SetPosition(Pos);
	MyObjects.push_back(T2); // store in the vector ready for the game loop to process
	T2->StoreGraphicClass(&Graphics);
	// make sure it knows the where the graphics class is, (for now it contains our attribute/uniform info)
	Graphics.Init(T2); // set it up


	// repeat for a different kind of model	this time a full Obj model
	T2 = new ShipModel((char*)"../../../Resources/Models/brian_03.obj", Handler);
	Pos = glm::vec3(-8.0f, 0.0f, 0.0f);
	T2->Scales = glm::vec3(0.8f, 0.8f, 0.8f);
	T2->SetPosition(Pos);
	T2->StoreGraphicClass(&Graphics);
	Graphics.Init(T2);

	MyObjects.push_back(T2);
	struct timeval t1, t2;
	struct timezone tz;
	float deltatime;
	float totaltime = 0.0f;
	unsigned int frames = 0;

	gettimeofday(&t1, &tz);

	int Counter = 0;
	while (Counter++ < 2000) // for a timed loop check the counter otherwise find another exit condition
	{
		gettimeofday(&t2, &tz);
		deltatime = (float)(t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 0.0000001f);
		t1 = t2;
		Update(esContext); // this is where the action happens

		glFlush();
		eglSwapBuffers(esContext->display, esContext->surface);


		// report frame rate
		totaltime += deltatime;
		frames++;
		if (totaltime > 1.0f)
		{
			printf("%4d frames rendered in %1.4f seconds -> FPS=%3.4f\n", frames, totaltime, frames / totaltime);
			// uncomment to see frames
			totaltime -= 1.0f;
			frames = 0;
		}
	}
}


//Program Entry

int main(int argc, char* argv[])
{
	MyFiles* Handler = new MyFiles(); // here have a nice file handler
	Graphics.GetDisplay();
	Graphics.esInitContext(p_state);


	Graphics.init_ogl(p_state, 1024, 768, 1024, 768);
	// this is the window size we set , you can set it to anything, the 3rd and 4th params are for older pis'
	Graphics.GetRes(Graphics.ResolutionX, Graphics.ResolutionY);

	ObjectData object_data; // create a single instance of object_data to initially draw all objects with the same data.

	p_state->object_data = &object_data;
	// this is....inefficient... an expansion of the old hello triangle demo's consider reworking it.


	// now go do the Game loop	
	MainLoop(p_state);

	// here we should do a graceful exit	
	// I said graceful.....
}
