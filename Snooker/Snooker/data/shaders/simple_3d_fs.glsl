#version 330 core

in vec2 UV;
out vec4 out_Color;

uniform sampler2D myTextureSampler;

void main(void)
{
	out_Color = texture(myTextureSampler, UV ).rgb;
}
