#version 330

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;

//Fog Colour Material
uniform vec3 fogColour;

//View Matrix
uniform mat4 matrixView;

// Input Variables (received from Vertex Shader)
in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;
in float fogFactor;

// Uniform: The Texture
uniform sampler2D texture0;

// Output Variable (sent down through the Pipeline)
out vec4 outColor;

//Point Light Structure
struct POINT
{
	int on;
	vec3 position;
	vec3 diffuse;
	vec3 specular;
	float att_quadratic;
};
//Point Lights
uniform POINT lightPoint1;

//Point Light Function
vec4 PointLight(POINT light)
{
	vec4 color = vec4(0, 0, 0, 0);

	vec3 L = normalize(matrixView * vec4(light.position,1) - position).xyz;
	float NdotL = dot(normal, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;

	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normal);
	float RdotV = dot(R, V);

	if(NdotL > 0 && RdotV > 0)
		color += vec4(materialSpecular * light.specular * pow(RdotV, shininess), 1);


	if (light.att_quadratic > 0)
	{
		float dist = length(matrixView * vec4(light.position, 1) - position);
		float att = 1 / (light.att_quadratic * dist * dist);
		return color * att;
	}
	else
		return color;
}

void main(void) 
{
	outColor = color;

	//Point Lights
	if(lightPoint1.on == 1) 
		outColor += PointLight(lightPoint1);

	outColor *= texture(texture0, texCoord0.st);

	outColor = mix(vec4(fogColour, 1), outColor, fogFactor);
}
