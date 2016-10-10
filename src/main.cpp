/**
* @author Ricardo Barros Duarte d'Oliveira
* Implementa��o de renderiza��o de terreno baseada em
* Duchaineau, Mark, et al. "ROAMing terrain: real-time optimally adapting
* meshes." Proceedings of the 8th Conference on Visualization'97. IEEE Computer
* Society Press, 1997.
*/

#ifdef defined(_WIN32) || defined(WIN32)
#define OS_Windows
#endif
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <math.h>
#include <glm/glm.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctime>
#include <iostream>
#include <vector>

#include "../include/utils.h"
#include "../include/Terrain.h"
#include "../include/Vbo.h"

using namespace std;
std::vector<glm::vec3>* glVertexArray;
std::vector<glm::vec3>* glNormalArray;
std::vector<glm::vec3>* glTangentArray;
std::vector<glm::vec3>* glBitangentArray;
std::vector<glm::vec2>* glUvArray;
Mouse mouse;
/**
* M�todo para renderizar a cena
*/
void sceneRenderer(void)
{
  renderScene();

  glutSwapBuffers(); // Copia o mapa para a janela
}

/**
* M�todo para mapear o teclado
*/
void keyMap(unsigned char key, GLint x, GLint y)
{
  switch (key)
  {
    case 'a':
      animateToggle();
      break;
    case 'c':
      cameraMode();
      break;
    case 'w':
      renderMode();
      break;
    case 'r':
      frustrumToggle();
      break;

    case '+':
      toggleMoreDetail();
      break;
    case '-':
      toggleLessDetail();
      break;

    case '1':
      KeyFOVDown();
      break;
    case '2':
      KeyFOVUp();
      break;
  }
}

/**
* M�todo para mapear as teclas especiais do GLUT
*/
void GLUTKeySpecialDown(GLint key, GLint x, GLint y)
{
  switch (key)
  {
    case GLUT_KEY_UP:

      KeyForward();//KeyUp();
      break;
    case GLUT_KEY_DOWN:
      KeyBackward();//KeyDown();
      break;
    case GLUT_KEY_LEFT:
      KeyLeft();
      break;
    case GLUT_KEY_RIGHT:
      KeyRight();
      break;
  }
}

void idleFunction(void)
{
  idleFn();
  sceneRenderer();
}

/**
* M�todo respons�vel em mapear os eventos do mouse
*/
void GLUTMouseClick(GLint button, GLint state, GLint x, GLint y)
{
  if (button == GLUT_LEFT_BUTTON)
    {
    if (state == GLUT_DOWN)
    {
      glRotate = 1;
      glStartX = -1;
    } else
      glRotate = GL_ZERO;
  }
}

void mousePosition(GLint x, GLint y)
{
  mouse.x = x;
  mouse.y = y;
}

void glPrint(int x, int y, char *text)
{
  std::string str(text);

  glDisable(GL_TEXTURE_2D); //added this
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(GL_ZERO, WINDOW_WIDTH, GL_ZERO, WINDOW_WIDTH);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glRasterPos2i(x, y);
  void * font = GLUT_BITMAP_HELVETICA_10;
  for (string::iterator i = str.begin(); i != str.end(); ++i)
  {
    char c = *i;
    glColor3d(GL_ONE, GL_ONE, GL_ONE);
    glutBitmapCharacter(font, c);
  }
  glMatrixMode(GL_PROJECTION); //swapped this with...
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW); //...this
  glPopMatrix();
  //added this
  glEnable(GL_TEXTURE_2D);
}

/**
* Corpo principal da aplica��o
*/
int main(int argc, char *argv[])
{
  glVertexArray = new std::vector<glm::vec3>();
  glNormalArray = new std::vector<glm::vec3>();
  glTangentArray = new std::vector<glm::vec3>();
  glBitangentArray = new std::vector<glm::vec3>();
  glUvArray = new std::vector<glm::vec2>();
  appendUvData(glUvArray);
  // Configura��o iniciais do GLUT


  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowPosition(GL_ZERO, GL_ZERO);
  glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);


  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);

  if (glutCreateWindow("Renderizar Terreno conforme ROAM") < GL_ZERO)
  {
    printf("ERROR: No window system found!\n");
    exit(GL_ZERO);
  }

  // Conjunto de fun��es para redimesionamento, para quando estiver ocioso e mapeamento de teclado e mouse
  glutReshapeFunc(changeSize);
  glutIdleFunc(idleFunction);
  glutKeyboardFunc(keyMap);
  glutMouseWheelFunc(mouseWheel);
  glutSpecialFunc(GLUTKeySpecialDown);
  glutMouseFunc(GLUTMouseClick);
  glutMotionFunc(mouseMove);
  glutPassiveMotionFunc(mousePosition);
  glutDisplayFunc(sceneRenderer);
  glewExperimental = true; // Needed for core profile
  GLint glewOk = glewInit();
  if(glewOk==GLEW_OK)
  {
    shaderPlumbing();
    // Configura��o do OpenGL
    SetupRC();
    drawMode();

    // Carrega o arquivo do terreno conforme o tamanho pre-definido
    loadTerrain(MAP_SIZE, &glHeightMap);

    GLint nAvgFrames = -1;

    // Come�a a anima��o, calcula o tempo inicial em milisegundos, come�a o loop do GLUT e calcula o valor m�dio de frames por segundo
    if (roamInit(glHeightMap) == GL_ZERO)
    {
      glAnimate = 1;
      glStartTime = time(GL_ZERO);
      glutMainLoop();
      glEndTime = time(GL_ZERO);
      nAvgFrames = (glFrames * 1000) / (glEndTime - glStartTime);
    }

    freeTerrain();

    glVertexArray->clear();
    glNormalArray->clear();
    glTangentArray->clear();
    glBitangentArray->clear();
    glUvArray->clear();
    terminateShader();
    return nAvgFrames;
  }
  else
  {
    printf("Failed to initialize GLEW!\n");
    return -GL_ONE;
  }
}

/**
* Fun��o stub para manter compatibilidade com Windows
*/
#ifdef OS_Windows
  int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpCmdLine, int nCmdShow) {
    return main(nCmdShow, &lpCmdLine);
  }
#endif
