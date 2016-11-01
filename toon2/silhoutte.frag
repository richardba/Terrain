uniform vec4 _LightColor0;
uniform float _LitOutlineThickness;
uniform float _UnlitOutlineThickness;
uniform vec4 _OutlineColor;
varying vec3 viewDir,lightDir,normal;

void main()
{
	gl_FragColor = vec4(0.0);
} 
