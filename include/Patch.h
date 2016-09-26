
#ifndef MESH_H
#define MESH_H

#define VARIANCE_DEPTH (9)

class Terrain;

/**
* TriTreeNode Struct
*/
struct TriTreeNode
{
  TriTreeNode *leftChild;
  TriTreeNode *rightChild;
  TriTreeNode *baseNeighbor;
  TriTreeNode *leftNeighbor;
  TriTreeNode *rightNeighbor;
};

/**
* Patch Class
*/
class Patch
{
protected:
  unsigned char *heightMap;
  GLint patchWorldX, patchWorldY;

  unsigned char patchDiffLeft[ 1<<(VARIANCE_DEPTH)];
  unsigned char patchDiffRight[1<<(VARIANCE_DEPTH)];

  unsigned char *currentDifference;
  unsigned char dirtyDifference;
  unsigned char isPatchVisible;

  TriTreeNode baseLeft;
  TriTreeNode baseRight;

public:
  TriTreeNode *getBaseLeft()
  {
    return &baseLeft;
  }
  TriTreeNode *getBaseRight()
  {
    return &baseRight;
  }
  char isDirty()
  {
    return dirtyDifference;
  }
  GLint  isVisibile( )
  {
    return isPatchVisible;
  }
  void SetVisibility( GLint eyeX, GLint eyeY, GLint leftX, GLint leftY, GLint rightX, GLint rightY );

  virtual void Init( GLint heightX, GLint heightY, GLint worldX, GLint worldY, unsigned char *hMap );
  virtual void Reset();
  virtual void Tessellate();
  virtual void Render();
  virtual void ComputeVariance();

  virtual void			Split( TriTreeNode *tri);
  virtual void			recursiveTessellate( TriTreeNode *tri, GLint leftX, GLint leftY, GLint rightX, GLint rightY, GLint apexX, GLint apexY, GLint node );
  virtual void			recursiveRender( TriTreeNode *tri, GLint leftX, GLint leftY, GLint rightX, GLint rightY, GLint apexX, GLint apexY );
  virtual unsigned char	recursiveComputeDiff(	GLint leftX,  GLint leftY,  unsigned char leftZ,
          GLint rightX, GLint rightY, unsigned char rightZ,
          GLint apexX,  GLint apexY,  unsigned char apexZ,
          GLint node);
};


#endif
