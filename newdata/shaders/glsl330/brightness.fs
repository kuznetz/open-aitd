#version 330

uniform sampler2D sceneTexture;
uniform sampler2D maskTexture;
uniform vec3 brightness;
uniform int mode; // brightness: 0 = from vec3, 1 = from texture

in vec2 fragTexCoord;
out vec4 finalColor;

void main()
{
    vec4 texColor = texture(sceneTexture, fragTexCoord);
    if (mode == 0) {
        finalColor = vec4(texColor.rgb * brightness, 1.0); //texColor.a
    } else {
        vec4 maskValue = texture(maskTexture, fragTexCoord);
        //finalColor = maskValue;
        finalColor = vec4(texColor.rgb * maskValue.r, 1.0); //texColor.a
    }
}