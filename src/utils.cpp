#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/utils.h"
#include "../include/Terrain.h"
#include "../include/Vbo.h"
#include "../include/Shader.h"

/* Constantes responsáveis pelo modo da câmera */
#define FOLLOW          (0)
#define OBSERVE         (1)
#define DRIVE           (2)
#define FLY_MODE        (3)

#define FAR_CLIP        (2500.0f)
#define FOV_ANGLE       (90.0f)
#define NEAR_CLIP       (1.f)
#define ZERO_F          (0.f)

/**
* Variáveis globais
* @glAnimationAngle   define o angulo da animação, inicialmente zero
* @glCameraPos        um array com as posições da camera
* @glCameraRotation   um array de projeção, contendo o eixo de rotação da camera
* @glPerspective      define a perspectiva da camera
* @glViewPosition     um array com as posições da view
* @glAnimate          responsavel pela animação do frustrum
* @glCamera           modo de visualização da camera, esse modo pode ser navegavel ou observavel topologicamente
* @glDrawFrustum      flag responsavel pela exibição do Frustum, inicialmente ligado
* @glDrawMode         modo de renderização da malha (e.g.: wireframe, textured)
* @glFrames           contador de frames da cena
* @glFrameDiff        representa o inicio da diferença de frames para calculo do framerate
* @glNumTrisDesired   representa o número de triangulos para a subdivisão por frame
* @glNumTrisRendered  representa o número de triangulos renderizados para um determinado mesh
* @glRotate           responsavel pela rotação do frustrum
* @glStartX           coordenada de posição inicial da camera no modo navegavel
* @glStartY           coordenada de posição inicial da camera no modo navegavel
* @glTexture          define a textura
* @glTerrain          define o terreno
* @glFoVX             define o campo de visão
* @glEndTime          representa o final do loop da aplicação openGL
* @glStartTime        representa o inicio do loop da aplicação openGL
* @glHeightMap        uma string que representa or arquivo do mapa de altura
* @glHeightMaster     uma string para auxiliar nas operações de manipulação do arquivo do mapa de altura

*/
GLfloat glAnimationAngle = ZERO_F;
GLfloat glCameraPos[]	= {ZERO_F, ZERO_F, -555.f};
GLfloat glCameraRotation[]	= {42.f, -181.f, ZERO_F};
GLfloat glPerspective;
GLfloat glViewPosition[]		= {ZERO_F, 5.f, ZERO_F};
GLint glAnimate     = GL_ZERO;
GLint glCamera      = DRIVE;
GLint glDrawFrustum = 1;
GLint glDrawMode     = DRAW_USE_TEXTURE;
GLint glFrames;
float glFrameDiff = 50;
GLint glNumTrisDesired = 10000;
GLint glNumTrisRendered;
GLint glRotate      = GL_ZERO;
GLint glStartX       = -1;
GLint glStartY;
GLuint glTexture=1;
Terrain glTerrain;
Shader shaderHandle;
GLuint glVertexArray;
GLuint glVertexBuffer;
float glFoVX = 90.0f;
long glEndTime;
long glStartTime;
unsigned char *glHeightMap;
unsigned char *glHeightMaster;


GLvoid shaderPlumbing()
{
	//position data
	glBindVertexArray(glVertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, glVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*vertexBuffer->size(), vertexBuffer->data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(glGetAttribLocation(shaderHandle.id(), "inPosition"));
	glVertexAttribPointer(glGetAttribLocation(shaderHandle.id(), "inPosition"), 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	//color data
//	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffers[1]);
//	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(GLfloat)*nvertices, colors->data(), GL_STATIC_DRAW);
//	glEnableVertexAttribArray(glGetAttribLocation(shaderHandle, "color"));
//	glVertexAttribPointer(glGetAttribLocation(shaderHandle, "color"), 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
}

/**
* Método responsável pela normalização do vetor de normais, onde dado um vetor com três posições, calcula-se o tamanho do vetor,
* caso o tamanho seja zero, então trataremos o tamanho como um para previnir divisões por zero, então obtemos o vetor de normais
* resultante das divisões sucessivas de cada elemento pelo tamanho onde o tamanho é a raiz quadrada da soma do quadrado de cada elemento.
* @param vector   um vetor contendo três posições para normalização do vetor de normais
*/
void reduceToUnit(float vector[3])
{
    float length;
    length = sqrtf((vector[0]*vector[0]) +
                   (vector[1]*vector[1]) +
                   (vector[2]*vector[2]));
    if(length == .0f)
    {
        length = 1.0f;
    }
    vector[0] /= length;
    vector[1] /= length;
    vector[2] /= length;
}

/**
* Método responsável pelo calculo das normais onde dado uma matriz de três posições se calcula dois vetores resultantes
* e a partir desses vetores é feito um produto vetorial para obtenção do vetor de normais, que sera armazenado
* na variável normal, que será passada para o método de normalização.
* @param vector uma matriz contendo as coordenadas dos pontos
* @param normal o vetor resultante
*/
void calcNormal(float vector[3][3], float normal[3])
{
    float vector1[3], vector2[3];
    static const GLint x = GL_ZERO;
    static const GLint y = 1;
    static const GLint z = 2;

    vector1[x] = vector[0][x] - vector[1][x];
    vector1[y] = vector[0][y] - vector[1][y];
    vector1[z] = vector[0][z] - vector[1][z];

    vector2[x] = vector[1][x] - vector[2][x];
    vector2[y] = vector[1][y] - vector[2][y];
    vector2[z] = vector[1][z] - vector[2][z];

    normal[x] = vector1[y]*vector2[z] - vector1[z]*vector2[y];
    normal[y] = vector1[z]*vector2[x] - vector1[x]*vector2[z];
    normal[z] = vector1[x]*vector2[y] - vector1[y]*vector2[x];

    reduceToUnit(normal);
}


/**
* Método para leitura do Heightmap
* @param size           tamanho do mapa
* @param startPointer   ponteiro que representa o inicio da posição em memoria do Heightmap
*/
void loadTerrain(GLint size, unsigned char **startPointer)
{
    FILE *fp;
    char fileName[30];

    glHeightMaster = (unsigned char *)malloc( size * size * sizeof(unsigned char) + size * 2 );

    *startPointer = glHeightMaster + size;

    sprintf(fileName, "Heightmap%d.data", size);
    fp = fopen(fileName, "rb");

    if ( fp == NULL )
    {
        sprintf( fileName, "Heightmap.ved", size );
        fp = fopen(fileName, "rb");
        if ( fp != NULL )
            fseek( fp, 40, SEEK_SET );
    }

    if (fp == NULL)
    {
        memset( glHeightMaster, GL_ZERO, size * size + size * 2 );
        return;
    }
    fread(glHeightMaster + size, GL_ONE, (size * size), fp);
    fclose(fp);

    memcpy( glHeightMaster, glHeightMaster + size * size, size );

    memcpy( glHeightMaster + size * size + size, glHeightMaster + size, size );
}

/**
* Método para liberar o array do Height Field
*/
void freeTerrain()
{
    if ( glHeightMaster )
        free( glHeightMaster );
}

/**
* Método para mudança no modo de renderização
*/
void drawMode()
{
    switch (glDrawMode)
    {
    case DRAW_USE_TEXTURE:
        glDisable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);
        glPolygonMode(GL_FRONT, GL_FILL);
        break;

    case DRAW_USE_LIGHTING:
        glEnable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
        glPolygonMode(GL_FRONT, GL_FILL);
        break;

    case DRAW_USE_FILL_ONLY:
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
        glPolygonMode(GL_FRONT, GL_FILL);
        break;

    default:
    case DRAW_USE_WIREFRAME:
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
        glPolygonMode(GL_FRONT, GL_LINE);
        break;
    }
}

/**
* Inicialização da implementação do ROAM, onde checa-se algumas definições foram apropriadamente carregadas, aloca-se um espaço em memoria para a textura,
* é gerado um padrão randômico para a textura e padrão é aplicado em um tom creme, para dar aspecto de solo
*/
GLint roamInit(unsigned char *map)
{
    if ( glNumTrisDesired > POOL_SIZE )
        return -1;

    if ( POOL_SIZE < 100 )
        return -1;
    glBindTexture(GL_TEXTURE_2D, glTexture);

    unsigned char *pTexture = (unsigned char *)malloc(TEXTURE_SIZE*TEXTURE_SIZE*3);
    unsigned char *pTexWalk = pTexture;

    if ( !pTexture )
        exit(GL_ZERO);

    for ( GLint x = GL_ZERO; x < TEXTURE_SIZE; x++ )
        for ( GLint y = GL_ZERO; y < TEXTURE_SIZE; y++ )
        {
            GLint color = (GLint)(128.0+(40.0 * rand())/RAND_MAX);
            if ( color > 255 )color = 255;
            if ( color < GL_ZERO )    color = GL_ZERO;
            *(pTexWalk++) = color/1.2;
            *(pTexWalk++) = color/1.5;
            *(pTexWalk++) = color/1.7;
        }

    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, TEXTURE_SIZE, TEXTURE_SIZE, GL_RGB, GL_UNSIGNED_BYTE, pTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    free(pTexture);

    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

    glTerrain.Init(map);

    return GL_ZERO;
}

/**
* Método responsável em fazer chamada de funções responsáveis para renderização de um frame do cenário
*/
void roamDrawFrame()
{
    glTerrain.Reset();
    glTerrain.Tessellate();
    glTerrain.Render();
}

/**
* Método para renderização do Frustum
*/
void drawFrustum()
{
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);

    glPointSize(2.f);
    glLineWidth(3.f);

    glBegin(GL_LINES);

    glColor3f(GL_ONE, GL_ZERO, GL_ZERO);
    glVertex3f(	glViewPosition[0],
                glViewPosition[1],
                glViewPosition[2] );

    glVertex3f(	glViewPosition[0] + 50.0f * sinf( glPerspective * M_PI / 180.0f ),
                glViewPosition[1],
                glViewPosition[2] - 50.0f * cosf( glPerspective * M_PI / 180.0f ));


    glColor3f(GL_ZERO, GL_ONE, GL_ZERO);
    glVertex3f(	glViewPosition[0],
                glViewPosition[1],
                glViewPosition[2] );

    glVertex3f(	glViewPosition[0] + 1000.0f * sinf( (glPerspective-45.0f) * M_PI / 180.0f ),
                glViewPosition[1],
                glViewPosition[2] - 1000.0f * cosf( (glPerspective-45.0f) * M_PI / 180.0f ));

    glVertex3f(	glViewPosition[0],
                glViewPosition[1],
                glViewPosition[2] );

    glVertex3f(	glViewPosition[0] + 1000.0f * sinf( (glPerspective+45.0f) * M_PI / 180.0f ),
                glViewPosition[1],
                glViewPosition[2] - 1000.0f * cosf( (glPerspective+45.0f) * M_PI / 180.0f ));

    const float PI_DIV_180 = M_PI / 180.0f;
    const float FOV_DIV_2 = glFoVX/2;

    GLint ptEyeX = (GLint)(glViewPosition[0] - MESH_SIZE * sinf( glPerspective * PI_DIV_180 ));
    GLint ptEyeY = (GLint)(glViewPosition[2] + MESH_SIZE * cosf( glPerspective * PI_DIV_180 ));

    GLint ptLeftX = (GLint)(ptEyeX + 100.0f * sinf( (glPerspective-FOV_DIV_2) * PI_DIV_180 ));
    GLint ptLeftY = (GLint)(ptEyeY - 100.0f * cosf( (glPerspective-FOV_DIV_2) * PI_DIV_180 ));

    GLint ptRightX = (GLint)(ptEyeX + 100.0f * sinf( (glPerspective+FOV_DIV_2) * PI_DIV_180 ));
    GLint ptRightY = (GLint)(ptEyeY - 100.0f * cosf( (glPerspective+FOV_DIV_2) * PI_DIV_180 ));

    glColor3f(GL_ONE, GL_ONE, GL_ZERO);
    glVertex3f(	(float)ptEyeX,
                glViewPosition[1],
                (float)ptEyeY );

    glVertex3f(	(float)ptLeftX,
                glViewPosition[1],
                (float)ptLeftY);

    glVertex3f(	(float)ptEyeX,
                glViewPosition[1],
                (float)ptEyeY );

    glVertex3f(	(float)ptRightX,
                glViewPosition[1],
                (float)ptRightY);

    glEnd();

    glLineWidth(1.f);
    glColor3f(GL_ONE, GL_ONE, GL_ONE);

    drawMode();
}

/**
*
*/
void mouseWheel(GLint button, GLint dir, GLint x, GLint y)
{
    if (dir > GL_ZERO)
    {
        KeyFOVDown();
    }
    else
    {
        KeyFOVUp();
    }

    return;
}

/**
*
*/
void cameraMode(void)
{
    glCamera++;
    if ( glCamera > FLY_MODE )
        glCamera = FOLLOW;

    if (glCamera == FOLLOW)
        glAnimate = GL_ONE;
}

void renderMode(void)
{
    glDrawMode++;
    if ( glDrawMode > DRAW_USE_WIREFRAME )
        glDrawMode = DRAW_USE_TEXTURE;

    drawMode( );
}

/**
*
*/
void KeyForward(void)
{
    switch ( glCamera )
    {
    default:
    case FOLLOW:
        break;

    case OBSERVE:
        glCameraPos[2] += 5.0f;
        break;

    case DRIVE:
        glViewPosition[0] += 5.0f * sinf( glCameraRotation[ROTATE_YAW] * M_PI / 180.0f );
        glViewPosition[2] -= 5.0f * cosf( glCameraRotation[ROTATE_YAW] * M_PI / 180.0f );

        if ( glViewPosition[0] > MAP_SIZE ) glViewPosition[0] = MAP_SIZE;
        if ( glViewPosition[0] < GL_ZERO) glViewPosition[0] = GL_ZERO;

        if ( glViewPosition[2] > MAP_SIZE ) glViewPosition[2] = MAP_SIZE;
        if ( glViewPosition[2] < GL_ZERO) glViewPosition[2] = GL_ZERO;

        glViewPosition[1] = (MULT_SCALE * glHeightMap[(GLint)glViewPosition[0] + ((GLint)glViewPosition[2] * MAP_SIZE)]) + 4.0f;
        break;

    case FLY_MODE:
        glViewPosition[0] += 5.0f * sinf( glCameraRotation[ROTATE_YAW]   * M_PI / 180.0f )
                                   * cosf( glCameraRotation[ROTATE_PITCH] * M_PI / 180.0f );
        glViewPosition[2] -= 5.0f * cosf( glCameraRotation[ROTATE_YAW]   * M_PI / 180.0f )
                             * cosf( glCameraRotation[ROTATE_PITCH] * M_PI / 180.0f );
        glViewPosition[1] -= 5.0f * sinf( glCameraRotation[ROTATE_PITCH] * M_PI / 180.0f );
        break;
    }
}

/**
*
*/
void KeyLeft(void)
{
    if ( glCamera == OBSERVE )
        glCameraRotation[1] -= 5.0f;
}

/**
*
*/
void KeyBackward(void)
{
    switch ( glCamera )
    {
    default:
    case FOLLOW:
        break;

    case OBSERVE:
        glCameraPos[2] -= 5.0f;
        break;

    case DRIVE:
        glViewPosition[0] -= 5.0f * sinf( glCameraRotation[ROTATE_YAW] * M_PI / 180.0f );
        glViewPosition[2] += 5.0f * cosf( glCameraRotation[ROTATE_YAW] * M_PI / 180.0f );

        if ( glViewPosition[0] > MAP_SIZE ) glViewPosition[0] = MAP_SIZE;
        if ( glViewPosition[0] < GL_ZERO) glViewPosition[0] = GL_ZERO;

        if ( glViewPosition[2] > MAP_SIZE ) glViewPosition[2] = MAP_SIZE;
        if ( glViewPosition[2] < GL_ZERO) glViewPosition[2] = GL_ZERO;

        glViewPosition[1] = (MULT_SCALE * glHeightMap[(GLint)glViewPosition[0] + ((GLint)glViewPosition[2] * MAP_SIZE)]) + 4.0f;
        break;

    case FLY_MODE:
        glViewPosition[0] -= 5.0f * sinf( glCameraRotation[ROTATE_YAW]   * M_PI / 180.0f )
                                   * cosf( glCameraRotation[ROTATE_PITCH] * M_PI / 180.0f );
        glViewPosition[2] += 5.0f * cosf( glCameraRotation[ROTATE_YAW]   * M_PI / 180.0f )
                             * cosf( glCameraRotation[ROTATE_PITCH] * M_PI / 180.0f );
        glViewPosition[1] += 5.0f * sinf( glCameraRotation[ROTATE_PITCH] * M_PI / 180.0f );
        break;
    }
}

/**
*
*/
void KeyRight(void)
{
    if ( glCamera == OBSERVE )
        glCameraRotation[1] += 5.0f;
}

void animateToggle(void)
{
    glAnimate = !glAnimate;
}

void frustrumToggle(void)
{
    glDrawFrustum = !glDrawFrustum;
}

void KeyUp(void)
{
    if ( glCamera == OBSERVE )
        glCameraPos[1] -= 5.f;
}

void KeyDown(void)
{
    if ( glCamera == OBSERVE )
        glCameraPos[1] += 5.f;
}

void toggleMoreDetail(void)
{
    glNumTrisDesired += 500;
    if ( glNumTrisDesired > 20000 )
        glNumTrisDesired = 20000;
}

void toggleLessDetail(void)
{
    glNumTrisDesired -= 500;
    if ( glNumTrisDesired < 500 )
        glNumTrisDesired = 500;
}

void changeSize(GLsizei w, GLsizei h)
{
    GLfloat fAspect;
    if(h == GL_ZERO)
        h = 1;

    glViewport(GL_ZERO, GL_ZERO, w, h);

    fAspect = (GLfloat)w/(GLfloat)h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    {
        double  left;
        double  right;
        double  bottom;
        double  top;

        right = NEAR_CLIP * tan( glFoVX/2.0 * M_PI/180.f );
        top = right / fAspect;
        bottom = -top;
        left = -right;
        glFrustum(left, right, bottom, top, NEAR_CLIP, FAR_CLIP);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void KeyFOVDown(void)
{
    glFoVX -= 1.0f;
    changeSize(WINDOW_WIDTH, WINDOW_HEIGHT);
}

void KeyFOVUp(void)
{
    glFoVX += 1.0f;
    changeSize(WINDOW_WIDTH, WINDOW_HEIGHT);
}

void renderScene(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    switch (glCamera)
    {
    default:
    case FOLLOW:
        glRotatef(-glAnimationAngle,ZERO_F, 1.f,ZERO_F);
        glTranslatef(-glViewPosition[0], -glViewPosition[1], -glViewPosition[2]);

        glPerspective = -glAnimationAngle;
        break;

    case OBSERVE:

        glTranslatef(0.f, glCameraPos[1], glCameraPos[2]);

        glRotatef(glCameraRotation[ROTATE_PITCH], 1.f, .0f, .0f);
        glRotatef(glCameraRotation[ROTATE_YAW],   .0f, 1.f, .0f);

        glTranslatef(-((GLfloat) MAP_SIZE * .5f), .0f, -((GLfloat) MAP_SIZE * .5f));

        glPerspective = -glAnimationAngle;
        break;

    case DRIVE:
    case FLY_MODE:
        glAnimate = GL_ZERO;

        glRotatef(glCameraRotation[ROTATE_PITCH], 1.f, .0f, .0f);
        glRotatef(glCameraRotation[ROTATE_YAW],   .0f, 1.f, .0f);

        glTranslatef(-glViewPosition[0], -glViewPosition[1], -glViewPosition[2]);

        glPerspective = glCameraRotation[ROTATE_YAW];
        break;
    }
    roamDrawFrame();

    if ( glDrawFrustum )
        drawFrustum();

    glPopMatrix();

    glFrames++;

    glPrint(10, 10, "Arrows keys to move around");
    glPrint(10, 30, "A - Animate screen");
    glPrint(10, 50, "C - Camera mode");
    glPrint(10, 70, "W - Render mode");
    glPrint(10, 90, "R - Toggle frustrum");
}

void mouseMove(GLint mouseX, GLint mouseY)
{
    if ( glRotate &&
            (glCamera != FOLLOW))
    {
        GLint dx, dy;

        if ( glStartX == -1 )
        {
            glStartX = mouseX;
            glStartY = mouseY;
        }

        dx = mouseX - glStartX;

        if ( glCamera == OBSERVE )
            dy = mouseY - glStartY;
        else
            dy = glStartY - mouseY;

        glCameraRotation[0] = glCameraRotation[0] + (GLfloat) dy * .5f;
        glCameraRotation[1] += (GLfloat) dx * .5f;

        glStartX = mouseX;
        glStartY = mouseY;
    }
}

void idleFn(void)
{
    if (glAnimate)
    {
        glAnimationAngle = glAnimationAngle + 0.4f;

        glViewPosition[0] = ((GLfloat) MAP_SIZE / 4.f) + ((sinf(glAnimationAngle * M_PI / 180.f) + 1.f) * ((GLfloat) MAP_SIZE / 4.f));
        glViewPosition[2] = ((GLfloat) MAP_SIZE / 4.f) + ((cosf(glAnimationAngle * M_PI / 180.f) + 1.f) * ((GLfloat) MAP_SIZE / 4.f));

        glViewPosition[1] = (MULT_SCALE * glHeightMap[(GLint)glViewPosition[0] + ((GLint)glViewPosition[2] * MAP_SIZE)]) + 4.0f;
    }
}

void SetupRC()
{
    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);

    glClearColor( .40f, .53f, .60f, 1.0f );

    GLfloat  whiteLight[]    = { .45f,  .45f, .45f, 1.f };
    GLfloat  ambientLight[]  = { .25f,  .25f, .25f, 1.f };
    GLfloat  diffuseLight[]  = { .50f,  .50f, .50f, 1.f };
    GLfloat	 lightPos[]      = { .00f, 300.00f, .0f, .0f };

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ambientLight);
    glLightfv(GL_LIGHT0,GL_AMBIENT,ambientLight);
    glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuseLight);
    glLightfv(GL_LIGHT0,GL_POSITION,lightPos);
    glEnable(GL_LIGHT0);

    glEnable(GL_COLOR_MATERIAL);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, whiteLight );

    static GLfloat s_vector[4] = { 1.0/(GLfloat)TEXTURE_SIZE, GL_ZERO, GL_ZERO, 0 };
    static GLfloat t_vector[4] = { GL_ZERO, GL_ZERO, 1.0/(GLfloat)TEXTURE_SIZE, 0 };

    glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
    glTexGenfv( GL_S, GL_OBJECT_PLANE, s_vector );

    glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
    glTexGenfv( GL_T, GL_OBJECT_PLANE, t_vector );
}
