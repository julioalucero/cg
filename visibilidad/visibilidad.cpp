#include <cmath> // sqrt
#include <cstdlib> // exit
#include <iostream> // cout
#include <GL/glut.h>

/*
// comentar este bloque si se quiere una ventana de comando
#ifdef _WIN32
 #pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
#endif
*/

using namespace std;

//------------------------------------------------------------
// variables globales y defaults

int
  w=480,h=360, // tamaño de la ventana
  boton=-1, // boton del mouse clickeado
  xclick,yclick; // x e y cuando clickeo un boton
float // luces y colores en float
  lambient[]={.5f,.5f,.5f,1}, // luz ambiente
  ldiffuse[]={.7f,.7f,.7f,1}, // luz difusa
  fondo[]={.95f,.95f,.97f,1.f},   // color de fondo
  face_color[]={.6f,.5f,.2f,.5f}, // color de caras
  line_color[]={.3f,.2f,.1f,.5f}, // color de lineas
  escala=100,escala0, // escala de los objetos window/modelo pixeles/unidad
  eye[]={0,0,5}, target[]={0,0,0}, up[]={0,1,0}, // camara, mirando hacia y vertical
  znear=2, zfar=8, //clipping planes cercano y alejado de la camara (en 5 => veo de 3 a -3)
  lpos[]={2,1,5,1}, // posicion luz, l[4]: 0 => direccional -- 1 => posicional
  amy,amy0, // angulo del modelo alrededor del eje y
  ac0,rc0; // angulo resp x y distancia al target de la camara al clickear

bool // variables de estado de este programa
  luz_camara=true,  // luz fija a la camara o al espacio
  perspectiva=true, // perspectiva u ortogonal
  rota=false,       // gira continuamente los objetos respecto de y
  dibuja=true,      // false si esta minimizado
  wire=true,        // dibuja lineas o no
  relleno=true,     // dibuja relleno o no
  cl_info=true,     // informa por la linea de comandos
  antialias=false,  // antialiasing
  color=true;       // color/material

short modifiers=0;  // ctrl, alt, shift (de GLUT)

static const double R2G=45/atan(1.0);

// temporizador:
static const int ms_lista[]={1,2,5,10,20,50,100,200,500,1000,2000,5000},ms_n=12;
static int ms_i=4,msecs=ms_lista[ms_i]; // milisegundos por frame


////////////////////////////////////////////////////////////////////////////////////////
// dibujo

/* @@@@ CONSIGNA
Dibuje las líneas delanteras gruesas y las traseras finas (y si quiere y puede, de puntos)
Lea:
Inicialización y borrado del z-buffer:
  glutInitDisplayMode(GLUT_DEPTH|GLUT_RGBA|GLUT_DOUBLE);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
Función de comparación de z: glDepthFunc();
Enmascaramiento: glDepthMask(); glColorMask();
Modos de dibujo de polígonos: glPolygonMode();
Face Culling: glCullFace(); glFrontFace();
Offset de z: glEnable(GL_POLYGON_OFFSET_FILL); glPolygonOffset (1,1);
Color y Espesor: glColor(); glLineWidth();
Antialiasing: glEnable y glHint con GL_[POINT, LINE y POLYGON]_SMOOTH
Como dejar todo como estaba: glPushAttrib(); glPopAttrib();
Para líneas de puntos (opcional): glEnable(GL_LINE_STIPPLE); glLineStipple();
*/

void drawObjects(){
  if (relleno) {
    glColor4fv(face_color);
    glutSolidIcosahedron();
  }
  else{
    glColorMask(false,false,false,false);
    glutSolidIcosahedron();
    glColorMask(true,true,true,true);
  }

  if (wire) {
    if (!color) glDisable(GL_LIGHTING); // lineas sin material (siempre en color)
    glLineWidth(3); // espesor de lineas
    glColor4fv(line_color); // color de lineas
    glutWireIcosahedron();
    if (!color) glEnable(GL_LIGHTING); // vuelve a habilitar material

    glDepthFunc(GL_GREATER);
    glLineWidth(1);

    glEnable(GL_LINE_STIPPLE);
    GLint factor = 2;
    GLushort patron = 0x0C0F;
    glLineStipple(factor,patron);

    glutWireIcosahedron();

    glDepthFunc(GL_LEQUAL);
    glDisable(GL_LINE_STIPPLE);
  }
}
////////////////////////////////////////////////////////////////////////////////////////


//------------------------------------------------------------
// redibuja los objetos
// Cada vez que hace un redisplay
void Display_cb() {
//  if (cl_info) cout << "Display\t"; cout.flush();
  if (!dibuja) return;
  // borra los buffers de pantalla y z
  glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

  drawObjects();
  glutSwapBuffers();

#ifdef _DEBUG
  // chequea errores
  int errornum=glGetError();
  while(errornum!=GL_NO_ERROR){
    if (cl_info){
           if(errornum==GL_INVALID_ENUM)
             cout << "GL_INVALID_ENUM" << endl;
      else if(errornum==GL_INVALID_VALUE)
             cout << "GL_INVALID_VALUE" << endl;
      else if (errornum==GL_INVALID_OPERATION)
             cout << "GL_INVALID_OPERATION" << endl;
      else if (errornum==GL_STACK_OVERFLOW)
             cout << "GL_STACK_OVERFLOW" << endl;
      else if (errornum==GL_STACK_UNDERFLOW)
             cout << "GL_STACK_UNDERFLOW" << endl;
      else if (errornum==GL_OUT_OF_MEMORY)
             cout << "GL_OUT_OF_MEMORY" << endl;
    }
    errornum=glGetError();
  }
#endif // _DEBUG
}

//------------------------------------------------------------
// Regenera la matriz de proyeccion
// cuando cambia algun parametro de la vista
void regen() {
//  if (cl_info) cout << "regen" << endl;
  if (!dibuja) return;

  // matriz de proyeccion
  glMatrixMode(GL_PROJECTION);  glLoadIdentity();

  double w0=(double)w/2/escala,h0=(double)h/2/escala; // semiancho y semialto en el target

  // frustum, perspective y ortho son respecto al eye pero con los z positivos (delante del ojo)
  if (perspectiva){ // perspectiva
    double // "medio" al cuete porque aqui la distancia es siempre 5
      delta[3]={(target[0]-eye[0]), // vector ojo-target
                (target[1]-eye[1]),
                (target[2]-eye[2])},
      dist=sqrt(delta[0]*delta[0]+
                delta[1]*delta[1]+
                delta[2]*delta[2]);
    w0*=znear/dist,h0*=znear/dist;  // w0 y h0 en el near
    glFrustum(-w0,w0,-h0,h0,znear,zfar);
  }
  else { // proyeccion ortogonal
    glOrtho(-w0,w0,-h0,h0,znear,zfar);
  }

  glMatrixMode(GL_MODELVIEW); glLoadIdentity(); // matriz del modelo->view

  if (luz_camara) // luz fija a la camara
    glLightfv(GL_LIGHT0,GL_POSITION,lpos);  // ubica la luz
  gluLookAt(   eye[0],   eye[1],   eye[2],
            target[0],target[1],target[2],
                up[0],    up[1],    up[2]);// ubica la camara

  glRotatef(amy,0,1,0); // rota los objetos alrededor de y

  if (!luz_camara) // luz fija en el espacio del modelo
    glLightfv(GL_LIGHT0,GL_POSITION,lpos);  // ubica la luz

  Display_cb();
}

//------------------------------------------------------------
// Animacion

// Si no hace nada hace esto
// glutIdleFunc lo hace a la velocidad que de la maquina
// glutTimerFunc lo hace cada tantos milisegundos

// glutTimerFunc funciona mal, hace cosas raras que son muy visibles
// cuando la espera (msecs) es grande

// El "framerate" real (cuadros por segundo)
// depende de la complejidad del modelo (lod) y la aceleracion por hardware
void Idle_cb() {
  static int suma,counter=0;// esto es para analisis del framerate real
  // Cuenta el lapso de tiempo
  // (int!!?? en seguida se hace >MAX_INT pero ... funciona)
  static int anterior=glutGet(GLUT_ELAPSED_TIME); // milisegundos desde que arranco
  if (msecs!=1){ // si msecs es 1 no pierdo tiempo
    int tiempo=glutGet(GLUT_ELAPSED_TIME), lapso=tiempo-anterior;
    if (lapso<msecs) return;
    suma+=lapso;
    if (++counter==100) {
      cout << "<ms/frame>= " << suma/100.0 << endl;
      counter=suma=0;
    }
    anterior=tiempo;
  }
  if (rota) { // los objetos giran 1 grado alrededor de y
    amy+=1; if (amy>360) amy-=360;
    regen();
  }
  else Display_cb(); // redibuja
}

//------------------------------------------------------------
// Maneja cambios de ancho y alto de la ventana
void Reshape_cb(int width, int height){
  h=height; w=width;
  if (cl_info) cout << "reshape: " << w << "x" << h << endl;

  if (w==0||h==0) {// minimiza
    dibuja=false; // no dibuja mas
    glutIdleFunc(0); // no llama a cada rato a esa funcion
    return;
  }
  else if (!dibuja&&w&&h){// des-minimiza
    dibuja=true; // ahora si dibuja
    glutIdleFunc(Idle_cb); // registra de nuevo el callback
  }

  glViewport(0,0,w,h); // region donde se dibuja

  regen(); //regenera la matriz de proyeccion
}

//------------------------------------------------------------
// Teclado
/*
GLUT ACTIVE SHIFT //Set if the Shift modifier or Caps Lock is active.
GLUT ACTIVE CTRL //Set if the Ctrl modifier is active.
GLUT ACTIVE ALT //Set if the Alt modifier is active.
*/
inline short get_modifiers() {return modifiers=(short)glutGetModifiers();}

// Maneja pulsaciones del teclado (ASCII keys)
// x,y posicion del mouse cuando se teclea
void Keyboard_cb(unsigned char key,int x=0,int y=0) {
  switch (key){
    case 27: // escape => exit
      get_modifiers();
      if (!modifiers)
        exit(EXIT_SUCCESS);
      break;
    case 'a': case 'A': // Antialiasing
      antialias=!antialias;
      if (antialias){
        glEnable(GL_POINT_SMOOTH); glEnable(GL_LINE_SMOOTH); glEnable(GL_POLYGON_SMOOTH);
        if (cl_info) cout << "Antialiasing" << endl;
      }
      else {
        glDisable(GL_POINT_SMOOTH); glDisable(GL_LINE_SMOOTH); glDisable(GL_POLYGON_SMOOTH);
        if (cl_info) cout << "Sin Antialiasing" << endl;
      }
      break;
    case 'f': case 'F': // relleno
      relleno=!relleno;
      if (cl_info) cout << ((relleno)? "Relleno" : "Sin Relleno") << endl;
      break;
    case 'i': case 'I': // info
      cl_info=!cl_info;
      cout << ((cl_info)? "Info" : "Sin Info") << endl;
      break;
    case 'j': case 'J': // luz fija a la camara o en el espacio
      luz_camara=!luz_camara;
      if (cl_info)
        cout << "Luz fija " << ((luz_camara)? "a la camara" : "en el espacio") << endl;
      regen();
      break;
    case 'l': case 'L': // wire
      wire=!wire;
      if (cl_info) cout << ((wire)? "Lineas" : "Sin Lineas") << endl;
      break;
    case 'm': case 'M': // color/material
      color=!color;
      if (color) glDisable(GL_LIGHTING); else glEnable(GL_LIGHTING);
      if (cl_info) cout << ((color)? "Color" : "Material") << endl;
      break;
    case 'p': case 'P':  // perspectiva
      perspectiva=!perspectiva;
      if (cl_info) cout << ((perspectiva)? "Perspectiva" : "Ortogonal") << endl;
      regen();
      break;
    case 'r': case 'R': // rotacion
      rota=!rota;
      if (cl_info) cout << ((rota)? "Gira" : "No Gira") << endl;
      break;
  }
  if (!rota) glutIdleFunc(0); // no llama a cada rato a esa funcion
  else glutIdleFunc(Idle_cb); // registra el callback
  Display_cb();
}

// Special keys (non-ASCII)
/*
  GLUT KEY F[1,12] F[1,12] function key.
  GLUT KEY LEFT Left directional key.
  GLUT KEY UP Up directional key.
  GLUT KEY RIGHT Right directional key.
  GLUT KEY DOWN Down directional key.
  GLUT KEY PAGE UP Page up directional key.
  GLUT KEY PAGE DOWN Page down directional key.
  GLUT KEY HOME Home directional key.
  GLUT KEY END End directional key.
  GLUT KEY INSERT Inset directional key.
*/
// aca es int key
void Special_cb(int key,int xm=0,int ym=0) {
  if (key==GLUT_KEY_F4){ // alt+f4 => exit
    get_modifiers();
    if (modifiers==GLUT_ACTIVE_ALT)
      exit(EXIT_SUCCESS);
  }
  if (key==GLUT_KEY_UP||key==GLUT_KEY_DOWN){ // camara
    // la camara gira alrededor del eje -x
    double yc=eye[1]-target[1],zc=eye[2]-target[2],
           rc=sqrt(yc*yc+zc*zc),ac=atan2(yc,zc);
    ac+=((key==GLUT_KEY_UP) ? 1 : -1)/R2G;
    yc=rc*sin(ac); zc=rc*cos(ac);
    up[1]=zc; up[2]=-yc;  // perpendicular
    eye[1]=target[1]+yc; eye[2]=target[2]+zc;
    regen();
  }
  if (key==GLUT_KEY_LEFT){ // gira
    amy-=1;
    regen();
  }
  if (key==GLUT_KEY_RIGHT){ // gira
    amy+=1;
    regen();
  }
  if (key==GLUT_KEY_PAGE_UP||key==GLUT_KEY_PAGE_DOWN){ // velocidad
    if (key==GLUT_KEY_PAGE_DOWN) ms_i++;
    else ms_i--;
    if (ms_i<0) ms_i=0; if (ms_i==ms_n) ms_i--;
    msecs=ms_lista[ms_i];
    if (cl_info){
      if (msecs<1000)
        cout << 1000/msecs << "fps" << endl;
      else
        cout << msecs/1000 << "s/frame)" << endl;
    }
  }
}

//------------------------------------------------------------
// Menu
void Menu_cb(int value)
{
  switch (value){
    case 'a':
      Keyboard_cb('a');
      return;
    case 'f':
      Keyboard_cb('f');
      return;
    case 'i':
      Keyboard_cb('i');
      return;
    case 'j':
      Keyboard_cb('j');
      return;
    case 'l':
      Keyboard_cb('l');
      return;
    case 'm':
      Keyboard_cb('m');
      return;
    case 'p':
      Keyboard_cb('p');
      return;
    case 'r':
      Keyboard_cb('r');
      return;
    case (256+GLUT_KEY_UP):
      Special_cb(GLUT_KEY_UP);
      return;
    case (256+GLUT_KEY_DOWN):
      Special_cb(GLUT_KEY_DOWN);
      return;
    case (256+GLUT_KEY_LEFT):
      Special_cb(GLUT_KEY_LEFT);
      return;
    case (256+GLUT_KEY_RIGHT):
      Special_cb(GLUT_KEY_RIGHT);
      return;
    case (256+GLUT_KEY_PAGE_UP):
      Special_cb(GLUT_KEY_PAGE_UP);
      return;
    case (256+GLUT_KEY_PAGE_DOWN):
      Special_cb(GLUT_KEY_PAGE_DOWN);
      return;
    case 27: //esc
      exit(EXIT_SUCCESS);
  }
}

//------------------------------------------------------------
// Movimientos del mouse
void Motion_cb(int xm, int ym){ // drag
  if (boton==GLUT_LEFT_BUTTON){
    if (modifiers==GLUT_ACTIVE_SHIFT){ // cambio de escala
      escala=escala0*exp((yclick-ym)/100.0);
      regen();
    }
    else { // manipulacion
      double yc=eye[1]-target[1],zc=eye[2]-target[2];
      double ac=ac0+(ym-yclick)*180.0/h/R2G;
      yc=rc0*sin(ac); zc=rc0*cos(ac);
      up[1]=zc; up[2]=-yc;  // perpendicular
      eye[1]=target[1]+yc; eye[2]=target[2]+zc;
      amy=amy0+(xm-xclick)*180.0/w;
      regen();
    }
  }
}

// Clicks del mouse
// GLUT LEFT BUTTON, GLUT MIDDLE BUTTON, or GLUT RIGHT BUTTON
// The state parameter is either GLUT UP or GLUT DOWN
// glutGetModifiers may be called to determine the state of modifier keys
void Mouse_cb(int button, int state, int x, int y){
  static bool old_rota=false;
  if (button==GLUT_LEFT_BUTTON){
    if (state==GLUT_DOWN) {
      xclick=x; yclick=y;
      boton=button;
      old_rota=rota; rota=false;
      get_modifiers();
      glutMotionFunc(Motion_cb); // callback para los drags
      if (modifiers==GLUT_ACTIVE_SHIFT){ // cambio de escala
        escala0=escala;
      }
      else { // manipulacion
        double yc=eye[1]-target[1],zc=eye[2]-target[2];
        rc0=sqrt(yc*yc+zc*zc); ac0=atan2(yc,zc);
        amy0=amy;
      }
    }
    else if (state==GLUT_UP){
      rota=old_rota;
      boton=-1;
      glutMotionFunc(0); // anula el callback para los drags
    }
  }
}

//------------------------------------------------------------
// pregunta a OpenGL por el valor de una variable de estado
int integerv(GLenum pname){
  int value;
  glGetIntegerv(pname,&value);
  return value;
}
#define _PRINT_INT_VALUE(pname) #pname << ": " << integerv(pname) <<endl

//------------------------------------------------------------
// Inicializa GLUT y OpenGL
void initialize() {
  // pide z-buffer, color RGBA y double buffering
  glutInitDisplayMode(GLUT_DEPTH|GLUT_RGBA|GLUT_DOUBLE);

  glutInitWindowSize(w,h); glutInitWindowPosition(50,50);

  glutCreateWindow("Visibilidad"); // crea el main window

  //declara los callbacks
  //los que no se usan no se declaran
  glutDisplayFunc(Display_cb); // redisplays
  glutReshapeFunc(Reshape_cb); // cambio de alto y ancho
  glutKeyboardFunc(Keyboard_cb); // teclado
  glutSpecialFunc(Special_cb); // teclas especiales
  glutMouseFunc(Mouse_cb); // botones picados
  if (!(dibuja&&rota)) glutIdleFunc(0); // no llama a cada rato a esa funcion
  else glutIdleFunc(Idle_cb); // registra el callback

  // crea el menu
  glutCreateMenu(Menu_cb);
    glutAddMenuEntry("     [p]_Perspectiva/Ortogonal ", 'p');
    glutAddMenuEntry("     [r]_Rota                  ", 'r');
    glutAddMenuEntry("     [l]_Lineas                ", 'l');
    glutAddMenuEntry("     [f]_Caras Rellenas        ", 'f');
    glutAddMenuEntry("     [a]_Antialiasing ON/OFF   ", 'a');
    glutAddMenuEntry("     [m]_Color/Material        ", 's');
    glutAddMenuEntry("    [Up]_Sube Camara           ", (256+GLUT_KEY_UP));
    glutAddMenuEntry("  [Down]_Baja Camara           ", (256+GLUT_KEY_DOWN));
    glutAddMenuEntry("  [Left]_Gira objeto           ", (256+GLUT_KEY_LEFT));
    glutAddMenuEntry(" [Right]_Gira objeto           ", (256+GLUT_KEY_RIGHT));
    glutAddMenuEntry("  [PgUp]_Aumenta Framerate     ", (256+GLUT_KEY_PAGE_UP));
    glutAddMenuEntry("  [Pgdn]_Disminuye Framerate   ", (256+GLUT_KEY_PAGE_DOWN));
    glutAddMenuEntry("     [j]_Luz fija ON/OFF       ", 'j');
    glutAddMenuEntry("     [i]_Info ON/OFF           ", 'i');
    glutAddMenuEntry("   [Esc]_Exit                  ", 27);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  // ========================
  // estado normal del OpenGL
  // ========================

  glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); // habilita el z-buffer
  glEnable(GL_POLYGON_OFFSET_FILL); glPolygonOffset (1,1); // coplanaridad

  // antialiasing
  if (antialias){
    glEnable(GL_POINT_SMOOTH); glEnable(GL_LINE_SMOOTH); glEnable(GL_POLYGON_SMOOTH);
  }
  else {
    glDisable(GL_POINT_SMOOTH); glDisable(GL_LINE_SMOOTH); glDisable(GL_POLYGON_SMOOTH);
  }
  glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);
  glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
  glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);

  // color de fondo
  glClearColor(fondo[0],fondo[1],fondo[2],fondo[3]);

  // direccion de los poligonos
  glFrontFace(GL_CCW); glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  glEnable(GL_CULL_FACE); glCullFace(GL_BACK);

  // color o material+luces
  if (color) glDisable(GL_LIGHTING); else glEnable(GL_LIGHTING);

  // define luces
  glLightfv(GL_LIGHT0,GL_AMBIENT,lambient);
  glLightfv(GL_LIGHT0,GL_DIFFUSE,ldiffuse);
  // caras de atras y adelante distintos (1) o iguales (0)
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,1);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,0);
  glEnable(GL_LIGHT0);

  // material estandar
  static const float
    fambient[]={0.4f,0.2,0.1,.5f},
    fdiffuse[]={0.5f,0.2f,0.1f,.5f},
    fspecular[]={0.6f,0.2f,0.1f,.5f},
    bambient[]={0.2f,0.4f,0.4f,0.5f},
    bdiffuse[]={0.2f,0.5f,0.4f,.5f},
    bspecular[]={0.2f,0.6f,0.4f,.5f};
  static const int
    fshininess=50,
    bshininess=10;
  glMaterialfv(GL_FRONT,GL_AMBIENT,fambient);
  glMaterialfv(GL_FRONT,GL_DIFFUSE,fdiffuse);
  glMaterialfv(GL_FRONT,GL_SPECULAR,fspecular);
  glMateriali(GL_FRONT,GL_SHININESS,fshininess);
  glMaterialfv(GL_BACK,GL_AMBIENT,bambient);
  glMaterialfv(GL_BACK,GL_DIFFUSE,bdiffuse);
  glMaterialfv(GL_BACK,GL_SPECULAR,bspecular);
  glMateriali(GL_BACK,GL_SHININESS,bshininess);

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

  regen(); // para que setee las matrices antes del 1er draw
}

//------------------------------------------------------------
// main
int main(int argc,char** argv) {
  glutInit(&argc,argv);// inicializa glut
  initialize(); // condiciones iniciales de la ventana y OpenGL
  glutMainLoop(); // entra en loop de reconocimiento de eventos
  return 0;
}
