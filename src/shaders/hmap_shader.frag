#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

void main()
{
    // Texel color fetching from texture sampler
    vec4 texel = texture(texture0, fragTexCoord);

    // Calculate final fragment color
    finalColor = vec4(0.96*texel.r, 0.79*texel.r, 1.0*texel.r, texel.a)*colDiffuse*fragColor;
}

