#pragma once
#include "MeshUtils/MeshUtils.h"

using namespace mu;
using FBXNode = void;

class IFBXExporterContext
{
public:
    enum class Topology
    {
        Points,
        Lines,
        Triangles,
        Quads,
    };

    enum class Format
    {
        FBXBinary,
        FBXAscii,
        FBXEncrypted,
        Obj,
    };

    virtual void release() = 0;
    virtual bool clear() = 0;

    virtual bool createScene(const char *name) = 0;
    virtual bool write(const char *path, Format format = Format::FBXBinary) = 0;

    virtual FBXNode* getRootNode() = 0;
    virtual FBXNode* findNodeByName(const char *name) = 0;

    virtual FBXNode* createNode(FBXNode *parent, const char *name) = 0;
    virtual void     setTRS(FBXNode *node, float3 t, quatf r, float3 s) = 0;
    virtual void     addMesh(FBXNode *node, Topology topology, int num_indices, int num_vertices,
        const int indices[], const float3 points[], const float3 normals[], const float4 tangents[], const float2 uv[], const float4 colors[]) = 0;
    virtual void     addSkin(FBXNode *node, Weights4 weights[], FBXNode *bones[], float4x4 bindposes[], int num_bones) = 0;

protected:
    virtual ~IFBXExporterContext() {}
};

npAPI IFBXExporterContext* CreateFBXExporter();
