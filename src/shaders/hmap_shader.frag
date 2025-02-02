#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
//uniform vec4 colDiffuse;

uniform int view_mode;
uniform float snow_threshold;
uniform float gain;

// Output fragment color
out vec4 finalColor;

#define PI 3.1419295

#define BAND_THRESH 0.007

void main()
{
    // Texel color fetching from texture sampler
    float v = texture(texture0, fragTexCoord).r;
    float v_1 = texture(texture0, fragTexCoord + vec2(0, 1.0/1024.0)).r;
    float v_2 = texture(texture0, fragTexCoord + vec2(1.0/1024.0, 0)).r;
    float d1 = abs(v - v_1);
    float d2 = abs(v - v_2);

    if (view_mode == 0) {
        vec4 vv = gain * vec4(0.4 + 0.6*v, 0.4 + 0.6*v, 0.4+0.6*v, 1.0f);
        //vec4 vv = gain * vec4(v, v, v, 1.0);
        vv.w = 1.0;
        if (d1 + d2 > snow_threshold) {
            //finalColor = vec4(0.40,0.37,0.28,1.0) * vv;
            finalColor = vec4(0.40,0.36,0.30,1.0) * vv;
        } else {
            //finalColor = vec4(0.7,0.9,0.5,1.0) * vv;
            finalColor = vec4(1.0,1.0,1.0,1.0) * vv;
        } 
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
    } else if (view_mode == 7){
        float r = cos(4*PI * (v));
        float g = cos(4*PI * (0.667 + v));
        float b = cos(4*PI * (2*0.667 + v));
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

