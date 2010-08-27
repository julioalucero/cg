// Rasterización de Lineas y Circunferencias
// Lineas rasterizadas por DDA o Bresenham
// Consigna: Buscar las @@@@ y seguir las instrucciones

#include <iostream> // cout
#include <cstdlib> // exit
#include <cmath> // fabs
#include <GL/glut.h>

using namespace std;

//------------------------------------------------------------
// variables globales
int
  w=480,h=360, // tamaño inicialde la ventana
  xini,yini,xfin,yfin, // puntos extremos
  pdef=0, // cantidad de puntos definidos (0, 1 o 2)
  modo=1, // metodo (1:Bres 2:DDa 3:Circ)
  xglob = 0, yglob = 0;

float // luces y colores en float
  fondo[]={0.85f,0.9f,0.95f,1.f},
  line_color[]={.4f,.6f,.8f,.5f},
  point_color[]={.1f,.2f,.3f,.5f};

bool
  edit_ini=true, // edita el punto inicial (false=el final)
  cl_info=true; // informa por la linea de comandos


//------------------------------------------------------------
// funciones globales

// utiles

template <class T> static inline int redondea(const T& value)
{return (value>0)? (int(value+.5)): (int(value-.5));}

template <class T> static inline void intercambia(T&t1, T&t2) //swap en conflicto con stl
{T t0=t1; t1=t2; t2=t0;} // cambia el contenido y no la direccion (lento pero seguro)

// mínimo y maximo
template <class T> static inline const T &minimo(const T &t1,const T &t2)
  {if (t1<t2) return t1; else return t2;}
template <class T> static inline const T &maximo(const T &t1,const T &t2)
  {if (t1>t2) return t1; else return t2;}

// potencia >0 (ojo que no testea negativos!!)
template <class T> static inline T pown(const T &inp, int p)
  {T ret=1;  while (p--) ret*=inp; return ret;} // ojo el 1
template <class T> static inline T cuadrado(const T &inp)
  {return inp*inp;}

// @@@@ Sólo para el curioso, no es necesario entender esto.
// raiz cuadrada entera
// s=(s+x/s)>>1, con x>1 y empezando de x/2 da el mayor entero cuyo cuadrado es < x
// yo calculo raiz de 4x, si es impar redondeo a mas y si es par a menos
static int raiz(int x){
  if (x<=0) return 0;
  int s=x<<1,sant=s<<1;
  x=sant;
  while (s<sant) {
    sant=s;
    s=(s+x/s)>>1;
  }
  s=sant; s>>=1;
  if ((s<<1)!=sant) s++;
  return s;
}

// RASTERIZACION

void linea_DDA(float x1, float y1, float x2, float y2) {
  glBegin(GL_POINTS);
  float x, y, dx =x2-x1, dy=y2-y1, m; int t;
  if (redondea(dx)==0 && redondea(dy)==0) {
    glVertex2i(redondea(x1), redondea(y1));
    glEnd();
    return;
  }
  if (fabs(dy)<fabs(dx)){
    if (dx>0)
      for (m=dy/dx, y=y1, t=redondea(x1); t<=x2; t++, y+=m) glVertex2i(t,redondea(y));
    else
      for (m=dy/dx, y=y2, t=redondea(x2); t<=x1; t++, y+=m) glVertex2i(t,redondea(y));
  }
  else {
    if (dy>0)
      for (m=dx/dy, x=x1, t=redondea(y1); t<=y2; t++, x+=m) glVertex2i(redondea(x),t);
    else
      for (m=dx/dy, x=x2, t=redondea(y2); t<=y1; t++, x+=m) glVertex2i(redondea(x),t);
  }
  glEnd();
}

void linea_Bresenham(int x1, int y1, int x2, int y2) {
  int dx=x2-x1, dy=y2-y1, x,y,NE,E,D,xstep,ystep;
  glBegin(GL_POINTS);
  if (!(dx|dy)) {
    glVertex2i(x1,y1);
    glEnd();
    return;
  }

  if (abs(dx)>abs(dy)) {
    if (dx<0) {intercambia(x1,x2); intercambia(y1,y2); dx=-dx; dy=-dy;}
    x=x1; y=y1; NE=dx<<1; // (<<1 = *2, shift de un bit)
    ystep=1; if (dy<0) {ystep=-1; dy=-dy;}
    E=dy<<1; D=E-dx;
    while (x<x2) {
      glVertex2i(x,y);
      if (D>0) {y+=ystep; D-=NE;}
      x++; D+=E;
    }
  }
  else{
    if (dy<0) {intercambia(y1,y2); intercambia(x1,x2); dy=-dy; dx=-dx;}
    y=y1; x=x1; NE=dy<<1; // (<<1 = *2, shift de un bit)
    xstep=1; if (dx<0) {xstep=-1; dx=-dx;}
    E=dx<<1; D=E-dy;
    while (y<y2) {
      glVertex2i(x,y);
      if (D>0) {x+=xstep; D-=NE;}
      y++; D+=E;
    }
  }
  glVertex2i(x, y); // punto final
  glEnd();
}
void linea_Bresenham(float x1, float y1, float x2, float y2) {
  linea_Bresenham(redondea(x1), redondea(x2), redondea(y1), redondea(y2));
}

void circulo_Bresenham(int xc, int yc, int r) {
  //@@@@ IMPLEMENTAR SEGUNDA DIFERENCIA
  
  glBegin(GL_POINTS);
  if (!r){
    glVertex2i(xc,yc);
    glEnd();
    return;
  }

  int x=0, y=r, h=1-r;
  int e = 3;
  int se = 5 - (r<<1);
  while (y>x){
    glVertex2i( x+xc, y+yc);
    glVertex2i( x+xc,-y+yc);
    glVertex2i(-x+xc, y+yc);
    glVertex2i(-x+xc,-y+yc);

    glVertex2i( y+xc, x+yc);
    glVertex2i( y+xc,-x+yc);
    glVertex2i(-y+xc, x+yc);
    glVertex2i(-y+xc,-x+yc);
//cout << x << "," << y << "\n";
    if (h>=0){ // SE
        h += se;
	se += 4;
      y--; 
    }
    else { // E
      h += e;
      se += 2;
    }
    e += 2;
    x++;
  }
  // x==y => no hace falta intercambiar
  glVertex2i( x+xc, y+yc);
  glVertex2i( x+xc,-y+yc);
  glVertex2i(-x+xc, y+yc);
  glVertex2i(-x+xc,-y+yc);
  glEnd();
//cout << endl;
}

void circulo_Bresenham(int xc, int yc, float r) {
  circulo_Bresenham(xc,yc,redondea(r));
}
void circulo_Bresenham(int xc, int yc, int x1, int y1) {
  circulo_Bresenham(xc,yc,redondea(raiz(float(cuadrado(x1-xc))+float(cuadrado(y1-yc)))));
}
void circulo_Bresenham(float xc, float yc, float r) {
  circulo_Bresenham(redondea(xc),redondea(yc),redondea(r));
}
void circulo_Bresenham(float xc, float yc, float x1, float y1) {
  circulo_Bresenham(redondea(xc),redondea(yc),redondea(raiz(cuadrado(x1-xc)+cuadrado(y1-yc))));
}

void tic(int x, int y){
  // tic con punto grueso
  glPointSize(3.0);
  glBegin(GL_POINTS);
    glVertex2i(x  ,y  );
  glEnd();
  glPointSize(1.0);
/*
  // tic con crucesita
  glBegin(GL_POINTS);
    glVertex2i(x-1,y  );
    glVertex2i(x  ,y-1);
    glVertex2i(x+1,y  );
    glVertex2i(x  ,y-1);
  glEnd();
*/
}

//============================================================
// callbacks

//------------------------------------------------------------
// redibuja
void Display_cb() { // Este tiene que estar
  //static int counter=0; cout << "display: " << counter++ << endl;

  glClear(GL_COLOR_BUFFER_BIT);

  if (pdef==0) {glutSwapBuffers(); return;} // no hay nada

  // puntos
  glColor4fv(point_color);
  tic(xini,yini);
  if (pdef==1) {glutSwapBuffers(); return;} // solo se definó el 1er punto
  tic(xfin,yfin);

  // linea
  glColor4fv(line_color);
  if      (modo==1) linea_Bresenham  (xini,yini,xfin,yfin);
  else if (modo==2) linea_DDA        (xini,yini,xfin,yfin);
  else if (modo==3) circulo_Bresenham(xini,yini,xfin,yfin);

  glutSwapBuffers(); // lo manda al monitor

#ifdef _DEBUG
  // testea la presencia de errores de OpenGL
  int errornum=glGetError();
  while(errornum!=GL_NO_ERROR){
         if(errornum==GL_INVALID_ENUM)
           cout << "GL_INVALID_ENUM" << endl;
    else if (errornum==GL_INVALID_OPERATION)
           cout << "GL_INVALID_OPERATION" << endl;
    else if (errornum==GL_STACK_OVERFLOW)
           cout << "GL_STACK_OVERFLOW" << endl;
    else if (errornum==GL_STACK_UNDERFLOW)
           cout << "GL_STACK_UNDERFLOW" << endl;
    else if (errornum==GL_OUT_OF_MEMORY)
           cout << "GL_OUT_OF_MEMORY" << endl;
    errornum=glGetError();
  }
#endif // _DEBUG
}

//------------------------------------------------------------
// Maneja cambios de ancho y alto de la ventana
void Reshape_cb(int width, int height){
//  cout << "reshape " << width << "x" << height << endl;
  if (!width||!height) return; // minimizado ==> nada
  w=width; h=height;
  glViewport(0,0,w,h); // región donde se dibuja (toda la ventana)
  // rehace la matriz de proyección (la porcion de espacio visible)
  glMatrixMode(GL_PROJECTION);  glLoadIdentity();
  glOrtho(0,w,0,h,-1,1);
  glMatrixMode(GL_MODELVIEW);
  Display_cb(); // Redibuja mientras hace el reshape
}

//------------------------------------------------------------
// Teclado y Mouse

// Maneja pulsaciones del teclado (ASCII keys)
// x,y posicion del mouse cuando se teclea
void Keyboard_cb(unsigned char key,int x=0,int y=0) {
  if (key=='i'||key=='I'){ // info
    cl_info=!cl_info;
    cout << ((cl_info)? "Info" : "Sin Info") << endl;
  }
  else if (key=='b'||key=='B'){ // linea Bresenham
    if (modo==1) return;
    modo=1;
    Display_cb();
    if (cl_info) cout << "Linea Bresenham" << endl;
  }
  else if (key=='d'||key=='D'){ // linea DDA
    if (modo==2) return;
    modo=2;
    Display_cb();
    if (cl_info) cout << "Linea DDA" << endl;
  }
  else if (key=='c'||key=='C'){ // Circulo Bresenham
    if (modo==3) return;
    modo=3;
    Display_cb();
    if (cl_info) cout << "Circulo Bresenham" << endl;
  }
  if (key==127||key==8){ // DEL o Backspace ==> borra
    if (pdef==0) return;
    pdef=0; // nada
    glClear(GL_COLOR_BUFFER_BIT);
    glutSwapBuffers();
  }
}

// Special keys (non-ASCII)
// teclas de funcion, flechas, page up/dn, home/end, insert
void Special_cb(int key,int xm=0,int ym=0) {
  if (key==GLUT_KEY_F4 && glutGetModifiers()==GLUT_ACTIVE_ALT) // alt+f4 => exit
    exit(EXIT_SUCCESS);
}

// Movimientos del mouse
void Motion_cb(int x, int y){ // drag
  y=h-y; // el 0 esta ariba
  if (edit_ini) {xini=x;yini=y;} else {xfin=x;yfin=y;} // reasigna
  Display_cb();
}

int distancia(int x, int y, int p){
  if (p==1){
	return (fabs(x - xini) + fabs(y - yini));
  }
  else{
    return (fabs(x - xfin) + fabs(y - yfin));
  }
}

bool tolerancia(int x, int y){
  int d1 = distancia(x,y,1);
  int d2 = distancia(x,y,2);
  return (d1 <= d2)? true : false;
}

void Motion_pt(int x, int y){
	y = h - y;
	edit_ini = tolerancia(x, y);

	int p = xglob - x;
	int z = yglob - y;
	
    if (edit_ini){
	  xini += - p;
	  yini += - z;
	}
	else{
	  xfin += - p;
	  yfin += - z;
	}
	xglob += - p;
	yglob += - z;
	Display_cb();
}

// Clicks del mouse
void Mouse_cb(int button, int state, int x, int y){
  y=h-y; // el 0 esta ariba
  if (button==GLUT_LEFT_BUTTON){
    if (state==GLUT_DOWN) {
      //@@@@ IMPLEMENTAR:
      // EL PUNTO CLIKEADO MAS CERCANO (TOLERANCIA) DEBE EDITARSE
      // no pico cerca de ninguno
      if (pdef==0){ // pone el primer punto
        xini=x;yini=y;
        pdef=1;
        Display_cb();
        glutMotionFunc(Motion_cb); // callback para los drags
        edit_ini=true;
	cout << "punto 1" << endl;
        return;
      }
      else if (pdef==1){ // pone el segundo punto
        xfin=x;yfin=y;
        pdef=2;
        Display_cb();
        glutMotionFunc(Motion_cb); // callback para los drags
        edit_ini=false;
        return;
      }
      else {
        xglob = x;
	yglob = y;
	glutMotionFunc(Motion_pt);
        return;
      }
    } // down
    else if (state==GLUT_UP) glutMotionFunc(0); // anula el callback para los drags
  } // left
}

//------------------------------------------------------------
// Menu
void Menu_cb(int value)
{
  switch (value){
    case 'i':
      Keyboard_cb('i');
      return;
    case 'd':
      Keyboard_cb('d');
      return;
    case 'b':
      Keyboard_cb('b');
      return;
    case 'c':
      Keyboard_cb('c');
      return;
    case 127:
      Keyboard_cb(127);
      return;
  }
}

int integerv(GLenum pname){
  int value;
  glGetIntegerv(pname,&value);
  return value;
}
#define _PRINT_INT_VALUE(pname) #pname << ": " << integerv(pname) <<endl

//------------------------------------------------------------
// Inicializa GLUT y OpenGL
void initialize() {
  glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE);
  glutInitWindowSize(w,h); glutInitWindowPosition(50,50);
  glutCreateWindow("Raster"); // crea el main window

  //declara los callbacks
  //los que no se usan no se declaran
  glutDisplayFunc(Display_cb); // redisplays
  glutReshapeFunc(Reshape_cb);
  glutKeyboardFunc(Keyboard_cb);
  glutSpecialFunc(Special_cb);
  glutMouseFunc(Mouse_cb);

  //@@@@ Entender como funciona el menu y como se asocia a los callbacks de teclado
  glutCreateMenu(Menu_cb);
    glutAddMenuEntry("[b] Linea Bresenham", 'b');
    glutAddMenuEntry("[d] Linea DDA", 'd');
    glutAddMenuEntry("[c] Circunferencia", 'c');
    glutAddMenuEntry("---------------------------", 255);
    glutAddMenuEntry("[DEL] Borra", 127);
    glutAddMenuEntry("[i] Info ON/OFF", 'i');
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  // ========================
  // estado normal del OpenGL
  // ========================
  glClearColor(fondo[0],fondo[1],fondo[2],1);  // color de fondo
  glMatrixMode(GL_MODELVIEW); glLoadIdentity(); // constante

  // ========================
  // info
  if (cl_info)
    cout << "Vendor:         " << glGetString(GL_VENDOR) << endl
         << "Renderer:       " << glGetString(GL_RENDERER) << endl
         << "GL_Version:     " << glGetString(GL_VERSION) << endl
         << "GL_Extensions:  " << glGetString(GL_EXTENSIONS) << endl
         << "GLU_Version:    " << gluGetString(GLU_VERSION) << endl
         << "GLU_Extensions: " << gluGetString(GLU_EXTENSIONS) << endl
         << _PRINT_INT_VALUE(GL_DOUBLEBUFFER)
         << _PRINT_INT_VALUE(GL_STEREO)
         << _PRINT_INT_VALUE(GL_AUX_BUFFERS)
         << _PRINT_INT_VALUE(GL_RED_BITS)
         << _PRINT_INT_VALUE(GL_GREEN_BITS)
         << _PRINT_INT_VALUE(GL_BLUE_BITS)
         << _PRINT_INT_VALUE(GL_ALPHA_BITS)
         << _PRINT_INT_VALUE(GL_DEPTH_BITS)
         << _PRINT_INT_VALUE(GL_STENCIL_BITS)
         << _PRINT_INT_VALUE(GL_ACCUM_RED_BITS)
         << _PRINT_INT_VALUE(GL_ACCUM_GREEN_BITS)
         << _PRINT_INT_VALUE(GL_ACCUM_BLUE_BITS)
         << _PRINT_INT_VALUE(GL_ACCUM_ALPHA_BITS)
         ;
  // ========================
}

//------------------------------------------------------------
// main
int main(int argc,char** argv) {
  glutInit(&argc,argv);// inicializa glut
  initialize(); // condiciones iniciales de la ventana y OpenGL
  glutMainLoop(); // entra en loop de reconocimiento de eventos
  return 0;
}
