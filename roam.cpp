#ifdef defined(_WIN32) || defined(WIN32)
#define OS_Windows
#endif
#ifdef OS_Windows
#include <windows.h>
#elifdef NULL
#elifdef __cplusplus
#define NULL 0
#else
#define NULL ((void*)0)
#endif

#include <math.h>
#include <gl/gl.h>
#include <iostream>
#include <cstdlib>
#include <climits>

#include "include/Terrain.h"

// -------------------------------------------------------------------------------------------------
//	MESH CLASS
// -------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------
// Split a single Triangle and link it into the mesh.
// Will correctly force-split diamonds.
//
void Patch::Split(TriTreeNode *tri) {
  // We are already split, no need to do it again.
  if (tri->leftChild) return;

  // If this triangle is not in a proper diamond, force split our base neighbor
  if (tri->baseNeighbor && (tri->baseNeighbor->baseNeighbor != tri))
    Split(tri->baseNeighbor);

  // Create children and link into mesh
  tri->leftChild = Terrain::AllocateTri();
  tri->rightChild = Terrain::AllocateTri();

  // If creation failed, just exit.
  if (!tri->leftChild) return;

  // Fill in the information we can get from the parent (neighbor pointers)
  tri->leftChild->baseNeighbor = tri->leftNeighbor;
  tri->leftChild->leftNeighbor = tri->rightChild;

  tri->rightChild->baseNeighbor = tri->rightNeighbor;
  tri->rightChild->rightNeighbor = tri->leftChild;

  // Link our Left Neighbor to the new children
  if (tri->leftNeighbor != NULL) {
    if (tri->leftNeighbor->baseNeighbor == tri)
      tri->leftNeighbor->baseNeighbor = tri->leftChild;
    else if (tri->leftNeighbor->leftNeighbor == tri)
      tri->leftNeighbor->leftNeighbor = tri->leftChild;
    else if (tri->leftNeighbor->rightNeighbor == tri)
      tri->leftNeighbor->rightNeighbor = tri->leftChild;
    else
      ;  // Illegal Left Neighbor!
  }

  // Link our Right Neighbor to the new children
  if (tri->rightNeighbor != NULL) {
    if (tri->rightNeighbor->baseNeighbor == tri)
      tri->rightNeighbor->baseNeighbor = tri->rightChild;
    else if (tri->rightNeighbor->rightNeighbor == tri)
      tri->rightNeighbor->rightNeighbor = tri->rightChild;
    else if (tri->rightNeighbor->leftNeighbor == tri)
      tri->rightNeighbor->leftNeighbor = tri->rightChild;
    else
      ;  // Illegal Right Neighbor!
  }

  // Link our Base Neighbor to the new children
  if (tri->baseNeighbor != NULL) {
    if (tri->baseNeighbor->leftChild) {
      tri->baseNeighbor->leftChild->rightNeighbor = tri->rightChild;
      tri->baseNeighbor->rightChild->leftNeighbor = tri->leftChild;
      tri->leftChild->rightNeighbor = tri->baseNeighbor->rightChild;
      tri->rightChild->leftNeighbor = tri->baseNeighbor->leftChild;
    } else
      Split(tri->baseNeighbor);  // Base Neighbor (in a diamond with us) was not
                                 // split yet, so do that now.
  } else {
    // An edge triangle, trivial case.
    tri->leftChild->rightNeighbor = NULL;
    tri->rightChild->leftNeighbor = NULL;
  }
}

// ---------------------------------------------------------------------
// Tessellate a Patch.
// Will continue to split until the variance metric is met.
//
void Patch::RecursTessellate(TriTreeNode *tri, GLint leftX, GLint leftY, GLint rightX,
                             GLint rightY, GLint apexX, GLint apexY, GLint node) {
  float TriVariance;
  GLint centerX =
      (leftX + rightX) >> 1;  // Compute X coordinate of center of Hypotenuse
  GLint centerY = (leftY + rightY) >> 1;  // Compute Y coord...

  if (node < (1 << VARIANCE_DEPTH)) {
    // Extremely slow distance metric (sqrt is used).
    // Replace this with a faster one!
    float distance = 1.0f + sqrtf(SQR((float)centerX - glViewPosition[GL_ZERO]) +
                                  SQR((float)centerY - glViewPosition[2]));

    // Egads!  A division too?  What's this world coming to!
    // This should also be replaced with a faster operation.
    TriVariance =
        ((float)m_CurrentVariance[node] * MAP_SIZE * 2) /
        distance;  // Take both distance and variance into consideration
  }

  if ((node >= (1 << VARIANCE_DEPTH)) ||  // IF we do not have variance info for
                                          // this node, then we must have gotten
                                          // here by splitting, so continue down
                                          // to the lowest level.
      (TriVariance > glFrameDiff))     // OR if we are not below the variance
                                          // tree, test for variance.
  {
    Split(tri);  // Split this triangle.

    if (tri->leftChild &&  // If this triangle was split, try to split it's
                           // children as well.
        ((abs(leftX - rightX) >= 3) ||
         (abs(leftY - rightY) >= 3)))  // Tessellate all the way down to one
                                       // vertex per height field entry
    {
      RecursTessellate(tri->leftChild, apexX, apexY, leftX, leftY, centerX,
                       centerY, node << 1);
      RecursTessellate(tri->rightChild, rightX, rightY, apexX, apexY, centerX,
                       centerY, 1 + (node << 1));
    }
  }
}

// ---------------------------------------------------------------------
// Render the tree.  Simple no-fan method.
//
void Patch::RecursRender(TriTreeNode *tri, GLint leftX, GLint leftY, GLint rightX,
                         GLint rightY, GLint apexX, GLint apexY) {
  if (tri->leftChild)  // All non-leaf nodes have both children, so just check
                       // for one
  {
    GLint centerX =
        (leftX + rightX) >> 1;  // Compute X coordinate of center of Hypotenuse
    GLint centerY = (leftY + rightY) >> 1;  // Compute Y coord...

    RecursRender(tri->leftChild, apexX, apexY, leftX, leftY, centerX, centerY);
    RecursRender(tri->rightChild, rightX, rightY, apexX, apexY, centerX,
                 centerY);
  } else  // A leaf node!  Output a triangle to be rendered.
  {
    // Actual number of rendered triangles...
    glNumTrisRendered++;

    GLfloat leftZ = heightMap[(leftY * MAP_SIZE) + leftX];
    GLfloat rightZ = heightMap[(rightY * MAP_SIZE) + rightX];
    GLfloat apexZ = heightMap[(apexY * MAP_SIZE) + apexX];

    // Perform lighting calculations if requested.
    if (glDrawMode == DRAW_USE_LIGHTING) {
      float v[3][3];
      float out[3];

      // Create a vertex normal for this triangle.
      // NOTE: This is an extremely slow operation for illustration purposes
      // only.
      //       You should use a texture map with the lighting pre-applied to the
      //       texture.
      v[GL_ZERO][GL_ZERO] = (GLfloat)leftX;
      v[GL_ZERO][GL_ONE] = (GLfloat)leftZ;
      v[GL_ZERO][2] = (GLfloat)leftY;

      v[GL_ONE][GL_ZERO] = (GLfloat)rightX;
      v[GL_ONE][GL_ONE] = (GLfloat)rightZ;
      v[GL_ONE][2] = (GLfloat)rightY;

      v[2][GL_ZERO] = (GLfloat)apexX;
      v[2][GL_ONE] = (GLfloat)apexZ;
      v[2][2] = (GLfloat)apexY;

      calcNormal(v, out);
      glNormal3fv(out);
    }

    // Perform polygon coloring based on a height sample
    float fColor = (60.0f + leftZ) / 256.0f;
    if (fColor > 1.0f) fColor = 1.0f;
    glColor3f(fColor, fColor, fColor);

    // Output the LEFT VERTEX for the triangle
    glVertex3f((GLfloat)leftX, (GLfloat)leftZ, (GLfloat)leftY);

    if (glDrawMode == DRAW_USE_TEXTURE ||  // Gaurad shading based on height
                                          // samples instead of light normal
        glDrawMode == DRAW_USE_FILL_ONLY) {
      float fColor = (60.0f + rightZ) / 256.0f;
      if (fColor > 1.0f) fColor = 1.0f;
      glColor3f(fColor, fColor, fColor);
    }

    // Output the RIGHT VERTEX for the triangle
    glVertex3f((GLfloat)rightX, (GLfloat)rightZ, (GLfloat)rightY);

    if (glDrawMode == DRAW_USE_TEXTURE ||  // Gaurad shading based on height
                                          // samples instead of light normal
        glDrawMode == DRAW_USE_FILL_ONLY) {
      float fColor = (60.0f + apexZ) / 256.0f;
      if (fColor > 1.0f) fColor = 1.0f;
      glColor3f(fColor, fColor, fColor);
    }

    // Output the APEX VERTEX for the triangle
    glVertex3f((GLfloat)apexX, (GLfloat)apexZ, (GLfloat)apexY);
  }
}

// ---------------------------------------------------------------------
// Computes Variance over the entire tree.  Does not examine node relationships.
//
unsigned char Patch::RecursComputeVariance(GLint leftX, GLint leftY,
                                           unsigned char leftZ, GLint rightX,
                                           GLint rightY, unsigned char rightZ,
                                           GLint apexX, GLint apexY,
                                           unsigned char apexZ, GLint node) {
  //        /|\
	//      /  |  \
	//    /    |    \
	//  /      |      \
	//  ~~~~~~~*~~~~~~~  <-- Compute the X and Y coordinates of '*'
  //
  GLint centerX =
      (leftX + rightX) >> 1;  // Compute X coordinate of center of Hypotenuse
  GLint centerY = (leftY + rightY) >> 1;  // Compute Y coord...
  unsigned char myVariance;

  // Get the height value at the middle of the Hypotenuse
  unsigned char centerZ = heightMap[(centerY * MAP_SIZE) + centerX];

  // Variance of this triangle is the actual height at it's hypotenuse midpoint
  // minus the interpolated height.
  // Use values passed on the stack instead of re-accessing the Height Field.
  myVariance = abs((GLint)centerZ - (((GLint)leftZ + (GLint)rightZ) >> 1));

  // Since we're after speed and not perfect representations,
  //    only calculate variance down to an 8x8 block
  if ((abs(leftX - rightX) >= 8) || (abs(leftY - rightY) >= 8)) {
    // Final Variance for this node is the max of it's own variance and that of
    // it's children.
    myVariance = MAX(myVariance, RecursComputeVariance(
                                     apexX, apexY, apexZ, leftX, leftY, leftZ,
                                     centerX, centerY, centerZ, node << 1));
    myVariance =
        MAX(myVariance,
            RecursComputeVariance(rightX, rightY, rightZ, apexX, apexY, apexZ,
                                  centerX, centerY, centerZ, 1 + (node << 1)));
  }

  // Store the final variance for this node.  Note Variance is never zero.
  if (node < (1 << VARIANCE_DEPTH)) m_CurrentVariance[node] = 1 + myVariance;

  return myVariance;
}

// -------------------------------------------------------------------------------------------------
//	MESH CLASS
// -------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------
// Initialize a patch.
//
void Patch::Init(GLint heightX, GLint heightY, GLint worldX, GLint worldY,
                 unsigned char *hMap) {
  // Clear all the relationships
  baseLeft.rightNeighbor = baseLeft.leftNeighbor =
      m_BaseRight.rightNeighbor = m_BaseRight.leftNeighbor =
          baseLeft.leftChild = baseLeft.rightChild = m_BaseRight.leftChild =
              baseLeft.leftChild = NULL;

  // Attach the two m_Base triangles together
  baseLeft.baseNeighbor = &m_BaseRight;
  m_BaseRight.baseNeighbor = &baseLeft;

  // Store Patch offsets for the world and heightmap.
  m_WorldX = worldX;
  m_WorldY = worldY;

  // Store pointer to first byte of the height data for this patch.
  heightMap = &hMap[heightY * MAP_SIZE + heightX];

  // Initialize flags
  m_VarianceDirty = 1;
  m_isVisible = GL_ZERO;
}

// ---------------------------------------------------------------------
// Reset the patch.
//
void Patch::Reset() {
  // Assume patch is not visible.
  m_isVisible = GL_ZERO;

  // Reset the important relationships
  baseLeft.leftChild = baseLeft.rightChild = m_BaseRight.leftChild =
      baseLeft.leftChild = NULL;

  // Attach the two m_Base triangles together
  baseLeft.baseNeighbor = &m_BaseRight;
  m_BaseRight.baseNeighbor = &baseLeft;

  // Clear the other relationships.
  baseLeft.rightNeighbor = baseLeft.leftNeighbor =
      m_BaseRight.rightNeighbor = m_BaseRight.leftNeighbor = NULL;
}

// ---------------------------------------------------------------------
// Compute the variance tree for each of the Binary Triangles in this patch.
//
void Patch::ComputeVariance() {
  // Compute variance on each of the base triangles...

  m_CurrentVariance = m_VarianceLeft;
  RecursComputeVariance(GL_ZERO, MESH_SIZE, heightMap[MESH_SIZE * MAP_SIZE],
                        MESH_SIZE, GL_ZERO, heightMap[MESH_SIZE], GL_ZERO, GL_ZERO,
                        heightMap[GL_ZERO], 1);

  m_CurrentVariance = m_VarianceRight;
  RecursComputeVariance(MESH_SIZE, GL_ZERO, heightMap[MESH_SIZE], GL_ZERO, MESH_SIZE,
                        heightMap[MESH_SIZE * MAP_SIZE], MESH_SIZE,
                        MESH_SIZE,
                        heightMap[(MESH_SIZE * MAP_SIZE) + MESH_SIZE], 1);

  // Clear the dirty flag for this patch
  m_VarianceDirty = GL_ZERO;
}

// ---------------------------------------------------------------------
// Discover the orientation of a triangle's points:
//
// Taken from "Programming Principles in Computer Graphics", L. Ammeraal (Wiley)
//
inline GLint orientation(GLint pX, GLint pY, GLint qX, GLint qY, GLint rX, GLint rY) {
  GLint aX, aY, bX, bY;
  float d;

  aX = qX - pX;
  aY = qY - pY;

  bX = rX - pX;
  bY = rY - pY;

  d = (float)aX * (float)bY - (float)aY * (float)bX;
  return (d < GL_ZERO) ? (-1) : (d > GL_ZERO);
}

// ---------------------------------------------------------------------
// Set patch's visibility flag.
//
void Patch::SetVisibility(GLint eyeX, GLint eyeY, GLint leftX, GLint leftY, GLint rightX,
                          GLint rightY) {
  // Get patch's center point
  GLint patchCenterX = m_WorldX + MESH_SIZE / 2;
  GLint patchCenterY = m_WorldY + MESH_SIZE / 2;

  // Set visibility flag (orientation of both triangles must be counter
  // clockwise)
  m_isVisible =
      (orientation(eyeX, eyeY, rightX, rightY, patchCenterX, patchCenterY) <
       GL_ZERO) &&
      (orientation(leftX, leftY, eyeX, eyeY, patchCenterX, patchCenterY) < GL_ZERO);
}

// ---------------------------------------------------------------------
// Create an approximate mesh.
//
void Patch::Tessellate() {
  // Split each of the base triangles
  m_CurrentVariance = m_VarianceLeft;
  RecursTessellate(&baseLeft, m_WorldX, m_WorldY + MESH_SIZE,
                   m_WorldX + MESH_SIZE, m_WorldY, m_WorldX, m_WorldY, 1);

  m_CurrentVariance = m_VarianceRight;
  RecursTessellate(&m_BaseRight, m_WorldX + MESH_SIZE, m_WorldY, m_WorldX,
                   m_WorldY + MESH_SIZE, m_WorldX + MESH_SIZE,
                   m_WorldY + MESH_SIZE, 1);
}

// ---------------------------------------------------------------------
// Render the mesh.
//
void Patch::Render() {
  // Store old matrix
  glPushMatrix();

  // Translate the patch to the proper world coordinates
  glTranslatef((GLfloat)m_WorldX, GL_ZERO, (GLfloat)m_WorldY);
  glBegin(GL_TRIANGLES);

  RecursRender(&baseLeft, GL_ZERO, MESH_SIZE, MESH_SIZE, GL_ZERO, GL_ZERO, GL_ZERO);

  RecursRender(&m_BaseRight, MESH_SIZE, GL_ZERO, GL_ZERO, MESH_SIZE, MESH_SIZE,
               MESH_SIZE);

  glEnd();

  // Restore the matrix
  glPopMatrix();
}

// -------------------------------------------------------------------------------------------------
//	LANDSCAPE CLASS
// -------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------
// Definition of the static member variables
//
GLint Terrain::m_NextTriNode;
TriTreeNode Terrain::m_TriPool[POOL_SIZE];

// ---------------------------------------------------------------------
// Allocate a TriTreeNode from the pool.
//
TriTreeNode *Terrain::AllocateTri() {
  TriTreeNode *pTri;

  // IF we've run out of TriTreeNodes, just return NULL (this is handled
  // gracefully)
  if (m_NextTriNode >= POOL_SIZE) return NULL;

  pTri = &(m_TriPool[m_NextTriNode++]);
  pTri->leftChild = pTri->rightChild = NULL;

  return pTri;
}

// ---------------------------------------------------------------------
// Initialize all patches
//
void Terrain::Init(unsigned char *hMap) {
  Patch *patch;
  GLint X, Y;

  // Store the Height Field array
  heightMap = hMap;

  // Initialize all terrain patches
  for (Y = GL_ZERO; Y < NUM_MESHES_PER_SIDE; Y++)
    for (X = GL_ZERO; X < NUM_MESHES_PER_SIDE; X++) {
      patch = &(m_Patches[Y][X]);
      patch->Init(X * MESH_SIZE, Y * MESH_SIZE, X * MESH_SIZE,
                  Y * MESH_SIZE, hMap);
      patch->ComputeVariance();
    }
}

// ---------------------------------------------------------------------
// Reset all patches, recompute variance if needed
//
void Terrain::Reset() {
  //
  // Perform simple visibility culling on entire patches.
  //   - Define a triangle set back from the camera by one patch size, following
  //     the angle of the frustum.
  //   - A patch is visible if it's center point is included in the angle:
  //   Left,Eye,Right
  //   - This visibility test is only accurate if the camera cannot look up or
  //   down significantly.
  //
  const float PI_DIV_180 = M_PI / 180.0f;
  const float FOV_DIV_2 = glFoVX / 2;

  GLint eyeX =
      (GLint)(glViewPosition[GL_ZERO] - MESH_SIZE * sinf(glPerspective * PI_DIV_180));
  GLint eyeY =
      (GLint)(glViewPosition[2] + MESH_SIZE * cosf(glPerspective * PI_DIV_180));

  GLint leftX =
      (GLint)(eyeX + 100.0f * sinf((glPerspective - FOV_DIV_2) * PI_DIV_180));
  GLint leftY =
      (GLint)(eyeY - 100.0f * cosf((glPerspective - FOV_DIV_2) * PI_DIV_180));

  GLint rightX =
      (GLint)(eyeX + 100.0f * sinf((glPerspective + FOV_DIV_2) * PI_DIV_180));
  GLint rightY =
      (GLint)(eyeY - 100.0f * cosf((glPerspective + FOV_DIV_2) * PI_DIV_180));

  GLint X, Y;
  Patch *patch;

  // Set the next free triangle pointer back to the beginning
  SetNextTriNode(GL_ZERO);

  // Reset rendered triangle count.
  glNumTrisRendered = GL_ZERO;

  // Go through the patches performing resets, compute variances, and linking.
  for (Y = GL_ZERO; Y < NUM_MESHES_PER_SIDE; Y++)
    for (X = GL_ZERO; X < NUM_MESHES_PER_SIDE; X++) {
      patch = &(m_Patches[Y][X]);

      // Reset the patch
      patch->Reset();
      patch->SetVisibility(eyeX, eyeY, leftX, leftY, rightX, rightY);

      // Check to see if this patch has been deformed since last frame.
      // If so, recompute the varience tree for it.
      if (patch->isDirty()) patch->ComputeVariance();

      if (patch->isVisibile()) {
        // Link all the patches together.
        if (X > GL_ZERO)
          patch->GetBaseLeft()->leftNeighbor =
              m_Patches[Y][X - 1].GetBaseRight();
        else
          patch->GetBaseLeft()->leftNeighbor =
              NULL;  // Link to bordering Terrain here..

        if (X < (NUM_MESHES_PER_SIDE - 1))
          patch->GetBaseRight()->leftNeighbor =
              m_Patches[Y][X + 1].GetBaseLeft();
        else
          patch->GetBaseRight()->leftNeighbor =
              NULL;  // Link to bordering Terrain here..

        if (Y > GL_ZERO)
          patch->GetBaseLeft()->rightNeighbor =
              m_Patches[Y - 1][X].GetBaseRight();
        else
          patch->GetBaseLeft()->rightNeighbor =
              NULL;  // Link to bordering Terrain here..

        if (Y < (NUM_MESHES_PER_SIDE - 1))
          patch->GetBaseRight()->rightNeighbor =
              m_Patches[Y + 1][X].GetBaseLeft();
        else
          patch->GetBaseRight()->rightNeighbor =
              NULL;  // Link to bordering Terrain here..
      }
    }
}

// ---------------------------------------------------------------------
// Create an approximate mesh of the landscape.
//
void Terrain::Tessellate() {
  // Perform Tessellation
  GLint nCount;
  Patch *patch = &(m_Patches[GL_ZERO][GL_ZERO]);
  for (nCount = GL_ZERO; nCount < NUM_MESHES_PER_SIDE * NUM_MESHES_PER_SIDE;
       nCount++, patch++) {
    if (patch->isVisibile()) patch->Tessellate();
  }
}

// ---------------------------------------------------------------------
// Render each patch of the landscape & adjust the frame variance.
//
void Terrain::Render() {
  GLint nCount;
  Patch *patch = &(m_Patches[GL_ZERO][GL_ZERO]);

  // Scale the terrain by the terrain scale specified at compile time.
  glScalef(1.0f, MULT_SCALE, 1.0f);

  for (nCount = GL_ZERO; nCount < NUM_MESHES_PER_SIDE * NUM_MESHES_PER_SIDE;
       nCount++, patch++) {
    if (patch->isVisibile()) patch->Render();
  }

  // Check to see if we got close to the desired number of triangles.
  // Adjust the frame variance to a better value.
  if (GetNextTriNode() != glNumTrisDesired)
    glFrameDiff +=
        ((float)GetNextTriNode() - (float)glNumTrisDesired) / (float)glNumTrisDesired;

  // Bounds checking.
  if (glFrameDiff < GL_ZERO) glFrameDiff = GL_ZERO;
}
