#version 450 compatibility

uniform bool use_texture;
uniform sampler2D image;

uniform vec4 top_color;
uniform vec4 bottom_color;

uniform vec2 pos;
uniform vec2 dim;

uniform bool disable_corners;
uniform float corner_size;

in vec2 frag_pos;
in vec2 frag_uv;

out vec4 FragColor;

bool in_bounds(float x, float mini, float maxi)
{
	return x >= mini && x <= maxi;
}

void main()
{
	vec2 local_pos = (frag_pos-pos);

	// if the closest vertex is supposed to be rounded and the fragment is located in a quarter-circle.
	if (!disable_corners && corner_size > 0 && !(in_bounds(local_pos.x, corner_size, dim.x-corner_size) || in_bounds(local_pos.y, corner_size, dim.y-corner_size)))
	{
		vec2 corner = round(frag_uv)*dim; // Closest corner to fragment
		vec2 dir = round(frag_uv)*2-1; // Offset direction (ALWAYS A MULTIPLE OF 45DEG) for curve origin from the closest corner.
		float dist = distance(local_pos, corner - dir*corner_size);
		
		if (dist > corner_size)
		{
			discard;
		}
	}

	vec4 color = mix(top_color, bottom_color, frag_uv.y);

	if (use_texture)
	{
		float texture_alpha = texture(image, frag_uv).r;
		color.w *= texture_alpha;
	}

	FragColor = color;
}