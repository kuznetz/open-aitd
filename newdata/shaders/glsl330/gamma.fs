#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

uniform float gamma;

vec3 gammaCorrection (vec3 colour) {
  return pow(colour, vec3(1. / gamma));
}

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord)*colDiffuse*fragColor;

    // Convert texel color to grayscale using NTSC conversion weights
    //float gray = dot(texelColor.rgb, vec3(0.299, 0.587, 0.114));
    //finalColor = vec4(gray, gray, gray, texelColor.a);
    
    vec3 result = gammaCorrection(texelColor.rgb);
    finalColor = vec4(result.r, result.g, result.b, 1);
}