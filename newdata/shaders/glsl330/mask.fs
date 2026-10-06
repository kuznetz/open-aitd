#version 330

// Input vertex attributes
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;    // Default texture
uniform sampler2D texture1;    // Mask

// Custom uniforms
uniform vec2 resolution;

// Output fragment color
out vec4 finalColor;

void main() {
    // Sample all textures
    vec4 tex0 = texture(texture0, fragTexCoord);
    vec4 tex1 = texture(texture1, fragTexCoord);
    
    vec2 screenPos = gl_FragCoord.xy / resolution;
    
    //finalColor = vec4(tex1.r, tex1.r, tex1.r, tex0.a);
    //finalColor = vec4(tex0.rgb, min(tex0.a, 1-tex1.r));
    // tex0 (the object color texture) is premultiplied alpha, so the occluder
    // mask must be applied to both the color and the alpha channels.
    finalColor = vec4(tex0.rgb, tex0.a) * (1.0 - tex1.r);
}