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
    /* raw albedo */
    float v = texture(texture0, fragTexCoord).r;

    /* gradient */
    float s0 = texture(texture0, fragTexCoord - 0.5 * vec2(1.0/res)).r;
    float d1 = abs(s0 - texture(texture0, fragTexCoord + 0.5 * vec2(0, 1.0/res)).r);
    float d2 = abs(s0 - texture(texture0, fragTexCoord + 0.5 * vec2(1.0/res, 0)).r);
    float gradient = sqrt(d1*d1 + d2*d2);

    switch (mode) {
        /* snow cover visualizer */
        case 0: {
            /* gradient adjusted for snow pooling (samples further away) */
            s0 = texture(texture0, fragTexCoord - 0.5*snow_pooling * vec2(1.0/res)).r;
            d1 = abs(s0 - texture(texture0, fragTexCoord + 0.5*snow_pooling * vec2(0, 1.0/res)).r);
            d2 = abs(s0 - texture(texture0, fragTexCoord + 0.5*snow_pooling * vec2(1.0/res, 0)).r);
            float snow_gradient = sqrt(d1*d1 + d2*d2);

            if (snow_gradient >= snow_pooling*snow_threshold) {
                finalColor = vec4(0.40,0.36,0.30,1.0);
                finalColor = vec4(10*pow(gradient, 1.0/2.2)) * finalColor + 0.15*finalColor;
            } else {
                finalColor = vec4(0.7) + 0.6*vec4(v, v, 0.2+0.80*v, 1.0f);
            }
        } break;

        /* raw albedo */
        case 1: {
            finalColor = vec4(v);
        } break;

        /* exaggerated albedo */
        case 2: {
            finalColor = vec4(5*pow(v, 4.2));
        } break;

        /* grayscale gradient */
        case 3: {
            finalColor = vec4(5*pow(gradient, 1.0/2.2));
        } break;

        /* pseudo coloring gradient */
        case 4: {
            float flattened_gradient = pow(gradient, 1.0/2.2);
            float rate = 4;
            float r = cos(2*PI * (rate*flattened_gradient));
            float g = cos(2*PI * (0.667 + rate*flattened_gradient));
            float b = cos(2*PI * (2*0.667 + rate*flattened_gradient));
            finalColor = vec4(r*r, g*g, b*b, 1.0);
        } break;

        default: {
            finalColor = vec4(1.0, 0.0, 1.0, 1.0);
        }
    }

    finalColor.w = 1.0;
}


