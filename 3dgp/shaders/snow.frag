#version 330

in float age;
in vec4 position;
uniform sampler2D texture0;
out vec4 outColor;

void main()
{
	if (length(position) > 10) 
		discard;

	outColor = texture(texture0, gl_PointCoord);
	outColor.a = 1 - outColor.r * outColor.g * outColor.b;
	//outColor.a *= 1 - age;
}
