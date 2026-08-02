#version 330

uniform sampler2D texture0;   // Main image
uniform sampler2D texture1;   // Mask (in R channel)
uniform vec2 texSize;         // Texture size in pixels

in vec2 fragTexCoord;
out vec4 finalColor;

void main() {
    vec2 uv = fragTexCoord;
    vec2 step = 1.0 / texSize;

    // Offsets for 4 samples (corners of the pixel)
    vec2 offsets[4] = vec2[4](
        vec2(-0.5, -0.5), vec2( 0.5, -0.5),
        vec2(-0.5,  0.5), vec2( 0.5,  0.5)
    );

    // Arrays to store sample results
    vec4 tex0_samples[4];
    float alpha_samples[4];
    vec3 color_samples[4];

    // Single pass – exactly 4 texture fetches
    for (int i = 0; i < 4; i++) {
        vec2 sampleUV = uv + offsets[i] * step;
        tex0_samples[i] = texture(texture0, sampleUV);
        float mask = texture(texture1, sampleUV).r;
        alpha_samples[i] = tex0_samples[i].a * (1.0 - mask);
        color_samples[i] = tex0_samples[i].rgb;
    }

    // Compute average alpha and variance
    float avgAlpha = (alpha_samples[0] + alpha_samples[1] + alpha_samples[2] + alpha_samples[3]) / 4.0;
    float variance = 0.0;
    for (int i = 0; i < 4; i++) {
        float diff = alpha_samples[i] - avgAlpha;
        variance += diff * diff;
    }
    variance /= 4.0;

    const float threshold = 0.001;  // Tune this value as needed

    if (variance < threshold) {
        // Inside uniform area – use the first sample (any works)
        // This keeps the image sharp without blurring
        finalColor = vec4(color_samples[0], alpha_samples[0]);
    } else {
        // On an edge – average all 4 samples (alpha-weighted)
        vec3 colorAccum = vec3(0.0);
        float alphaAccum = 0.0;
        for (int i = 0; i < 4; i++) {
            colorAccum += color_samples[i] * alpha_samples[i];
            alphaAccum += alpha_samples[i];
        }
        if (alphaAccum > 0.0) {
            finalColor = vec4(colorAccum / alphaAccum, alphaAccum / 4.0);
        } else {
            finalColor = vec4(0.0, 0.0, 0.0, 0.0);
        }
    }
}