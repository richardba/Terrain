#include <GL/glew.h>
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "textfile.h"

GLint loc;
GLuint v, f, f2, p1, p2;
GLuint program[2];
float lpos[4] = { 1.0, 0.0, 1.0, 0.0 };

void renderOutline()
{
    glUseProgram(program[1]);
    glCullFace(GL_FRONT);
    glDepthMask(GL_FALSE);
    glEnable(GL_DEPTH_TEST);
    glutSolidTorus(0.5, 1, 20, 20);

    glCullFace(GL_FRONT);
    glDepthMask(GL_TRUE);
    glUseProgram(program[0]);
}

void changeSize(int w, int h)
{

    if (h == 0)
        h = 1;

    float ratio = 1.0 * w / h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glViewport(0, 0, w, h);

    gluPerspective(45, ratio, 1, 100);
    glMatrixMode(GL_MODELVIEW);
}

float a = 0;

void renderScene(void)
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderOutline();
    glLoadIdentity();
    gluLookAt(0.0, 5.0, 5.0,
        0.0, 0.0, 0.0,
        0.0f, 1.0f, 0.0f);

    glLightfv(GL_LIGHT0, GL_POSITION, lpos);
    glRotatef(a, 0, 1, 0);
    //glutSolidTorus(0.6, 1.1, 20, 20);
    glutSolidTorus(0.5, 1, 20, 20);
    a += 0.05;

    glutSwapBuffers();
}

void processNormalKeys(unsigned char key, int x, int y)
{

    if (key == 27)
        exit(0);
}

#define printOpenGLError() printOglError(__FILE__, __LINE__)

int printOglError(char* file, int line)
{

    GLenum glErr;
    int retCode = 0;

    glErr = glGetError();
    while (glErr != GL_NO_ERROR) {
        printf("glError in file %s @ line %d: %s\n", file, line, gluErrorString(glErr));
        retCode = 1;
        glErr = glGetError();
    }
    return retCode;
}

void printShaderInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten = 0;
    char* infoLog;

    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

    if (infologLength > 0) {
        infoLog = (char*)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
        printf("%s\n", infoLog);
        free(infoLog);
    }
}

void printProgramInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten = 0;
    char* infoLog;

    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

    if (infologLength > 0) {
        infoLog = (char*)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
        printf("%s\n", infoLog);
        free(infoLog);
    }
}

void setShaders(GLuint p, std::string filename)
{

    char *vs = NULL, *fs = NULL, *fs2 = NULL;

    v = glCreateShader(GL_VERTEX_SHADER);
    f = glCreateShader(GL_FRAGMENT_SHADER);
    f2 = glCreateShader(GL_FRAGMENT_SHADER);

    std::string vsName = filename + ".vert", fsName = filename + ".frag";
    vs = textFileRead(vsName.c_str());
    fs = textFileRead(fsName.c_str());

    const char* vv = vs;
    const char* ff = fs;

    glShaderSource(v, 1, &vv, NULL);
    glShaderSource(f, 1, &ff, NULL);

    free(vs);
    free(fs);

    glCompileShader(v);
    glCompileShader(f);

    printShaderInfoLog(v);
    printShaderInfoLog(f);
    printShaderInfoLog(f2);

    program[p] = glCreateProgram();
    glAttachShader(program[p], v);
    glAttachShader(program[p], f);

    glLinkProgram(program[p]);
    printProgramInfoLog(program[p]);

    loc = glGetUniformLocation(p1, "time");
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Toon Shading");

    glutDisplayFunc(renderScene);
    glutIdleFunc(renderScene);
    glutReshapeFunc(changeSize);
    glutKeyboardFunc(processNormalKeys);

    glEnable(GL_DEPTH_TEST);
    glClearColor(.40f, .53f, .60f, 1.0f);
    //	glEnable(GL_CULL_FACE);

    glewInit();
    if (glewIsSupported("GL_VERSION_2_0"))
        printf("Ready for OpenGL 2.0\n");
    else {
        printf("OpenGL 2.0 not supported\n");
        exit(1);
    }

    setShaders(1, "silhoutte");
    setShaders(0, "metallic");

    glutMainLoop();

    return 0;
}
