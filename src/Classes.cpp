/**
* @author Ricardo Barros Duarte d'Oliveira
* Arquivo que implementa os metodos das classes Patch e Terrain
*/

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
#include <stdio.h>
#include <cstdlib>
#include <climits>
#include "../include/Terrain.h"
#include "../include/Vbo.h"
#include "../include/utils.h"
glm::vec3 maximal = glm::vec3(0);

inline GLint orientation(GLint pX, GLint pY, GLint qX, GLint qY, GLint rX, GLint rY)
{
    GLint aX, aY, bX, bY;
    float d;

    aX = qX - pX;
    aY = qY - pY;

    bX = rX - pX;
    bY = rY - pY;

    d = (float)aX * (float)bY - (float)aY * (float)bX;
    return (d < GL_ZERO) ? (-1) : (d > GL_ZERO);
}

void Patch::Split(TriTreeNode *tri)
{
    if (tri->leftChild) return;

    if (tri->baseNeighbor && (tri->baseNeighbor->baseNeighbor != tri))
        Split(tri->baseNeighbor);

    tri->leftChild = Terrain::AllocateTri();
    tri->rightChild = Terrain::AllocateTri();

    if (!tri->leftChild) return;

    tri->leftChild->baseNeighbor = tri->leftNeighbor;
    tri->leftChild->leftNeighbor = tri->rightChild;

    tri->rightChild->baseNeighbor = tri->rightNeighbor;
    tri->rightChild->rightNeighbor = tri->leftChild;

    if (tri->leftNeighbor != NULL)
    {
        if (tri->leftNeighbor->baseNeighbor == tri)
            tri->leftNeighbor->baseNeighbor = tri->leftChild;
        else if (tri->leftNeighbor->leftNeighbor == tri)
            tri->leftNeighbor->leftNeighbor = tri->leftChild;
        else if (tri->leftNeighbor->rightNeighbor == tri)
            tri->leftNeighbor->rightNeighbor = tri->leftChild;
        else
            ;
    }

    if (tri->rightNeighbor != NULL)
    {
        if (tri->rightNeighbor->baseNeighbor == tri)
            tri->rightNeighbor->baseNeighbor = tri->rightChild;
        else if (tri->rightNeighbor->rightNeighbor == tri)
            tri->rightNeighbor->rightNeighbor = tri->rightChild;
        else if (tri->rightNeighbor->leftNeighbor == tri)
            tri->rightNeighbor->leftNeighbor = tri->rightChild;
        else
            ;
    }

    if (tri->baseNeighbor != NULL)
    {
        if (tri->baseNeighbor->leftChild)
        {
            tri->baseNeighbor->leftChild->rightNeighbor = tri->rightChild;
            tri->baseNeighbor->rightChild->leftNeighbor = tri->leftChild;
            tri->leftChild->rightNeighbor = tri->baseNeighbor->rightChild;
            tri->rightChild->leftNeighbor = tri->baseNeighbor->leftChild;
        }
        else
            Split(tri->baseNeighbor);
    }
    else
    {
        tri->leftChild->rightNeighbor = NULL;
        tri->rightChild->leftNeighbor = NULL;
    }
}

void Patch::recursiveTessellate(TriTreeNode *tri, GLint leftX, GLint leftY, GLint rightX,
                             GLint rightY, GLint apexX, GLint apexY, GLint node)
{
    float TriVariance;
    GLint centerX =
        (leftX + rightX) >> 1;
    GLint centerY = (leftY + rightY) >> 1;

    if (node < (1 << VARIANCE_DEPTH))
    {
        float distance = 1.f + sqrtf(SQR((float)centerX - glViewPosition[0]) +
                                      SQR((float)centerY - glViewPosition[2]));

        TriVariance =
            ((float)currentDifference[node] * MAP_SIZE * 2) /
            distance;
    }

    if ((node >= (1 << VARIANCE_DEPTH)) ||
            (TriVariance > glFrameDiff))
    {
        Split(tri);

        if (tri->leftChild &&
                ((abs(leftX - rightX) >= 3) ||
                 (abs(leftY - rightY) >= 3)))
        {
            recursiveTessellate(tri->leftChild, apexX, apexY, leftX, leftY, centerX,
                             centerY, node << 1);
            recursiveTessellate(tri->rightChild, rightX, rightY, apexX, apexY, centerX,
                             centerY, 1 + (node << 1));
        }
    }
}

/**
* Mudar o metódo para fazer uso de shader
*/
void Patch::recursiveRender(TriTreeNode *tri, GLint leftX, GLint leftY, GLint rightX,
                         GLint rightY, GLint apexX, GLint apexY)
{
  if (tri->leftChild)
  {
      GLint centerX =
          (leftX + rightX) >> 1;
      GLint centerY = (leftY + rightY) >> 1;

      recursiveRender(tri->leftChild, apexX, apexY, leftX, leftY, centerX, centerY);
      recursiveRender(tri->rightChild, rightX, rightY, apexX, apexY, centerX,
                   centerY);
  }
  else
  {
//      Vertex v1, v2, v3;
      glm::vec3 vertex1, vertex2, vertex3, normal, tangent, bitangent;
      glNumTrisRendered++;

      GLfloat leftZ = heightMap[(leftY * MAP_SIZE) + leftX];
      GLfloat rightZ = heightMap[(rightY * MAP_SIZE) + rightX];
      GLfloat apexZ = heightMap[(apexY * MAP_SIZE) + apexX];

      if (glDrawMode == DRAW_USE_LIGHTING)
      {
          float v[3][3];
          float out[3];

          v[0][0] = (GLfloat)leftX;
          v[0][1] = (GLfloat)leftZ;
          v[0][2] = (GLfloat)leftY;

          v[1][0] = (GLfloat)rightX;
          v[1][1] = (GLfloat)rightZ;
          v[1][2] = (GLfloat)rightY;

          v[2][0] = (GLfloat)apexX;
          v[2][1] = (GLfloat)apexZ;
          v[2][2] = (GLfloat)apexY;

          calcNormal(v, out);
          glNormal3fv(out);
          normal=glm::vec3(out[0], out[1], out[2]);
          glNormalArray->push_back(normal);
      }

      float fColor = (60.0f + leftZ) / 256.0f;
      if (fColor > 1.0f) fColor = 1.0f;
      glColor3f(fColor, fColor, fColor);

      glVertex3f((GLfloat)leftX, (GLfloat)leftZ, (GLfloat)leftY);
      vertex1 = glm::vec3((GLfloat)leftX, (GLfloat)leftZ, (GLfloat)leftY);
      glVertexArray->push_back(vertex1);

      if (glDrawMode == DRAW_USE_TEXTURE || glDrawMode == DRAW_USE_FILL_ONLY)
      {
        float fColor = (60.0f + rightZ) / 256.0f;
        if (fColor > 1.0f) fColor = 1.0f;
        glColor3f(fColor, fColor, fColor);
      }

      glVertex3f((GLfloat)rightX, (GLfloat)rightZ, (GLfloat)rightY);
      vertex2 = glm::vec3((GLfloat)rightX, (GLfloat)rightZ, (GLfloat)rightY);
      glVertexArray->push_back(vertex2);

      if (glDrawMode == DRAW_USE_TEXTURE || glDrawMode == DRAW_USE_FILL_ONLY)
      {
        float fColor = (60.0f + apexZ) / 256.0f;
        if (fColor > 1.0f) fColor = 1.0f;
        glColor3f(fColor, fColor, fColor);
      }

      glVertex3f((GLfloat)apexX, (GLfloat)apexZ, (GLfloat)apexY);
      vertex3 = glm::vec3((GLfloat)apexX, (GLfloat)apexZ, (GLfloat)apexY);
      glVertexArray->push_back(vertex3);

      // Shortcuts for UVs
      glm::vec2 uv0 = iterateUv();
      glm::vec2 uv1 = iterateUv();
      glm::vec2 uv2 = iterateUv();

      // Edges of the triangle : postion delta
      glm::vec3 deltaPos1 = vertex2-vertex1;
      glm::vec3 deltaPos2 = vertex3-vertex1;

      glm::vec2 deltaUV1 = uv1-uv0;
      glm::vec2 deltaUV2 = uv2-uv0;

      float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
      tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r;
      bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;
      tangent = glm::normalize(tangent - normal * glm::dot(normal, tangent));
      if (glm::dot(glm::cross(normal, tangent), bitangent) < 0.0f){
        tangent = tangent * -1.0f;
      }
      // Set the same tangent for all three vertices of the triangle.
      // They will be merged later, in vboindexer.cpp
      glTangentArray->push_back(tangent);

      // Same thing for binormals
      glBitangentArray->push_back(bitangent);
      if(DEBUG_MODE)
      {
        if(vertex1.y>maximal.y)
          maximal.y = vertex1.y;
        else if(vertex2.y>maximal.y)
          maximal.y = vertex2.y;
        else if(vertex2.y>maximal.y)
          maximal.y = vertex2.y;

        if(vertex1.x>maximal.x)
        maximal.x = vertex1.x;
        else if(vertex2.x>maximal.x)
          maximal.x = vertex2.x;
        else if(vertex2.x>maximal.x)
          maximal.x = vertex2.x;
      }

  }
}

unsigned char Patch::recursiveComputeDiff(GLint leftX, GLint leftY,
        unsigned char leftZ, GLint rightX,
        GLint rightY, unsigned char rightZ,
        GLint apexX, GLint apexY,
        unsigned char apexZ, GLint node)
{
    GLint centerX =
        (leftX + rightX) >> 1;
    GLint centerY = (leftY + rightY) >> 1;
    unsigned char myVariance;

    unsigned char centerZ = heightMap[(centerY * MAP_SIZE) + centerX];

    myVariance = abs((GLint)centerZ - (((GLint)leftZ + (GLint)rightZ) >> 1));

    if ((abs(leftX - rightX) >= 8) || (abs(leftY - rightY) >= 8))
    {
        myVariance = MAX(myVariance, recursiveComputeDiff(
                             apexX, apexY, apexZ, leftX, leftY, leftZ,
                             centerX, centerY, centerZ, node << 1));
        myVariance =
            MAX(myVariance,
                recursiveComputeDiff(rightX, rightY, rightZ, apexX, apexY, apexZ,
                                      centerX, centerY, centerZ, 1 + (node << 1)));
    }

    if (node < (1 << VARIANCE_DEPTH)) currentDifference[node] = 1 + myVariance;

    return myVariance;
}


void Patch::Init(GLint heightX, GLint heightY, GLint worldX, GLint worldY,
                 unsigned char *hMap)
{
    baseLeft.rightNeighbor = baseLeft.leftNeighbor =
                                 baseRight.rightNeighbor = baseRight.leftNeighbor =
                                         baseLeft.leftChild = baseLeft.rightChild = baseRight.leftChild =
                                                 baseLeft.leftChild = NULL;

    baseLeft.baseNeighbor = &baseRight;
    baseRight.baseNeighbor = &baseLeft;

    patchWorldX = worldX;
    patchWorldY = worldY;

    heightMap = &hMap[heightY * MAP_SIZE + heightX];

    dirtyDifference = 1;
    isPatchVisible = GL_ZERO;
}

void Patch::Reset()
{
    isPatchVisible = GL_ZERO;

    baseLeft.leftChild = baseLeft.rightChild = baseRight.leftChild =
                             baseLeft.leftChild = NULL;

    baseLeft.baseNeighbor = &baseRight;
    baseRight.baseNeighbor = &baseLeft;

    baseLeft.rightNeighbor = baseLeft.leftNeighbor =
                                 baseRight.rightNeighbor = baseRight.leftNeighbor = NULL;
}

void Patch::ComputeVariance()
{

    currentDifference = patchDiffLeft;
    recursiveComputeDiff(GL_ZERO, MESH_SIZE, heightMap[MESH_SIZE * MAP_SIZE],
                          MESH_SIZE, GL_ZERO, heightMap[MESH_SIZE], GL_ZERO, GL_ZERO,
                          heightMap[0], 1);

    currentDifference = patchDiffRight;
    recursiveComputeDiff(MESH_SIZE, GL_ZERO, heightMap[MESH_SIZE], GL_ZERO, MESH_SIZE,
                          heightMap[MESH_SIZE * MAP_SIZE], MESH_SIZE,
                          MESH_SIZE,
                          heightMap[(MESH_SIZE * MAP_SIZE) + MESH_SIZE], 1);

    dirtyDifference = GL_ZERO;
}

void Patch::SetVisibility(GLint eyeX, GLint eyeY, GLint leftX, GLint leftY, GLint rightX,
                          GLint rightY)
{
    GLint patchCenterX = patchWorldX + MESH_SIZE / 2;
    GLint patchCenterY = patchWorldY + MESH_SIZE / 2;

    isPatchVisible = 1;
//        (orientation(eyeX, eyeY, rightX, rightY, patchCenterX, patchCenterY) <
//         GL_ZERO) &&
//        (orientation(leftX, leftY, eyeX, eyeY, patchCenterX, patchCenterY) < GL_ZERO);
}

void Patch::Tessellate()
{
    currentDifference = patchDiffLeft;
    recursiveTessellate(&baseLeft, patchWorldX, patchWorldY + MESH_SIZE,
                     patchWorldX + MESH_SIZE, patchWorldY, patchWorldX, patchWorldY, 1);

    currentDifference = patchDiffRight;
    recursiveTessellate(&baseRight, patchWorldX + MESH_SIZE, patchWorldY, patchWorldX,
                     patchWorldY + MESH_SIZE, patchWorldX + MESH_SIZE,
                     patchWorldY + MESH_SIZE, 1);
}

// Mudar método responsavel
void Patch::Render()
{
    glPushMatrix();

    glTranslatef((GLfloat)patchWorldX, GL_ZERO, (GLfloat)patchWorldY);
    glBegin(GL_TRIANGLES);
    glVertexArray->clear();
    glNormalArray->clear();
    glTangentArray->clear();
    glBitangentArray->clear();
    recursiveRender(&baseLeft, GL_ZERO, MESH_SIZE, MESH_SIZE, GL_ZERO, GL_ZERO, GL_ZERO);

    recursiveRender(&baseRight, MESH_SIZE, GL_ZERO, GL_ZERO, MESH_SIZE, MESH_SIZE,
                 MESH_SIZE);
    glEnd();

    glPopMatrix();
}

GLint Terrain::nextTriNode;
TriTreeNode Terrain::triPoolArr[POOL_SIZE];

TriTreeNode *Terrain::AllocateTri()
{
    TriTreeNode *pTri;

    if (nextTriNode >= POOL_SIZE) return NULL;

    pTri = &(triPoolArr[nextTriNode++]);
    pTri->leftChild = pTri->rightChild = NULL;

    return pTri;
}

void Terrain::Init(unsigned char *hMap)
{
    Patch *patch;
    GLint X, Y;

    heightMap = hMap;

    for (Y = GL_ZERO; Y < NUM_MESHES_PER_SIDE; Y++)
        for (X = GL_ZERO; X < NUM_MESHES_PER_SIDE; X++)
        {
            patch = &(numPatches[Y][X]);
            patch->Init(X * MESH_SIZE, Y * MESH_SIZE, X * MESH_SIZE,
                        Y * MESH_SIZE, hMap);
            patch->ComputeVariance();
        }
}

void Terrain::Reset()
{
    const float PI_DIV_180 = M_PI / 180.0f;
    const float FOV_DIV_2 = glFoVX / 2;

    GLint eyeX =
        (GLint)(glViewPosition[0] - MESH_SIZE * sinf(glPerspective * PI_DIV_180));
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

    setNextTriNode(GL_ZERO);

    glNumTrisRendered = GL_ZERO;

    for (Y = GL_ZERO; Y < NUM_MESHES_PER_SIDE; Y++)
        for (X = GL_ZERO; X < NUM_MESHES_PER_SIDE; X++)
        {
            patch = &(numPatches[Y][X]);

            patch->Reset();
            patch->SetVisibility(eyeX, eyeY, leftX, leftY, rightX, rightY);

            if (patch->isDirty()) patch->ComputeVariance();

            if (patch->isVisibile())
            {
                if (X > GL_ZERO)
                    patch->getBaseLeft()->leftNeighbor =
                        numPatches[Y][X - 1].getBaseRight();
                else
                    patch->getBaseLeft()->leftNeighbor =
                        NULL;

                if (X < (NUM_MESHES_PER_SIDE - 1))
                    patch->getBaseRight()->leftNeighbor =
                        numPatches[Y][X + 1].getBaseLeft();
                else
                    patch->getBaseRight()->leftNeighbor =
                        NULL;

                if (Y > GL_ZERO)
                    patch->getBaseLeft()->rightNeighbor =
                        numPatches[Y - 1][X].getBaseRight();
                else
                    patch->getBaseLeft()->rightNeighbor =
                        NULL;

                if (Y < (NUM_MESHES_PER_SIDE - 1))
                    patch->getBaseRight()->rightNeighbor =
                        numPatches[Y + 1][X].getBaseLeft();
                else
                    patch->getBaseRight()->rightNeighbor =
                        NULL;
            }
        }
}

void Terrain::Tessellate()
{
    GLint nCount;
    Patch *patch = &(numPatches[0][0]);
    for (nCount = GL_ZERO; nCount < NUM_MESHES_PER_SIDE * NUM_MESHES_PER_SIDE;
            nCount++, patch++)
    {
        if (patch->isVisibile()) patch->Tessellate();
    }
}

void Terrain::Render()
{
    GLint nCount;
    Patch *patch = &(numPatches[0][0]);

    glScalef(1.0f, MULT_SCALE, 1.0f);

    for (nCount = GL_ZERO; nCount < NUM_MESHES_PER_SIDE * NUM_MESHES_PER_SIDE;
            nCount++, patch++)
    {
        if (patch->isVisibile()) patch->Render();
    }

    if (getNextTriNode() != glNumTrisDesired)
        glFrameDiff +=
            ((float)getNextTriNode() - (float)glNumTrisDesired) / (float)glNumTrisDesired;

    if (glFrameDiff < GL_ZERO) glFrameDiff = GL_ZERO;
}
