#version 450 compatibility

layout (location = 0) in vec2 vert_pos;
layout (location = 1) in vec2 vert_uv;

// x,y is scale and aspect; z,w is camera position in world coords
uniform vec2 transform;

out vec2 frag_pos;
out vec2 frag_uv;

void main()
{
	frag_pos = vert_pos;
	frag_uv = vert_uv;
	vec2 ndc_pos = (vert_pos*transform-0.5)*2;
	gl_Position = vec4(ndc_pos.x, -ndc_pos.y,0.5,1);
}