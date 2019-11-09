#version 330

// Uniforms: Transformation Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixView;
uniform mat4 matrixModelView;

// Uniforms: Material Colours
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;

//Uniform: Fog Density
uniform float fogDensity;

layout (location = 0) in vec3 aVertex;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;

out vec4 color;
out vec4 position;
out vec3 normal;
out vec2 texCoord0;
out float fogFactor;

//Point Light Structure
struct POINT
{
	int on;
	vec3 position;
	vec3 diffuse;
	vec3 specular;
};
//Point Lights
uniform POINT lightPoint1;

//Ambient Light Structure
struct AMBIENT
{	
	int on;
	vec3 color;
};
//Ambient Lights
uniform AMBIENT lightAmbient, lightEmissive;

//Directional Light Structure
struct DIRECTIONAL
{	
	int on;
	vec3 direction;
	vec3 diffuse;
};
//Directional Lights
uniform DIRECTIONAL lightDir;

//Ambient Light Function
vec4 AmbientLight(AMBIENT light)
{
	// Calculate Ambient Light
	return vec4(materialAmbient * light.color, 1);
}

//Directional Light Function
vec4 DirectionalLight(DIRECTIONAL light)
{
	// Calculate Directional Light
	vec4 color = vec4(0, 0, 0, 0);
	vec3 L = normalize(mat3(matrixView) * light.direction);
	float NdotL = dot(normal, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;
	return color;
}

void main(void) 
{
	// calculate position
	position = matrixModelView * vec4(aVertex, 1.0);
	gl_Position = matrixProjection * position;

	// calculate normal
	normal = normalize(mat3(matrixModelView) * aNormal);

	// calculate texture coordinate
	texCoord0 = aTexCoord;

	//calculate fog factor
	fogFactor = exp2(-fogDensity * length(position));

	// calculate light
	color = vec4(0, 0, 0, 1);

	if (lightAmbient.on == 1) 
		color += AmbientLight(lightAmbient);
	if (lightDir.on == 1) 
		color += DirectionalLight(lightDir);

	if(lightEmissive.on == 1)
		color += AmbientLight(lightEmissive);
}
