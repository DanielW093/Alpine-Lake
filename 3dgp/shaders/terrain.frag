#version 330

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;

//Texture Transition Values
uniform float snowOpacity;

//Fog Colour Material
uniform vec3 fogColour;

//Water Related Materials
uniform vec3 waterColor;

//View Matrix
uniform mat4 matrixView;

// Input Variables (received from Vertex Shader)
in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;
in float fogFactor;
in float eyeAlt;		// observer's altitude above the observed vertex

//Multitexturing Inputs
in float waterDepth; //water depth (positive for the bed, negative for the shore)
in float grassDepth; //grass depth (positive for under grass end, negative for above)
in float snowDepth; //snow depth (positive for under snow end, negative for above)

// Texture
uniform sampler2D textureBed;
uniform sampler2D textureShore;
uniform sampler2D textureSnow;
uniform sampler2D texturePeak;

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

//Directional Light Structure
struct DIRECTIONAL
{	
	int on;
	vec3 direction;
	vec3 diffuse;
};
//Directional Lights
uniform DIRECTIONAL lightDir;

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
	outColor = color;

	//Point Lights
	if(lightPoint1.on == 1) 
		outColor += PointLight(lightPoint1);
	
	if (lightDir.on == 1) 
		outColor += DirectionalLight(lightDir);

	//multitexturing
	float isAboveWater = clamp(-waterDepth, 0, 1);
	float isAboveGrass = clamp(-grassDepth, 0, 10);
	float isAboveSnow = clamp(-snowDepth, 0, 20);

	vec4 grassTexture = mix(texture(textureShore, texCoord0), texture(textureSnow, texCoord0), snowOpacity);

	if(isAboveGrass == 0)	//Mix Pebbles into Grass
		outColor *= mix(texture(textureBed, texCoord0), grassTexture, isAboveWater);
	else if (isAboveGrass > 0 && isAboveSnow == 0)	//Mix Grass into Snow
		outColor *= mix(grassTexture, texture(textureSnow, texCoord0), isAboveGrass/10);
	else if (isAboveSnow > 0) //Mix Snow into Snowy Rock
		outColor *= mix(texture(textureSnow, texCoord0), texture(texturePeak, texCoord0), isAboveSnow/20);

	if(waterDepth > 0 && eyeAlt != 0)
	{
		float underwaterFogDensity = 0.5;
		float underwaterFogFactor = exp2(-underwaterFogDensity * length(position) * waterDepth / abs(eyeAlt));
		outColor = mix(vec4(waterColor, 1), outColor, underwaterFogFactor);
	}
	else
	{
		//Render Fog
		outColor = mix(vec4(fogColour, 1), outColor, fogFactor);
	}
}
