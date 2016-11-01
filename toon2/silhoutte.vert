varying vec3 viewDir,lightDir,normal;

void main()
{
	lightDir = normalize(vec3(gl_LightSource[0].position));
	normal = gl_NormalMatrix * gl_Normal;
  viewDir = vec3(gl_ModelViewMatrix * gl_Vertex); 

	gl_Position = ftransform()+vec4(normal/10,0.0);
} 
