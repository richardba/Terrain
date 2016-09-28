#ifndef VERTEX_H
#define VERTEX_H
#include <vector>
/**
* Estrutura referente as coordenadas que representam um ponto num plano
*/
struct Vertex
{
  /** Coordenada X do vertice */
  GLfloat x;
  /** Coordenada Y do vertice */
  GLfloat y;
  /** Coordenada Z do vertice */
  GLfloat z;
  /*
  * Normal da coordenada X do vertice /
  GLfloat nX;
  * Normal da coordenada Y do vertice /
  GLfloat nY;
  * Normal da coordenada Z do vertice /
  GLfloat nZ;*/
};
extern std::vector<Vertex>::iterator vertexIterator;
extern std::vector<Vertex>* vbo;
#endif

