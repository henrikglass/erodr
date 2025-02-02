#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform int mode;
uniform float res;
uniform float snow_threshold;
uniform float snow_pooling;
uniform float brightness;

out vec4 finalColor;

#define PI 3.1419295

void main()
{
    float v = texture(texture0, fragTexCoord).r;

    switch (mode) {
        /* snow cover visualizer */
        case 0: {
            float s0 = texture(texture0, fragTexCoord - 0.5*snow_pooling * vec2(1.0/res)).r;
            float s1 = texture(texture0, fragTexCoord + 0.5*snow_pooling * vec2(0, 1.0/res)).r;
            float s2 = texture(texture0, fragTexCoord + 0.5*snow_pooling * vec2(1.0/res, 0)).r;
            float d1 = abs(s0 - s1);
            float d2 = abs(s0 - s2);
            float slope = d1 + d2;

            //vec4 vv = brightness * vec4(0.4 + 0.6*v, 0.35 + 0.65*v, 0.3+0.7*v, 1.0f);
            vec4 vv = brightness * vec4(v, v, v, 1.0f);
            vv.w = 1.0;
            if (slope > snow_pooling*snow_threshold) {
                finalColor = vec4(0.40,0.36,0.30,1.0) * vv;
            } else {
                finalColor = vec4(1.0,1.0,1.0,1.0);// * vv;
            } 
        } break;

        /* grayscale */
        case 1: {
            finalColor = vec4(v, v, v, 1.0);
        } break;

        /* grayscale * grayscale */
        case 2: {
            finalColor = vec4(v*v, v*v, v*v, 1.0);
        } break;

        /* pseudo coloring */
        case 3: {
            float r = cos(2*PI * (v));
            float g = cos(2*PI * (0.667 + v));
            float b = cos(2*PI * (2*0.667 + v));
            finalColor = vec4(r*r, g*g, b*b, 1.0);
        } break;

        default: {
            finalColor = vec4(1.0, 0.0, 1.0, 1.0);
        }
    }

}

