#include "input.h"

//loaders and the like
#include "spriteMeshLoader.h"
#include "colladaLoader.h"

//actors
#include "msbLight.h"

//#include "videoTextureActor.h"

//buttons
#include "listButton.h"
#include "sliderButton.h"
#include "userPopUp.h"

#include "assignButton.h"
#include "textInputButton.h"

#include "moveButton.h"
#include "rotateButton.h"


//nodes
#include "node.h"

#include "layer.h"

//data
#include "meshData.h"

#ifdef TARGET_WIN32

    #include <Commdlg.h>
    #include <stdio.h>
    #include <conio.h>
    #include <tchar.h>

#endif

//static link
Renderer* Renderer::rendererInstance=NULL;

//************************************************************
//
//RTTY actorInfo - Information about every Actor:
//                 Class Name, Type, Size
//
//************************************************************


void Renderer::fillGlobalLists(){



    //actors
    createActorID(new Actor);
    //createActorID(new VideoTextureActor);

    createActorID(new MsbLight);


    ////buttons
    createActorID(new BasicButton);
    createActorID(new Layer);
    createActorID(new ListButton);
    createActorID(new SliderButton);

    createActorID(new UserPopUp);

    createActorID(new TextInputButton);
    createActorID(new AssignButton);
    createActorID(new MoveButton);
    createActorID(new RotateButton);

    createActorID(new Node);
}


//************************************************************
//
//Constructor and Destructor
//
//************************************************************

Renderer::Renderer(){


    backgroundTex="NULL";
    backgroundColor=Vector4f(0.0,0.3,0.5,1);

    lastShader="NULL";

    currentLayer=0;

    startSceneFilename="";

    bDrawLighting=true;
    bRenderStereo=true;
    bDrawMenu=true;
    bDrawNodes=true;
    bUseBlending=true;
    bMultisample=true;

    bFullscreen=false;

    lightLoc=Vector3f(0,3,15);                  //light Location
    ambient=Vector3f(1,1,1);

    fov=45;
    nearClip=0.2;
    farClip=1000;
    frustumTop=0.083;
    frustumBottom=-0.083;

    eyeDistance=0.10;
    bkgOffset = 50.0;

    mouseSensitivity=0.005;
    moveSpeed=0.1;

    screenX=0;
    screenY=0;
    windowX=0;
    windowY=0;

    lighting_tx = 0; // the light texture
    lighting_fb = 0; // the framebuffer object to render to that texture

    depth_tx = 0;
    depth_fb = 0;
    depth_size = 512;

    scene_tx = 0;
    scene_fb = 0;
    scene_size = 512;

    multiSample_fb = 0;
    multiSample_db = 0;
    multiSample_depth = 0;
    multiSample_color = 0;


    leftEye_tx = 0;
    leftEye_fb = 0;
    leftEyeDepth_tx = 0;
    leftEyeDepth_fb = 0;

    rightEye_tx = 0;
    rightEye_fb = 0;
    rightEyeDepth_tx = 0;
    rightEyeDepth_fb = 0;

    rightEyeFBO=NULL;
    leftEyeFBO=NULL;
    postOverlay=NULL;


	deltaTime=0.0;

	drawBuffers[0] = GL_COLOR_ATTACHMENT0_EXT;
	drawBuffers[1] = GL_COLOR_ATTACHMENT1_EXT;
	drawBuffers[2] = GL_COLOR_ATTACHMENT2_EXT;
	drawBuffers[3] = GL_COLOR_ATTACHMENT3_EXT;

}

Renderer::~Renderer(){

            glDeleteFramebuffersEXT(1, &lighting_fb);
            glDeleteFramebuffersEXT(1, &shadow_fb);


            glDeleteFramebuffersEXT(1, &depth_fb);
            glDeleteFramebuffersEXT(1, &scene_fb);

            glDeleteRenderbuffersEXT(1, &multiSample_db);
            glDeleteRenderbuffersEXT(1, &multiSample_depth);
            glDeleteRenderbuffersEXT(1, &multiSample_color);
            glDeleteFramebuffersEXT(1, &multiSample_fb);

}

Renderer* Renderer::getInstance(){

    if (rendererInstance)
        return rendererInstance;
    else{
        rendererInstance=new Renderer;
        return rendererInstance;
        }
}

//************************************************************
//
//  Windowing stuff and screen setup
//
//************************************************************

void Renderer::initWindow(int x, int y, string windowName){

    //screenX=x;
    //screenY=y;
    input->screenX=screenX;
    input->screenY=screenY;
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
//    glutInitDisplayString("rgba double depth>=24 sample=8");

    if (bFullscreen)
      {
      // windowXxwindowY, 32bit pixel depth, 60Hz refresh rate
      char* gmString  = new char[64];
      sprintf(gmString," %ix%i:32@60",windowX,windowY);
      glutGameModeString( gmString );
      // start fullscreen game mode
      glutEnterGameMode();
      }
    else
      {
      glutInitWindowSize(x,y);
      glutInitWindowPosition(input->windowX,input->windowY);
      glutCreateWindow(windowName.c_str());
      }
}



void Renderer::reDrawScreen(int w, int h){

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if(h == 0)
		h = 1;

//	float ratio = 1.0* w / h;

	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(fov,(screenY==0)?(1):((float)screenX/screenY),nearClip,farClip);
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(camActor->location.x, camActor->location.y,camActor->location.z,
		      camActor->zAxis.x,camActor->zAxis.y,camActor->zAxis.z,
			  camActor->yAxis.x, camActor->yAxis.y,camActor->yAxis.z);
    input->screenX=screenX;
    input->screenY=screenY;
}

// load render settings
void Renderer::loadPreferences(){


#ifndef TARGET_WIN32
	//switch to working directory!!
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyBundleURL(mainBundle);
    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
    {
        // error!
    }
    CFRelease(resourcesURL);

	string myPath=path;
	//get rid of Moviesandbox.app at the end!
	myPath.erase(myPath.end()-24, myPath.end() );

	cout << myPath << endl;
	chdir( ( ofToDataPath("..").c_str() ) );
#endif

    input=Input::getInstance();
    colladaLoader=new ColladaLoader();
    spriteMeshLoader=new SpriteMeshLoader();

    //open config xml file
    //configure renderer
    cout << "Loading Config file" <<endl;

    TiXmlDocument doc( "config.xml" );
    if (!doc.LoadFile()) {
        cout << "Cannot find config file, or config file corrupt. Exiting..." << endl;
        exit(0);
        }


    TiXmlHandle hDoc(&doc);
    TiXmlElement * element;
    TiXmlHandle hRoot(0);

    //***********************************************************************
    //Get the "Moviesandbox" element
    //***********************************************************************
    element=hDoc.FirstChildElement().Element();
    // should always have a valid root but handle gracefully if it doesn't
    if (!element) return;

    // save this for later
    hRoot=TiXmlHandle(element);

    //now load the configuration
    cout << "loading render settings" << endl;

    element=hRoot.FirstChild( "Rendersettings" ).Element();

    int val=0;
    double dVal=0.0;
    string mStr;

    //resolution
    element->Attribute("WindowSizeX", &val);
    windowX=val;
    element->Attribute("WindowSizeY", &val);
    windowY=val;

    //renderscreen
    element->Attribute("ScreenSizeX", &val);
    screenX=val;
    element->Attribute("ScreenSizeY", &val);
    screenY=val;

    //fullscreen on/off
    element->Attribute("bFullScreen", &val);
    bFullscreen=bool(val);

    backgroundTex=element->Attribute("BackgroundTex");


    //light drawing on/off
    mStr=element->Attribute("bDrawLighting", &val);
    bDrawLighting=bool(val);

    //stereo Render on/off
    mStr=element->Attribute("bRenderStereo", &val);
    bRenderStereo=bool(val);

    //multisampling on/off
    mStr=element->Attribute("bMultisample", &val);
    bMultisample=bool(val);

    element->Attribute("numSamples", &val);
    numSamples=val;

    //rendertarget texture resolutions
    element->Attribute("ShadowSize", &val);
    shadow_size=val;

    element->Attribute("SceneSize", &val);
    scene_size=val;


    element->Attribute("MouseSensitivity", &dVal);
    mouseSensitivity=dVal;
    element->Attribute("MoveSpeed", &dVal);
    moveSpeed=dVal;

    element->Attribute("FOV", &dVal);
    fov=dVal;

    //setting start scene
    startSceneFilename=element->Attribute("StartSceneFile");

    //setting libraries
    element=hRoot.FirstChild( "Library" ).Element();

    while (element){
        library.push_back(element->Attribute("Library"));
        element=element->NextSiblingElement("Library");
    }


}


//************************************************************
//
//  Setting up and calling all Actors' update function
//
//************************************************************

void Renderer::setup(){


    //generate Class and Type Lists
    fillGlobalLists();


   //create base layer

    addLayer("baseLayer");


    FreeImage_Initialise();


    input->setup();          //controller gets created here!


    glEnable(GL_TEXTURE_RECTANGLE_ARB);

    //this is setting up the menu - I don't want to make this xml based now, it's too complicated
    for (int i=0;i<(int)library.size();i++){
        input->loadTextures(library[i]);
        input->loadShaders(library[i]);
    }


    //background Color

#ifdef TARGET_WIN32
	if (!GLEE_EXT_framebuffer_multisample){
		bMultisample=false;
		cout << "Multisampling not supported for FBOs, switching them off..." << endl;
	}

    //also, set back dataPath

    //god, this took forever to figure out...

#endif

    //picking!


    cout << "Setup Error check: ";
    checkOpenGLError();

    glGenTextures(1, &pickTexture);
    glBindTexture(GL_TEXTURE_2D, pickTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB,  1, 1, 0, GL_BGRA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glBindTexture (GL_TEXTURE_2D, 0);

	int maxColorBuffers;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &maxColorBuffers);
	cout << "max colorbuffers: " << maxColorBuffers << endl;


    //frame buffer objects
    //always need them with layer system!

	//buffer to copy from for FSAA multisampling in FBOs
	createFBO(&multiSample_fb, NULL, &multiSample_db, scene_size, false, "multisampleBuffer");

    createFBO(&lighting_fb, &lighting_tx, NULL, scene_size, false, "lighting");
    createFBO(&shadow_fb, &shadow_tx, NULL, shadow_size, false, "shadow");

    cout << "Setup Error check: ";
    checkOpenGLError();

    //enable Blending for everyone!
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);


    /*
     *  Enable Hardware Point Sprites throughout
     */

    //Setup Point sprite textures,
    glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
    glTexEnvf( GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE );
    //Enable Vertex Shader point-size control
    glEnable( GL_VERTEX_PROGRAM_POINT_SIZE );

    //always enable (if disabling, re-enable afterwards!

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    if (bMultisample)
        glEnable(GL_MULTISAMPLE);   //just in case we force multisampling

    glEnable(GL_NORMALIZE);

    cout << "Setup Error check: ";
    checkOpenGLError();

}


void Renderer::createFBO(GLuint* fbObject, GLuint* fbTexture, GLuint* fbDepth, int fbSize, bool bDepth, string name){
    //-------------------------------------------------------
    // framebuffer object


    //set up renderbuffer

    int maxsamples;
    glGetIntegerv(GL_MAX_SAMPLES_EXT,&maxsamples);


	GLenum sampleType=GL_RGBA16F_ARB;
	//GLenum sampleType=GL_RGBA32F_ARB;

    if (!bDepth){

        if (name=="multisampleBuffer"){

			//see if 16bit multisample is allowed


            glGenRenderbuffersEXT(1, &multiSample_color);
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, multiSample_color);

            if (bMultisample)
                glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, numSamples, GL_RGBA16F_ARB, fbSize, fbSize);
            else
                glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, sampleType, fbSize, fbSize);

            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

			//COLOR COMPONENTS

			glGenRenderbuffersEXT(1, &multiSample_depth);
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, multiSample_depth);
            if (bMultisample)
                glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, numSamples, GL_RGBA16F_ARB, fbSize, fbSize);
            else
                glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, sampleType, fbSize, fbSize);
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

            glGenRenderbuffersEXT(1, &multiSample_pick);
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, multiSample_pick);
            if (bMultisample)
                glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, numSamples, GL_RGBA16F_ARB, fbSize, fbSize);
            else
                glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, sampleType, fbSize, fbSize);
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

            glGenRenderbuffersEXT(1, &multiSample_lightData);
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, multiSample_lightData);
            if (bMultisample)
                glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, numSamples, GL_RGBA16F_ARB, fbSize, fbSize);
            else
                glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, sampleType, fbSize, fbSize);
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

			//DEPTH COMPONENT

            glGenRenderbuffersEXT(1, &multiSample_db);
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, multiSample_db);
            if (bMultisample)
                glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, numSamples, GL_DEPTH_COMPONENT, fbSize, fbSize);
            else
                glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, fbSize, fbSize);

            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

            glGenFramebuffersEXT (1, fbObject);
            glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, *fbObject);


			// attach renderbuffer
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, multiSample_color);
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_RENDERBUFFER_EXT, multiSample_depth);
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_RENDERBUFFER_EXT, multiSample_pick);
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT3_EXT, GL_RENDERBUFFER_EXT, multiSample_lightData);
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, multiSample_db);

		}

        else{
            //glGenRenderbuffersEXT(1, fbDepth);
            //glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, *fbDepth);
            //glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, fbSize, fbSize);
            //glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

            glGenFramebuffersEXT (1, fbObject);
            glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, *fbObject);

            // attach renderbuffer
            //glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, *fbDepth);
        }
    }
    // create the texture we'll use for the shadow map

    if (name!="multisampleBuffer"){
        glGenTextures(1, fbTexture);
        glBindTexture(GL_TEXTURE_2D, *fbTexture);

        if (bDepth){

            glGenFramebuffersEXT (1, fbObject);
            glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, *fbObject);

            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

            glTexImage2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, fbSize, fbSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            //float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
            //glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR,borderColor);
                // set up hardware shadow mapping
                //this is needed to get good results for shadow2DProj
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);

                //this is needed to read a texture2D from a DepthMap!
                //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE);

            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
            glFramebufferTexture2DEXT (GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, *fbTexture, 0);
        }
        else{
            cout << "no depth in FBO!" << name << endl;
            // attach colorBuffer to a texture
            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
            glTexImage2D(GL_TEXTURE_2D, 0, sampleType,  fbSize, fbSize, 0, GL_RGBA, GL_FLOAT, NULL);

            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);

            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );


            glFramebufferTexture2DEXT (GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, *fbTexture, 0);
        }

    }


    if (bDepth){
        glDrawBuffer (GL_NONE);
        glReadBuffer (GL_NONE);
    }

    // verify all is well and restore state
    checkFBOStatus();
    glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

    glBindTexture (GL_TEXTURE_2D, 0);

    if (name=="multisampleBuffer")   return;

    textureList[name]=new textureObject;
    textureList[name]->texture=(uint)*fbTexture;
    textureList[name]->nextTexture="NULL";
    textureList[name]->frameRate=0;

    if (bDepth)
      textureList[name]->bAlpha=true;
    else
      textureList[name]->bAlpha=false;

    textureList[name]->bWrap=false;
    textureList[name]->texFilename="NULL";

    cout << "FBO texture name " << name << endl;

    //-------------------------------------------------------
    //end framebuffer object

}

void Renderer::checkFBOStatus(){

    GLenum status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
    switch (status){
        case GL_FRAMEBUFFER_COMPLETE_EXT:
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            cerr << "FBO configuration unsupported" << endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            cerr << "FBO DrawBuffer incomplete" << endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT:
            cerr << "FBO Multisample incomplete" << endl;
            break;


        default:
            cerr << "FBO programmer error" << endl;
            break;
    }
}



void Renderer::update(){


	float updateTime=glutGet(GLUT_ELAPSED_TIME);

	//first update Nodes!
	for (unsigned int i=0;i<buttonList.size();i++){
		Node* myNode=dynamic_cast<Node*>(buttonList[i]);
		NodeIO* myNodeIO=dynamic_cast<NodeIO*>(buttonList[i]);
		if (myNode && bDrawNodes)
			buttonList[i]->update(deltaTime);
		if (myNodeIO && bDrawNodes)
			buttonList[i]->update(deltaTime);
		if (!myNode && !myNodeIO)
			buttonList[i]->update(deltaTime);
	}

	//then update Actors!
	for (int i=0;i<(int)actorList.size();i++){
        actorList[i]->objectID=(float)i;
        actorList[i]->update(deltaTime);
	}

	//then helpers - we are using these for brush and grid and stuff
	for (int i=0;i<(int)helperList.size();i++){
        helperList[i]->update(deltaTime);
	}

	//then input!
	input->update(deltaTime);

	updateTime=glutGet(GLUT_ELAPSED_TIME) - updateTime;
	input->updateTime=updateTime;

}



void Renderer::addLayer(string layerName){

        Layer* lay = new Layer;
        lay->setup();
        lay->name=layerName;
        lay->textureID=lay->name+"_Color";
        lay->depthTextureID=lay->name+"_Depth";
        lay->pickTextureID=lay->name+"_Pick";
		lay->lightDataTextureID=lay->name+"_lightData";

        layerList.push_back(lay);
        createFBO(&(lay->colorFBO), &(lay->colorTex), NULL, scene_size, false, lay->textureID);
        createFBO(&(lay->depthFBO), &(lay->depthTex), NULL, scene_size, false, lay->depthTextureID);
        createFBO(&(lay->pickFBO),  &(lay->pickTex),  NULL, scene_size, false, lay->pickTextureID);
		createFBO(&(lay->lightDataFBO),  &(lay->lightDataTex),  NULL, scene_size, false, lay->lightDataTextureID);
        currentLayer=layerList.size()-1;

        cout << "Added new Layer:" << layerName << endl;

}


//************************************************************
//
//  Drawing to the screen - Actors, Buttons and RenderToTexture
//
//************************************************************


void Renderer::setupCamera(bool bCalculateMatrices){

//setup Projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(fov,(screenY==0)?(1):((float)screenX/screenY),nearClip,farClip);


//setup camera
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
/*
	gluLookAt(camActor->location.x, camActor->location.y,camActor->location.z,
		      camActor->zAxis.x,camActor->zAxis.y,camActor->zAxis.z,
			  camActor->yAxis.x, camActor->yAxis.y,camActor->yAxis.z);
*/
	gluLookAt(0,0,0,
		      0,0,-1,
			  0,1,0);

    if (bCalculateMatrices){
        glGetFloatv(GL_PROJECTION_MATRIX,projectionMatrix);
        glGetFloatv(GL_MODELVIEW_MATRIX,cameraMatrix);
        inverseCameraMatrix=cameraMatrix.inverse();
        inverseProjectionMatrix=projectionMatrix.inverse();
        //inverseCameraMatrix=cameraMatrix.transpose();
    }

}



void Renderer::draw(){

    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);


    glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

    setupCamera(true);


    //TODO:Stereo with Layers!
    /*
    if (bRenderStereo){
        drawStereoscopic();
    }
    */


	drawSceneTexture();

	/////////////////////////////////////////////////////
    /// 2D Elements from here
    /////////////////////////////////////////////////////

    /*
	 *	Set Ortho viewing transformation
	 */

    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,screenX,screenY,0,-1,1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    glActiveTexture(GL_TEXTURE0);

    glDisable(GL_DEPTH_TEST);

	/*
	 *	Draw Final Image
	 */

    glClearColor( backgroundColor.r,backgroundColor.g,backgroundColor.b,backgroundColor.a );
    glClear(GL_COLOR_BUFFER_BIT);

    for (int i=0;i<(int)layerList.size();i++){

		if (bDrawLighting){
            drawDeferredLighting(layerList[i]);
        }
        else
            layerList[i]->sceneShaderID="texture";
        //then, draw our final composite

        drawButton(layerList[i]);
    }

    /*
    *   DisplayDebug
    */

    setupShading("font");
    input->displayDebug();

    /*
	 *	Draw all Buttons
	 */

	draw2D();

    glUseProgram(0);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE);

    frames++;
    deltaTime=glutGet(GLUT_ELAPSED_TIME)-currentTime;
    currentTime=glutGet(GLUT_ELAPSED_TIME);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

    glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);

}

void Renderer::drawShadows(MsbLight* myLight){

	glPushAttrib(GL_VIEWPORT_BIT);
    //glShadeModel(GL_FLAT);
    glViewport (0, 0, shadow_size, shadow_size);

    //setup projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glPushMatrix();

    //set perspective

        gluPerspective(110.0f, 1.0f, nearClip, 1000.0f);             //this sets the framing of the light!

        glGetFloatv(GL_PROJECTION_MATRIX,lightProjectionMatrix);

        //setup camera
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glPushMatrix();

            Vector3f lAxis = myLight->location+myLight->zAxis;
            //lAxis=myLight->zAxis;
            gluLookAt( myLight->location.x, myLight->location.y, myLight->location.z,
            lAxis.x, lAxis.y, lAxis.z,
            0.0f, 1.0f, 0.0f);

            glGetFloatv(GL_MODELVIEW_MATRIX,lightViewMatrix);


    //glDisable(GL_BLEND);
    glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, multiSample_fb);
	GLenum depthOnly={GL_COLOR_ATTACHMENT1_EXT};
	glDrawBuffers(1,&depthOnly);
    for (int i=0;i<(int)layerList.size();i++){

        glClearColor( -1.0f, -1.0f, -1.0f, -1.0f );

        glClear( GL_COLOR_BUFFER_BIT |
                 GL_DEPTH_BUFFER_BIT );

        draw3D(layerList[i]);

      glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, multiSample_fb );
      glReadBuffer(GL_COLOR_ATTACHMENT1_EXT);

      glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, shadow_fb );
      glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

      glBlitFramebufferEXT( 0, 0, scene_size-1, scene_size-1, 0, 0, scene_size-1, scene_size-1, GL_COLOR_BUFFER_BIT, GL_NEAREST );

    }
    glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);

        glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glPopAttrib (); // restore the viewport

 }

void Renderer::drawSceneTexture(){

    glPushAttrib(GL_VIEWPORT_BIT);

    glViewport (0, 0, scene_size, scene_size);

    glMatrixMode(GL_MODELVIEW);

	float draw3DTime=glutGet(GLUT_ELAPSED_TIME);
/*
    int max_buffers;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &max_buffers);
    cout << "Max Draw Buffers: " << max_buffers << endl;
*/

    glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, multiSample_fb);

    glDrawBuffers(4,drawBuffers);

    glClearColor( -1.0f, -1.0f, -1.0f, -1.0f );


    for (int i=0;i<(int)layerList.size();i++){

        glClear( GL_COLOR_BUFFER_BIT |
                 GL_DEPTH_BUFFER_BIT );


        //disable blending for second, third and fourth buffer

        glDisableIndexedEXT(GL_BLEND,1);
        glDisableIndexedEXT(GL_BLEND,2);
        glDisableIndexedEXT(GL_BLEND,3);

        glActiveTexture(GL_TEXTURE0);


		 //drawbuffers are set up here!
        draw3D(layerList[i]);


        //color blitting

        glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, multiSample_fb );
        glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);

        glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, layerList[i]->colorFBO );
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

        glBlitFramebufferEXT( 0, 0, scene_size, scene_size, 0, 0, scene_size, scene_size, GL_COLOR_BUFFER_BIT, GL_NEAREST );

        //depth blitting
        glReadBuffer(GL_COLOR_ATTACHMENT1_EXT);

        glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, layerList[i]->depthFBO );
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

        glBlitFramebufferEXT( 0, 0, scene_size, scene_size, 0, 0, scene_size, scene_size, GL_COLOR_BUFFER_BIT, GL_NEAREST );

        //picking blitting
        glReadBuffer(GL_COLOR_ATTACHMENT2_EXT);

        glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, layerList[i]->pickFBO );
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

        glBlitFramebufferEXT( 0, 0, scene_size, scene_size, 0, 0, scene_size, scene_size, GL_COLOR_BUFFER_BIT, GL_NEAREST );

        //lightInfo blitting

        glReadBuffer(GL_COLOR_ATTACHMENT3_EXT);

        glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, layerList[i]->lightDataFBO );
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

        glBlitFramebufferEXT( 0, 0, scene_size, scene_size, 0, 0, scene_size, scene_size, GL_COLOR_BUFFER_BIT, GL_NEAREST );

	}

    //cleanup
    glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, 0 );
    glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, 0 );
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );

	pick(input->mouseX,input->mouseY);

	draw3DTime=glutGet(GLUT_ELAPSED_TIME) - draw3DTime;
	input->draw3DTime=draw3DTime;

    //TODO:draw brush here?

    glPopAttrib();
    //now draw the resulting image into a quad!
}

void Renderer::drawStereoscopic(){

    float right, left;

    right=screenX/screenY * frustumTop;
    left=screenX/screenY * frustumBottom;

    float nleft, nright;

    nright=right - (-0.0075 * right);                   //Thanks to Flocki for the magic numbers
    nleft =left  - (-0.0075 * right);

  glPushAttrib(GL_VIEWPORT_BIT);

    /***********************************************************
    Left Eye
    ************************************************************/

  glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, multiSample_fb);
  glDrawBuffers(2, drawBuffers);

    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    glClear( GL_COLOR_BUFFER_BIT |
			 GL_DEPTH_BUFFER_BIT );

    glViewport (0, 0, scene_size, scene_size);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glFrustum(nleft, nright, frustumBottom, frustumTop, nearClip, farClip);
    glTranslatef(eyeDistance,0,0);

    //drawBackground(-eyeDistance);
    glMatrixMode(GL_MODELVIEW);

    draw3D(layerList[0]);

    //color blitting
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
    glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, multiSample_fb );
    glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, leftEye_fb );
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glBlitFramebufferEXT( 0, 0, scene_size-1, scene_size-1, 0, 0, scene_size-1, scene_size-1, GL_COLOR_BUFFER_BIT, GL_NEAREST );

    //depth blitting
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
    glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, multiSample_fb );
    glReadBuffer(GL_COLOR_ATTACHMENT1_EXT);
    glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, leftEyeDepth_fb );
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glBlitFramebufferEXT( 0, 0, scene_size-1, scene_size-1, 0, 0, scene_size-1, scene_size-1, GL_COLOR_BUFFER_BIT, GL_NEAREST );

    //cleanup
    glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, 0 );
    glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, 0 );
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );


    /***********************************************************
    Right Eye
    ************************************************************/

    nright=right - (-0.0075 * right);                   //Thanks to Flocki for the magic numbers
    nleft =left  - (-0.0075 * right);


  glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, multiSample_fb);

    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    glClear( GL_COLOR_BUFFER_BIT |
			 GL_DEPTH_BUFFER_BIT );

    glViewport (0, 0, scene_size, scene_size);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glFrustum(nleft, nright, frustumBottom, frustumTop, nearClip, farClip);
    glTranslatef(-eyeDistance,0,0);

   // drawBackground(eyeDistance);
    glMatrixMode(GL_MODELVIEW);

    draw3D(layerList[0]);

  glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );

    //color blit
    glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, multiSample_fb );
    glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, rightEye_fb );
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glBlitFramebufferEXT( 0, 0, scene_size-1, scene_size-1, 0, 0, scene_size-1, scene_size-1, GL_COLOR_BUFFER_BIT, GL_NEAREST );
    //depth blit
    glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, multiSample_fb );
    glReadBuffer(GL_COLOR_ATTACHMENT1_EXT);
    glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, rightEyeDepth_fb );
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glBlitFramebufferEXT( 0, 0, scene_size-1, scene_size-1, 0, 0, scene_size-1, scene_size-1, GL_COLOR_BUFFER_BIT, GL_NEAREST );

    //cleanup
    glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, 0 );
    glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, 0 );
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );

  glPopAttrib (); // restore the viewport
}


void Renderer::drawDeferredLighting(Layer* layer){

       //bind depth and pick Textures
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, layer->depthTex);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, layer->pickTex);

		//bind fxTexture
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, layer->lightDataTex);

        //preserve our unlit color content
        string oldTextureID=layer->textureID;

        //bind lighting base texture and clear it
        glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, lighting_fb);

        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

        glClear( GL_COLOR_BUFFER_BIT |
                 GL_DEPTH_BUFFER_BIT );

        glBindFramebufferEXT (GL_FRAMEBUFFER_EXT,0);


        glPushAttrib(GL_VIEWPORT_BIT);
        glViewport (0, 0, scene_size, scene_size);

        //set our textureID to lighting pass
        layer->textureID="lighting";
        //set our shader to
        layer->sceneShaderID="deferredLight";

        ///loop from here for every shadowed light!

        for (int i=0;i<(int)lightList.size(); i++){

            if (lightList[i]->bCastShadows)
                drawShadows(lightList[i]);

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0,screenX,screenY,0,-1,1);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            float castShadow=(float)lightList[i]->bCastShadows;
            //update light
            glLightfv(GL_LIGHT0,GL_POSITION,&lightList[i]->location.x);
            glLightfv(GL_LIGHT0,GL_DIFFUSE,&lightList[i]->color.r);
            glLightfv(GL_LIGHT0,GL_LINEAR_ATTENUATION,&lightList[i]->lightDistance);
            glLightfv(GL_LIGHT0,GL_SPOT_CUTOFF,&castShadow);

            //just to make sure...
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, lighting_tx);

            //set shadowTexture
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, shadow_tx);

			//set shadowTexture
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, textureList[backgroundTex]->texture);

            ///light&shadow rendering

            //render lighting pass into multisampled (TODO: make non-multisampled!) lighting FBO
            //bind multisample FBO
            glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, lighting_fb);

            //only clear depth
            glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
            glClear( GL_DEPTH_BUFFER_BIT );

            //draw using lighting_tx as base texture!
            drawButton(layer);

            glBindFramebufferEXT( GL_FRAMEBUFFER_EXT,0);
        }             //repeat for every shadowed light!

        glPopAttrib();

        //set our textureID to lighting pass
        layer->textureID=oldTextureID;
        //set our shader to
        layer->sceneShaderID="post";

        //light as 3rd texture!
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, lighting_tx);

}



void Renderer::draw3D(Layer* currentLayer){


    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

	glDrawBuffers(4, drawBuffers);

    //draw color

    for (int i=0;i<(int)currentLayer->actorList.size(); i++){

        //update matrices when hidden
        if(currentLayer->actorList[i]->bHidden){
            glPushMatrix();
            transformActorMatrix(currentLayer->actorList[i]);
            glPopMatrix();
        }
        else {
            //draw in all buffers for pickable actors
            if (currentLayer->actorList[i]->bPickable){
                drawActor(currentLayer->actorList[i]);
            }
            //don't draw in picking buffer for non-pickable actors
        }
    }

    //reset texture Matrix transform
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);


    //this for xyz axis

    //throws openGL error!
/*

        //glDepthMask(GL_FALSE);
        setupShading("color");
        for (int i=0;i<(int)currentLayer->actorList.size();i++){
            drawOrientation(currentLayer->actorList[i]);
        }

*/

}

void Renderer::draw2D(){

    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    //colored Buttons  first
    for (unsigned int i=0;i<buttonList.size();i++){

                    Node* myNode=dynamic_cast<Node*>(buttonList[i]);
                    NodeIO* myNodeIO=dynamic_cast<NodeIO*>(buttonList[i]);
                    //if we disabled drawing nodes...
                    if ((myNode || myNodeIO) && bDrawNodes)
                        drawButton(buttonList[i]);
                    //always draw everything else
                    if (!myNode && !myNodeIO)
                        drawButton(buttonList[i]);
    }

    //finally font rendering
    setupShading("font");

    for (unsigned int i=0;i<buttonList.size();i++){

                Node* myNode=dynamic_cast<Node*>(buttonList[i]);
                NodeIO* myNodeIO=dynamic_cast<NodeIO*>(buttonList[i]);

                if ((myNode || myNodeIO) && bDrawNodes)
                    buttonList[i]->drawTooltip();

                if (!myNode && !myNodeIO)
                    buttonList[i]->drawTooltip();
    }

}

void Renderer::drawButton(BasicButton* b){

    //set Shader
    setupShading(b->sceneShaderID);
    b->updateShaders();

    //set Texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureList[b->textureID]->texture);

    glPushMatrix();

    //buttons only translate
    glTranslatef(b->location.x,b->location.y,b->location.z);

    //draw
    //TODO:phase out...
    b->drawPlane();
    //drawPlane(0.0, 0.0, b->scale.x, b->scale.y, b->color);

    glPopMatrix();

}

void Renderer::drawActor(Actor* a){


    if (a->bTextured)
        setupTexturing(a->textureID, a);

    //alpha blending
    glBlendFunc(a->blendModeOne,a->blendModeTwo);
//	glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
	glBlendFuncSeparate(a->blendModeOne,a->blendModeOne,GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    //start translating
    glPushMatrix();

    //translate according to base
    transformActorMatrix(a);

    //shader
    setupShading(a->sceneShaderID);
    a->updateShaders();

    if (!a->bZTest)  glDisable(GL_DEPTH_TEST);
    if (!a->bZWrite) glDepthMask(GL_FALSE);
    //set Color
    glColor4f(a->color.r,a->color.g,a->color.b,a->color.a);


    //Actual Drawing takes place here!
        if (a->drawType==DRAW_PLANE)            drawPlane(0.0,0.0,a->scale.x, -a->scale.y, a->color,true );
        else if (a->drawType==DRAW_VBOMESH)     drawColladaMesh(a);
        else if (a->drawType==DRAW_PARTICLES)   drawParticles(a);                 //Particles
        else if (a->drawType==DRAW_SPRITE)      a->drawSprite();
        else if (a->drawType==DRAW_CUBE)        drawCube(a->scale.x, a->scale.x);                 //Mesh
        else if (a->drawType==DRAW_TEA)         a->drawTeapot();
        else if (a->drawType==DRAW_SPECIAL)     a->draw();
        else if (a->drawType==DRAW_POINTPATCH)  drawPatch(a->scale.x,a->scale.y,a->particleScale);


    if (!a->bZTest)  glEnable(GL_DEPTH_TEST);
    if (!a->bZWrite) glDepthMask(GL_TRUE);

    //end translation
    glPopMatrix();
}

void Renderer::drawOrientation(Actor* a){

    //TODO: Plane orientation, yes/no?
    if (a->drawType==DRAW_PLANE)
        return;

    glPushMatrix();

    if (a->base){
        glMultMatrixf(a->base->baseMatrix);
        drawLine(Vector3f(0,0,0),(a->originalMatrix * a->transformMatrix).getTranslation(),Vector4f(1,1,1,1));
        glMultMatrixf(a->originalMatrix);
        glMultMatrixf(a->transformMatrix);
    }
    else{
        transformActorMatrix(a);
    }



    bool bComputeLight=a->bComputeLight;
    a->bComputeLight=false;

    a->updateShaders();

    //set color to specialSelected
    glColor4f(1,0,1,1);

    glLineWidth(4);

    //draw code for lines
    //red
    if (input->specialSelected!=a)
        glColor4f(1,0,0,1);
    glBegin(GL_LINES);
    glVertex3f(0,0,0);
    glVertex3f(1,0,0);
    glEnd();

    //green
    if (input->specialSelected!=a)
        glColor4f(0.0,1.0,0.0,1);
    glBegin(GL_LINES);
    glVertex3f(0,0,0);
    glVertex3f(0,1,0);
    glEnd();

    //blue
    if (input->specialSelected!=a)
        glColor4f(0,0,1,1);
    glBegin(GL_LINES);
    glVertex3f(0,0,0);
    glVertex3f(0,0,1);
    glEnd();

    //draw hirarchy

    a->bComputeLight=bComputeLight;

    glPopMatrix();
}


void Renderer::draw3DOverlay(){

      //reset blending
        glDisable(GL_BLEND);

        //glViewport(0,0,screenX,screenY);
        glViewport(screenX,0,screenX,screenY);
        //TODO: should be drawSFX
        glActiveTexture(GL_TEXTURE2);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureList["leftEyeDepthTexture"]->texture);
        glActiveTexture(GL_TEXTURE0);


        glLoadIdentity();
//translate all the way back!
//glTranslatef(bkgOffset * offset,0,0.0);

setupShading("post");

    GLuint uniform_location=0;
    uniform_location = glGetUniformLocation(shaderList["post"]->shader, "tex");
    glUniform1iARB(uniform_location, 0);

    uniform_location=0;
    uniform_location = glGetUniformLocation(shaderList["post"]->shader, "depthTex");
    glUniform1iARB(uniform_location, 2);

    //zoom!
    float zoom3Dx=0.0, zoom3Dy=0.0;

    glTranslatef(640,512,0);
    glScalef(zoom3Dx,zoom3Dy,1.0);
    glTranslatef(-640,-512,0);

//do for all FBOs too!

    glBindTexture(GL_TEXTURE_2D, textureList["leftEyeTexture"]->texture);

    glColor4f(1.0,1.0,1.0,1.0);

/*
    glBegin(GL_QUADS);
        glTexCoord2f(0.0,1.0);
		glVertex3f(leftTopXLeftEye ,leftTopYLeftEye,0);
        glTexCoord2f(1.0,1.0);
		glVertex3f(rightTopXLeftEye,rightTopYLeftEye,0.0);
        glTexCoord2f(1.0,0.0);
		glVertex3f(rightBottomXLeftEye,rightBottomYLeftEye,0.0);
        glTexCoord2f(0.0,0.0);
		glVertex3f(leftBottomXLeftEye,leftBottomYLeftEye,0.0);
	glEnd();



 //       drawButton(leftEyeFBO);


        glViewport(2*screenX,0,screenX,screenY);
        //glViewport(screenX,0,screenX,screenY);
        //TODO: should be drawSFX

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, textureList["rightEyeDepthTexture"]->texture);
        glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, textureList["rightEyeTexture"]->texture);

    glColor4f(1.0,1.0,1.0,1.0);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0,1.0);
		glVertex3f(leftTopXRightEye ,leftTopYRightEye,0);
        glTexCoord2f(1.0,1.0);
		glVertex3f(rightTopXRightEye,rightTopYRightEye,0.0);
        glTexCoord2f(1.0,0.0);
		glVertex3f(rightBottomXRightEye,rightBottomYRightEye,0.0);
        glTexCoord2f(0.0,0.0);
		glVertex3f(leftBottomXRightEye ,leftBottomYRightEye,0.0);
	glEnd();
*/
//        drawButton(rightEyeFBO);

        //reset viewport
        glViewport(0,0,screenX,screenY);
        glLoadIdentity();
        //reset blending
        glEnable(GL_BLEND);
}

void Renderer::drawBone(float width, float height, float depth){


}

void Renderer::drawCube(float scale, float cubeSize){


glutSolidCube( cubeSize / max(scale , 1.0f) );
}

void Renderer::drawPlane(float x1,float  y1,float  x2,float  y2, Vector4f color, bool bCentered){



	//draw centered!
        //TODO:can be optional?
        float xOffset=0.0;
        float yOffset=0.0;

        if (bCentered){
            xOffset=(x2-x1)/2.0;
            yOffset=(y2-y1)/2.0;
        }


        GLfloat verts[] = { x1-xOffset, y1-yOffset,
                            x1-xOffset, y2-yOffset,
                            x2-xOffset, y2-yOffset,
                            x2-xOffset, y1-yOffset };

		GLfloat tex_coords[] = { 0, 0,
                                 0, 1,
                                 1, 1,
                                 1, 0 };

        GLfloat normals[] = { 0, 0, 1,
                              0, 0, 1,
                              0, 0, 1,
                              0, 0, 1 };
        GLfloat vColor[] ={ color.r, color.g, color.b, color.a,
                            color.r, color.g, color.b, color.a,
                            color.r, color.g, color.b, color.a,
                            color.r, color.g, color.b, color.a };

        glEnableClientState( GL_VERTEX_ARRAY );
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        glEnableClientState( GL_NORMAL_ARRAY );
        glEnableClientState( GL_COLOR_ARRAY);

		glTexCoordPointer(2, GL_FLOAT, 0, tex_coords );
		glVertexPointer(2, GL_FLOAT, 0, verts );
        glNormalPointer(GL_FLOAT, 0, normals );
        glColorPointer(4,GL_FLOAT,0, vColor);

		glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );

        glDisableClientState( GL_VERTEX_ARRAY );
		glDisableClientState( GL_TEXTURE_COORD_ARRAY );
        glDisableClientState( GL_NORMAL_ARRAY );
        glDisableClientState( GL_COLOR_ARRAY);


}

void Renderer::drawPatch(float width, float height, float resolution){

    //create a vertex array for a quad patch with "resolution" amount of vertices per side
    //lets do points for now...

    vector<Vector4f> vertices;
    vector<GLfloat> texCoords;

    for (int h=0;h<resolution;h++){

        //for every line...
        for (int l=0;l<resolution;l++){
            Vector4f myVertex;
            myVertex.x=float(l) * width/(resolution-1.0) - width/2.0f;            //x-coord
            myVertex.y=float(h) * height/(resolution-1.0) - height/2.0f;
            myVertex.z=0.0f;
            myVertex.w=1.0f;

            vertices.push_back(myVertex);


            //texCoords.push_back( float(l) /(resolution-1.0) );        //x-texCoord
            //texCoords.push_back( float(h) /(resolution-1.0) );        //y-texCoord
            texCoords.push_back( float(l));        //x-texCoord
            texCoords.push_back( float(h));        //y-texCoord

        }
    }

    // activate and specify pointer to vertex array
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    GLfloat *verts=&vertices[0].x;

    glVertexPointer(4, GL_FLOAT, 0, verts);
    glTexCoordPointer(2, GL_FLOAT, 0, &texCoords[0]);

    // draw the patch as points
    glDrawArrays(GL_POINTS, 0, resolution* resolution );

    // deactivate vertex arrays after drawing
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void Renderer::drawLine(Vector3f start, Vector3f end, Vector4f startColor, Vector4f endColor){

    glLineWidth(4.0);

    glBegin(GL_LINES);
        glColor4f(startColor.r,startColor.g,startColor.b,startColor.a);
        glVertex3f(start.x,start.y,start.z);
        glColor4f(endColor.r,endColor.g,endColor.b,endColor.a);
        glVertex3f(end.x,end.y,end.z);
    glEnd();

}


void Renderer::drawParticles (Actor* a){

    MeshData* myMesh=vboList[a->vboMeshID];

    if (!myMesh)
        return;

    if (myMesh->bTextured)
        glEnable( GL_POINT_SPRITE_ARB );

    if (myMesh->vData.size()>0){

          GLfloat   *vertexIDs=&myMesh->vData[0].vertexID;

          GLfloat *verts=&myMesh->vData[0].location.x;
          GLfloat *normals=&myMesh->vData[0].normal.x;
          GLfloat *colors=&myMesh->vData[0].color.r;
          GLfloat *secondaryColors=&myMesh->vData[0].secondaryColor.r;
          GLfloat *vertexWeights=&myMesh->vData[0].vertexWeights.x;
          GLfloat *boneReferences=&myMesh->vData[0].boneReferences.x;



          glEnableClientState(GL_VERTEX_ARRAY);
          glEnableClientState(GL_NORMAL_ARRAY);
          glEnableClientState(GL_COLOR_ARRAY);
          glEnableClientState(GL_SECONDARY_COLOR_ARRAY);

		  glVertexPointer(4, GL_FLOAT,sizeof(myMesh->vData[0]),verts);
          glNormalPointer(GL_FLOAT,sizeof(myMesh->vData[0]),normals);
		  glColorPointer(4, GL_FLOAT,sizeof(myMesh->vData[0]),colors);
		  glSecondaryColorPointer(3, GL_FLOAT,sizeof(myMesh->vData[0]),secondaryColors);



			//vertexID from here
            GLint indexThree;
            indexThree=glGetAttribLocation(shaderList[a->sceneShaderID]->shader,"vertexID");
            glEnableVertexAttribArray(indexThree);
            glVertexAttribPointer(indexThree,1,GL_FLOAT,false,sizeof(myMesh->vData[0]),vertexIDs);
            GLint indexOne,indexTwo;

            //skeletal Stuff from here

          glDrawArrays(GL_POINTS,0,myMesh->vData.size());

          glDisableClientState(GL_VERTEX_ARRAY);
          glDisableClientState(GL_NORMAL_ARRAY);
          glDisableClientState(GL_COLOR_ARRAY);
          glDisableClientState(GL_SECONDARY_COLOR_ARRAY);

          glDisableVertexAttribArray(indexThree);
      }

      if (myMesh->bTextured)
        glDisable(GL_POINT_SPRITE_ARB);
}

void Renderer::drawColladaMesh (Actor* a){

    MeshData* myMesh=vboList[a->vboMeshID];

        if (!myMesh)
            return;

	glPushMatrix();


        glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, myMesh->vertexBufferObject[0]);
        glVertexPointer(myMesh->verticesPerShapeCount, GL_FLOAT, 0, 0);



        glBindBufferARB(GL_ARRAY_BUFFER_ARB, myMesh->normalBufferObject[0]);
        glNormalPointer(GL_FLOAT, 0, 0);


        glBindBufferARB(GL_ARRAY_BUFFER_ARB, myMesh->texCoordBufferObject[0]);
        glTexCoordPointer(myMesh->texCoordPerVertexCount, GL_FLOAT, 0, 0);

        if (myMesh->colorBufferObject.size()>0){

            glEnableClientState(GL_COLOR_ARRAY);
			glEnableClientState(GL_SECONDARY_COLOR_ARRAY);

            glBindBufferARB(GL_ARRAY_BUFFER_ARB, myMesh->colorBufferObject[0]);
            glColorPointer(4, GL_FLOAT, 0, 0);

			glBindBufferARB(GL_ARRAY_BUFFER_ARB, myMesh->secondaryColorBufferObject[0]);
			glSecondaryColorPointer(3,GL_FLOAT, 0, 0);
        }



        //skinning from here!
        //use two index arrays for skinning
        GLint indexOne,indexTwo;

        if (myMesh->bIsSkeletal){

            indexOne=glGetAttribLocation(shaderList["skeletal"]->shader,"boneReferences");
			glEnableVertexAttribArray(indexOne);
			glBindBufferARB(GL_ARRAY_BUFFER, myMesh->boneReferenceObject[0]);
			glVertexAttribPointer(indexOne,4,GL_FLOAT,false,0,0);

			indexTwo=glGetAttribLocation(shaderList["skeletal"]->shader,"vertexWeights");
			glEnableVertexAttribArray(indexTwo);
			glBindBufferARB(GL_ARRAY_BUFFER, myMesh->vertexWeightsObject[0]);
			glVertexAttribPointer(indexTwo,4,GL_FLOAT,false,0,0);
		}




		glDrawArrays(myMesh->vertexInterpretation, 0, myMesh->vertexCount[0]);

        glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        if (myMesh->bIsSkeletal){
        	glDisableVertexAttribArray(indexOne);
			glDisableVertexAttribArray(indexTwo);
            }

        if (myMesh->colorBufferObject.size()>0){
                glDisableClientState(GL_COLOR_ARRAY);
                glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
        }

    glPopMatrix();

}


void Renderer::drawSprite(){

    glBegin(GL_POINTS);
    glVertex3f(0,0,0);
    glEnd();
}

void Renderer::setupShading(string shaderName){

    if (shaderName!=lastShader && shaderList[shaderName]){
        glUseProgram(shaderList[shaderName]->shader);
        lastShader=shaderName;
        }
    if (!shaderList[shaderName])
        cout << "found bad shader: " << shaderName << endl;

}

void Renderer::setupTexturing(string texName, Actor* a){

    if (!a->ofTexturePtr){
        glBindTexture(GL_TEXTURE_2D, textureList[texName]->texture);
    }else{
        glEnable(a->ofTexturePtr->texData.textureTarget);
        glEnable (GL_ARB_texture_rectangle);
        glBindTexture(a->ofTexturePtr->texData.textureTarget, a->ofTexturePtr->texData.textureID);
    }

    if (!a)
        return;

    //texture animation
    if (textureList[texName]->nextTexture!="NULL" && currentTime - a->textTimer > textureList[texName]->frameRate ){
        a->textTimer += textureList[texName]->frameRate;
        a->textureID=textureList[texName]->nextTexture;
        }

    transformTextureMatrix(a);

}



void Renderer::transformActorMatrix(Actor* a){


    glMultMatrixf(a->baseMatrix);

    a->orientation=a->location+a->zAxis;
}

void Renderer::transformTextureMatrix(Actor* a){

    glActiveTexture(GL_TEXTURE0);
    glMatrixMode( GL_TEXTURE );
    glLoadIdentity();

        // make changes to the texture
        glTranslatef(a->texTranslation.x,a->texTranslation.y,a->texTranslation.z);

        glRotatef(a->texRotation.x,1,0,0);
        glRotatef(a->texRotation.y,0,1,0);
        glRotatef(a->texRotation.z,0,0,1);

        glScalef(a->texScale.x,a->texScale.y,a->texScale.z);

    glMatrixMode(GL_MODELVIEW);
}

//************************************************************
//
//Picking: determining what Actor the mouse points to
//         and mouse 3d coordinate
//
//************************************************************


//picking needs mouse coordinates
void Renderer::pick(int x, int y){

    ///World Position and object ID
    //draw pickTex of current layer, just on mouse coordinate, one pixel wide
    //read pixel color at mouse coordinate
    //color = xyz location
    //alpha = object id

    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, layerList[currentLayer]->pickFBO );
    float mousePos[4];

    //create small picking texture
    glBindTexture(GL_TEXTURE_2D,pickTexture);
    float xRatio=(float)scene_size/(float)screenX;
    float yRatio=(float)scene_size/(float)screenY;



    glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,(int) (input->mouseX * xRatio),(int) ((screenY-input->mouseY)*yRatio) ,1 ,1 );
    glGetTexImage(GL_TEXTURE_2D,0,GL_BGRA,GL_FLOAT,&mousePos);


    input->mouse3D.x=mousePos[2];
    input->mouse3D.y=mousePos[1];
    input->mouse3D.z=mousePos[0];

    if (mousePos[3]>=0){
        int aID=(int)ceil(mousePos[3]);
        if ((int) actorList.size() > aID)
            input->worldTarget=actorList[aID];
    }
    else
        input->worldTarget=NULL;

    //special stuff
    //grid
    if ((int)floor(mousePos[3])==-2)
        input->worldTarget=grid;



    /// World Normal!

    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, layerList[currentLayer]->depthFBO );
    //create small picking texture
    glBindTexture(GL_TEXTURE_2D,pickTexture);
    xRatio=(float)scene_size/(float)screenX;
    yRatio=(float)scene_size/(float)screenY;

    glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,(int) (input->mouseX * xRatio),(int) ((screenY-input->mouseY)*yRatio) ,1 ,1 );
    glGetTexImage(GL_TEXTURE_2D,0,GL_BGRA,GL_FLOAT,&mousePos);
    //get normal!
    Vector4f myNormal;
    myNormal.x=mousePos[2];
    myNormal.y=mousePos[1];
    myNormal.z=mousePos[0];
    myNormal.w=0.0;
    myNormal= inverseCameraMatrix * myNormal;

    //TODO: implement vertexID
    //int vertexID = (int)ceil(mousePos[2] * 65536.0) + (int)ceil(mousePos[1]);

  //  cout << "vertexID: " <<vertexID << endl;

    input->worldNormal.x=myNormal.x;
    input->worldNormal.y=myNormal.y;
    input->worldNormal.z=myNormal.z;

    input->worldNormal.normalize();

	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT,0);
    glBindTexture(GL_TEXTURE_2D,0);

}

//************************************************************
//
//       Texture and Shader Loading and Initialisation functions
//
//
//************************************************************

//generates a texture from a RAW file - needs implementation of textureList!
GLuint Renderer::LoadTextureRAW( const char * filename,int size, int wrap ){
}

bool Renderer::LoadTextureTGA( string filename, bool wrap, bool bAlpha, string texID ){

    GLuint texture;

    FIBITMAP * myBitmap = FreeImage_Load(FIF_TARGA,filename.c_str(),0);

    FreeImage_FlipVertical(myBitmap);

    //allocate texture List
    glGenTextures( 1, &texture );

    // select our current texture
    glBindTexture( GL_TEXTURE_2D, texture );

    // when texture area is small, bilinear filter the closest mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR );
    // when texture area is large, bilinear filter the first mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap ? GL_REPEAT : GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap ? GL_REPEAT : GL_CLAMP );

    // build our texture and mipmaps
    if (bAlpha)
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, FreeImage_GetWidth(myBitmap), FreeImage_GetHeight(myBitmap), 0, GL_BGRA, GL_UNSIGNED_BYTE, FreeImage_GetBits(myBitmap) );
    else
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, FreeImage_GetWidth(myBitmap), FreeImage_GetHeight(myBitmap), 0, GL_BGR, GL_UNSIGNED_BYTE, FreeImage_GetBits(myBitmap) );

    FreeImage_Unload(myBitmap);

    textureList[texID]=new textureObject;
    textureList[texID]->texture=texture;
    textureList[texID]->bAlpha=bAlpha;
    textureList[texID]->bWrap=wrap;
    textureList[texID]->texFilename=filename;

    return true;
}


bool Renderer::loadShader(string vertexShaderFileName, string fragmentShaderFileName, string shaderProgramName){

    GLuint fragmentShader;
    GLuint vertexShader;

    GLuint shaderProgram;


    char * vertexShaderFile;               //actually holds the whole file
    char * fragmentShaderFile;             //actually holds the whole file

    cout << "*************************************************************" << endl;

    //setup shader
    vertexShader=glCreateShader(GL_VERTEX_SHADER);
    fragmentShader=glCreateShader(GL_FRAGMENT_SHADER);

    cout << "processing: " << vertexShaderFileName << "\n";
    cout << "processing: " << fragmentShaderFileName << "\n";

    vertexShaderFile=textFileRead((char*)vertexShaderFileName.c_str());
    fragmentShaderFile=textFileRead((char*)fragmentShaderFileName.c_str());

    const char* ptrV = vertexShaderFile;
    const char* ptrF = fragmentShaderFile;

    glShaderSource(vertexShader, 1, &ptrV,NULL);
    glShaderSource(fragmentShader, 1, &ptrF,NULL);

    glCompileShader(vertexShader);
    if (vertexShader==0){
      cout << "could not compile vertex shader " << vertexShaderFileName << endl;
      return false;
    }

    glCompileShader(fragmentShader);
    if (fragmentShader==0){
      cout << "could not compile fragment shader " << fragmentShaderFileName << endl;
      return false;
    }

    cout << "Info log for " << vertexShaderFileName << endl;
    printShaderInfoLog(vertexShader);

    cout << "Info log for " << fragmentShaderFileName << endl;
    printShaderInfoLog(fragmentShader);

    //Link shaders
    shaderProgram=glCreateProgram();
    if (shaderProgram==0){
      cout << "could not compile shader " << shaderProgramName << endl;
      return false;
    }

    glAttachShader(shaderProgram,vertexShader);
    glAttachShader(shaderProgram,fragmentShader);

    glLinkProgram(shaderProgram);

    cout << "Info log for " << shaderProgramName << endl;
    printProgramInfoLog(shaderProgram);


    //cleanUp
    free(vertexShaderFile);
    free(fragmentShaderFile);

    shaderList[shaderProgramName]=new shaderObject;
    shaderList[shaderProgramName]->shader=shaderProgram;
    shaderList[shaderProgramName]->vertexShaderFilename=vertexShaderFileName;
    shaderList[shaderProgramName]->fragmentShaderFilename=fragmentShaderFileName;
    cout << "registered program!" << shaderProgram << "\n";

    cout << "*************************************************************" << endl;
    return true;
}

void Renderer::printShaderInfoLog(GLuint obj){
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
        free(infoLog);
    }
}

void Renderer::printProgramInfoLog(GLuint obj){
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
        free(infoLog);
    }
}

void Renderer::checkOpenGLError(){

    GLenum err=glGetError();

    switch(err){

        case GL_INVALID_ENUM:
            cout << "ERROR: invalid enum" << endl;
            break;

        case GL_INVALID_VALUE:
            cout << "ERROR: invalid value" << endl;
            break;

        case GL_INVALID_OPERATION:
            cout << "ERROR: invalid operation" << endl;
            break;

        case GL_STACK_OVERFLOW:
            cout << "ERROR: stack overflow" << endl;
            break;

        case GL_STACK_UNDERFLOW:
            cout << "ERROR: stack underflow" << endl;
            break;

        case GL_OUT_OF_MEMORY:
            cout << "ERROR: out of memory" << endl;
            break;

        default:
            cout << "No Error" << endl;
            break;
    }
}
