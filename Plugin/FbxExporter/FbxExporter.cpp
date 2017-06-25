#include "pch.h"
#include "FbxExporter.h"
#include "fbxeContext.h"

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

fbxeAPI void fbxeAddMesh(fbxe::IContext *ctx, fbxe::Node *node, fbxe::Topology topology, int num_indices, int num_vertices,
    const int indices[], const float3 points[], const float3 normals[], const float4 tangents[], const float2 uv[], const float4 colors[])
{
    if (!ctx) { return; }
    return ctx->addMesh(node, topology, num_indices, num_vertices, indices, points, normals, tangents, uv, colors);
}

fbxeAPI void fbxeAddSkin(fbxe::IContext *ctx, fbxe::Node *node, Weights4 weights[], fbxe::Node *bones[], float4x4 bindposes[], int num_bones)
{
    if (!ctx) { return; }
    return ctx->addSkin(node, weights, bones, bindposes, num_bones);
}
