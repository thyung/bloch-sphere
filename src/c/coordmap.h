// copyrights (c) 2022 - thyung

#ifndef COORDMAP_H
#define COORDMAP_H

float newton_sqrt(float n);
void coordmap_3d2d(float x, float y, float z, float *px, float *py);
void coordmap_set_normal(float x, float y, float z);
void coordmap_set_scale(float xpixelperunit, float ypixelperunit);
void coordmap_set_offset(float xoffset, float yoffset);
int coordmap_is_front(float x, float y, float z);

#endif // COORDMAP_H