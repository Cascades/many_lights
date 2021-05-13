#include "many_lights/colors.h"
#include <glm/glm.hpp>

glm::vec3 ml::utils::hsv_to_rgb(glm::vec3 hsv)
{
    unsigned char hsv_h = static_cast<unsigned char>(hsv.x * 255);
    unsigned char hsv_s = static_cast<unsigned char>(hsv.y * 255);
    unsigned char hsv_v = static_cast<unsigned char>(hsv.z * 255);
    glm::vec3 rgb;
    unsigned char region, remainder, p, q, t;

    if (hsv_s == 0)
    {
        rgb.r = hsv_v;
        rgb.g = hsv_v;
        rgb.b = hsv_v;
        return rgb;
    }

    region = hsv_h / 43;
    remainder = (hsv_h - (region * 43)) * 6;

    p = (hsv_v * (255 - hsv_s)) >> 8;
    q = (hsv_v * (255 - ((hsv_s * remainder) >> 8))) >> 8;
    t = (hsv_v * (255 - ((hsv_s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
    case 0:
        rgb.r = hsv_v; rgb.g = t; rgb.b = p;
        break;
    case 1:
        rgb.r = q; rgb.g = hsv_v; rgb.b = p;
        break;
    case 2:
        rgb.r = p; rgb.g = hsv_v; rgb.b = t;
        break;
    case 3:
        rgb.r = p; rgb.g = q; rgb.b = hsv_v;
        break;
    case 4:
        rgb.r = t; rgb.g = p; rgb.b = hsv_v;
        break;
    default:
        rgb.r = hsv_v; rgb.g = p; rgb.b = q;
        break;
    }

    return rgb / 255.0f;
}