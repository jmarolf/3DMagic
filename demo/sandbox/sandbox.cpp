/* 
Copyright (c) 2011 Andrew Keating

This file is part of 3DMagic.

3DMagic is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

3DMagic is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with 3DMagic.  If not, see <http://www.gnu.org/licenses/>.

*/
/** Generates a 3D environment used to test different features
 */

// 3DMagic includes
#include <3DMagic.h>
using namespace Magic3D;

#include "../DemoBase.h"

// test
#include <Math/Generic/Vector.h>

// SDL includes
#include <SDL/SDL.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <memory>
#include <sstream>
#include <iostream>
#include <iomanip>
using std::cout;
using std::endl;
using std::shared_ptr;

// include freetype
#include <ft2build.h>
#include FT_FREETYPE_H // yes it's a macro include and yes it's the standard way

#define ROOM_SIZE (20.0f * FOOT)


Matrix4 projectionMatrix;

// resource manager
ResourceManager resourceManager;

// batches
std::shared_ptr<Mesh> floorBatch;
std::shared_ptr<Mesh> sphereBatch;
std::shared_ptr<Mesh> tinySphereBatch;
std::shared_ptr<Mesh> bigSphereBatch;
std::shared_ptr<Mesh> boxBatch;

// materials
std::shared_ptr<Material> floorMaterial;
std::shared_ptr<Material> sphereMaterial;
std::shared_ptr<Material> tinySphereMaterial;
std::shared_ptr<Material> bigSphereMaterial;

// collisions shapes
auto floorShape = std::make_shared<PlaneCollisionShape>( Vector3(0,1,0) );
auto sphereShape = std::make_shared<SphereCollisionShape>( 2*FOOT );
auto tinySphereShape = std::make_shared<SphereCollisionShape>( 1*FOOT );
auto bigSphereShape = std::make_shared<BoxCollisionShape>( 3.0f, 3.0f, 3.0f );

// objects
Object* bigBall;
Object* laser;
Object* floorObject;
Object* ceiling;
Object* wallObject;

// shader uniforms

Color groundColor(25,25,25);
float groundColorf[3];
Color skyColor(255,255,255);
float skyColorf[3];

// shaders
std::shared_ptr<GpuProgram> shader;
bool wireframe = false;

int screenWidth = 0;
int screenHeight = 0;

// tracks game time
StopWatch	timer;
StopWatch   physicsTimer;

bool lockCursor = false;
bool moveForward = false;
bool moveBack = false;
bool moveLeft = false;
bool moveRight = false;
bool releaseWater = false;

// builders
MeshBuilder batchBuilder;
MaterialBuilder materialBuilder;

// 3ds stuff
Mesh* chainMeshes;
Texture* chainTex;
Object* chainObject;

//FT_Face face;
StaticFont* font;
Image charImage(120, 120, 4);

Object* btBall; // graphical presence of ball used for bullet
Object* btBox;
bool fun = false;


bool paused = true;
int slow = 0;
float change = -1.0f;

/** Called when a normal key is pressed on the keyboard
 * @param key the key pressed
 * @param x the x-coord of the mouse at the time of the press
 * @param y the y-coord of the mouse at the time of the press
 */
void keyPressed(int key, FPCamera& camera, GraphicsSystem& graphics, World& world)
{
	Point3 origin;
	Vector3 forward;
	Vector3 side;
	Vector3 up;
	btTransform transform;
	Position p;
	Object* t;
    std::vector<Object*>::iterator it;
    //int i;
    Object::Properties prop;
    
	switch(key)
	{
		// space
		case ' ':
			/*if (wireframe)
				wireframe = false;
			else
				wireframe = true;*/
			camera.setLocation( Point3(0.0f, 6.0f * FOOT, 20.0f * FOOT) );
			//camera.getPosition().getForwardVector().set(0.0f, 0.0f, -1.0f);
			//camera.getPosition().getUpVector().set(0.0f, 1.0f, 0.0f);
			
			// manually set new position for ball
			break;
			
		// escape
		case 0x1B:
			exit(1);
			
		// w, forward
		case 'w':
			//cameraFrame.translate(cameraFrame.getForwardVector().getX()*FOOT, 0.0f, 
			//					  cameraFrame.getForwardVector().getZ()*FOOT);
			moveForward = true;
			break;
			
		// s, backward
		case 's':
			//cameraFrame.translate(-cameraFrame.getForwardVector().getX()*FOOT, 0.0f, 
			//					  -cameraFrame.getForwardVector().getZ()*FOOT);
			moveBack = true;
			break;
			
		// a, strafe left
		case 'a':
			//cameraFrame.getLocalXAxis(side);
			//cameraFrame.translate(side.getX()*FOOT, 0.0f, side.getZ()*FOOT);
			moveLeft = true;
			break;
			
		// d, strafe right
		case 'd':
			// can only move in the xz plane
			//cameraFrame.getLocalXAxis(side);
			//cameraFrame.translate(-side.getX()*FOOT, 0.0f, -side.getZ()*FOOT);
			moveRight = true;
			break;
			
		case '-':
			camera.elevate( -3*FOOT );
			break;
			
		case '=':
			camera.elevate( 3*FOOT );
			break;
			
		case 'g':
		    prop.mass = 1;
			t = new Object(std::make_shared<Model>(std::make_shared<Meshes>(bigSphereBatch), 
				bigSphereMaterial, bigSphereShape), prop );
			t->setLocation(Point3(0.0f, 5.0f, 0.0f));
			world.addObject(t);
			
			camera.lookat( Point3(0, 30, 0) );
			
			break;
		case 'h':
		    releaseWater = true;
			break;
		case 'p':
			if (paused)
			{
				paused = false;
				world.alignPhysicsStepToFPS(true);
			}
			else
			{
				paused = true;
				world.alignPhysicsStepToFPS(false);
				world.setPhysicsStepsPerFrame(0);
			}
			break;
			
		case 'z':
			break;
			
		case 'u':
		    if (lockCursor)
		    {
		        graphics.showCursor( true );
		        lockCursor = false;
		    }
		    else
		    {
		        graphics.warpMouse(screenWidth / 2, screenHeight / 2);
		        graphics.showCursor( false );
		        lockCursor = true;
		    }
		    break;
		    
		case ',':
		    if (slow != 0)
		        slow--;
		    cout << "physics speed is " << slow << "x" << endl;
		    break;
		    
		case '.':
		    slow++;
		    cout << "physics is " << slow << "x" << endl;
		    break;
		    
		case 'x':
		    fun = !fun;
		    break;
		    
		case 't':
		    if (chainObject)
		    {
		        world.removeObject(chainObject);
		        delete chainObject;
		        chainObject = NULL;
		    }
		    break;
		    
		case 'k':
		    wireframe = !wireframe;
		    world.setWireFrame(wireframe);

        default:
            break;
        
	}
	
}

/** Called when a normal key is pressed on the keyboard
 * @param key the key pressed
 * @param x the x-coord of the mouse at the time of the press
 * @param y the y-coord of the mouse at the time of the press
 */
void keyReleased(int key)
{	
	
	switch(key)
	{
			
		// w, forward
		case 'w':
			//cameraFrame.translate(cameraFrame.getForwardVector().getX()*FOOT, 0.0f, 
			//					  cameraFrame.getForwardVector().getZ()*FOOT);
			moveForward = false;
			break;
			
		// s, backward
		case 's':
			//cameraFrame.translate(-cameraFrame.getForwardVector().getX()*FOOT, 0.0f, 
			//					  -cameraFrame.getForwardVector().getZ()*FOOT);
			moveBack = false;
			break;
			
		// a, strafe left
		case 'a':
			//cameraFrame.getLocalXAxis(side);
			//cameraFrame.translate(side.getX()*FOOT, 0.0f, side.getZ()*FOOT);
			moveLeft = false;
			break;
			
		// d, strafe right
		case 'd':
			// can only move in the xz plane
			//cameraFrame.getLocalXAxis(side);
			//cameraFrame.translate(-side.getX()*FOOT, 0.0f, -side.getZ()*FOOT);
			moveRight = false;
			break;
			
		case 'h':
		    releaseWater = false;
		    break;
        
	}
	
}

/** Called when a special key is pressed on the keyboard
 * @param key the key pressed
 * @param x the x-coord of the mouse at the time of the press
 * @param y the y-coord of the mouse at the time of the press
 */
void specialKeyPressed(int key, int x, int y)
{
	
}


/** Called when the mouse is clicked
 * @param button the button on the mouse that was clicked (GLUT_LEFT_BUTTON,GLUT_MIDDLE_BUTTON, or GLUT_RIGHT_BUTTON)
 * @param state either GLUT_UP or GLUT_DOWN
 * @param x the x-coord of the mouse at the time of the press
 * @param y the y-coord of the mouse at the time of the press
 */
void mouseClicked(Event::MouseButtons button, int x, int y, FPCamera& camera, World& world)
{
	
	Position p;
	Object* t;
	static float speed = 1000 * 300;
	Object::Properties prop;
	
	switch(button)
	{
	    case Event::LEFT:
			p.set(camera.getPosition());
			p.translateLocal(0.0f, -1.5f*FOOT, -2.0f*FOOT);
			
			prop.mass = 100;
			t = new Object(std::make_shared<Model>(std::make_shared<Meshes>(sphereBatch), 
				sphereMaterial, sphereShape), prop);
			t->setPosition(p);
			world.addObject(t);
			t->applyForce(Vector3(p.getForwardVector().x()*speed, 
		        p.getForwardVector().y()*speed, p.getForwardVector().z()*speed) );
			break;
			
		case Event::MIDDLE: 
        case Event::RIGHT: 
        case Event::WHEEL_UP: 
        case Event::WHEEL_DOWN:
            break;
	}
}

/** Called when the mouse is moved with a button pressed
 * @param x the x-coord of the mouse pointer
 * @param y the y-coord of the mouse pointer
 */
void mouseMoved(int x, int y)
{
	
}


#define Y_AXIS_SENSITIVITY 0.3f
#define X_AXIS_SENSITIVITY 0.3f

/** Called when the mouse is moved without a button pressed
 * @param x the x-coord of the mouse pointer
 * @param y the y-coord of the mouse pointer
 */
void mouseMovedPassive(int x, int y, FPCamera& camera, GraphicsSystem& graphics)
{
    if (!lockCursor)
        return;
    
	// avoid reprocess from warp pointer call
	if (x == (screenWidth/2) && y == (screenHeight/2))
		return;
	
	camera.panView( -(x - (screenWidth/2))  * X_AXIS_SENSITIVITY, 
	                 (y - (screenHeight/2)) * Y_AXIS_SENSITIVITY 
	              );

	graphics.warpMouse(screenWidth / 2, screenHeight / 2);
}

class Sandbox : public DemoBase
{
	std::shared_ptr<Texture> charTex;
	std::shared_ptr<Texture> screenTex;

public:

	void setup()
	{
		resourceManager.addResourceDir("../../../../resources/");
		resourceManager.addResourceDir("../../../../../resources/");

		// bullet setup
		physics.setGravity(0,-9.8f*METER,0);

		graphics.enableDepthTest();

		static Color lightBlue(5, 230, 255);
		graphics.setClearColor(lightBlue );

		// init textures
		auto stoneTex = resourceManager.get<Texture>("textures/bareConcrete.tex.xml");
		auto marbleTex = resourceManager.get<Texture>("textures/marble.tex.xml");
		auto brickTex = resourceManager.get<Texture>("textures/singleBrick.tex.xml");

		Image blueImage( 1, 1, 4, Color(31, 97, 240, 255) );
		auto blueTex = std::make_shared<Texture>(blueImage);
		blueTex->setWrapMode(Texture::CLAMP_TO_EDGE);

		shared_ptr<FontResource> dejavuResource = resourceManager.get<FontResource>
			( "fonts/dejavu/DejaVuSerif-Italic.ttf" );
		Character q_char;
		dejavuResource->getMissingChar(&q_char, 20, 20);
		font = new StaticFont(q_char);
		for(unsigned int i=0; i < 128; i++)
		{
			Character* c = new Character();
			dejavuResource->getChar(c, i, 20, 20);
			font->setChar(c);
		}

		charImage.clear(Color(Color::PINK.getRed(), Color::PINK.getGreen(), Color::PINK.getBlue(), 125));
		//charImage.copyIn(font->getChar('Q').getBitmap().bitmap);
		charImage.drawAsciiText(*font, "Hola!", 10, 10, Color(255, 0, 0, 255));
		charTex = std::make_shared<Texture>(charImage);

		// init shader
		shader = resourceManager.get<GpuProgram>("shaders/HemisphereTex.gpu.xml");

		sphereBatch = std::make_shared<Mesh>();
		tinySphereBatch = std::make_shared<Mesh>();
		bigSphereBatch = std::make_shared<Mesh>();
		floorBatch = std::make_shared<Mesh>();
		boxBatch = std::make_shared<Mesh>();

		// init batches
		batchBuilder.buildSphere(sphereBatch.get(), 2*FOOT, 55, 32);
		batchBuilder.buildSphere(tinySphereBatch.get(), 1*FOOT, 4, 4);
		batchBuilder.buildBox(bigSphereBatch.get(), 3, 3, 3);
		batchBuilder.buildFlatSurface(floorBatch.get(), ROOM_SIZE*50, ROOM_SIZE*50, 20, 20, 
			true, 15*FOOT, 12*FOOT );
		float scale = 5.0f;
		batchBuilder.buildBox(boxBatch.get(), 6*INCH*scale, 3*INCH*scale, 3*INCH*scale );

		// init materials
		sphereMaterial = std::make_shared<Material>();
		materialBuilder.begin(sphereMaterial.get());
		materialBuilder.setGpuProgram(shader);
		materialBuilder.setTexture(charTex);
		materialBuilder.end();

		tinySphereMaterial = std::make_shared<Material>();
		materialBuilder.expand(tinySphereMaterial.get(), *sphereMaterial);
		materialBuilder.setTexture(blueTex);
		materialBuilder.end();

		bigSphereMaterial = std::make_shared<Material>();
		materialBuilder.expand(bigSphereMaterial.get(), *sphereMaterial);
		materialBuilder.setTexture(charTex);
		materialBuilder.end();

		floorMaterial = std::make_shared<Material>();
		materialBuilder.expand(floorMaterial.get(), *sphereMaterial);
		materialBuilder.setTexture(stoneTex);
		materialBuilder.end();

		auto brickMaterial = resourceManager.get<Material>("materials/Brick.xml");

		// 2D shader
		auto program2D = resourceManager.get<GpuProgram>("shaders/GpuProgram2D.xml");

		// circle in middle of screen
		std::shared_ptr<Mesh> circle2D = std::make_shared<Mesh>();
		//batchBuilder.build2DCircle(circle2D, 150, 150, 300, 5);
		batchBuilder.build2DRectangle(circle2D.get(), 0, 0, 300, 300);

		Image screenImage( 300, 300, 4, Color(31, 97, 240, 255) );
		screenImage.drawAsciiText(*font, "Hola!", 50, 50, Color(255, 255, 255, 255));
		screenTex = std::make_shared<Texture>(screenImage);
		screenTex->setWrapMode(Texture::CLAMP_TO_EDGE);

		auto circle2DMaterial = std::make_shared<Material>();
		materialBuilder.begin(circle2DMaterial.get());
		materialBuilder.setGpuProgram(program2D);
		materialBuilder.setTexture(screenTex);
		//materialBuilder.setRenderPrimitive(VertexArray::Primitives::TRIANGLE_FAN);
		materialBuilder.end();

		world->addObject(new Object(std::make_shared<Model>(std::make_shared<Meshes>(circle2D), circle2DMaterial)));

		auto logoTex = resourceManager.get<Texture>("textures/logo.tex.xml");

		auto logo2DMaterial = std::make_shared<Material>();
		materialBuilder.begin(logo2DMaterial.get());
		materialBuilder.setGpuProgram(program2D);
		materialBuilder.setTexture(logoTex);
		materialBuilder.end();

		auto logoBatch = std::make_shared<Mesh>();
		batchBuilder.build2DRectangle(logoBatch.get(), 200, 0, 173, 50);

		Object* logoObject = new Object(std::make_shared<Model>(std::make_shared<Meshes>(logoBatch), 
			logo2DMaterial));
		world->addObject(logoObject);



		// init objects
		Object::Properties prop;
		prop.mass = 1;
		btBall = new Object(std::make_shared<Model>(std::make_shared<Meshes>(sphereBatch), 
			sphereMaterial));
		btBall->setLocation(Point3(0.0f, 150*FOOT, 0.0f));
		world->addObject(btBall);

		floorObject = new Object(std::make_shared<Model>(std::make_shared<Meshes>(floorBatch), 
			floorMaterial, floorShape)); // static object
		world->addObject(floorObject);

		auto brickShape = resourceManager.get<CollisionShape>("shapes/BrickShape.xml");

		float wallWidth =40;
		float wallHeight = 10;
		float brickHeight = 0.375;
		float brickWidth = 0.75;
		float h = brickHeight/2;
		float xOffset = -(brickWidth*wallWidth)/2;
		float zOffset = -100*FOOT;
		prop.friction = 0.8f;
		for (int i=0; i < wallHeight; i++, h+=brickHeight)
		{
			float w = xOffset;
			if (i%2 != 0)
				w = brickWidth/2 + xOffset;
			for (int j=0; j < wallWidth; j++, w+=brickWidth)
			{
				if (i == wallHeight-1 && j == wallWidth-1)
					continue;
				btBox = new Object(std::make_shared<Model>(std::make_shared<Meshes>(boxBatch), 
					brickMaterial, brickShape), prop );
				btBox->setLocation( Point3(w, h, zOffset) );
				world->addObject(btBox);
			}
		}

		// 3ds model
		/*std::shared_ptr<Batches> chainBatches = resourceManager.get<Batches>("models/chainLink.3ds");
		Matrix4 scaleMatrix;
		scaleMatrix.createScaleMatrix(0.1f, 0.1f, 0.1f);
		chainBatches->applyTransform(scaleMatrix);
		auto chainMaterial = std::make_shared<Material>();
		materialBuilder.expand(chainMaterial.get(), *sphereMaterial);
		materialBuilder.setTexture(charTex);
		materialBuilder.end();
		auto chainShape = std::make_shared<TriangleMeshCollisionShape>(*chainBatches);
		chainObject = new Object(std::make_shared<Model>(std::make_shared<Meshes>(*chainBatches), 
			chainMaterial, chainShape));
		chainObject->setLocation(Point3(0.0f, 5.0f, 0.0f));
		world->addObject(chainObject);*/



		// set eye level
		camera.setLocation(Point3(0.0f, 6 * FOOT, ROOM_SIZE));
		camera.setStepSpeed( FOOT );
		camera.setStrafeSpeed( FOOT );

		// enable blending so transparency can happen
		graphics.enableBlending();

		srand((unsigned int)time(NULL));
	}


	virtual void tick(void)
	{	
		// move
		Vector3 side;
		if (moveForward)
			camera.step(1);
		if (moveBack)
			camera.step(-1);
		if (moveLeft)
			camera.strafe(1);
		if (moveRight)
			camera.strafe(-1);
	
		// release water
		if (releaseWater)
		{
			for (int i = 0; i < 20; i++)
			{
				Object::Properties prop;
				prop.mass = 0.1f;
				Object* t = new Object(std::make_shared<Model>(std::make_shared<Meshes>(tinySphereBatch), 
					tinySphereMaterial, tinySphereShape), prop);
				t->setLocation(Point3(0, 10.0f, 0));
				world->addObject(t);
            
				t->applyForce(Vector3(((float)(rand()%100))*0.01f, 0.0f, 
					((float)(rand()%100))*0.01f) );
			}
		}
    
		if (lightPos.getLocation().y() <= -400.0f)
			change = 1.0f;
		else if (lightPos.getLocation().y() >= 400.0f)
			change = -1.0f;
		lightPos.setLocation(
			lightPos.getLocation().withY(lightPos.getLocation().y()+change)
		);
    
    
		// fun stuff
		float c1, c2, c3;
		static float acc = 0;
		float c;
		if (fun)
		{
			if ( rand()%2 && acc < 3)
				c = 1.0f;
			else if (acc > -3)
				c = -0.3f;
			else
				c = 0.3f;
			acc += c;
			batchBuilder.modify(boxBatch.get());
			for (int i=0; i < boxBatch->getVertexCount(); i++)
			{
				batchBuilder.getVertex3f(&c1, &c2, &c3);
            
				batchBuilder.vertex3f(
					c1<0?c1-c:c1+c, 
					c2<0?c2-c:c2+c, 
					c3<0?c3-c:c3+c
				);
			}
			batchBuilder.end();
		}

		Point3 endPoint = physics.createRay(camera.getPosition().getLocation(), camera.getPosition().getForwardVector(), 1000);

		btBall->setPosition(Position(
			endPoint,
			btBall->getPosition().getForwardVector(),
			btBall->getPosition().getUpVector()
		));


		Image screenImage( 300, 300, 4, Color(31, 97, 240, 255) );
		std::stringstream ss;

		ss << std::setprecision(2) << std::fixed << endPoint.x() << ", " << endPoint.y() 
			<< ", " << endPoint.z();
		screenImage.drawAsciiText(*font, ss.str().c_str(), 50, 50, Color::WHITE);

		ss.str("");
		ss << "Fps: " << world->getActualFPS();
		screenImage.drawAsciiText(*font, ss.str().c_str(), 50, 100, Color::WHITE);

		ss.str("");
		ss << "Objects: " << world->getObjectCount();
		screenImage.drawAsciiText(*font, ss.str().c_str(), 50, 150, Color::WHITE);

		ss.str("");
		ss << "Vertices: " << world->getVertexCount();
		screenImage.drawAsciiText(*font, ss.str().c_str(), 50, 200, Color::WHITE);

		screenTex->set(screenImage);
    
	}

	virtual void handleEvent(const Event& event)
	{
		switch(event.data.type)
		{
			case Event::VIDEO_RESIZE:
				screenHeight = event.data.resize.h;
				screenWidth = event.data.resize.w;
				break;

			case Event::KEY_DOWN:
				keyPressed( event.data.key.key, this->camera, this->graphics, *this->world);
	            break;
	                
	        case Event::MOUSE_MOTION:
				mouseMovedPassive( event.data.motion.x, event.data.motion.y, this->camera, this->graphics );
	            break;
	                
	        case Event::MOUSE_BUTTON_DOWN:
	            mouseClicked( event.data.button.button, event.data.button.x, 
					event.data.button.y, this->camera, *this->world);
	            break;
	                
	        case Event::MOUSE_BUTTON_UP:
	            break;
	                
	        case Event::KEY_UP:
	            keyReleased( event.data.key.key );
	            break;
		}
	}

};


/** Main program entry point
 */
int main(int argc, char* argv[])
{
	
	Sandbox sandbox;

	sandbox.setup();
	sandbox.start();
    
	return 0;
}