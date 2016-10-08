/**
* @author Ricardo Barros Duarte d'Oliveira
* Implementação de renderização de terreno baseada em
* Duchaineau, Mark, et al. "ROAMing terrain: real-time optimally adapting
* meshes." Proceedings of the 8th Conference on Visualization'97. IEEE Computer
* Society Press, 1997.
*/

#ifdef defined(_WIN32) || defined(WIN32)
#define OS_Windows
#endif
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
std::vector<glm::vec3>* vertexBuffer;
std::vector<glm::vec3>* normals;
std::vector<glm::vec3>* tangents;
std::vector<glm::vec3>* bitangents;
std::vector<glm::vec2>* uv;
/**
* Método para renderizar a cena
*/
void sceneRenderer(void)
{
  renderScene();

  glutSwapBuffers(); // Copia o mapa para a janela
}

/**
* Método para mapear o teclado
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
* Método para mapear as teclas especiais do GLUT
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
* Método responsável em mapear os eventos do mouse
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
* Corpo principal da aplicação
*/
int main(int argc, char *argv[])
{
  vertexBuffer = new std::vector<glm::vec3>();
  normals = new std::vector<glm::vec3>();
  tangents = new std::vector<glm::vec3>();
  bitangents = new std::vector<glm::vec3>();
  uv = new std::vector<glm::vec2>();
  appendUvData(uv);
  // Configuração iniciais do GLUT
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowPosition(GL_ZERO, GL_ZERO);
  glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

  if (glutCreateWindow("Renderizar Terreno conforme ROAM") < GL_ZERO)
  {
    printf("ERROR: No window system found!\n");
    exit(GL_ZERO);
  }

  // Conjunto de funções para redimesionamento, para quando estiver ocioso e mapeamento de teclado e mouse
  glutReshapeFunc(changeSize);
  glutIdleFunc(idleFunction);
  glutKeyboardFunc(keyMap);
  glutMouseWheelFunc(mouseWheel);
  glutSpecialFunc(GLUTKeySpecialDown);
  glutMouseFunc(GLUTMouseClick);
  glutMotionFunc(mouseMove);
  glutDisplayFunc(sceneRenderer);
  // Configuração do OpenGL
  SetupRC();
  drawMode();

  // Carrega o arquivo do terreno conforme o tamanho pre-definido
  loadTerrain(MAP_SIZE, &glHeightMap);

  GLint nAvgFrames = -1;

  // Começa a animação, calcula o tempo inicial em milisegundos, começa o loop do GLUT e calcula o valor médio de frames por segundo
  if (roamInit(glHeightMap) == GL_ZERO)
  {
    glAnimate = 1;
    glStartTime = time(GL_ZERO);
    glutMainLoop();
    glEndTime = time(GL_ZERO);
    nAvgFrames = (glFrames * 1000) / (glEndTime - glStartTime);
  }

  freeTerrain();

  return nAvgFrames;
}

/**
* Função stub para manter compatibilidade com Windows
*/
#ifdef OS_Windows
  int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpCmdLine, int nCmdShow) {
    return main(nCmdShow, &lpCmdLine);
  }
#endif
