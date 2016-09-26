/**
* @author Ricardo Barros Duarte d'Oliveira
* Arquivo contendo as v�riaveis globais e m�todos para n�o serem redefinidos
*/
#define WINDOW_HEIGHT (480)
#define WINDOW_WIDTH  (640)
// Vari�veis do pipeline gr�fico
extern GLuint vertexArrayId;
extern GLuint vertexBuffer;
extern GLuint colorBuffer;
extern GLuint shaderHandle;

// Var�aveis compartilhadas
extern GLint glAnimate;
extern GLint glFrames;
extern GLint glRotate;
extern GLint glStartX;
extern GLint glStartY;
extern long glStartTime, glEndTime;
extern unsigned char *glHeightMap;

// M�todos
extern GLint roamInit(unsigned char *map);
extern void animateToggle(void);
extern void calcNormal(float v[3][3], float out[3]);
extern void cameraMode(void);
extern void changeSize(GLsizei w, GLsizei h);
extern void drawFrustum();
extern void drawMode();
extern void freeTerrain();
extern void frustrumToggle(void);
extern void glPrint(int x, int y, char *text);
extern void idleFn(void);
extern void KeyBackward(void);
extern void KeyDown(void);
extern void KeyForward(void);
extern void KeyFOVDown(void);
extern void KeyFOVUp(void);
extern void KeyLeft(void);
extern void KeyRight(void);
extern void KeyUp(void);
extern void loadTerrain(GLint size, unsigned char **dest);
extern void mouseMove(GLint mouseX, GLint mouseY);
extern void mouseWheel(GLint, GLint, GLint, GLint);
extern void reduceToUnit(float vector[3]);
extern void renderMode(void);
extern void renderScene(void);
extern void roamDrawFrame();
extern void SetupRC();
extern void toggleLessDetail(void);
extern void toggleMoreDetail(void);
