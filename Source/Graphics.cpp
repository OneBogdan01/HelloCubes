
#include "Graphics.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>
using namespace glm;

	
Graphics::Graphics()
{
	// don't really want to do anything here but could move init here
}
Graphics::~Graphics()
{
	
}


void Graphics::esInitContext(Target_State *p_state)
{
	if (p_state != NULL)
	{
		memset(p_state, 0, sizeof(Target_State));
	}
}

bool Graphics::GetDisplay()
{
	/*
	 * X11 native display initialization
	 */

	x_display = XOpenDisplay(NULL);
	if (x_display == NULL)
	{
		printf("Sorry to say we can't open an XDisplay and this will fail\n");
		return false;
	}
	else
		printf("we got an XDisplay\n");

	root = DefaultRootWindow(x_display);
	return true;
}

/*
Set up the EGL and Set up OpenGL states needed
Also set up the X11 window for rendering on a standard X11 system (like Rpi4/5, not like Rpi4)
*/

void Graphics::init_ogl(Target_State* state, int width, int height, int FBResX, int FBResY)

{

	XSetWindowAttributes swa;
	XSetWindowAttributes  xattr;
	Atom wm_state;
	XWMHints hints;
	XEvent xev;

#define ES_WINDOW_RGB           0
	state->width = width;
	state->height = height;

	EGLint numConfigs;
	EGLint majorVersion;
	EGLint minorVersion;





	root = DefaultRootWindow(x_display);

	swa.event_mask = ExposureMask | PointerMotionMask | KeyPressMask | KeyReleaseMask;
	swa.background_pixmap = None;
	swa.background_pixel = 0;
	swa.border_pixel = 0;
	swa.override_redirect = true;

	win = XCreateWindow(
		x_display,
		root,
		0,		// puts it at the top left of the screen
		0,
		state->width,	//set size  
		state->height,
		0,
		CopyFromParent,
		InputOutput,
		CopyFromParent,
		CWEventMask,
		&swa);

	if (win == 0)
	{
		printf("Sorry to say we can't create a window and this will fail\n");
		exit(0);       // we need to trap this;
	}
	else
		printf("we got an (Native) XWindow\n");

	state->nativewindow = (EGLNativeWindowType)win;

	XSelectInput(x_display, win, KeyPressMask | KeyReleaseMask);
	xattr.override_redirect = TRUE;
	XChangeWindowAttributes(x_display, win, CWOverrideRedirect, &xattr);

	hints.input = TRUE;
	hints.flags = InputHint;
	XSetWMHints(x_display, win, &hints);


	char* title = (char*)"x11 Demo";
	// make the window visible on the screen
	XMapWindow(x_display, win);
	XStoreName(x_display, win, title);

	// get identifiers for the provided atom name strings
	wm_state = XInternAtom(x_display, "_NET_WM_STATE", FALSE);

	memset(&xev, 0, sizeof(xev));
	xev.type = ClientMessage;
	xev.xclient.window = win;
	xev.xclient.message_type = wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = 1;
	xev.xclient.data.l[1] = FALSE;
	XSendEvent(
		x_display,
		DefaultRootWindow(x_display),
		FALSE,
		SubstructureNotifyMask,
		&xev);
	// end of xdisplay

	// Get EGLDisplay	
	egldisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);       //eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (egldisplay == EGL_NO_DISPLAY)
	{
		printf("Sorry to say we have an GetDisplay error and this will fail");
		exit(0);          // we need to trap this;
	}
	else  	printf("we got an EGLDisplay\n");

	// Initialize EGL
	if (!eglInitialize(egldisplay, &majorVersion, &minorVersion))
	{
		printf("Sorry to say we have an EGLinit error and this will fail");
		EGLint err = eglGetError();    // should be getting error values that make sense now
		exit(err);            // we need to trap this;
	}
	else 	printf("we initialised EGL\n");


	// Get configs
	if (!eglGetConfigs(egldisplay, NULL, 0, &numConfigs))
	{
		printf("Sorry to say we have EGL config errors and this will fail");
		EGLint err = eglGetError();
		exit(err);            // we need to trap this;
	}
	else 	printf("we got %i Configs\n", numConfigs);


	// Choose config
	if (!eglChooseConfig(egldisplay, attribute_list, &config, 1, &numConfigs))
	{
		printf("Sorry to say we have config choice issues, and this will fail");
		EGLint err = eglGetError();
		exit(err);            // we need to trap this;
	}
	else	printf("we chose our config\n");




	// Create a GL context

#ifdef GLES3
	context = eglCreateContext(egldisplay, config, NULL, GiveMeGLES3);
#else
	context = eglCreateContext(egldisplay, config, EGL_NO_CONTEXT, GiveMeGLES2);
#endif
	if (context == EGL_NO_CONTEXT)
	{
		EGLint err = eglGetError();
		exit(err);             // we need to trap this;
	}
	else	printf("Created a context ok\n");

	// Create a surface
	surface = eglCreateWindowSurface(egldisplay, config, state->nativewindow, NULL);     // this fails with a segmentation error?

	if (surface == EGL_NO_SURFACE)
	{
		EGLint err = eglGetError();
		exit(err);               // we need to trap this;
	}
	else 	printf("we got a Surface\n");


	// Make the context current
	if (!eglMakeCurrent(egldisplay, surface, surface, context))
	{
		EGLint err = eglGetError();
		exit(err);            // we need to trap this;
	}

	state->display = egldisplay;
	state->surface = surface;
	state->context = context;

	EGLBoolean test = eglSwapInterval(egldisplay, 01);        // 1 to lock speed to 60fps (assuming we are able to maintain it), 0 for immediate swap (may cause tearing) which will indicate actual frame rate
	// on xu4 this seems to have no effect

	// Some OpenGLES2.0 states that we might need
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(TRUE);
	glDepthRangef(0.0f, 1.0f);
	glClearDepthf(1.0f);

	//these are the options you can have for the depth, play with them?
	//#define GL_NEVER                          0x0200
	//#define GL_LESS                           0x0201
	//#define GL_EQUAL                          0x0202
	//#define GL_LEQUAL                         0x0203
	//#define GL_GREATER                        0x0204
	//#define GL_NOTEQUAL                       0x0205
	//#define GL_GEQUAL                         0x0206

	int t; // a dum,ing variable to extra some useful data

	printf("This SBC supports version %i.%i of EGL\n", majorVersion, minorVersion);
	printf("This GPU supplied by  :%s\n", glGetString(GL_VENDOR));
	printf("This GPU supports     :%s\n", glGetString(GL_VERSION));
	printf("This GPU Renders with :%s\n", glGetString(GL_RENDERER));
	printf("This GPU supports     :%s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));


	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &t);
	printf("This GPU MaxTexSize is    :%i\n", t);
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &t);
	printf("This GPU supports %i Texture units\n", t); // pi4 16


	printf("This GPU supports these extensions	:%s\n", glGetString(GL_EXTENSIONS));

	glViewport(0, 0, state->width, state->height);
	glClearColor(0.1f, 0.4f, 0.0f, 1.0f);
	//		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glCullFace(GL_BACK);

	if (glGetError() == GL_NO_ERROR)	return;
	else
		printf("Oh bugger, Some part of the EGL/OGL graphic init failed\n");
}


/*
 Now we have be able to create a shader object, pass the shader source
 and them compile the shader. Take note... this is called load shader but it actually works from a memory buffer
 but... hint hint, nothing stopping you loading that memory buffer with a text file?
*/
GLuint Graphics::LoadShader(GLenum type, const char *shaderSrc)
{	
	// 1st create the shader object
		GLuint TheShader = glCreateShader(type);
	
	if (TheShader == 0) return FALSE; // can't allocate so stop.
	
// pass the shader source then compile it
	glShaderSource(TheShader, 1, &shaderSrc, NULL);
	glCompileShader(TheShader);
	
	GLint  IsItCompiled;
	
	// After the compile we need to check the status and report any errors
		glGetShaderiv(TheShader, GL_COMPILE_STATUS, &IsItCompiled);
	if (!IsItCompiled)
	{
		GLint RetinfoLen = 0;
		glGetShaderiv(TheShader, GL_INFO_LOG_LENGTH, &RetinfoLen);
		if (RetinfoLen > 1)
		{
			 // standard output for errors
			char* infoLog = (char*) malloc(sizeof(char) * RetinfoLen);
			glGetShaderInfoLog(TheShader, RetinfoLen, NULL, infoLog);
			fprintf(stderr, "Error compiling this shader:\n%s\n", infoLog);
			free(infoLog);
		}
		glDeleteShader(TheShader);
		return FALSE;
	}
	return TheShader;
}



/*
Initialise an object so that it uses the correct shader to display
return int TRUE if all good.
*/

int Graphics::Init(ObjectModel* TheModel)
{

	// hard code for now student can optimise :D but consdier moving to files for easy of access and maybe a shader class to keep good order

		GLbyte vShaderStr[] =
		"precision mediump float;     \n"
		"attribute vec3 a_position;   \n"
		"attribute vec2 a_texCoord;   \n"
		"uniform mat4 MVP;            \n"
		"varying vec2 v_texCoord;     \n"
		"void main()                  \n"
		"{ 							  \n"
		" gl_Position =  MVP * vec4(a_position,1);\n"
		" v_texCoord = a_texCoord;  \n"
		"}                            \n";

	GLbyte fShaderStr[] =
		"precision mediump float;                            \n"
		"varying vec2 v_texCoord;                            \n"
		"uniform sampler2D s_texture;                        \n"
		"uniform vec4  Ambient;\n"
		"void main()                                         \n"
		"{                                                   \n"
		"  gl_FragColor = texture2D( s_texture, v_texCoord )*Ambient;\n"
		"}                                                   \n";
		
	
	// Load and compile the vertex/fragment shaders
   TheModel->vertexShader = LoadShader(GL_VERTEX_SHADER, (const char*)vShaderStr);
	TheModel->fragmentShader = LoadShader(GL_FRAGMENT_SHADER, (const char*)fShaderStr);
	
	// Create the program object	
		TheModel->programObject = glCreateProgram();
	if (TheModel->programObject == 0) 	return 0;
	
	// now we have the V and F shaders  attach them to the progam object
		glAttachShader(TheModel->programObject, TheModel->vertexShader);
	glAttachShader(TheModel->programObject, TheModel->fragmentShader);
	
	// Link the program
		glLinkProgram(TheModel->programObject);
	// Check the link status
		GLint AreTheylinked;
	glGetProgramiv(TheModel->programObject, GL_LINK_STATUS, &AreTheylinked);
	if (!AreTheylinked)
	{
		GLint RetinfoLen = 0;
		// check and report any errors
				glGetProgramiv(TheModel->programObject, GL_INFO_LOG_LENGTH, &RetinfoLen);
		if (RetinfoLen > 1)
		{
			GLchar* infoLog = (GLchar*)malloc(sizeof(char) * RetinfoLen);
			glGetProgramInfoLog(TheModel->programObject, RetinfoLen, NULL, infoLog);
			fprintf(stderr, "Error linking program:\n%s\n", infoLog);
			free(infoLog);
		}
		glDeleteProgram(TheModel->programObject);
		return FALSE;
	}

	

	// Get the attribute locations
TheModel->positionLoc = glGetAttribLocation(TheModel->programObject, "a_position");
	TheModel->texCoordLoc = glGetAttribLocation(TheModel->programObject, "a_texCoord");

	// Get the sampler location
	TheModel->samplerLoc = glGetUniformLocation(TheModel->programObject, "s_texture");


	
	if (glGetError() == GL_NO_ERROR)	return TRUE;
	else
		printf("Oh bugger, Model graphic init failed\n");
	return FALSE;
	
}



///
// Create a simple width x height texture image with RGBA format
//
GLuint Graphics::CreateSimpleTexture2D(int width, int height, char* TheData)
{
	// Texture object handle
	GLuint textureId;

	// Use tightly packed data
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Generate a texture object
	glGenTextures(1, &textureId);

	// Bind the texture object
	glBindTexture(GL_TEXTURE_2D, textureId);

	// Load the texture

	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGBA,
		width,
		height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		TheData);
//glGetError is a very primative way to check for GL errors, though in this case its quite spefific	
	if (glGetError() != GL_NO_ERROR) printf("Oh bugger");

	// Set the filtering mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	if (glGetError() == GL_NO_ERROR)	return textureId;
	printf("Oh bugger");

	return textureId;
}




void Graphics::GetRes(uint32_t &ReturnX, uint32_t &ReturnY)
{
	

	// you will need to make sure you have  installed libXrandr to use this..
	// sudo apt-get install libxrandr-dev
		
	//
	//     CONNECT TO X-SERVER, GET ROOT WINDOW ID
	//
	//
	//     GET POSSIBLE SCREEN RESOLUTIONS
	//
	xrrs   = XRRSizes(x_display, 0, &num_sizes);
	//
	//     LOOP THROUGH ALL POSSIBLE RESOLUTIONS,
	//     GETTING THE SELECTABLE DISPLAY FREQUENCIES
	//
	
	
	for(int i = 0 ; i < num_sizes ; i++) {
		short   *rates;
	
// print them out, just to see what we have
		printf("\n\t%2i : %4i x %4i   (%4imm x%4imm ) ", i, xrrs[i].width, xrrs[i].height, xrrs[i].mwidth, xrrs[i].mheight);

		rates = XRRRates(x_display, 0, i, &num_rates);

		for (int j = 0; j < num_rates; j++) {
			possible_frequencies[i][j] = rates[j];
			printf("%4i ", rates[j]);
		}
	}

	printf("\n");
	
	
}
void Graphics::SetRes(int index, uint32_t &screenX, uint32_t& screenY)
{
	//
	//     Set RESOLUTION AND FREQUENCY
	//
	GetRes(screenX, screenY); // check what we have
// store the original info for and end
	conf                   = XRRGetScreenInfo(x_display, DefaultRootWindow(x_display));
	original_rate          = XRRConfigCurrentRate(conf);
	original_size_id       = XRRConfigCurrentConfiguration(conf, &original_rotation);

	printf("\n\tCURRENT SIZE ID  : %i\n", original_size_id);
	printf("\tCURRENT ROTATION : %i \n", original_rotation);
	printf("\tCURRENT RATE     : %i Hz\n\n", original_rate);
	// find 1280x720		
		
		//
		int SelectedRes = index;  // pretty much any system can handle 1024x768 
		printf("\tCHANGED TO %i x %i PIXELS, %i Hz\n\n", xrrs[SelectedRes].width, xrrs[SelectedRes].height, possible_frequencies[SelectedRes][0]);
	XRRSetScreenConfigAndRate(x_display, conf, DefaultRootWindow(x_display) , SelectedRes, RR_Rotate_0, possible_frequencies[SelectedRes][0], CurrentTime);
	screenX = xrrs[SelectedRes].width;
	screenY = xrrs[SelectedRes].height;
}

void Graphics::RestoreRes()
{
	printf("\tRESTORING %i x %i PIXELS, %i Hz\n\n", xrrs[original_size_id].width, xrrs[original_size_id].height, original_rate);
	XRRSetScreenConfigAndRate(x_display, conf, DefaultRootWindow(x_display), original_size_id, original_rotation, original_rate, CurrentTime);
	
}	

