#include <iostream>
#include <math.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <GL/glut.h>
#include <simplexnoise.cpp>

using namespace std;

const float PI = 3.1415926535;

class Camera {
public:
	float x, y, z, yaw, pitch; //degrees for pitch and yaw
	Camera(float x_, float y_, float z_, float yaw_, float pitch_) :
						x(x_), 		y(-y_), 		z(z_), 	yaw(yaw_), pitch(pitch_) {}
	
	void PitchTo(float posx, float posy, float posz) {
		pitch = atan2(y-posy, sqrt((posx-x)*(posx-x)+(posz-z)*(posz-z))) * 180 / PI; 
	}
	
	void YawTo(float posx, float posz) {
		yaw = atan2(posx-x, posz-z) * 180 / PI;
	}
	
	void Move(float dx, float dy, float dz) {
		y += dy;
		z += sin(yaw*PI/180)*dx + cos(yaw*PI/180)*dz;
		x += sin(-yaw*PI/180)*dz + cos(yaw*PI/180)*dx;
	}
	
	void Rotate(float dyaw, float dpitch) {
		yaw += dyaw;
		pitch += dpitch;
	}
	
	void Transform() {
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(pitch, 1.f, 0.f, 0.f);
		glRotatef(yaw, 0.f, 1.f, 0.f);
		glTranslatef(-x, -y, -z);
	}
};

class Gradient {
	float sr, sb, sg, er, eb, eg;
public:
	Gradient(float sr_, float sg_, float sb_, float er_, float eg_, float eb_) :
							sr(sr_),	sb(sb_),		sg(sg_),	er(er_),		eb(eb_),	 eg(eg_) {}
	float r(float val) {return sr*(1-val) + er*val;}
	float g(float val) {return sg*(1-val) + eg*val;}
	float b(float val) {return sb*(1-val) + eb*val;}
};

int main()
{
	// create the window
	sf::Window window(sf::VideoMode(800, 600), "OpenGL", sf::Style::Default, sf::ContextSettings(32));
	window.setVerticalSyncEnabled(true);
	
	glClearDepth(1.f);
	glClearColor(0.3f, 0.3f, 0.3f, 0.f);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	
	vector<float> hrow(101, 0);
	vector<vector<float> > hmap(101, hrow);
	vector<vector<float> > cvals(hmap);
	float seed = .5;
	for(int x = 0; x < 101; x++) {
		for(int z = 0; z < 101; z++) {
			hmap[x][z] = scaled_octave_noise_3d(4, .5, .02, 0, 15, x, z, seed);
			cvals[x][z] = scaled_octave_noise_3d(6, .7, .02, 0, 15, x, z, seed);
		}
	}
	Gradient cground(.53, .25, .03, 0, .20, .07);
	Camera cam(50.f, 10.f, 50.f, 0, 225);
	
	long int t = 0;
	// run the main loop
	bool running = true;
	while (running)
	{
		t++;
		// handle events
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				// end the program
				running = false;
			}
			else if (event.type == sf::Event::Resized)
			{
				// adjust the viewport when the window is resized
				glViewport(0, 0, event.size.width, event.size.height);
			}
		}
		
		//Keyboard Handling
		float inc = .15;
		float rinc = 1.5;
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
			cam.Move(-inc, 0, 0);}
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
			cam.Move(inc, 0, 0);}
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
			cam.Move(0, 0, inc);}
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
			cam.Move(0, 0, -inc);}
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
			cam.Move(0, -inc, 0);}
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F)) {
			cam.Move(0, inc, 0);}
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) {
			cam.Rotate(-rinc, 0);}
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) {
			cam.Rotate(rinc, 0);}
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::C)) {
			cam.Rotate(0, -rinc);}
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X)) {
			cam.Rotate(0, rinc);}
		float xfrac = 1-fmod(cam.x, 1.0);
		float zfrac = 1-fmod(cam.z, 1.0);
		int xf = floor(cam.x);
		int zf = floor(cam.z);
		cam.y = -2+(hmap[xf][zf]*xfrac+hmap[xf+1][zf]*(1-xfrac))*zfrac+(hmap[xf][zf+1]*xfrac+hmap[xf+1][zf+1]*(1-xfrac))*(1-zfrac);
		
		// clear the buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//// Setup a perspective projection & Camera position
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(90.f, 1.333f, .1f, 300.0f);//fov, aspect, zNear, zFar
		
		cam.Transform();
		
		glBegin(GL_QUADS);
			for(int x = 0; x < 100; x++) {
				for(int z = 0; z < 100; z++) {
					float cval = (cvals[x][z]+cvals[x+1][z+1])*.5/10;
					glColor3f(cground.r(cval), cground.g(cval), cground.b(cval));
					//glColor3f((x%5)*.25, (z%5)*.25, ((z+x)%3)*.5);
					glVertex3f(float(x), hmap[x][z], float(z));
					glVertex3f(float(x), hmap[x][z+1], float(z+1));
					glVertex3f(float(x+1), hmap[x+1][z+1], float(z+1));
					glVertex3f(float(x+1), hmap[x+1][z], float(z));
				}
			}
		glEnd();
		
		// end the current frame (internally swaps the front and back buffers)
		window.display();
	}
	
	// release resources...
	
	return 0;
}