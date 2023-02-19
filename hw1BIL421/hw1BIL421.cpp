/* This program is a video game that aimed destroy all the evil aliens (purple alien) with the bomb, while destroying
as few good aliens as possible (green alien).

The program also reacts correctly when the window is
moved or resized by clearing the new window to black*/


#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <cmath>
#include < GL/freeglut.h>
#include <GL/glut.h>
#include <string>
#include <Windows.h>
#pragma comment(lib, "Winmm.lib")
#include <mmsystem.h>


/* globals */

GLsizei wh = 400, ww = 400; /* initial window size */
GLfloat size_o = (wh + ww) / 45;   /* half side length of square */

#define PI 3.14159265

typedef struct {
	GLfloat x;
	GLfloat y;
} Point2D;

typedef struct {
	GLubyte r;
	GLubyte g;
	GLubyte b;
} Color;

typedef struct {
	Point2D center;
	Color color;
	GLfloat Vx;			//Velocity for x axis
	GLfloat Vy;			//Velocity for y axis
	int lvl;			//Level of the alien
	int x_rotation;		// We will use this value to decide if the alien will go through x+, x- or not move in x line
	int y_rotation;		// We will use this value to decide if the alien will go through y+, y- or not move in y line
	//x_rotation == 0 does not change location through x
	//x_rotation == 1 goes in +x
	//x_rotation == 2 goes in -x
	//same rules for y_rotation
	bool is_active;
	//The time cadaver displays on screen
	int cadaver_timer;
} Alien;

typedef struct {
	Point2D center;
	Color color;
	bool is_active;
	int lvl;			//Level of the bomb (this will increase in each step)
	int r;
	int duration;
} Bomb;

Bomb bombs[20];
Alien aliens[20];
int score = 0;
bool isPaused = false;
bool isInAdvancedMode = false;
int size_bombs = (int)sizeof(bombs) / sizeof(Bomb);
int size_aliens = (int)sizeof(aliens) / sizeof(Alien);
int active_bomb_num = 0;
int killed_purple_alien_count = 0;
int killed_light_yellow_alien_count = 0;
int killed_middle_yellow_alien_count = 0;
int killed_dark_yellow_alien_count = 0;
bool is_finished = false;


/*This function produces random numbers*/
double myRandom(double min, double max)
{
	double val = (double)rand() / RAND_MAX;
	val = min + val * (max - min);
	return val;
}

/*Create aliens with their random locations and velocities*/
void makeAlien(GLfloat x, GLfloat y, int z, int num)
{
	aliens[num].is_active = true;
	aliens[num].x_rotation = (GLubyte)rand() % 3;
	aliens[num].y_rotation = (GLubyte)rand() % 3;
	if (aliens[num].x_rotation == 0 && aliens[num].y_rotation == 0) {
		if (num % 2 == 0) {
			aliens[num].x_rotation = (GLubyte)rand() % 2 + 1;
			aliens[num].y_rotation = (GLubyte)rand() % 3;
		}
		else {
			aliens[num].x_rotation = (GLubyte)rand() % 3;
			aliens[num].y_rotation = (GLubyte)rand() % 2 + 1;
		}
	}

	aliens[num].center.x = x;
	aliens[num].center.y = y;
	aliens[num].lvl = z;
	if (z == 0) {
		aliens[num].color.r = 255;
		aliens[num].color.g = 255;
		aliens[num].color.b = 102;
	}
	else if (z == 1) {
		aliens[num].color.r = 255;
		aliens[num].color.g = 255;
		aliens[num].color.b = 51;
	}
	else if (z == 2) {
		aliens[num].color.r = 204;
		aliens[num].color.g = 204;
		aliens[num].color.b = 0;
	}
	else if (z == 3) {
		aliens[num].color.r = 195;
		aliens[num].color.g = 100;
		aliens[num].color.b = 217;
	}
	else if (z == 4) {
		aliens[num].color.r = 102;
		aliens[num].color.g = 0;
		aliens[num].color.b = 102;
	}
	else {
		aliens[num].color.r = 0;
		aliens[num].color.g = 0;
		aliens[num].color.b = 0;
	}

	aliens[num].Vx = (myRandom(0.0, 5.0) + 1);
	if (aliens[num].x_rotation == 0)
		aliens[num].Vx = 0;
	else if (aliens[num].x_rotation == 2)
		aliens[num].Vx *= -1;

	aliens[num].Vy = (myRandom(0.0, 5.0) + 1);
	if (aliens[num].y_rotation == 0)
		aliens[num].Vy = 0;
	else if (aliens[num].x_rotation == 2)
		aliens[num].Vy *= -1;

	aliens[num].cadaver_timer = 0;
	glutPostRedisplay();
}

/* rehaping routine called whenever window is resized or moved */
void myReshape(GLsizei w, GLsizei h)
{
	/*User not allowed to resize screen less than 100px. 
	When the screen size gets 0 our aliens and bombs are disappeared because of i am resizing them using the ratio of screen dimensions
	So when the dimension gets 0 value because of I multiply aliens's size and bomb's size with 0 they disappears
	*/
	if (w < 100) {
		w = 100;
	}
	if (h < 100) {
		h = 100;
	}

	/* adjust clipping box */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, (GLdouble)w, 0.0, (GLdouble)h);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* adjust viewport and clear */

	glViewport(0, 0, w, h);

	/*change size of the aliens*/
	size_o = (h + w) / 40;

	for (int i = 0; i < size_aliens; i++) {
		double x = aliens[i].center.x;
		double y = aliens[i].center.y;
		aliens[i].center.x = x * w / ww;
		aliens[i].center.y = y * h / wh;
		if (aliens[i].center.y >= (h - size_o)) {
			aliens[i].center.y = h - size_o - 1;
		}
		else if (aliens[i].center.y >= (h - size_o)) {
			aliens[i].center.y = size_o;
		}
		if (aliens[i].center.x >= (w - size_o)) {
			aliens[i].center.x = w - size_o - 1;
		}
		else if (aliens[i].center.x <= size_o) {
			aliens[i].center.x = size_o;
		}
		aliens[i].Vx = aliens[i].Vx * w / ww;
		aliens[i].Vy = aliens[i].Vy * h / wh;
	}

	for (int i = 0; i < size_bombs; i++) {
		double x = bombs[i].center.x;
		double y = bombs[i].center.y;
		bombs[i].center.x = x * w / ww;
		bombs[i].center.y = y * h / wh;
		bombs[i].r= (w + h) / 80;
	}


	/* set global size for use by drawing routine */
	ww = w;
	wh = h;

	glutPostRedisplay();
}

/*Initialize aliens bombs and other neccessary things*/
void myinit(void)
{

	glViewport(0, 0, ww, wh);

	/* Pick 2D clipping window to match size of screen window
	This choice avoids having to scale object coordinates
	each time window is resized */

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, (GLdouble)ww, 0.0, (GLdouble)wh);

	/* set clear color to black and clear window */

	glClearColor(0.0, 0.0, 0.0, 1.0);

	for (int i = 0; i < size_aliens; i++) {
		GLfloat x = (GLubyte)rand() % ww + 2*size_o;
		GLfloat y = (GLubyte)rand() % wh + 2*size_o;
		int z = 0;
		if (i < 4)
			z = 0;
		else if (i < 8)
			z = 1;
		else if (i < 12)
			z = 2;
		else if (i < 16)
			z = 3;
		else if (z < 20)
			z = 4;
		else
			std::cout << "error occured\n";
		makeAlien(x, y, z, i);
	}

	for (int j = 0; j < size_bombs; j++) {
		bombs[j].is_active = false;
	}
}

/*This mrthod used for print PAUSE or YOU WON or YOU LOST on the screen*/
void RenderString(float x, float y, void* font, const unsigned char* str, int r, int g, int b)
{
	if (r >= 0 && r < 256 && g >= 0 && g < 256 && b >= 0 && b < 256)
		glColor3f(r, g, b);
	glRasterPos2f(x, y);

	glutBitmapString(font, str);
	glutPostRedisplay();
}

void CreateBomb(int x, int y) {
	int num = 0;
	bool is_created = false;
	int sze = (int)sizeof(bombs) / sizeof(Bomb);
	for (int i = 0; i < sze; i++) {
		if (!bombs[i].is_active && !is_created) {
			PlaySound(TEXT("./phantasmHitEffect.wav"), NULL, SND_FILENAME | SND_ASYNC);
			bombs[i].center.x = x;
			bombs[i].center.y = wh - y;
			bombs[i].is_active = true;
			bombs[i].lvl = 0;
			bombs[i].duration = 0;
			bombs[i].r = (ww + wh) / 80;
			is_created = true;
			num++;
		}
		else if(bombs[i].is_active)
			num++;
	}
	active_bomb_num = num;
}


/*Two methods below is used for printing the score on screen*/
void printScore() {

	char buff[30];
	sprintf_s(buff, "SCORE: %d", score);

	glRasterPos2f(0, wh - 20);
	glColor3f(255.0, 0.0, 0.0);

	int k = 0;
	while (buff[k] != '\0') {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, buff[k]);
		++k;
	}
	
	glutPostRedisplay();
}

/*Display method*/
void myDisplay(void)
{
	int i = round(sizeof(aliens) / sizeof(Alien)) - 1;
	GLfloat x, y;
	glClear(GL_COLOR_BUFFER_BIT);

	/*Display Aliens*/
	/*Because currently aliens and bombs array have the same size and we represent depths by printing objects in different orders*/
	for (i; i >= 0; i--) {

		if (aliens[i].is_active || aliens[i].cadaver_timer >= 0) {
			x = aliens[i].center.x;
			y = aliens[i].center.y;
			GLubyte r = aliens[i].color.r;
			GLubyte g = aliens[i].color.g;
			GLubyte b = aliens[i].color.b;

			glColor3ub(aliens[i].color.r, aliens[i].color.g, aliens[i].color.b);

			if (r == g && g == b)
			{
				//kill the alien if its color changed before
				if(aliens[i].is_active)
					aliens[i].is_active = false;
				else
					aliens[i].cadaver_timer--;
			}

			glEnable(GL_POINT_SMOOTH);
			glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

			float angle;
			glBegin(GL_POLYGON);
			for (int j = 0; j < 30; ++j)
			{
				angle = 2 * PI * j / 30;
				glVertex2f(x + cos(angle) * size_o, y + sin(angle) * size_o);
			}
			glEnd();
		}

		/*Comment below lines (represented as Display by Order) if your bombs and aliens array has different amount of max size*/
		/*If you take comment this block you should uncomment the block represented as <Display Bombs> below*/
		/*Display by Order*/
		int level = i / 4;
		for (int val = 0; val < size_bombs; val++) {
			if (bombs[val].is_active && bombs[val].lvl==level) {
				int m1 = bombs[val].center.x;
				int m2 = bombs[val].center.y;

				glColor3ub(bombs[i].color.r, bombs[val].color.g, bombs[val].color.b);

				glEnable(GL_POINT_SMOOTH);
				glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

				float angle;
				glBegin(GL_POLYGON);
				for (int k = 0; k < 30; ++k)
				{
					angle = 2 * PI * k / 30;
					glVertex2f(m1 + cos(angle) * bombs[val].r, m2 + sin(angle) * bombs[val].r);
				}
				glEnd();
			}
		}
		/*Display by Order*/
	}

	/*If the display order of bombs does not matter, command Display by Order block then use this block for effectiveness*/
	/*Display Bombs*/
	/*for (int j = 0; j < size_bombs; j++) {
		if (bombs[j].is_active) {
			int m1 = bombs[j].center.x;
			int m2 = bombs[j].center.y;

			glColor3ub(bombs[j].color.r, bombs[j].color.g, bombs[j].color.b);

			glEnable(GL_POINT_SMOOTH);
			glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

			float angle;
			glBegin(GL_POLYGON);
			for (int k = 0; k < 30; ++k)
			{
				angle = 2 * PI * k / 30;
				glVertex2f(m1 + cos(angle) * bombs[j].r, m2 + sin(angle) * bombs[j].r);
			}
			glEnd();
		}
	}*/

	printScore();

	if (isPaused) {
		const unsigned char* t = reinterpret_cast<const unsigned char*>("PAUSED");
		RenderString((ww + wh) / 4, (ww + wh) / 4, GLUT_BITMAP_TIMES_ROMAN_24, t, 204, 0, 102);
	}
	if (killed_purple_alien_count >=8  && score > 0) {
		const unsigned char* t = reinterpret_cast<const unsigned char*>("YOU WON");
		RenderString((ww + wh) / 6, (ww + wh) / 3, GLUT_BITMAP_TIMES_ROMAN_24, t, 0, 0, 255);
	}
	else if (killed_purple_alien_count >= 8) {
		const unsigned char* t = reinterpret_cast<const unsigned char*>("YOU LOST");
		RenderString(ww / 3, 2 * ww / 3, GLUT_BITMAP_TIMES_ROMAN_24, t, 0, 0, 255);
	}

	glutSwapBuffers();
}

/*returns true when an alien is killed by the bomb[a]*/
bool BombTest(int a) {

	//because of my design if i dont control lvl 5 bombs, the for loop below will count aliens more than we have
	if (bombs[a].lvl >= 5) {
		return false;
	}

	int level = bombs[a].lvl * 4;
	for (int i = level; i < level + 4; i++) {
		if (aliens[i].is_active) {

			GLfloat m1 = (aliens[i].center.x + size_o) - (bombs[a].center.x + bombs[a].r);
			GLfloat m2 = (aliens[i].center.y + size_o) - (bombs[a].center.y + bombs[a].r);
			GLfloat hip = sqrt(m1 * m1 + m2 * m2);

			if ( hip < (size_o + bombs[a].r)){
				aliens[i].color.r = 96.0;
				aliens[i].color.g = 96.0;
				aliens[i].color.b = 96.0;
				aliens[i].cadaver_timer = 700;
				if (aliens[i].lvl == 0) {
					killed_light_yellow_alien_count++;
					score = score - 4000;
				}
				else if (aliens[i].lvl == 1) {
					killed_middle_yellow_alien_count++;
					score = score - 2000;
				}
				else if (aliens[i].lvl == 2) {
					killed_dark_yellow_alien_count++;
					score = score - 1000;
				}
				else if (aliens[i].lvl == 3) {
					killed_purple_alien_count++;
					score = score + 1000;
				}
				else if (aliens[i].lvl == 4) {
					killed_purple_alien_count++;
					score = score + 2000;
				}
				return true;
			}
		}
	}
	return false;
}

/*We use this function to change bomb's color when the bomb gets deeper*/
void setBombColor(int a) {

	/*color of the bombs*/
	if (bombs[a].lvl == 0) {
		bombs[a].color.g = 255;
		bombs[a].color.r = 153;
		bombs[a].color.b = 153;
	}
	else if (bombs[a].lvl == 1) {
		bombs[a].color.g = 255;
		bombs[a].color.r = 51;
		bombs[a].color.b = 153;
	}
	else if (bombs[a].lvl == 2) {
		bombs[a].color.g = 255;
		bombs[a].color.r = 0;
		bombs[a].color.b = 128;
	}
	else if (bombs[a].lvl == 3) {
		bombs[a].color.g = 204;
		bombs[a].color.r = 0;
		bombs[a].color.b = 102;
	}
	else if (bombs[a].lvl == 4) {
		bombs[a].color.g = 153;
		bombs[a].color.r = 0;
		bombs[a].color.b = 76;
	}
	else if (bombs[a].lvl == 5) {
		bombs[a].color.g = 0;
		bombs[a].color.r = 0;
		bombs[a].color.b = 0;
		bombs[a].is_active = false;
	}
	else {
		bombs[a].color.g = 55;
		bombs[a].color.r = 55;
		bombs[a].color.b = 55;
		bombs[a].is_active = false;
	}
}

/*detect the collision and change the velocities*/
void detectCollision(int i, int j) {

	GLfloat x = abs(aliens[i].center.x - aliens[j].center.x);
	GLfloat y = abs(aliens[i].center.y - aliens[j].center.y);
	GLfloat m = sqrt(x * x + y * y);
	
	if (m <= 2*size_o) {

		GLfloat temp_x = aliens[i].Vx;
		GLfloat temp_y = aliens[i].Vy;

		aliens[i].Vx = aliens[j].Vx;
		aliens[i].Vy = aliens[j].Vy;
		aliens[j].Vx = temp_x;
		aliens[j].Vy = temp_y;
	}
}

/*method for find collided aliens*/
void tryPairs() {
	for (int i = 0; i < 20; i+=4) {

		if (aliens[i].is_active) {
			if (aliens[i + 1].is_active) {
				detectCollision(i, i + 1);
			}
			if (aliens[i + 2].is_active) {
				detectCollision(i, i + 2);
			}
			if (aliens[i + 3].is_active) {
				detectCollision(i, i + 3);
			}
		}

		if (aliens[i + 1].is_active) {
			if (aliens[i + 2].is_active) {
				detectCollision(i + 1, i + 2);
			}
			if (aliens[i + 3].is_active) {
				detectCollision(i + 1, i + 3);
			}
		}

		if (aliens[i + 2].is_active) {
			if (aliens[i + 3].is_active) {
				detectCollision(i + 2, i + 3);
			}
		}
	}
}

/*Timer function*/
void myTimer(int waiting_time) {
	if(!isPaused && !isInAdvancedMode){
		for (int i = 0; i < size_aliens; i++) {

			if (aliens[i].is_active) {
				aliens[i].center.x = aliens[i].center.x + aliens[i].Vx;
				aliens[i].center.y = aliens[i].center.y + aliens[i].Vy;
				if (aliens[i].center.y >= (wh - size_o) || aliens[i].center.y <= size_o) {
					aliens[i].Vy *= -1;
				}
				if (aliens[i].center.x <= size_o || aliens[i].center.x >= (ww - size_o)) {
					aliens[i].Vx *= -1;
				}

			}
			
		}

		/*Bomb control*/
		if (active_bomb_num > 0) {
			for (int a = 0; a < size_bombs; a++) {
				if (bombs[a].is_active) {
					bool killed = BombTest(a);
					if (killed) {
						PlaySound(TEXT("./bye.wav"), NULL, SND_FILENAME | SND_ASYNC);
						bombs[a].is_active = false;
					}
					setBombColor(a);
					bombs[a].duration += 1;
					if (bombs[a].duration == 10) {
						bombs[a].duration = 0;
						bombs[a].lvl++;
					}
				}
			}
		}
	}
	
	/*Collision control*/
	//tryPairs();

	glutPostRedisplay();
	if (killed_purple_alien_count < 8)
		glutTimerFunc(100, myTimer, 0);
	else
		is_finished = true;

}

/*Advanced mode function for displaying the game step by step*/
void advancedFunction() {
	std::string str = "**************************************************";
	std::cout << str << "\n";
	std::string ww_wh = "Window Width: " + std::to_string(ww) + " Window Height: " + std::to_string(ww);
	std::cout << ww_wh << "\n";
	std::string kill_count = "Killed lvl 0 aliens count: " + std::to_string(killed_light_yellow_alien_count) + " Killed lvl 1 aliens count: " + std::to_string(killed_middle_yellow_alien_count) + " Killed lvl 2 aliens count: " + std::to_string(killed_dark_yellow_alien_count);
	std::cout << kill_count << "\n";
	std::string kill_p = "Killed evil aliens count: " + std::to_string(killed_purple_alien_count);
	std::cout << kill_p << "\n";

	for (int i = 0; i < size_aliens; i++) {

		/*print all variables first*/
		GLfloat loc_x = aliens[i].center.x;
		GLfloat loc_y = aliens[i].center.y;
		bool is_negative_x = aliens[i].x_rotation == 2;
		bool is_negative_y = aliens[i].y_rotation == 2;
		GLfloat vx = aliens[i].Vx;
		GLfloat vy = aliens[i].Vy;
		std::string a = std::to_string(i) + ". Alien Location: x: " + std::to_string(loc_x) + " y: " + std::to_string(loc_y) + " level: " + std::to_string(aliens[i].lvl) + " velocity x: " + std::to_string(vx) + " velocity y: " + std::to_string(vy) + " units per second";
		std::cout << a << "\n";

		if (aliens[i].is_active) {
			aliens[i].center.x = aliens[i].center.x + aliens[i].Vx;
			aliens[i].center.y = aliens[i].center.y + aliens[i].Vy;
			if (aliens[i].center.y >= (wh - size_o) || aliens[i].center.y <= size_o) {
				aliens[i].Vy *= -1;
			}
			if (aliens[i].center.x <= size_o || aliens[i].center.x >= (ww - size_o)) {
				aliens[i].Vx *= -1;
			}

		}

	}

	/*Bomb control*/
	if (active_bomb_num > 0) {
		for (int a = 0; a < size_bombs; a++) {
			if (bombs[a].is_active) {
				bool killed = BombTest(a);
				if (killed) {
					PlaySound(TEXT("C:/Users/ENA/source/repos/hw1BIL421/hw1BIL421/bye.wav"), NULL, SND_FILENAME | SND_ASYNC);
					bombs[a].is_active = false;
				}
				setBombColor(a);
				bombs[a].duration += 1;
				if (bombs[a].duration == 10) {
					bombs[a].duration = 0;
					bombs[a].lvl++;
				}
			}
		}
	}

	/*Collision control*/
	//tryPairs();

	glutPostRedisplay();
}

/*Mouse controls*/
void myMouse(int btn, int state, int x, int y)
{
	if (!is_finished && !isPaused && !isInAdvancedMode) {
		if (btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
			CreateBomb(x, y);
		glutPostRedisplay();
	}
}

/*Keyboard controls*/
void myKeyboard(unsigned char key, int x, int y)
{
	if ((key == 'Q') || (key == 'q'))
		exit(0);
	if (!is_finished) {
		if ((key == 's') || (key == 'S')) {
			if (!isInAdvancedMode)
				isInAdvancedMode = true;
			else {
				advancedFunction();
			}
			
		}
		else if ((key == 'p') || (key == 'P')) {
			if (isPaused || isInAdvancedMode) {
				isPaused = false;
				isInAdvancedMode = false;
			}
			else if (!isPaused) {
				isPaused = true;
			}
		}
	}
}

/*Main*/
int main(int argc, char** argv)
{
	srand(time(0));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(ww, wh);
	glutCreateWindow("HIT THE POINT");
	myinit();
	glutReshapeFunc(myReshape);
	glutMouseFunc(myMouse);
	glutDisplayFunc(myDisplay);
	glutKeyboardFunc(myKeyboard);
	glutTimerFunc(100, myTimer, 0);

	glutMainLoop();

}