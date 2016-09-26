#ifndef VERTEX_H
#define VERTEX_H
#include <vector>
struct Vertex
{
  float x, y, z;
  float nX, nY, nZ;
};
std::vector<Vertex>::iterator vertexIterator;
std::vector<Vertex>* vbo;
#endif

