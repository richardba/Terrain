
#ifndef LANDSCAPE_H
#define LANDSCAPE_H

#include "Patch.h"

// ---------------------------------------------------------------------
// Various Pre-Defined map sizes & their #define counterparts:

#define MAP_1024

#ifdef MAP_2048

// ------- 2048x2048 MAP -------
#define MAP_SIZE (2048)
#define NUM_MESHES_PER_SIDE (32)

#else
#ifdef MAP_1024

// ------- 1024x1024 MAP -------
#define MAP_SIZE (1024)
#define NUM_MESHES_PER_SIDE (16)

#else

// ------- 512x512 MAP -------
#define MAP_SIZE (512)
#define NUM_MESHES_PER_SIDE (8)

#endif
#endif

// ---------------------------------------------------------------------
// Scale of the terrain ie: 1 unit of the height map == how many world units (meters)?
// 1.0f == 1 meter resolution
// .5f == 1/2 meter resolution
// .25f == 1/4 meter resolution
// etc..
#define MULT_SCALE (.5f)

// How many TriTreeNodes should be allocated?
#define POOL_SIZE (25000)

// Some more definitions
#define MESH_SIZE (MAP_SIZE/NUM_MESHES_PER_SIDE)
#define TEXTURE_SIZE (128)

// Drawing Modes
#define DRAW_USE_TEXTURE   (0)
#define DRAW_USE_LIGHTING  (1)
#define DRAW_USE_FILL_ONLY (2)
#define DRAW_USE_WIREFRAME (3)

// Rotation Indexes
#define ROTATE_PITCH (0)
#define ROTATE_YAW   (1)
#define ROTATE_ROLL	 (2)


#define SQR(x) ((x) * (x))
#define MAX(a,b) ((a < b) ? (b) : (a))
#define DEG2RAD(a) (((a) * M_PI) / 18.0f)
#ifndef M_PI
#define M_PI (3.14159265358979323846f)
#endif // defined

// External variables and functions:
extern float glFoVX;
extern float glFrameDiff;
extern GLfloat glCameraRotation[];
extern GLfloat glPerspective;
extern GLfloat glViewPosition[];
extern GLuint glTexture;
extern GLint glNumTrisDesired;
extern GLint glDrawMode;
extern GLint glNumTrisRendered;
extern void calcNormal(float v[3][3], float out[3]);
extern void reduceToUnit(float vector[3]);

//
// Terrain Class
// Holds all the information to render an entire landscape.
//
class Terrain
{
protected:
	unsigned char *heightMap;										// HeightMap of the Terrain
	Patch m_Patches[NUM_MESHES_PER_SIDE][NUM_MESHES_PER_SIDE];	// Array of patches

	static GLint	m_NextTriNode;										// Index to next free TriTreeNode
	static TriTreeNode m_TriPool[POOL_SIZE];						// Pool of TriTree nodes for splitting

	static GLint GetNextTriNode() { return m_NextTriNode; }
	static void SetNextTriNode( GLint nNextNode ) { m_NextTriNode = nNextNode; }

public:
	static TriTreeNode *AllocateTri();

	virtual void Init(unsigned char *hMap);
	virtual void Reset();
	virtual void Tessellate();
	virtual void Render();

};


#endif
