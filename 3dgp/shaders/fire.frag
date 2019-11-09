#version 330

in float age;
uniform sampler2D texture0;
out vec4 outColor;

void main()
{
	outColor = texture(texture0, gl_PointCoord);

	// alpha
	float alpha = 1 - outColor.r * outColor.g * outColor.b;
    alpha *= 1 - age;

	// RGB
	float gradient = pow(1 - age, 1.5);
	vec3 yellow = vec3(1, 1, 0);
	vec3 red = vec3(1, 0, 0);
	outColor = vec4(mix(yellow, red, 1 - gradient), alpha);
}
