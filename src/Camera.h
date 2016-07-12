#ifndef CAMERA_H_
#define CAMERA_H_

#include "Node.h"

typedef struct
{
    Node node;
    mat4x4 view_matrix;
    vec3 direction;
    vec3 up;
    vec3 right;
    float yaw, pitch, roll;
} Camera;

void ConstructCamera(Camera* camera);
void UpdateCamera(Camera* camera);

#endif
