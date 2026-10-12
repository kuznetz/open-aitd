#version 330

uniform sampler2D sceneTexture;
uniform sampler2D maskTexture;
uniform vec3  brightness;      // Global brightness (mode 0)
uniform float bgBrightness;    // Mask background brightness (mode 1) = world.brightnessCur
uniform float spotBrightness;  // Spot brightness (mode 1)      = world.lightSpotBrightCur
uniform int   mode;            // brightness: 0 = from vec3, 1 = from texture

in vec2 fragTexCoord;
out vec4 finalColor;

void main()
{
    vec4 texColor = texture(sceneTexture, fragTexCoord);
    if (mode == 0) {
        finalColor = vec4(texColor.rgb * brightness, 1.0); //texColor.a
    } else {
        // The mask stores only the spot geometry: 0.0 outside the spot, 1.0 at the center
        float g = texture(maskTexture, fragTexCoord).r;
        float b = mix(bgBrightness, spotBrightness, g);
        finalColor = vec4(texColor.rgb * b, 1.0); //texColor.a
    }
}