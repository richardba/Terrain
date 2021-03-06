#version 330 core

in vec2 UV;
in vec3 Position_worldspace;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;

in vec3 LightDirection_tangentspace;
in vec3 EyeDirection_tangentspace;
in float height;

out vec3 color;

uniform sampler2D DiffuseTextureSampler;
uniform sampler2D NormalTextureSampler;
uniform sampler2D SpecularTextureSampler;
uniform mat4 V;
uniform mat4 M;
uniform mat3 MV3x3;
uniform vec3 LightPosition_worldspace;

void main(){

	vec3 LightColor = vec3(2);
	float LightPower = 40.0;
	
	vec3 MaterialDiffuseColor = texture( DiffuseTextureSampler, UV ).rgb;
	vec3 MaterialAmbientColor = vec3(0.1,0.1,0.1) * MaterialDiffuseColor;
	vec3 MaterialSpecularColor = texture( SpecularTextureSampler, UV ).rgb * 0.3;

	vec3 TextureNormal_tangentspace = normalize(texture( NormalTextureSampler, vec2(UV.x,-UV.y) ).rgb*2.0 - 1.0);
	
	float distance = length( LightPosition_worldspace - Position_worldspace );

	vec3 n = TextureNormal_tangentspace;
	vec3 l = normalize(LightDirection_tangentspace);
	float cosTheta = clamp( dot( n,l ), 0,1 );

	vec3 E = normalize(EyeDirection_tangentspace);
	vec3 R = reflect(-l,n);
	float cosAlpha = clamp( dot( E,R ), 0,1 );
	color = 
		MaterialAmbientColor +
		MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance*distance) +
		MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,5) / (distance*distance);
    float intensity = dot(LightPosition_worldspace,n);
  color = (ceil(color*7)/7)+vec3(height,0,0);
}