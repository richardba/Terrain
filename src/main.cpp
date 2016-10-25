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
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <math.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctime>
#include <iostream>
#include <vector>

#include "../include/utils.h"
#include "../include/Terrain.h"
#include "../include/Vbo.h"
#include "../include/Shader.h"

using namespace std;
std::vector<glm::vec3>* glVertexArray;
std::vector<glm::vec3>* glNormalArray;
std::vector<glm::vec3>* glTangentArray;
std::vector<glm::vec3>* glBitangentArray;
std::vector<glm::vec2>* glUvArray;
GLFWwindow* window;
Shader* toon;
Mouse mouse;
/**
* Método para renderizar a cena
*/
void sceneRenderer(void)
{
  if(!GLFW_MODE)
  {
    renderScene(NULL);
    glutSwapBuffers(); // Copia o mapa para a janela
  }
  else
  {
    renderScene(window);
    glfwSwapBuffers(window);
  }
}

/**
* Método para mapear o teclado
*/
void keyMap(unsigned char key, GLint x, GLint y)
{
  switch (key)
  {
    case 'a':
      //animateToggle();
      break;
    case 'c':
      //cameraMode();
      break;
    case 'w':
      //renderMode();
      break;
    case 'r':
      //frustrumToggle();
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
* Corpo principal da aplicação
*/
int main(int argc, char *argv[])
{
  glVertexArray = new std::vector<glm::vec3>();
  glNormalArray = new std::vector<glm::vec3>();
  glTangentArray = new std::vector<glm::vec3>();
  glBitangentArray = new std::vector<glm::vec3>();
  glUvArray = new std::vector<glm::vec2>();
  bool loaded = loadOBJ("uv.obj", glUvArray);

  if(!GLFW_MODE)
  {
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
    glutPassiveMotionFunc(mousePosition);
    glutDisplayFunc(sceneRenderer);
  }

  if(GLFW_MODE){
    if( !glfwInit() )
    {
      fprintf( stderr, "Failed to initialize GLFW\n" );
      getchar();
      return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE); // So that glBegin/glVertex/glEnd work

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "Tutorial 13 - Normal Mapping", NULL, NULL);
    if( window == NULL )
    {
      fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
      getchar();
      glfwTerminate();
      return -1;
    }
    glfwMakeContextCurrent(window);
  }

  glewExperimental = true; // Needed for core profile
  if(!glewInit())
  {
    if(TOON_MODE)
    {
        toon = toonShading();
        toon->bind();
        toon->unbind();
    }

    shaderPlumbing();
    // Configuração do OpenGL
    SetupRC();
    drawMode();

    // Carrega o arquivo do terreno conforme o tamanho pre-definido
    loadTerrain(MAP_SIZE, &glHeightMap);
    GLint nAvgFrames = -1;
    if(GLFW_MODE)
    {
      glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
      // Hide the mouse and enable unlimited mouvement
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

      // Set the mouse at the center of the screen
      glfwPollEvents();
      glfwSetCursorPos(window, 1024/2, 768/2);
    }
    // Começa a animação, calcula o tempo inicial em milisegundos, começa o loop do GLUT e calcula o valor médio de frames por segundo
    if (roamInit(glHeightMap) == GL_ZERO)
    {
      glAnimate = 0;
      if(GLFW_MODE)
      {
        GLuint frameRate=0;
        do
        {
          computeMatricesFromInputs(window);
          glStartTime = glfwGetTime();
          frameRate++;
          sceneRenderer();
          glEndTime = glfwGetTime();
          if(glStartTime - glEndTime >= 1.0)
          {
            nAvgFrames = 1000.0/double(frameRate);
            frameRate = 0;
          }
          glfwPollEvents();
        }
        while(glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
              glfwWindowShouldClose(window) == 0 );
      }
      else
      {
        glStartTime = time(GL_ZERO);
        glutMainLoop();
        glEndTime = time(GL_ZERO);
        nAvgFrames = (glFrames * 1000) / (glEndTime - glStartTime);
      }

    }

    freeTerrain();

    glVertexArray->clear();
    glNormalArray->clear();
    glTangentArray->clear();
    glBitangentArray->clear();
    glUvArray->clear();
    terminateShader();
    if(GLFW_MODE)
      glfwTerminate();
    return nAvgFrames;
  }
  else
  {
    printf("Failed to initialize GLEW!\n");
    return -GL_ONE;
  }
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
