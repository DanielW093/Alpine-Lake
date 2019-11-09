#version 330

// Uniforms: Transformation Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixView;
uniform mat4 matrixModelView;
uniform mat4 matrixInvertedView;

//Uniform: Fog Density
uniform float fogDensity;

//Uniform: Animation Time
uniform float time; //real time

layout (location = 0) in vec3 aVertex;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;

out vec4 color;
out vec4 position;
out vec3 normal;
out vec2 texCoord0;
out float fogFactor;
out float reflFactor;	//reflection coefficient
out vec3 texCoordCubeMap;

float wave(float A, float x, float y, float t)
{
	t *= 2;
	return A * (
					sin(2.0 * (x * 0.2 + y * 0.7) + t * 1.0) +
					sin(2.0 * (x * 0.7 + y * 0.2) + t * 0.8) +
					pow(sin(2.0 * (x * 0.6 + y * 0.5) + t * 1.2), 2) +
					pow(sin(2.0 * (x * 0.8 + y * 0.2) + t * 1.1), 2));
}

void main(void) 
{
	//Calulate the wave
	vec3 wVertex = aVertex;
	vec3 wNormal = aNormal;

	float a = 0.05;
	wVertex.y = wave(a, wVertex.x, wVertex.z, time);

	float d = 0.1;
	float dx = (wave(a, wVertex.x + d/2, wVertex.z, time) -
				wave(a, wVertex.x - d/2, wVertex.z, time)) / d;
	float dz = (wave(a, wVertex.x, wVertex.z + d/2, time) -
				wave(a, wVertex.x, wVertex.z - d/2, time)) / d;

	wNormal = normalize(vec3(-dx, 1, -dz));

	// calculate position
	position = matrixModelView * vec4(wVertex, 1.0);
	gl_Position = matrixProjection * position;

	// calculate normal
	normal = normalize(mat3(matrixModelView) * wNormal);

	// calculate texture coordinate
	texCoord0 = aTexCoord;

	//calculate fog factor
	fogFactor = exp2(-fogDensity * length(position));

	//calculate reflection coefficient
	//(Uses Schlick's approximation of Fresnel formula)
	float cosTheta = dot(normal, normalize(-position.xyz));
	float R0 = 0.02;
	reflFactor = R0 + (1 - R0) * pow(1.0 - cosTheta, 5);

	texCoordCubeMap = mat3(matrixInvertedView) * mix(reflect(position.xyz, normal.xyz), normal.xyz, 0.2);
}
