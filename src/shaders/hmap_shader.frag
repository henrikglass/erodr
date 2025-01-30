#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
//uniform vec4 colDiffuse;

uniform int view_mode;

// Output fragment color
out vec4 finalColor;

#define PI 3.1419295

void main()
{
    // Texel color fetching from texture sampler
    float v = texture(texture0, fragTexCoord).r;

    if (view_mode == 0) {
        finalColor = vec4(0.96*v, 0.79*v, 1.0*v, 1.0);
    } else if (view_mode == 1) {
        finalColor = vec4(v, v, v, 1.0);
    } else if (view_mode == 2) {
        finalColor = vec4(1.0 - v, 1.0 - v, 1.0 - v, 1.0);
    } else if (view_mode == 3) {
        finalColor = vec4(v*v, v*v, v*v, 1.0);
    } else if (view_mode == 4) {
        finalColor = vec4(1.0 - v*v, 1.0 - v*v, 1.0 - v*v, 1.0);
    } else if (view_mode == 5){
        float r = cos(1*PI * (v));
        float g = cos(1*PI * (0.667 + v));
        float b = cos(1*PI * (2*0.667 + v));
        finalColor = vec4(r*r, g*g, b*b, 1.0);
    } else if (view_mode == 6){
        float r = cos(2*PI * (v));
        float g = cos(2*PI * (0.667 + v));
        float b = cos(2*PI * (2*0.667 + v));
        finalColor = vec4(r*r, g*g, b*b, 1.0);
    } else {
        finalColor = vec4(1.0, 0.0, 1.0, 1.0);
    }


    // Calculate final fragment color
    //finalColor = vec4(0.96*texel.r, 0.79*texel.r, 1.0*texel.r, texel.a)*colDiffuse*fragColor;
    //finalColor = vec4(0.96*texel.r, 0.79*texel.r, 1.0*texel.r, texel.a);//*colDiffuse*fragColor;
    //finalColor = vec4(0.84*texel.r, 0.86*texel.r, 0.90*texel.r, texel.a)*colDiffuse*fragColor;
    //float v = texel.r;
    //float r = cos(2*PI * (v));
    //float g = cos(2*PI * (0.667 + v));
    //float b = cos(2*PI * (2*0.667 + v));
    //finalColor = vec4(r*r, g*g, b*b, 1.0)*colDiffuse*fragColor;
}

