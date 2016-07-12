#include "Camera.h"

static vec3 CAMERA_DIRECTION = { 0.f, 0.f, -1.f };
static vec3 CAMERA_UP = { 0.f, 1.f, 0.f };
static vec3 CAMERA_RIGHT = { 1.f, 0.f, 0.f };

void ConstructCamera(Camera* camera)
{
    ConstructNode(&camera->node);
    int i;
    for(i = 0; i < 3; i++)
    {
        camera->direction[i] = CAMERA_DIRECTION[i];
        camera->up[i] = CAMERA_UP[i];
        camera->right[i] = CAMERA_RIGHT[i];
    }
    camera->yaw = 0.f;
    camera->pitch = 0.f;
    camera->roll = 0.f;
    vec3 center;
    vec3_add(center, camera->node.position, camera->direction);
    mat4x4_look_at(camera->view_matrix,
                   camera->node.position,
                   center,
                   camera->up);
}

void UpdateCamera(Camera* camera)
{
    quat tmp1, tmp2, tmp3, tmp4;
    quat_rotate(tmp1, camera->yaw, CAMERA_UP);
    quat_rotate(tmp2, camera->pitch, CAMERA_RIGHT);
    quat_rotate(tmp3, camera->roll, camera->direction);
    quat_mul(tmp4, tmp1, tmp2);
    quat_mul(camera->node.rotation, tmp4, tmp3);
    UpdateNode(&camera->node);
    quat_mul_vec3(camera->direction,
                  camera->node.orientation,
                  CAMERA_DIRECTION);
    quat_mul_vec3(camera->up,
                  camera->node.orientation,
                  CAMERA_UP);
    vec3_mul_cross(camera->right, camera->direction, camera->up);
    vec3 center;
    vec3_add(center, camera->node.position, camera->direction);
    mat4x4_look_at(camera->view_matrix,
                   camera->node.position,
                   center,
                   camera->up);
}
