#define WINDOW_HEIGHT 800
#define WINDOW_WIDTH 600

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "../include/Shader.h"
#include "../include/Texture.h"
#include "../include/Controls.h"
#include "../include/Objloader.h"
#include "../include/Vboindexer.h"
#include "../include/Tangentspace.h"
int main( void )
{
	if( !glfwInit() )
	{
		puts("This system is not compatible with GLFW.");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow( WINDOW_HEIGHT, WINDOW_WIDTH, "Toon Shading", NULL, NULL);
	if( window == NULL ){
		puts("This system is not compatible with GLFW.");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		puts("This hardware is not compatible with shaders.");
		getchar();
		glfwTerminate();
		return -1;
	}

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

	glClearColor(.40f, .53f, .60f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	GLuint programID = LoadShaders( "terrain.vert", "terrain.frag" );

	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
	GLuint ModelView3x3MatrixID = glGetUniformLocation(programID, "MV3x3");

	GLuint DiffuseTexture = loadDDS("diffuse.DDS");
	GLuint NormalTexture = loadBMP_custom("normal.bmp");
	GLuint SpecularTexture = loadDDS("specular.DDS");

	GLuint DiffuseTextureID  = glGetUniformLocation(programID, "DiffuseTextureSampler");
	GLuint NormalTextureID  = glGetUniformLocation(programID, "NormalTextureSampler");
	GLuint SpecularTextureID  = glGetUniformLocation(programID, "SpecularTextureSampler");

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ("terrain.obj", vertices, uvs, normals);

	std::vector<glm::vec3> tangents;
	std::vector<glm::vec3> bitangents;
	computeTangentBasis(
		vertices, uvs, normals,
		tangents, bitangents
	);

	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	std::vector<glm::vec3> indexed_tangents;
	std::vector<glm::vec3> indexed_bitangents;
	indexVBO_TBN(
		vertices, uvs, normals, tangents, bitangents,
		indices, indexed_vertices, indexed_uvs, indexed_normals, indexed_tangents, indexed_bitangents
	);


	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	GLuint tangentbuffer;
	glGenBuffers(1, &tangentbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, tangentbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_tangents.size() * sizeof(glm::vec3), &indexed_tangents[0], GL_STATIC_DRAW);

	GLuint bitangentbuffer;
	glGenBuffers(1, &bitangentbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, bitangentbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_bitangents.size() * sizeof(glm::vec3), &indexed_bitangents[0], GL_STATIC_DRAW);

	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	GLuint silhoutte = glGetUniformLocation(programID, "silhoutteColor");
  GLuint offset = glGetUniformLocation(programID, "offset");
	double lastTime = glfwGetTime();
	int nbFrames = 0;

	do
	{

		double currentTime = glfwGetTime();
		nbFrames++;
		if ( currentTime - lastTime >= 1.0 )
    {
			printf("%f ms/frame\n", 1000.0/double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(programID);

		computeMatricesFromInputs(window);
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 ModelViewMatrix = ViewMatrix * ModelMatrix;
		glm::mat3 ModelView3x3Matrix = glm::mat3(ModelViewMatrix);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
		glUniformMatrix3fv(ModelView3x3MatrixID, 1, GL_FALSE, &ModelView3x3Matrix[0][0]);



		glm::vec3 lightPos = glm::vec3(0,6,0);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, DiffuseTexture);
		glUniform1i(DiffuseTextureID, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, NormalTexture);
		glUniform1i(NormalTextureID, 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, SpecularTexture);
		glUniform1i(SpecularTextureID, 2);


		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,
			2,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
			2,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);

		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, tangentbuffer);
		glVertexAttribPointer(
			3,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);

		glEnableVertexAttribArray(4);
		glBindBuffer(GL_ARRAY_BUFFER, bitangentbuffer);
		glVertexAttribPointer(
			4,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		glDrawElements(
			GL_TRIANGLES,
			indices.size(),
			GL_UNSIGNED_SHORT,
			(void*)0
		);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);

		glfwSwapBuffers(window);
		glfwPollEvents();


	}
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &tangentbuffer);
	glDeleteBuffers(1, &bitangentbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &DiffuseTexture);
	glDeleteTextures(1, &NormalTexture);
	glDeleteTextures(1, &SpecularTexture);
	glDeleteVertexArrays(1, &VertexArrayID);

	glfwTerminate();

	return 0;
}

