// copyrights (c) 2022 - thyung

#include "coordmap.h"

static float s_normal_x, s_normal_y, s_normal_z;
static float s_offset_x, s_offset_y;
static float s_pixelperunit_x, s_pixelperunit_y;
static float s_e1_x, s_e1_y, s_e1_z;
static float s_e2_x, s_e2_y, s_e2_z;

float newton_sqrt(float n) {
    float x = n;
    x = 0.5 * (x + (n / x));
    x = 0.5 * (x + (n / x));
    x = 0.5 * (x + (n / x));
    x = 0.5 * (x + (n / x));
    x = 0.5 * (x + (n / x));
    return x;
}

void coordmap_3d2d(float x, float y, float z, float *px, float *py) {
    *px = s_e1_x * x + s_e1_y * y + s_e1_z * z;
    *py = s_e2_x * x + s_e2_y * y + s_e2_z * z;

    // scale and offset
    *px = *px * s_pixelperunit_x + s_offset_x;
    *py = *py * s_pixelperunit_y + s_offset_y; 
}

void coordmap_set_normal(float x, float y, float z) {
    float k;
    float e11, e12, e13;
    float e21, e22, e23;
    float e1_len, e2_len;
    
    s_normal_x = x;
    s_normal_y = y;
    s_normal_z = z;

    k = -s_normal_z / 
        (s_normal_x * s_normal_x + s_normal_y * s_normal_y + s_normal_z * s_normal_z);
    e21 = k * s_normal_x;
    e22 = k * s_normal_y;
    e23 = 1 + k * s_normal_z;
    e11 = e22 * z - e23 * y;
    e12 = e23 * x - e21 * z;
    e13 = e21 * y - e22 * x;

    // normalize e1 and e2
    e1_len = newton_sqrt(e11*e11 + e12*e12 + e13*e13);
    e2_len = newton_sqrt(e21*e21 + e22*e22 + e23*e23);
    s_e1_x = e11 / e1_len;
    s_e1_y = e12 / e1_len;
    s_e1_z = e13 / e1_len;
    s_e2_x = e21 / e2_len;
    s_e2_y = e22 / e2_len;
    s_e2_z = e23 / e2_len;
}

void coordmap_set_scale(float pixelperunit_x, float pixelperunit_y) {
    s_pixelperunit_x = pixelperunit_x;
    s_pixelperunit_y = pixelperunit_y;
}

void coordmap_set_offset(float offset_x, float offset_y) {
    s_offset_x = offset_x;
    s_offset_y = offset_y;
}

int coordmap_is_front(float x, float y, float z) {
    float dot_prod = 0;
    dot_prod = s_normal_x * x + s_normal_y * y + s_normal_z * z;
    return (dot_prod >= 0);
}