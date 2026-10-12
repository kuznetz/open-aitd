#version 330

uniform sampler2D sceneTexture;
uniform sampler2D maskTexture; // Soft circle texture, sampled directly (no mask render target)
uniform vec3  brightness;      // Global brightness (mode 0)
uniform float bgBrightness;    // Mask background brightness (mode 1) = world.brightnessCur
uniform float spotBrightness;  // Spot brightness (mode 1)      = world.lightSpotBrightCur
uniform vec2  spotCenter;      // Spot center in UV space [0..1]
uniform vec2  spotSizeUV;      // Spot quad size / screen size (per axis, handles aspect)
uniform int   mode;            // brightness: 0 = from vec3, 1 = from texture

in vec2 fragTexCoord;
out vec4 finalColor;

void main()
{
    vec4 texColor = texture(sceneTexture, fragTexCoord);
    if (mode == 0) {
        finalColor = vec4(texColor.rgb * brightness, 1.0); //texColor.a
    } else {
        // Map the fragment to the soft-circle texture UV.
        // This is equivalent to drawing softCircleTex into a size x size rect centered at spotCenter.
        // The texture border texels are 0 and the texture is clamped, so outside the spot g == 0.
        vec2 texUV = (fragTexCoord - spotCenter) / spotSizeUV + 0.5;
        float g = texture(maskTexture, texUV).r;
        float b = mix(bgBrightness, spotBrightness, g);
        finalColor = vec4(texColor.rgb * b, 1.0); //texColor.a
    }
}
