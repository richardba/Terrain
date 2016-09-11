
#ifndef MESH_H
#define MESH_H

// Depth of variance tree: should be near SQRT(MESH_SIZE) + 1
#define VARIANCE_DEPTH (9)

// Predefines...
class Terrain;

//
// TriTreeNode Struct
// Store the triangle tree data, but no coordinates!
//
struct TriTreeNode
{
	TriTreeNode *leftChild;
	TriTreeNode *rightChild;
	TriTreeNode *baseNeighbor;
	TriTreeNode *leftNeighbor;
	TriTreeNode *rightNeighbor;
};

//
// Patch Class
// Store information needed at the Patch level
//
class Patch
{
  protected:
    unsigned char *heightMap;									// PoGLinter to height map to use
    GLint m_WorldX, m_WorldY;										// World coordinate offset of this patch.

    unsigned char m_VarianceLeft[ 1<<(VARIANCE_DEPTH)];			// Left variance tree
    unsigned char m_VarianceRight[1<<(VARIANCE_DEPTH)];			// Right variance tree

    unsigned char *m_CurrentVariance;							// Which varience we are currently using. [Only valid during the Tessellate and ComputeVariance passes]
    unsigned char m_VarianceDirty;								// Does the Varience Tree need to be recalculated for this Patch?
    unsigned char m_isVisible;									// Is this patch visible in the current frame?

    TriTreeNode baseLeft;										// Left base triangle tree node
    TriTreeNode m_BaseRight;									// Right base triangle tree node

  public:
    // Some encapsulation functions & extras
    TriTreeNode *GetBaseLeft()  { return &baseLeft; }
    TriTreeNode *GetBaseRight() { return &m_BaseRight; }
    char isDirty()     { return m_VarianceDirty; }
    GLint  isVisibile( ) { return m_isVisible; }
    void SetVisibility( GLint eyeX, GLint eyeY, GLint leftX, GLint leftY, GLint rightX, GLint rightY );

    // The static half of the Patch Class
    virtual void Init( GLint heightX, GLint heightY, GLint worldX, GLint worldY, unsigned char *hMap );
    virtual void Reset();
    virtual void Tessellate();
    virtual void Render();
    virtual void ComputeVariance();

    // The recursive half of the Patch Class
    virtual void			Split( TriTreeNode *tri);
    virtual void			RecursTessellate( TriTreeNode *tri, GLint leftX, GLint leftY, GLint rightX, GLint rightY, GLint apexX, GLint apexY, GLint node );
    virtual void			RecursRender( TriTreeNode *tri, GLint leftX, GLint leftY, GLint rightX, GLint rightY, GLint apexX, GLint apexY );
    virtual unsigned char	RecursComputeVariance(	GLint leftX,  GLint leftY,  unsigned char leftZ,
                            GLint rightX, GLint rightY, unsigned char rightZ,
                            GLint apexX,  GLint apexY,  unsigned char apexZ,
                            GLint node);
};


#endif
