#pragma once
#include "MeshUtils/MeshUtils.h"


namespace fbxe {

using namespace mu;


class IContext
{
public:
    virtual void release() = 0;
    virtual bool clear() = 0;

    virtual bool createScene(const char *name) = 0;
    virtual bool write(const char *path, Format format = Format::FBXBinary) = 0;

    virtual Node* getRootNode() = 0;
    virtual Node* findNodeByName(const char *name) = 0;

    virtual Node* createNode(Node *parent, const char *name) = 0;
    virtual void setTRS(Node *node, float3 t, quatf r, float3 s) = 0;
    virtual void addMesh(Node *node, Topology topology, int num_indices, int num_vertices,
        const int indices[], const float3 points[], const float3 normals[], const float4 tangents[], const float2 uv[], const float4 colors[]) = 0;
    virtual void addSkin(Node *node, Weights4 weights[], Node *bones[], float4x4 bindposes[], int num_bones) = 0;

protected:
    virtual ~IContext() {}
};

fbxeAPI IContext* CreateContext(const ExportOptions *opt);

} // namespace fbxe
