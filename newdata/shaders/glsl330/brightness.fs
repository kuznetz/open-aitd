#version 330
uniform sampler2D texture0;
uniform vec3 brightness;
in vec2 fragTexCoord;
out vec4 finalColor;

void main()
{
	vec4 texColor = texture(texture0, fragTexCoord);
	finalColor = vec4(texColor.rgb * brightness, texColor.a);
}
