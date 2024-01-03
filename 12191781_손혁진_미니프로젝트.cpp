#include <gl/glut.h>
#include <stdio.h>
#include <windows.h>
#include<GL/freeglut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmpfuncs.h"
#include "ObjParser.h"
#include <GL/glext.h>
#include<cmath>
#include <mmsystem.h>    
#include <time.h>

#pragma comment(lib,"winmm.lib")
#pragma warning(disable: 4996)
#define _CRT_SECURE_NO_WARNINGS
#define M_PI 3.1415926535897
using std::cos;
using std::sin;
using std::sqrt;

int light_state = 0;
int selected_menu = 0;
int play_mode = 0; //���Ӹ�带 �����ϱ� ���� ����
double PI = 3.1415926;
double radius = 60;
double theta = 45 * PI / 180, phi = 1 * PI / 180;
double cam[3];
double center[3] = { 0, -7 ,0 };
double up[3] = { 0,1,0 };
int current_width, current_height;
float g_nSkySize = 100;

// ���� ȿ���� �ʿ��� ������
double shake_amplitude = 0.01; // ��鸲�� ����
double shake_frequency = 10.0; // ��鸲�� ���ļ�
double shake_duration = 0.1; // ��鸲�� ���� �ð�
double shake_start_time = 0.0; // ��鸲 ���� �ð�

//�������� �ʿ��� ������
double laser_damage = 10; //�������� �°� ƨ�ܳ����� �Ÿ�
double laser_speed = 1.2; //�������� �ӵ�
float laser1_dir = -1;
float laser2_dir = -1;
bool laser1_valid = 0;
bool laser2_valid = 0;
double car1_laser[3] = { 0 };
double car2_laser[3] = { 0 };

//���׿��� �ʿ��� ������
double meteorX;
double meteorY;
double meteorZ;
time_t lastUpdateTime = 0;
time_t crashUpdateTime1 = 0;
time_t crashUpdateTime2 = 0;
time_t crashTime1 = time(NULL);
time_t crashTime2 = time(NULL);
bool is_meteor_crash1 = 0;
bool is_meteor_crash2 = 0;

double car1_coord[3] = { 5,0,10 };
double car2_coord[3] = { 5,0,-10 };
bool isAWSDPressed[4] = { false, false, false, false }; // ������ Ű�� ���� ����
bool isArrowPressed[4] = { false, false, false, false }; // ������ Ű�� ���� ����
bool isGpressed = false; //1P ������ �߻� ��ư ���� ����
bool isMpressed = false; //2P ������ �߻� ��ư ���� ����

float car1_rotation = 0.0; // 1P �ڵ��� ȸ�� ����
float car2_rotation = 0.0; // 2P �ڵ��� ȸ�� ����
float car_speed = 0.5;
float interpolation_speed = 0.5; // ���� �ӵ�(0.0 ~ 1.0), ���� ���� ������
float car1_target_rotation = 0;
float car2_target_rotation = 0;
double deadline = 30;

//�÷��̾ �ƿ��Ǿ������� ����
int car1_out = 0;
int car2_out = 0;


// object var
ObjParser* map; //������ �������� 30�̴�.
ObjParser* car1;
ObjParser* car2;
ObjParser* meteor;
GLUquadricObj* qobj = gluNewQuadric();

GLuint g_nCubeTex;
GLuint g_nCarTex;
GLuint g_nCar2Tex;
GLuint g_nMapTex;
GLuint g_nStart2Tex;
GLuint g_ndraw2Tex;
GLuint g_n_1p_win2Tex;
GLuint g_n_2p_win2Tex;
GLuint g_n_meteor2Tex;


void obj_list();

void init();
void idle();
void resize(int width, int height);
void specialkeyboard(int key, int x, int y);
void specialkeyboardUp(int key, int x, int y);
void keyboard(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void processInput();
void mousewheel(int button, int dir, int x, int y);
void mouse(int button, int state, int x, int y);
void popup_menu();
void reset();
void checkDistance();
double calculateDistance(double x1, double y1, double z1, double x2, double y2, double z2);
void print_key_manual();

void light_default();

void draw_playmode1();
void draw_playmode2();
void draw_playmode3();
void draw_start();
void draw_end();
void set_start_Texture();
void set_draw_Texture();
void set_1p_win_Texture();
void set_2p_win_Texture();
void set_meteor_Texture();

void draw();
void draw_car1_laser();
void draw_car2_laser();
void draw_meteor();
void draw_axis();
void draw_obj_with_texture(ObjParser* objParser, GLuint names);
void drawskybox();
void skyboxTexture();
void setmapTexture();
void setcarTexture();
void setcar2Texture();
void draw_string(void* font, const char* str, float x_position, float y_position, float red, float green, float blue);



int main(int argc, char** argv)
{
	/* Window �ʱ�ȭ */
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);	// GLUT_DOUBLE , GLUT_RGBA�� ����
	glutInitWindowSize(800, 700);
	glutInitWindowPosition(400, 100);
	glutCreateWindow("12191781 ������");
	init();

	PlaySound(TEXT("Project/sounds/futurechill.wav"), NULL, SND_ASYNC | SND_ALIAS | SND_LOOP); //������� ���
	print_key_manual();

	/* quadric object �Ӽ� ���� */
	gluQuadricDrawStyle(qobj, GLU_FILL);
	gluQuadricNormals(qobj, GLU_SMOOTH);

	obj_list();
	popup_menu();
	glutIdleFunc(idle);
	/* Callback �Լ� ���� */
	glutReshapeFunc(resize);
	glutDisplayFunc(draw);
	glutSpecialFunc(specialkeyboard);
	glutSpecialUpFunc(specialkeyboardUp);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutMouseWheelFunc(mousewheel);
	glutMouseFunc(mouse);
	/* Looping ���� */
	glutMainLoop();
	return 0;
}

void init(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	/* Default Object Color Set : Black */
	glColor3f(1.0f, 1.0f, 1.0f);
	/* resize �ݹ��Լ� ���� */
	glutReshapeFunc(resize);
	//�⺻ ���� ����
	light_default();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* TEXTURE MAPPING SET */
	skyboxTexture();
	setmapTexture();
	setcarTexture();
	setcar2Texture();
	set_meteor_Texture();
	set_start_Texture();
	set_draw_Texture();
	set_1p_win_Texture();
	set_2p_win_Texture();


	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);//polygon�� ���� ������ �����ϰ� texture�� ����
	glEnable(GL_TEXTURE_2D);
}
void draw(void) // ���� �Լ� ����
{
	if (play_mode == 0) { //���� ���� ������
		draw_start();
	}
	else if (play_mode == 1) { //�ڱ��� ���� ���
		draw_playmode1();
	}
	else if (play_mode == 2) { //�ڱ��� �ִ� ���
		draw_playmode2();
	}
	else if (play_mode == 3) { //�ڱ��� + ��� �������� ���
		draw_playmode3();
	}
	else if (play_mode == 4) { //���� ������
		draw_end();
	}
	glutSwapBuffers();
	glFlush();
}
void draw_playmode1() { //�ڱ����� ���� �븻���

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// ��ü�� ��/�� ���踦 ���� DEPTH �߰�
	glEnable(GL_TEXTURE_2D);
	resize(800, 700);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_DEPTH_TEST);	// �ڿ� �ִ� object�� ������ ���̴� ���� ����

	cam[0] = radius * sin(theta) * cos(phi);
	cam[1] = radius * sin(theta) * sin(phi);
	cam[2] = radius * cos(theta);
	if (theta <= PI)gluLookAt(cam[0], cam[2], cam[1],
		center[0], center[1], center[2],
		up[0], up[1], up[2]);
	else if (theta > PI)gluLookAt(cam[0], cam[2], cam[1],
		center[0], center[1], center[2],
		up[0], -up[1], up[2]);

	//glTranslatef(0, 2, 0);
	//draw_axis();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, -2, 0);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	drawskybox();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	draw_obj_with_texture(map, g_nMapTex);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(car1_coord[0], car1_coord[1], car1_coord[2]);
	glRotatef(car1_rotation, 0.0, 1.0, 0.0); // y�� ���� 1p �ڵ��� ȸ��
	GLfloat car1_light_position[] = { 0,0,0,1 };
	glLightfv(GL_LIGHT0, GL_POSITION, car1_light_position);
	draw_obj_with_texture(car1, g_nCarTex);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(car2_coord[0], car2_coord[1], car2_coord[2]);
	GLfloat car2_light_position[] = { 0,0,0,1 };
	glLightfv(GL_LIGHT1, GL_POSITION, car2_light_position);
	glRotatef(car2_rotation, 0.0, 1.0, 0.0); // y�� ���� 2p �ڵ��� ȸ��
	draw_obj_with_texture(car2, g_nCar2Tex);
	glPopMatrix();

	glPushMatrix();
	draw_car1_laser();
	glPopMatrix();

	glPushMatrix();
	draw_car2_laser();
	glPopMatrix();

	processInput();
	checkDistance();
}
void draw_playmode2() { //�ڱ����� �ִ� ���

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// ��ü�� ��/�� ���踦 ���� DEPTH �߰�
	glEnable(GL_TEXTURE_2D);
	resize(800, 700);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_DEPTH_TEST);	// �ڿ� �ִ� object�� ������ ���̴� ���� ����

	cam[0] = radius * sin(theta) * cos(phi);
	cam[1] = radius * sin(theta) * sin(phi);
	cam[2] = radius * cos(theta);
	if (theta <= PI)gluLookAt(cam[0], cam[2], cam[1],
		center[0], center[1], center[2],
		up[0], up[1], up[2]);
	else if (theta > PI)gluLookAt(cam[0], cam[2], cam[1],
		center[0], center[1], center[2],
		up[0], -up[1], up[2]);

	//glTranslatef(0, 2, 0);
	//draw_axis();
	//glPopMatrix();

	//glPushMatrix();
	glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
	glRotatef(90.0, 1.0, 0.0, 0.0); //�ڱ��� ������
	glutSolidTorus(0.1, deadline, 30, 50); //�ڱ��� �׸���
	glRotatef(-90.0, 1.0, 0.0, 0.0);//���� ��� ���� ����
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, -2, 0);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	drawskybox();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	draw_obj_with_texture(map, g_nMapTex);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(car1_coord[0], car1_coord[1], car1_coord[2]);
	glRotatef(car1_rotation, 0.0, 1.0, 0.0); // y�� ���� 1p �ڵ��� ȸ��
	GLfloat car1_light_position[] = { 0,0,0,1 };
	glLightfv(GL_LIGHT0, GL_POSITION, car1_light_position);
	draw_obj_with_texture(car1, g_nCarTex);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(car2_coord[0], car2_coord[1], car2_coord[2]);
	GLfloat car2_light_position[] = { 0,0,0,1 };
	glLightfv(GL_LIGHT1, GL_POSITION, car2_light_position);
	glRotatef(car2_rotation, 0.0, 1.0, 0.0); // y�� ���� 2p �ڵ��� ȸ��
	draw_obj_with_texture(car2, g_nCar2Tex);
	glPopMatrix();

	glPushMatrix();
	draw_car1_laser();
	glPopMatrix();

	glPushMatrix();
	draw_car2_laser();
	glPopMatrix();

	processInput();
	checkDistance();
}
void draw_playmode3() { //�ڱ��� + �������� ���� �ִ� ���

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// ��ü�� ��/�� ���踦 ���� DEPTH �߰�
	glEnable(GL_TEXTURE_2D);
	resize(800, 700);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_DEPTH_TEST);	// �ڿ� �ִ� object�� ������ ���̴� ���� ����

	cam[0] = radius * sin(theta) * cos(phi);
	cam[1] = radius * sin(theta) * sin(phi);
	cam[2] = radius * cos(theta);
	if (theta <= PI)gluLookAt(cam[0], cam[2], cam[1],
		center[0], center[1], center[2],
		up[0], up[1], up[2]);
	else if (theta > PI)gluLookAt(cam[0], cam[2], cam[1],
		center[0], center[1], center[2],
		up[0], -up[1], up[2]);

	//glTranslatef(0, 2, 0);
	//draw_axis();
	//glPopMatrix();

	//glPushMatrix();
	glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
	glRotatef(90.0, 1.0, 0.0, 0.0); //�ڱ��� ������
	glutSolidTorus(0.1, deadline, 30, 50); //�ڱ��� �׸���
	glRotatef(-90.0, 1.0, 0.0, 0.0);//���� ��� ���� ����
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, -2, 0);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	drawskybox();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	draw_obj_with_texture(map, g_nMapTex);
	glPopMatrix();

	if (is_meteor_crash1) {
		// ��1�� ���׿��� ���� ���
		int elapsed_time = (int)difftime(time(NULL), crashTime1);
		int remaining_time = 2 - elapsed_time;
		glPushMatrix();
		glTranslatef(car1_coord[0], car1_coord[1] + 5, car1_coord[2]);  // ��1�� �Ӹ� ���� �̵�
		glRotatef(90.0, 1.0, 0.0, 0.0); //������
		glColor4f(0.0f, 0.0f, 1.0f, 1.0f);  // ��ũ�� ������ ����
		gluPartialDisk(gluNewQuadric(), 1, 2, 50, 1, 0, remaining_time * 120);  // 1�ʴ� 120����
		glColor3f(1.0f, 1.0f, 1.0f);
		glPopMatrix();
	}
	if (is_meteor_crash2) {
		// ��2�� ���׿��� ���� ���
		int elapsed_time = (int)difftime(time(NULL), crashTime2);
		int remaining_time = 2 - elapsed_time;
		glPushMatrix();
		glTranslatef(car2_coord[0], car2_coord[1] + 5, car2_coord[2]);  // ��2�� �Ӹ� ���� �̵�
		glRotatef(90.0, 1.0, 0.0, 0.0); //������
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);  // ��ũ�� ������ ����
		gluPartialDisk(gluNewQuadric(), 1, 2, 50, 1, 0, remaining_time * 120);  // 1�ʴ� 120����
		glColor3f(1.0f, 1.0f, 1.0f); 
		glPopMatrix();
	}

	glPushMatrix();
	glTranslatef(car1_coord[0], car1_coord[1], car1_coord[2]);
	glRotatef(car1_rotation, 0.0, 1.0, 0.0); // y�� ���� 1p �ڵ��� ȸ��
	GLfloat car1_light_position[] = { 0,0,0,1 };
	glLightfv(GL_LIGHT0, GL_POSITION, car1_light_position);
	draw_obj_with_texture(car1, g_nCarTex);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(car2_coord[0], car2_coord[1], car2_coord[2]);
	GLfloat car2_light_position[] = { 0,0,0,1 };
	glLightfv(GL_LIGHT1, GL_POSITION, car2_light_position);
	glRotatef(car2_rotation, 0.0, 1.0, 0.0); // y�� ���� 2p �ڵ��� ȸ��
	draw_obj_with_texture(car2, g_nCar2Tex);
	glPopMatrix();

	glPushMatrix();
	draw_car1_laser();
	glPopMatrix();

	glPushMatrix();
	draw_car2_laser();
	glPopMatrix();

	draw_meteor();

	processInput();
	checkDistance();
}
void set_start_Texture() {
	glGenTextures(1, &g_nStart2Tex); //texture name (����)�� �Ҵ� �޴´�.
	int imgWidth, imgHeight, channels;
	glBindTexture(GL_TEXTURE_2D, g_nStart2Tex);
	char* bmp = (char*)"Project/img/start_page.bmp";
	uchar* img = readImageData(bmp, &imgWidth, &imgHeight, &channels);//image�� ���� �д� ��
	glTexImage2D(GL_TEXTURE_2D, 0, 3, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}
void set_draw_Texture() {
	glGenTextures(1, &g_ndraw2Tex); //texture name (����)�� �Ҵ� �޴´�.
	int imgWidth, imgHeight, channels;
	glBindTexture(GL_TEXTURE_2D, g_ndraw2Tex);
	char* bmp = (char*)"Project/img/draw.bmp";
	uchar* img = readImageData(bmp, &imgWidth, &imgHeight, &channels);//image�� ���� �д� ��
	glTexImage2D(GL_TEXTURE_2D, 0, 3, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}
void set_1p_win_Texture() {
	glGenTextures(1, &g_n_1p_win2Tex); //texture name (����)�� �Ҵ� �޴´�.
	int imgWidth, imgHeight, channels;
	glBindTexture(GL_TEXTURE_2D, g_n_1p_win2Tex);
	char* bmp = (char*)"Project/img/player_1_win.bmp";
	uchar* img = readImageData(bmp, &imgWidth, &imgHeight, &channels);//image�� ���� �д� ��
	glTexImage2D(GL_TEXTURE_2D, 0, 3, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}
void set_2p_win_Texture() {
	glGenTextures(1, &g_n_2p_win2Tex); //texture name (����)�� �Ҵ� �޴´�.
	int imgWidth, imgHeight, channels;
	glBindTexture(GL_TEXTURE_2D, g_n_2p_win2Tex);
	char* bmp = (char*)"Project/img/player_2_win.bmp";
	uchar* img = readImageData(bmp, &imgWidth, &imgHeight, &channels);//image�� ���� �д� ��
	glTexImage2D(GL_TEXTURE_2D, 0, 3, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}
void draw_start() {
	glDisable(GL_LIGHTING);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0f, 800.0f, 0.0f, 600.0f); //left,right,bottom,top)

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0, 1.0, 1.0);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glBindTexture(GL_TEXTURE_2D, g_nStart2Tex);
	glBegin(GL_QUADS);
	glNormal3f(-1.0, 0, 0);   // -x axis
	glTexCoord2f(0, 0); glVertex3f(0.0f, 0.0f, 0.0f);
	glTexCoord2f(1, 0); glVertex3f(800.0f, 0.0f, 0.0f);
	glTexCoord2f(1, 1); glVertex3f(800.0f, 600.0f, 0.0f);
	glTexCoord2f(0, 1); glVertex3f(0.0, 600.0f, 0.0f);

	glEnd();
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
}
void draw_end() {

	glDisable(GL_LIGHTING);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0f, 800.0f, 0.0f, 600.0f); //left,right,bottom,top)

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0, 1.0, 1.0);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	if (car1_out == 1 && car2_out)glBindTexture(GL_TEXTURE_2D, g_ndraw2Tex);
	if (car2_out == 1)glBindTexture(GL_TEXTURE_2D, g_n_1p_win2Tex);
	if (car1_out == 1)glBindTexture(GL_TEXTURE_2D, g_n_2p_win2Tex);
	glBegin(GL_QUADS);
	glNormal3f(-1.0, 0, 0);   // -x axis
	glTexCoord2f(0, 0); glVertex3f(0.0f, 0.0f, 0.0f);
	glTexCoord2f(1, 0); glVertex3f(800.0f, 0.0f, 0.0f);
	glTexCoord2f(1, 1); glVertex3f(800.0f, 600.0f, 0.0f);
	glTexCoord2f(0, 1); glVertex3f(0.0, 600.0f, 0.0f);

	glEnd();
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
}
void reset() {
	printf("Reset Game!\n");
	play_mode = 0;
	car1_coord[0] = 5;
	car1_coord[1] = 0;
	car1_coord[2] = 10;
	car2_coord[0] = 5;
	car2_coord[1] = 0;
	car2_coord[2] = -10;
	phi = 1 * PI / 180;
	deadline = 30;
	laser1_valid = false;
	laser2_valid = false;
	car1_out = 0;
	car2_out = 0;
	glutPostRedisplay();
}
void obj_list()
{
	map = new ObjParser("Project/map_floor2.obj");
	//car1 = new ObjParser("car1.obj");
	//car2 = new ObjParser("car2.obj");
	car1 = new ObjParser("Project/bumper_car.obj");
	car2 = new ObjParser("Project/bumper_car.obj");
	meteor = new ObjParser("Project/meteor.obj");
}
void draw_obj_with_texture(ObjParser* objParser, GLuint names)
{
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_CUBE_MAP); // ��ī�̹ڽ��� �ؽ�ó ���� ����
	glBindTexture(GL_TEXTURE_2D, names);
	glBegin(GL_TRIANGLES);
	for (unsigned int n = 0; n < objParser->getFaceSize(); n += 3) {
		glTexCoord2f(objParser->textures[objParser->textureIdx[n] - 1].x,
			objParser->textures[objParser->textureIdx[n] - 1].y);
		glNormal3f(objParser->normal[objParser->normalIdx[n] - 1].x,
			objParser->normal[objParser->normalIdx[n] - 1].y,
			objParser->normal[objParser->normalIdx[n] - 1].z);
		glVertex3f(objParser->vertices[objParser->vertexIdx[n] - 1].x,
			objParser->vertices[objParser->vertexIdx[n] - 1].y,
			objParser->vertices[objParser->vertexIdx[n] - 1].z);

		glTexCoord2f(objParser->textures[objParser->textureIdx[n + 1] - 1].x,
			objParser->textures[objParser->textureIdx[n + 1] - 1].y);
		glNormal3f(objParser->normal[objParser->normalIdx[n + 1] - 1].x,
			objParser->normal[objParser->normalIdx[n + 1] - 1].y,
			objParser->normal[objParser->normalIdx[n + 1] - 1].z);
		glVertex3f(objParser->vertices[objParser->vertexIdx[n + 1] - 1].x,
			objParser->vertices[objParser->vertexIdx[n + 1] - 1].y,
			objParser->vertices[objParser->vertexIdx[n + 1] - 1].z);

		glTexCoord2f(objParser->textures[objParser->textureIdx[n + 2] - 1].x,
			objParser->textures[objParser->textureIdx[n + 2] - 1].y);
		glNormal3f(objParser->normal[objParser->normalIdx[n + 2] - 1].x,
			objParser->normal[objParser->normalIdx[n + 2] - 1].y,
			objParser->normal[objParser->normalIdx[n + 2] - 1].z);
		glVertex3f(objParser->vertices[objParser->vertexIdx[n + 2] - 1].x,
			objParser->vertices[objParser->vertexIdx[n + 2] - 1].y,
			objParser->vertices[objParser->vertexIdx[n + 2] - 1].z);
	}
	glEnd();
	glEnable(GL_BLEND);
}
void resize(int width, int height) {
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (double)width / (double)height, 1, 1000);
	glMatrixMode(GL_MODELVIEW);
}
void draw_car1_laser() {
	if (laser1_valid) {//�� 1�� �������� �߻�� ��Ȳ�̸�
		GLfloat laser1_light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };// ������ ��ġ�� ���� ����
		GLfloat laser1_light_position[] = { car1_laser[0], 2, car1_laser[2], 1.0 };
		glLightfv(GL_LIGHT2, GL_DIFFUSE, laser1_light_diffuse);
		glLightfv(GL_LIGHT2, GL_POSITION, laser1_light_position);
		glEnable(GL_LIGHT2);

		if (laser1_dir == 0)car1_laser[0] -= laser_speed;
		else if (laser1_dir == 180)car1_laser[0] += laser_speed;
		else if (laser1_dir == 90)car1_laser[2] += laser_speed;
		else if (laser1_dir == -90)car1_laser[2] -= laser_speed;
		glTranslated(car1_laser[0], 2, car1_laser[2]);
		glRotated(laser1_dir + 90, 0, 1, 0); //�������� ��� ���⿡�� 1�ڷ� �������� y����� x�� 90��ŭ �� ȸ��
		glColor3f(0, 0, 1); //�Ķ��� ������
		glutSolidCylinder(0.2, 1, 30, 30);
		double hit_distance = calculateDistance(car1_laser[0], car1_laser[1], car1_laser[2], car2_coord[0], car2_coord[1], car2_coord[2]);
		if (hit_distance < 2) { //�� 2�� �������� ������
			if (laser1_dir == 0)car2_coord[0] -= laser_damage;
			else if (laser1_dir == 180)car2_coord[0] += laser_damage;
			else if (laser1_dir == 90)car2_coord[2] += laser_damage;
			else if (laser1_dir == -90)car2_coord[2] -= laser_damage;
			laser1_valid = false; //�������� ���ش�. (�ٽ� �߻� �������� ����)
			laser1_dir = -1;
		}
		//�������� �ʹ��� ������ �ٽ� ���
		if (calculateDistance(car1_laser[0], car1_laser[1], car1_laser[2], 0, 2, 0) > deadline) laser1_valid = false;
	}
}
void draw_car2_laser() {
	if (laser2_valid) { //�� 2�� �������� �߻�� ��Ȳ�̸�
		GLfloat laser2_light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };// ������ ��ġ�� ���� ����
		GLfloat laser2_light_position[] = { car2_laser[0], 2, car2_laser[2], 1.0 };
		glLightfv(GL_LIGHT3, GL_DIFFUSE, laser2_light_diffuse);
		glLightfv(GL_LIGHT3, GL_POSITION, laser2_light_position);
		glEnable(GL_LIGHT3);

		if (laser2_dir == 0)car2_laser[0] -= laser_speed; //������ �ӵ��� ���� ������ �ӵ��� ����
		else if (laser2_dir == 180)car2_laser[0] += laser_speed;
		else if (laser2_dir == 90)car2_laser[2] += laser_speed;
		else if (laser2_dir == -90)car2_laser[2] -= laser_speed;
		glTranslated(car2_laser[0], 2, car2_laser[2]);
		glRotated(laser2_dir + 90, 0, 1, 0);//�������� ��� ���⿡�� 1�ڷ� �������� y����� x�� 90��ŭ �� ȸ��
		glColor3f(1, 0, 0); //������ ������
		glutSolidCylinder(0.2, 1, 30, 30);
		double hit_distance = calculateDistance(car2_laser[0], car2_laser[1], car2_laser[2], car1_coord[0], car1_coord[1], car1_coord[2]);
		if (hit_distance < 2) { //�� 1�� �������� ������
			if (laser2_dir == 0)car1_coord[0] -= laser_damage;
			else if (laser2_dir == 180)car1_coord[0] += laser_damage;
			else if (laser2_dir == 90)car1_coord[2] += laser_damage;
			else if (laser2_dir == -90)car1_coord[2] -= laser_damage;
			laser2_valid = false; //�������� ���ش�. (�ٽ� �߻� �������� ����)
			laser2_dir = -1;
		}
		//�������� �ʹ��� ������ �ٽ� ���
		if (calculateDistance(car2_laser[0], car2_laser[1], car2_laser[2], 0, 2, 0) > deadline) laser2_valid = false; //�������� �ʹ��� ������ �ٽ� ���
	}
}
void draw_meteor() {
	time_t currentTime = time(NULL);
	if (lastUpdateTime == 0 || difftime(currentTime, lastUpdateTime) >= 3) { //3�ʰ� ������ ���׿��� �ٽ� ����
		meteorX = rand() % 30 - 15;
		meteorY = 30;
		meteorZ = rand() % 30 - 15;
	}

	// �׸��� �׸���
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glColor4f(0.0, 0.0, 0.0, 0.4);  // �׸����� ������ ����
	glTranslatef(meteorX, 0, meteorZ);  // �׸����� ��ġ�� ����
	glScalef(1, 0.001, 1);  // y ������ �ſ� ��� �����ϸ�
	glutSolidSphere(2, 10, 10);
	glEnable(GL_LIGHTING);
	glPopMatrix();

	lastUpdateTime = currentTime;
	glPushMatrix();
	glTranslatef(meteorX, meteorY, meteorZ);
	glColor4f(1.0, 1.0, 1.0, 0.5);
	draw_obj_with_texture(meteor, g_n_meteor2Tex);

	meteorY -= 0.5; // ���׿��� �������� �ӵ��� ����
	if (meteorY < -15) {  // ���׿��� ȭ�� ������ ������
		meteorX = rand() % 30 - 15;
		meteorY = 30;
		meteorZ = rand() % 30 - 15;
	}
	if (calculateDistance(meteorX, meteorY, meteorZ, car1_coord[0], car1_coord[1], car1_coord[2]) < 3) { //�� 1�� ���׿��� �΋H����
		is_meteor_crash1 = 1;
		crashTime1 = time(NULL); // �浹 �ð� ���
	}
	if (is_meteor_crash1 && difftime(currentTime, crashTime1) >= 2) { // �浹 �� 2�ʰ� �����ٸ�
		is_meteor_crash1 = 0; // �浹 ���¸� ����
	}
	if (calculateDistance(meteorX, meteorY, meteorZ, car2_coord[0], car2_coord[1], car2_coord[2]) < 3) { //�� 2�� ���׿��� �΋H����
		is_meteor_crash2 = 1;
		crashTime2 = time(NULL); // �浹 �ð� ���
	}
	if (is_meteor_crash2 && difftime(currentTime, crashTime2) >= 2) { // �浹 �� 2�ʰ� �����ٸ�
		is_meteor_crash2 = 0; // �浹 ���¸� ����
	}
	glPopMatrix();
}

void draw_string(void* font, const char* str, float x_position, float y_position, float red, float green, float blue) {
	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(-10, 10, -10, 10);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glColor3f(red, green, blue);
	glRasterPos3f(x_position, y_position, 0);
	for (unsigned int i = 0; i < strlen(str); i++) {
		glutBitmapCharacter(font, str[i]);
	}
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopAttrib();
	glEnable(GL_LIGHTING);
}
void skyboxTexture() { //SkyBox ť�� �ؽ��� �����ϱ�
	glGenTextures(1, &g_nCubeTex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, g_nCubeTex);
	int width, height, channels;
	uchar* img0 = readImageData("Project/img/corona_ft.bmp", &width, &height, &channels);
	uchar* img1 = readImageData("Project/img/corona_bk.bmp", &width, &height, &channels);
	uchar* img2 = readImageData("Project/img/corona_dn.bmp", &width, &height, &channels);
	uchar* img3 = readImageData("Project/img/corona_up.bmp", &width, &height, &channels);
	uchar* img4 = readImageData("Project/img/corona_rt.bmp", &width, &height, &channels);
	uchar* img5 = readImageData("Project/img/corona_lf.bmp", &width, &height, &channels);

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, width,
		height, 0, GL_RGB, GL_UNSIGNED_BYTE, img0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, width,
		height, 0, GL_RGB, GL_UNSIGNED_BYTE, img1);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, width,
		height, 0, GL_RGB, GL_UNSIGNED_BYTE, img2);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, width,
		height, 0, GL_RGB, GL_UNSIGNED_BYTE, img3);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, width,
		height, 0, GL_RGB, GL_UNSIGNED_BYTE, img4);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, width,
		height, 0, GL_RGB, GL_UNSIGNED_BYTE, img5);

	glBindTexture(GL_TEXTURE_CUBE_MAP, g_nCubeTex);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);

}
void setmapTexture(void) {
	int imgWidth, imgHeight, channels;
	uchar* img = readImageData("Project/img/Marble01_1K_BaseColor.bmp", &imgWidth, &imgHeight, &channels);

	int texNum = 1;
	glGenTextures(texNum, &g_nMapTex);
	glBindTexture(GL_TEXTURE_2D, g_nMapTex);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, img);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
void setcarTexture(void) {
	int imgWidth, imgHeight, channels;
	uchar* img = readImageData("Project/img/car1.bmp", &imgWidth, &imgHeight, &channels);

	int texNum = 1;
	glGenTextures(texNum, &g_nCarTex);
	glBindTexture(GL_TEXTURE_2D, g_nCarTex);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, img);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
void setcar2Texture(void) {
	int imgWidth, imgHeight, channels;
	uchar* img = readImageData("Project/img/car2.bmp", &imgWidth, &imgHeight, &channels);

	int texNum = 1;
	glGenTextures(texNum, &g_nCar2Tex);
	glBindTexture(GL_TEXTURE_2D, g_nCar2Tex);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, img);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
void set_meteor_Texture(void) {
	int imgWidth, imgHeight, channels;
	uchar* img = readImageData("Project/img/Meteor.bmp", &imgWidth, &imgHeight, &channels);
	int texNum = 1;
	glGenTextures(texNum, &g_n_meteor2Tex);
	glBindTexture(GL_TEXTURE_2D, g_n_meteor2Tex);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, img);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
void idle(void) {
	glutPostRedisplay();
}
void light_default() {
	glClearColor(0, 0, 0, 1.0f);

	/* Light0 =�� 1���� ���� ���� */
	GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat diffuseLight[] = { 0.0f, 1.0f, 1.0f, 1.0f };
	GLfloat specularLight[] = { 0.9f, 0.9f, 0.9f, 1.0f };
	/* Light1 =�� 2���� ���� ���� */
	GLfloat ambientLight2[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat diffuseLight2[] = { 1.0f, 0.0f, 1.0f, 1.0f };
	GLfloat specularLight2[] = { 0.9f, 0.9f, 0.9f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambientLight2);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseLight2);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specularLight2);

	/* �ؿ� diffuse���� ���� texture ���� ���Ѵ� ! */
	GLfloat ambientMaterial[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat diffuseMaterial[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat specularMaterial[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	/************* Material  setting *************/
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambientMaterial);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseMaterial);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specularMaterial);
	glMaterialf(GL_FRONT, GL_SHININESS, 128);

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	/* global light setting */
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	/* DEPTH TEST ENABLE */
	glFrontFace(GL_CCW);
}
void mousewheel(int button, int dir, int x, int y) {
	if (dir > 0) {
		//printf("Zoom In, ");
		radius -= 0.5;
		glutPostRedisplay();
	}
	else {
		//printf("Zoom Out, ");
		radius += 0.5;
		glutPostRedisplay();
	}
	//printf("Direction: %d Radius: %f\n", dir, radius);
}
void mouse(int button, int state, int x, int y) {
	// ���� ��ư Ŭ�� ��
	if (play_mode == 0) { //��ŸƮ ������
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
			//printf("x: %d, y: %d\n", x, y);
			if (x >= 145 && x <= 273) { //���� ȭ�鿡�� ���۹� Ŭ����
				if (y >= 504 && y <= 651) {
					print_key_manual();
					glutPostRedisplay();
				}
			}
			if (x >= 402 && x <= 612) { //���Ӹ�� 1 ���ý�
				if (y >= 434 && y <= 466) {
					play_mode = 1;
					glutPostRedisplay();
				}
			}
			if (x >= 401 && x <= 719) {//���Ӹ�� 2 ���ý�
				if (y >= 505 && y <= 541) {
					play_mode = 2;
					glutPostRedisplay();
				}
			}
			if (x >= 396 && x <= 779) {//���Ӹ�� 3 ���ý�
				if (y >= 573 && y <= 617) {
					play_mode = 3;
					glutPostRedisplay();
				}
			}
		}
	}
	if (play_mode == 4) {//���� ���� ȭ���� ��
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
			//printf("x: %d, y: %d\n", x, y);
			if (x >= 266 && x <= 547) { // Ŭ���� ��ġ�� ���� ��ư ��������
				if (y >= 95 && y <= 146) {
					reset();
				}
			}
		}
	}
}
void keyboard(unsigned char key, int x, int y) { // 1P ����Ű
	switch (key) {
	case 'w':
		isAWSDPressed[0] = true; // ���� Ű �Է� ���� Ȱ��ȭ
		break;
	case 's':
		isAWSDPressed[1] = true; // �Ʒ��� Ű �Է� ���� Ȱ��ȭ
		break;
	case 'a':
		isAWSDPressed[2] = true; // ���� Ű �Է� ���� Ȱ��ȭ
		break;
	case 'd':
		isAWSDPressed[3] = true; // ������ Ű �Է� ���� Ȱ��ȭ
		break;
	case 'g':
		isGpressed = true;
		break;
	case '/':
		isMpressed = true;
		break;
	case 'j': //���� ����
		phi -= 3 * PI / 180;
		if (phi < 0) {
			phi += 2 * PI;
		}
		glutPostRedisplay();
		break;
	case 'l':
		phi += 3 * PI / 180;
		if (phi < 0) {
			phi += 2 * PI;
		}
		glutPostRedisplay();
		break;
	case 'i':
		theta += 3 * PI / 180;
		if (theta > 2 * PI) {
			theta -= 2 * PI;
		}
		glutPostRedisplay();
		break;
	case 'k':
		theta -= 3 * PI / 180;
		if (theta < 0) {
			theta += 2 * PI;
		}
		glutPostRedisplay();
		break;
	case 'r':
		reset();
		break;
	case 'q':
		printf("Exit Game!\n");
		exit(0);
		break;
	default:
		break;
	}
	if (key == '1') play_mode = 1;
	if (key == '2') play_mode = 2;
	if (key == '3') play_mode = 3;
}

void keyboardUp(unsigned char key, int x, int y) { // 1P ����Ű ����
	switch (key) {
	case 'w':
		isAWSDPressed[0] = false; // ���� Ű �Է� ���� ��Ȱ��ȭ
		break;
	case 's':
		isAWSDPressed[1] = false; // �Ʒ��� Ű �Է� ���� ��Ȱ��ȭ
		break;
	case 'a':
		isAWSDPressed[2] = false; // ���� Ű �Է� ���� ��Ȱ��ȭ
		break;
	case 'd':
		isAWSDPressed[3] = false; // ������ Ű �Է� ���� ��Ȱ��ȭ
		break;
	case 'g':
		isGpressed = false;
		break;
	case '/':
		isMpressed = false;
		break;
	default:
		break;
	}
}

void specialkeyboard(int key, int x, int y) { // 2P ����Ű
	switch (key) {
	case GLUT_KEY_UP:
		isArrowPressed[0] = true; // ���� Ű �Է� ���� Ȱ��ȭ
		break;
	case GLUT_KEY_DOWN:
		isArrowPressed[1] = true; // �Ʒ��� Ű �Է� ���� Ȱ��ȭ
		break;
	case GLUT_KEY_LEFT:
		isArrowPressed[2] = true; // ���� Ű �Է� ���� Ȱ��ȭ
		break;
	case GLUT_KEY_RIGHT:
		isArrowPressed[3] = true; // ������ Ű �Է� ���� Ȱ��ȭ
		break;
	default:
		break;
	}
}
void specialkeyboardUp(int key, int x, int y) { // 2P ����Ű ����
	switch (key) {
	case GLUT_KEY_UP:
		isArrowPressed[0] = false; // ���� Ű �Է� ���� ��Ȱ��ȭ
		break;
	case GLUT_KEY_DOWN:
		isArrowPressed[1] = false; // �Ʒ��� Ű �Է� ���� ��Ȱ��ȭ
		break;
	case GLUT_KEY_LEFT:
		isArrowPressed[2] = false; // ���� Ű �Է� ���� ��Ȱ��ȭ
		break;
	case GLUT_KEY_RIGHT:
		isArrowPressed[3] = false; // ������ Ű �Է� ���� ��Ȱ��ȭ
		break;
	default:
		break;
	}
}
void processInput() {
	if(!is_meteor_crash1){
		if (isAWSDPressed[0]) {
			car1_coord[0] -= car_speed; // ���� �̵�
			car1_target_rotation = 0;
			car1_rotation = car1_rotation * (1 - interpolation_speed) + car1_target_rotation * interpolation_speed;

			//car1_rotation = 0.0; // ȸ������ ����
		}
		if (isAWSDPressed[1]) {
			car1_coord[0] += car_speed; // �Ʒ��� �̵�
			car1_target_rotation = 180;
			car1_rotation = car1_rotation * (1 - interpolation_speed) + car1_target_rotation * interpolation_speed;

			//car1_rotation = 180.0; // �Ʒ��� ȸ��
		}
		if (isAWSDPressed[2]) {
			car1_coord[2] += car_speed; // �������� �̵�
			car1_target_rotation = 90;
			car1_rotation = car1_rotation * (1 - interpolation_speed) + car1_target_rotation * interpolation_speed;

			//car1_rotation = 90.0; // �������� ȸ��
		}
		if (isAWSDPressed[3]) {
			car1_coord[2] -= car_speed; // ���������� �̵�
			car1_target_rotation = -90;
			car1_rotation = car1_rotation * (1 - interpolation_speed) + car1_target_rotation * interpolation_speed;

			//car1_rotation = -90.0; // ���������� ȸ��
			//printf("car1 x: %f, y: %f, z: %f\n", car1_coord[0], car1_coord[1], car1_coord[2]);
		}
		if (isGpressed) { //Ű�� �����ٰ�
			if (!laser1_valid) { //Ű�� �� ��
				Beep(6271.927, 10); //������ �Ҹ� ����
				laser1_valid = true; //�������� ���⵵�� �Ѵ�.
				laser1_dir = car1_target_rotation;
				memcpy(car1_laser, car1_coord, sizeof(car1_coord));
			}
		}
	}
	
	if (!is_meteor_crash2) {
		if (isArrowPressed[0]) {
			car2_coord[0] -= car_speed; // ���� �̵�
			car2_target_rotation = 0;
			car2_rotation = car2_rotation * (1 - interpolation_speed) + car2_target_rotation * interpolation_speed;

			//car2_rotation = 0.0; // ȸ������ ����
		}
		if (isArrowPressed[1]) {
			car2_coord[0] += car_speed; // �Ʒ��� �̵�
			car2_target_rotation = 180;
			car2_rotation = car2_rotation * (1 - interpolation_speed) + car2_target_rotation * interpolation_speed;
			//car2_rotation = 180.0; // �Ʒ��� ȸ��
		}
		if (isArrowPressed[2]) {
			car2_coord[2] += car_speed; // �������� �̵�
			car2_target_rotation = 90;
			car2_rotation = car2_rotation * (1 - interpolation_speed) + car2_target_rotation * interpolation_speed;

			//car2_rotation = 90.0; // �������� ȸ��
		}
		if (isArrowPressed[3]) {
			car2_coord[2] -= car_speed; // ���������� �̵�
			car2_target_rotation = -90;
			car2_rotation = car2_rotation * (1 - interpolation_speed) + car2_target_rotation * interpolation_speed;

			//car2_rotation = -90.0; // ���������� ȸ��
		}
		if (isMpressed) { //Ű�� �����ٰ�
			if (!laser2_valid) { //Ű�� �� ��
				Beep(4698.636, 30); //������ �Ҹ� ����
				laser2_valid = true; //�������� ���⵵�� �Ѵ�.
				laser2_dir = car2_target_rotation;
				memcpy(car2_laser, car2_coord, sizeof(car2_coord));
			}
		}
	}
}
void drawskybox() {
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, g_nCubeTex);
	glBegin(GL_QUADS);
	// px
	glTexCoord3d(1.0, -1.0, -1.0); glVertex3f(g_nSkySize, -g_nSkySize, -g_nSkySize);
	glTexCoord3d(1.0, -1.0, 1.0); glVertex3f(g_nSkySize, -g_nSkySize, g_nSkySize);
	glTexCoord3d(1.0, 1.0, 1.0); glVertex3f(g_nSkySize, g_nSkySize, g_nSkySize);
	glTexCoord3d(1.0, 1.0, -1.0); glVertex3f(g_nSkySize, g_nSkySize, -g_nSkySize);
	//nx
	glTexCoord3d(-1.0, -1.0, -1.0); glVertex3f(-g_nSkySize, -g_nSkySize, -g_nSkySize);
	glTexCoord3d(-1.0, -1.0, 1.0); glVertex3f(-g_nSkySize, -g_nSkySize, g_nSkySize);
	glTexCoord3d(-1.0, 1.0, 1.0); glVertex3f(-g_nSkySize, g_nSkySize, g_nSkySize);
	glTexCoord3d(-1.0, 1.0, -1.0); glVertex3f(-g_nSkySize, g_nSkySize, -g_nSkySize);
	//py
	glTexCoord3d(1.0, 1.0, 1.0); glVertex3f(g_nSkySize, g_nSkySize, g_nSkySize);
	glTexCoord3d(-1.0, 1.0, 1.0); glVertex3f(-g_nSkySize, g_nSkySize, g_nSkySize);
	glTexCoord3d(-1.0, 1.0, -1.0); glVertex3f(-g_nSkySize, g_nSkySize, -g_nSkySize);
	glTexCoord3d(1.0, 1.0, -1.0); glVertex3f(g_nSkySize, g_nSkySize, -g_nSkySize);
	//ny
	glTexCoord3d(1.0, -1.0, 1.0); glVertex3f(g_nSkySize, -g_nSkySize, g_nSkySize);
	glTexCoord3d(-1.0, -1.0, 1.0); glVertex3f(-g_nSkySize, -g_nSkySize, g_nSkySize);
	glTexCoord3d(-1.0, -1.0, -1.0); glVertex3f(-g_nSkySize, -g_nSkySize, -g_nSkySize);
	glTexCoord3d(1.0, -1.0, -1.0); glVertex3f(g_nSkySize, -g_nSkySize, -g_nSkySize);
	//pz
	glTexCoord3d(1.0, -1.0, 1.0); glVertex3f(g_nSkySize, -g_nSkySize, g_nSkySize);
	glTexCoord3d(-1.0, -1.0, 1.0); glVertex3f(-g_nSkySize, -g_nSkySize, g_nSkySize);
	glTexCoord3d(-1.0, 1.0, 1.0); glVertex3f(-g_nSkySize, g_nSkySize, g_nSkySize);
	glTexCoord3d(1.0, 1.0, 1.0); glVertex3f(g_nSkySize, g_nSkySize, g_nSkySize);
	//nz
	glTexCoord3d(1.0, -1.0, -1.0); glVertex3f(g_nSkySize, -g_nSkySize, -g_nSkySize);
	glTexCoord3d(-1.0, -1.0, -1.0); glVertex3f(-g_nSkySize, -g_nSkySize, -g_nSkySize);
	glTexCoord3d(-1.0, 1.0, -1.0); glVertex3f(-g_nSkySize, g_nSkySize, -g_nSkySize);
	glTexCoord3d(1.0, 1.0, -1.0); glVertex3f(g_nSkySize, g_nSkySize, -g_nSkySize);
	glEnd();
	glEnable(GL_LIGHTING);

}
void print_key_manual() {
	printf("+----------------------- ++-----------------------+\n");
	printf("|  ���� ����!, ������ �ڱ��� ������ �о����  |\n");
	printf("+------ 1P ����Ű -------++------ 2P ����Ű ------+\n");
	printf("|  W: ��                 ||  ��: ��               |\n");
	printf("|  S: ��                 ||  ��: ��               |\n");
	printf("|  A: ��                 ||  ��: ��               |\n");
	printf("|  D: ��                 ||  ��: ��               |\n");
	printf("|  G: ������ �߻�        ||   /: ������ �߻�      |\n");
	printf("+------------------------++-----------------------+\n");
	printf("|  R: ���� ����          ||   Q: ���� ����        |\n");
	printf("+------------------------++-----------------------+\n");
}
void main_menu_function(int option)
{
	//printf("Main menu %d has been selected\n", option);
	if (option == 999) {
		printf("Exit Game!\n");
		exit(0);
	}
	if (option == 1) {
		reset();
	}
	if (option == 2) {
		print_key_manual();
	}
}

void draw_axis(void) {
	glLineWidth(10);
	glBegin(GL_LINES);
	glColor3f(1, 0, 0); //x red
	glVertex3f(0, 0, 0);
	glVertex3f(10, 0, 0);

	glColor3f(0, 1, 0); // y green
	glVertex3f(0, 0, 0);
	glVertex3f(0, 10, 0);

	glColor3f(0, 0, 1); // z blue
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 10);

	glEnd();
	glLineWidth(10);
}
void popup_menu() {
	glutCreateMenu(main_menu_function);
	glutAddMenuEntry("Quit", 999);
	glutAddMenuEntry("Reset", 1);
	glutAddMenuEntry("Key Manual", 2);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}
double calculateDistance(double x1, double y1, double z1, double x2, double y2, double z2) {
	return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) + pow(z2 - z1, 2));
}

void shakeScreen() {
	shake_start_time = glutGet(GLUT_ELAPSED_TIME) / 1000.0; // ��鸲 ���� �ð� ����
}
void updateShake() {
	double current_time = glutGet(GLUT_ELAPSED_TIME) / 1000.0; // ���� �ð�
	double elapsed_time = current_time - shake_start_time; // ��� �ð�

	if (elapsed_time < shake_duration) {
		// ��鸲 ȿ�� ����
		double shake_factor = sin(2.0 * M_PI * shake_frequency * elapsed_time) * shake_amplitude;
		phi += shake_factor;
		glutPostRedisplay();
	}
}

void checkDistance() {
	if (glutGet(GLUT_ELAPSED_TIME) / 1000 % 1 == 0 && deadline > 0 && car1_out == 0 && car2_out == 0) {
		deadline -= 0.01; //�ð��� ���� ���� �ڱ����� ������ ���δ�.
	}
	// �� ���� �� ���� �Ÿ� ���
	double car1_distance = calculateDistance(0, 2, 0, car1_coord[0], car1_coord[1], car1_coord[2]);
	double car2_distance = calculateDistance(0, 2, 0, car2_coord[0], car2_coord[1], car2_coord[2]);

	// �Ÿ��� ������ �ʰ��ϸ� ó�� ����
	if (car1_distance > deadline) {
		car1_coord[1] -= 1; //������ ����������
		if (car1_coord[1] < -30) {
			car1_out=1;
			play_mode = 4;
		}
	}
	if (car2_distance > deadline) {
		car2_coord[1] -= 1;
		if (car2_coord[1] < -30) {
			car2_out=1;
			play_mode = 4;
		}
	}
	if (calculateDistance(car1_coord[0], car1_coord[1], car1_coord[2],
		car2_coord[0], car2_coord[1], car2_coord[2]) < 3.5) {//���� �� ���� �Ÿ� ���
		//printf("�� ������ �浹�Ͽ����ϴ�.\n");
		Beep(261.6256, 10); //�� �� �浹 �Ҹ� ����
		shakeScreen();
		//�÷��̾�1�� �ھ�����
		if (isAWSDPressed[0]) {
			car1_coord[0] += 2; //��Ű �������� => �Ϸ� ƨ���
			car2_coord[0] -= 5; //�÷��̾� 2�� �ݴ�� ���� ƨ���
		}
		if (isAWSDPressed[1]) {
			car1_coord[0] -= 2; //��Ű �������� => ������ ƨ���
			car2_coord[0] += 5;
		}
		if (isAWSDPressed[2]) {
			car1_coord[2] -= 2; //��Ű �������� => ��� ƨ���
			car2_coord[2] += 5;
		}
		if (isAWSDPressed[3]) {
			car1_coord[2] += 2; //��Ű �������� => �·� ƨ���
			car2_coord[2] -= 5;
		}

		//�÷��̾�2�� �ھ�����
		if (isArrowPressed[0]) {
			car2_coord[0] += 2; //��Ű �������� => �Ϸ� ƨ���
			car1_coord[0] -= 5; //�÷��̾� 2�� �ݴ�� ���� ƨ���
		}
		if (isArrowPressed[1]) {
			car2_coord[0] -= 2; //��Ű �������� => ������ ƨ���
			car1_coord[0] += 5;
		}
		if (isArrowPressed[2]) {
			car2_coord[2] -= 2; //��Ű �������� => ��� ƨ���
			car1_coord[2] += 5;
		}
		if (isArrowPressed[3]) {
			car2_coord[2] += 2; //��Ű �������� => �·� ƨ���
			car1_coord[2] -= 5;
		}
	}
	updateShake();
}
