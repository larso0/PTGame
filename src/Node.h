#ifndef NODE_H_
#define NODE_H_

#include <linmath.h>

typedef struct _Node
{
    struct _Node* parent;
    struct _Node* child;
    struct _Node* sibling;
    mat4x4 local_matrix;
    mat4x4 world_matrix;
    quat rotation;
    quat orientation;
    vec3 translation;
    vec3 position;
} Node;

void ConstructNode(Node* node);
void AddChildNode(Node* parent, Node* child);
void RemoveChildNode(Node* parent, Node* child);
void SetParentNode(Node* node, Node* parent);
void UpdateNode(Node* node);

#endif
