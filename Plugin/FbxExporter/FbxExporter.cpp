#include "pch.h"
#include "FbxExporter.h"
#include "fbxeContext.h"

using namespace fbxe;

fbxeAPI fbxe::IContext* fbxeCreateContext(const fbxe::ExportOptions *opt)
{
    return fbxe::CreateContext(opt);
}

fbxeAPI void fbxeReleaseContext(fbxe::IContext *ctx)
{
    if (!ctx) { return; }
    ctx->release();
}

fbxeAPI int fbxeCreateScene(fbxe::IContext *ctx, const char *name)
{
    if (!ctx) { return false; }
    return ctx->createScene(name);
}

fbxeAPI int fbxeWrite(fbxe::IContext *ctx, const char *path, fbxe::Format format)
{
    if (!ctx) { return false; }
    return ctx->write(path, format);
}

fbxeAPI fbxe::Node* fbxeGetRootNode(fbxe::IContext *ctx)
{
    if (!ctx) { return nullptr; }
    return ctx->getRootNode();
}
fbxeAPI fbxe::Node* fbxeFindNodeByName(fbxe::IContext *ctx, const char *name)
{
    if (!ctx) { return nullptr; }
    return ctx->findNodeByName(name);
}

fbxeAPI fbxe::Node* fbxeCreateNode(fbxe::IContext *ctx, fbxe::Node *parent, const char *name)
{
    if (!ctx) { return nullptr; }
    return ctx->createNode(parent, name);
}

fbxeAPI void fbxeSetTRS(fbxe::IContext *ctx, fbxe::Node *node, float3 t, quatf r, float3 s)
{
    if (!ctx) { return; }
    return ctx->setTRS(node, t, r, s);
}

fbxeAPI void fbxeAddMesh(fbxe::IContext *ctx, fbxe::Node *node, int num_vertices,
    const float3 points[], const float3 normals[], const float4 tangents[], const float2 uv[], const float4 colors[])
{
    if (!ctx) { return; }
    return ctx->addMesh(node, num_vertices, points, normals, tangents, uv, colors);
}

fbxeAPI void fbxeAddMeshSubmesh(fbxe::IContext *ctx, fbxe::Node *node, fbxe::Topology topology, int num_indices, const int indices[], int material)
{
    if (!ctx) { return; }
    return ctx->addMeshSubmesh(node, topology, num_indices, indices, material);
}

fbxeAPI void fbxeAddMeshSkin(fbxe::IContext *ctx, fbxe::Node *node, Weights4 weights[], int num_bones, fbxe::Node *bones[], float4x4 bindposes[])
{
    if (!ctx) { return; }
    return ctx->addMeshSkin(node, weights, num_bones, bones, bindposes);
}
