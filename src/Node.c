#include "Node.h"
#include <stdlib.h>

void ConstructNode(Node* node)
{
    node->parent = NULL;
    node->child = NULL;
    node->sibling = NULL;
    mat4x4_identity(node->local_matrix);
    mat4x4_identity(node->world_matrix);
    quat_identity(node->rotation);
    quat_identity(node->orientation);
    int i;
    for(i = 0; i < 3; i++)
    {
        node->translation[i] = 0.f;
        node->position[i] = 0.f;
    }
}

void AddChildNode(Node* parent, Node* child)
{
    if(child->parent) RemoveChildNode(child->parent, child);
    child->sibling = parent->child;
    child->parent = parent;
    parent->child = child;
}

void RemoveChildNode(Node* parent, Node* child)
{
    Node* prev = NULL;
    Node* node = parent->child;
    while(node && node != child)
    {
        prev = node;
        node = node->sibling;
    }
    if(node)
    {
        if(prev)
        {
            prev->sibling = child->sibling;
        }
        else
        {
            parent->child = child->sibling;
        }
        child->parent = NULL;
        child->sibling = NULL;
    }
}

void SetParentNode(Node* node, Node* parent)
{
    if(node->parent) RemoveChildNode(node->parent, node);
    if(parent) AddChildNode(parent, node);
}

void UpdateNode(Node* node)
{
    mat4x4_from_quat(node->local_matrix, node->rotation);
    mat4x4_translate_in_place(node->local_matrix,
                              node->translation[0],
                              node->translation[1],
                              node->translation[2]);
    if(node->parent)
    {
        mat4x4_mul(node->world_matrix,
                   node->parent->world_matrix,
                   node->local_matrix);
        quat_mul(node->orientation,
                 node->parent->orientation,
                 node->rotation);
        vec3 nt;
        vec3_norm(nt, node->translation);
        quat_mul_vec3(nt, node->parent->orientation, nt);
        vec3_scale(nt, nt, vec3_len(node->translation));
        vec3_add(node->position, node->parent->position, nt);
    }
    else
    {
        mat4x4_dup(node->world_matrix, node->local_matrix);
        int i;
        for(i = 0; i < 4; i++)
            node->orientation[i] = node->rotation[i];
        for(i = 0; i < 3; i++)
            node->position[i] = node->translation[i];
    }
    Node* child;
    for(child = node->child; child != NULL; child = child->sibling)
        UpdateNode(child);
}
