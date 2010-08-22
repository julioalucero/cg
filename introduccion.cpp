
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <GL/glut.h>

using namespace std;

  int
    w = 400,
    h = 300,
    npuntos = 0,
    pt[8];

void Display_cb(){
  glClear(GL_COLOR_BUFFER_BIT);

  if (!npuntos){
    glutSwapBuffers();
    return;
  }

  if (npuntos == 4){
    glColor3f(0.6f, 0.6f, 0.6f);
    glBegin(GL_POLYGON);
      for (int i=0; i < npuntos; i++){
        glVertex2i(pt[i*2], pt[i*2+1]);
      }
    glEnd();
  }

  glColor3f(0.2f,0.2f,0.2f);
  glPointSize(5.0);
  glBegin(GL_POINTS);
    for (int i=0; i<npuntos; i++){
      glVertex2i(pt[i*2], pt[i*2+1]);
    }
  glEnd();

  glutSwapBuffers();
}

  //void Mouse_cb(){
  
 // }

void inicializa(){
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

  glutInitWindowSize(w,h);
  glutInitWindowPosition(50,50);
  glutCreateWindow("Introduccion");

  glutDisplayFunc(Display_cb);
//  glutMouseFunc(Mouse_cb);

  glClearColor(0.2f, 0.3f, 0.4f, 0.5f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

int main(int argc, char** argv){
  glutInit(&argc, argv);
  inicializa();
  glutMainLoop(); 
  return 0;
}


