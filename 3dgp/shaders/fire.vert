#version 330

// Uniforms: Transformation Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixView;
uniform mat4 matrixModelView;

// Particle-specific Uniforms
uniform vec3 gravity = vec3(0.0, -0.05, 0.0);	// Gravity Acceleration in world coords
uniform float particleLifetime;					// Max Particle Lifetime
uniform float time;								// Animation Time

// Special Vertex Attributes
layout (location = 0) in vec3 aInitialPos;		// Initial Position
layout (location = 1) in vec3 aVelocity;		// Particle initial velocity
layout (location = 2) in float aStartTime;		// Particle "birth" time

// Output Variable (sent to Fragment Shader)
out float age;									// age of the particle (0..1)

void main()
{
	float t = mod(time - aStartTime, particleLifetime);
	vec3 pos = aInitialPos + aVelocity * t + gravity * t * t; 
	age = t / particleLifetime;

	// calculate position (normal calculation not applicable here)
	vec4 position = matrixModelView * vec4(pos, 1.0);
	gl_Position = matrixProjection * position;

	gl_PointSize = clamp(100 / length(position), 1,50);
}
