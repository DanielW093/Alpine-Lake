#version 330

//Fog Colour Material
uniform vec3 fogColour;

//Water Materials
uniform vec3 waterColor;
uniform vec3 skyColor;

//View Matrix
uniform mat4 matrixView;

// Input Variables (received from Vertex Shader)
in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;
in float fogFactor;
in float reflFactor;	//reflection coefficient

// VAriables: Dynamics Reflection
in vec3 texCoordCubeMap;
uniform samplerCube textureCubeMap;
uniform float reflectionPower;

// Output Variable (sent down through the Pipeline)
out vec4 outColor;

void main(void) 
{
	outColor = color;

	outColor = mix(vec4(waterColor, 0.2), vec4(skyColor,0.45), reflFactor);

	outColor = mix(outColor, texture(textureCubeMap, texCoordCubeMap), reflectionPower);

	outColor = mix(vec4(fogColour, 1), outColor, fogFactor);
}
