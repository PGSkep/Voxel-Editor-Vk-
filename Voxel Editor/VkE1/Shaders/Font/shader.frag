#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 3) uniform sampler2D texSampler;

layout(location = 0) in vec2 UV;

layout(location = 0) out vec4 outColor;

void main()
{
	vec4 textureColor = texture(texSampler, UV).rgba;
	if(textureColor.a < 0.5)
		discard;
	outColor = vec4(textureColor.rgb, 0.5);
}