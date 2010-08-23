#include <iostream> // cout
#include <cstdlib> // exit
#include <cmath> // fabs
#include <GL/glut.h>

using namespace std;

int
  w=480,
  h=360,
  npuntos=0,
  pt[8],
  ep=-1;


void Display_cb() {
  glClear(GL_COLOR_BUFFER_BIT);// rellena con color de fondo
 
  if (!npuntos){
    glutSwapBuffers();
    return;
  }
	
  if (npuntos==4) {
    glColor3f(.4f,.6f,.2f);
    glBegin(GL_POLYGON);
      for(int i=0;i<npuntos;i++){
        glVertex2i(pt[2*i],pt[2*i+1]);
      }
    glEnd();
		
    glColor3f(.1f,.2f,.3f);	
    glLineWidth(2.0);
    glBegin(GL_LINE_LOOP);
      glVertex2i(pt[0],pt[1]);
      glVertex2i(pt[2],pt[3]);
      glVertex2i(pt[4],pt[5]);
      glVertex2i(pt[6],pt[7]);
    glEnd();  
  }
	
  glColor3f(.1f,.2f,.3f);
  glPointSize(5.0);
  glBegin(GL_POINTS);
    for(int i=0;i<npuntos;i++){
      glVertex2i(pt[2*i],pt[2*i+1]);
    }
  glEnd();
	
  glutSwapBuffers();
}

void Reshape_cb(int width, int height){
  if (!width||!height) return;
  w = width;
  h = height;
  
  glViewport(0,0,w,h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0,w,0,h,-1,1);
  Display_cb();
}

void Motion_cb(int x, int y){
  y=h-y; 
	
  if (x<0) x=0;
  if (x>w-1) x=w-1;
  if (y<0) y=0;
  if (y>h-1) y=h-1;
	
  pt[2*ep]=x;
  pt[2*ep+1]=y;
	
  Display_cb();
}

void Mouse_cb(int button, int state, int x, int y){	
  if (button==GLUT_LEFT_BUTTON){
    if (state==GLUT_DOWN) { 
      y=h-y;
	
      float colors[3];
      glReadBuffer(GL_FRONT);
      glReadPixels(x,y,1,1,GL_RGB,GL_FLOAT, colors);
      if (colors[0] == .4f && colors[1] == .6f && colors[2] == .2f)
   	glutMotionFunc(Motion_cb);
  
      ep=-1;
      for (int i=0;i<npuntos;i++){
	int d = abs(x-pt[2*i])+abs(y-pt[2*i+1]);
        if (d>5) continue;
        ep=i;
        glutMotionFunc(Motion_cb);
        return;
      }
	if (npuntos==4) return;
	
	pt[2*npuntos]=x;
	pt[2*npuntos+1]=y;
	npuntos++;

	Display_cb();

	ep=npuntos-1;
	glutMotionFunc(Motion_cb);
	}
      else if (state==GLUT_UP)
        glutMotionFunc(0);
  }
}

void Keyboard_cb(unsigned char key,int x,int y) {
  if (key==27 && npuntos > 0){
    npuntos = 0;
    Display_cb();
  }
	
  if (key==127||key==8){
    if (ep==-1||ep==npuntos) return;
    for(int i=ep;i<npuntos-1;i++){
      pt[2*i] = pt[2*i+2];
      pt[2*i+1] = pt[2*i+3];
    }

    npuntos--;
    Display_cb();
 
    if (key==8) ep--;
  }
}

void Special_cb(int key,int xm=0,int ym=0) {
  if (key==GLUT_KEY_F4 && glutGetModifiers()==GLUT_ACTIVE_ALT)
   exit(EXIT_SUCCESS);
}

void inicializa() {
  glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE);
  glutInitWindowSize(w,h); glutInitWindowPosition(50,50);
  glutCreateWindow("introducción");

  glutDisplayFunc(Display_cb);
  glutMouseFunc(Mouse_cb);
  glutReshapeFunc(Reshape_cb);
  glutKeyboardFunc(Keyboard_cb);
  glutSpecialFunc(Special_cb);

  glClearColor(0.2f,0.4f,0.3f,0.1f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}
int main(int argc,char** argv) {
  glutInit(&argc,argv);
  inicializa(); 
  glutMainLoop();
  return 0; 
}
