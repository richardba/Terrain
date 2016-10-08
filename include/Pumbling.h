#ifdef __glew_h__
#ifndef PUMBLING_H
#define PUMBLING_H
void pumbleShader(GLuint *vertexArrayId,
                  GLuint *programId,
                  )
{
  GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	GLuint programID = LoadShaders( "NormalMapping.vertexshader", "NormalMapping.fragmentshader" );
}
#endif
#endif
