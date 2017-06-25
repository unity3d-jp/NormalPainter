#include "pch.h"
#include "FbxExporter/FbxExporter.h"
using namespace mu;


static void GenerateWaveMesh(
    std::vector<int>& counts,
    std::vector<int>& indices,
    std::vector<float3> &points,
    std::vector<float2> &uv,
    float size, float height,
    const int resolution,
    float angle,
    bool triangulate = false)
{
    const int num_vertices = resolution * resolution;

    // vertices
    points.resize(num_vertices);
    uv.resize(num_vertices);
    for (int iy = 0; iy < resolution; ++iy) {
        for (int ix = 0; ix < resolution; ++ix) {
            int i = resolution*iy + ix;
            float2 pos = {
                float(ix) / float(resolution - 1) - 0.5f,
                float(iy) / float(resolution - 1) - 0.5f
            };
            float d = std::sqrt(pos.x*pos.x + pos.y*pos.y);

            float3& v = points[i];
            v.x = pos.x * size;
            v.y = std::sin(d * 10.0f + angle) * std::max<float>(1.0 - d, 0.0f) * height;
            v.z = pos.y * size;

            float2& t = uv[i];
            t.x = pos.x * 0.5 + 0.5;
            t.y = pos.y * 0.5 + 0.5;
        }
    }

    // topology
    if (triangulate) {
        int num_faces = (resolution - 1) * (resolution - 1) * 2;
        int num_indices = num_faces * 3;

        counts.resize(num_faces);
        indices.resize(num_indices);
        for (int iy = 0; iy < resolution - 1; ++iy) {
            for (int ix = 0; ix < resolution - 1; ++ix) {
                int i = (resolution - 1)*iy + ix;
                counts[i * 2 + 0] = 3;
                counts[i * 2 + 1] = 3;
                indices[i * 6 + 0] = resolution*iy + ix;
                indices[i * 6 + 1] = resolution*(iy + 1) + ix;
                indices[i * 6 + 2] = resolution*(iy + 1) + (ix + 1);
                indices[i * 6 + 3] = resolution*iy + ix;
                indices[i * 6 + 4] = resolution*(iy + 1) + (ix + 1);
                indices[i * 6 + 5] = resolution*iy + (ix + 1);
            }
        }
    }
    else {
        int num_faces = (resolution - 1) * (resolution - 1);
        int num_indices = num_faces * 4;

        counts.resize(num_faces);
        indices.resize(num_indices);
        for (int iy = 0; iy < resolution - 1; ++iy) {
            for (int ix = 0; ix < resolution - 1; ++ix) {
                int i = (resolution - 1)*iy + ix;
                counts[i] = 4;
                indices[i * 4 + 0] = resolution*iy + ix;
                indices[i * 4 + 1] = resolution*(iy + 1) + ix;
                indices[i * 4 + 2] = resolution*(iy + 1) + (ix + 1);
                indices[i * 4 + 3] = resolution*iy + (ix + 1);
            }
        }
    }
}

void GenerateCylinderMesh(
    std::vector<int>& counts,
    std::vector<int>& indices,
    std::vector<float3> &points,
    std::vector<float2> &uv,
    float radius, float height,
    int cseg, int hseg,
    bool triangulate = false)
{
    const int num_vertices = cseg * hseg;

    // vertices
    points.resize(num_vertices);
    uv.resize(points.size());
    for (int ih = 0; ih < hseg; ++ih) {
        float y = (float(ih) / float(hseg - 1)) * height;
        for (int ic = 0; ic < cseg; ++ic) {
            int i = cseg * ih + ic;
            float ang = ((360.0f / cseg) * ic) * Deg2Rad;
            float3 pos{ std::cos(ang) * radius, y, std::sin(ang) * radius };
            float2 t{ float(ic) / float(cseg - 1), float(ih) / float(hseg - 1), };

            points[i] = pos;
            uv[i] = t;
        }
    }

    // topology
    if (triangulate) {
        const int num_faces = cseg * (hseg - 1) * 2;
        const int num_indices = num_faces * 6;

        counts.resize(num_faces, 3);
        indices.resize(num_indices);
        for (int ih = 0; ih < hseg - 1; ++ih) {
            for (int ic = 0; ic < cseg; ++ic) {
                auto *dst = &indices[(ih * cseg + ic) * 6];
                dst[0] = cseg * ih + ic;
                dst[1] = cseg * (ih + 1) + ic;
                dst[2] = cseg * (ih + 1) + ((ic + 1) % cseg);

                dst[3] = cseg * ih + ic;
                dst[4] = cseg * (ih + 1) + ((ic + 1) % cseg);
                dst[5] = cseg * ih + ((ic + 1) % cseg);
            }
        }
    }
    else {
        const int num_faces = cseg * (hseg - 1);
        const int num_indices = num_faces * 4;

        counts.resize(num_faces, 4);
        indices.resize(num_indices);
        for (int ih = 0; ih < hseg - 1; ++ih) {
            for (int ic = 0; ic < cseg; ++ic) {
                auto *dst = &indices[(ih * cseg + ic) * 4];
                dst[0] = cseg * ih + ic;
                dst[1] = cseg * (ih + 1) + ic;
                dst[2] = cseg * (ih + 1) + ((ic + 1) % cseg);
                dst[3] = cseg * ih + ((ic + 1) % cseg);
            }
        }
    }
}

void GenerateCylinderMeshWithSkinning(
    std::vector<int>& counts,
    std::vector<int>& indices,
    std::vector<float3> &points,
    std::vector<float2> &uv,
    std::vector<Weights4> &weights,
    float radius, float height,
    int cseg, int hseg,
    bool triangulate = false)
{
    GenerateCylinderMesh(counts, indices, points, uv, radius, height, cseg, hseg);

    size_t n = points.size();
    weights.resize(n);
    Weights4 w;
    for (size_t i = 0; i < n; ++i) {
        const auto& p = points[i];
        w.indices[0] = std::min<int>((int)p.y, 4);
        w.weights[0] = 1.0f;
        weights[i] = w;
    }
};



void TestFBXExportMesh()
{
    fbxe::ExportOptions opt;
    auto ctx = fbxeCreateContext(&opt);

    fbxeCreateScene(ctx, "MeshExportTest");
    auto root = fbxeGetRootNode(ctx);
    auto parent = fbxeCreateNode(ctx, root, "Parent");
    fbxeSetTRS(ctx, parent, { 0.0f, 1.0f, 2.0f }, quatf::identity(), float3::one());

    {
        std::vector<int> counts;
        std::vector<int> indices;
        std::vector<float3> points;
        std::vector<float2> uv;
        GenerateWaveMesh(counts, indices, points, uv, 1.0f, 0.25f, 128, 0.0f, false);

        auto mesh = fbxeCreateNode(ctx, root, "Mesh");
        fbxeAddMesh(ctx, mesh, fbxe::Topology::Quads, indices.size(),
            points.size(), indices.data(), points.data(), nullptr, nullptr, uv.data(), nullptr);
    }

    fbxeWrite(ctx, "mesh_binary.fbx", fbxe::Format::FBXBinary);
    fbxeWrite(ctx, "mesh_ascii.fbx", fbxe::Format::FBXAscii);
    fbxeWrite(ctx, "mesh_encrypted.fbx", fbxe::Format::FBXEncrypted);
    fbxeWrite(ctx, "mesh_obj.obj", fbxe::Format::Obj);
    fbxeReleaseContext(ctx);
}

void TestFBXExportSkinnedMesh()
{
    fbxe::ExportOptions opt;
    opt.scale_factor = 2.0f;
    auto ctx = fbxeCreateContext(&opt);

    fbxeCreateScene(ctx, "SkinnedMeshExportTest");
    auto root = fbxeGetRootNode(ctx);

    const int num_bones = 5;
    fbxe::Node *bones[num_bones];
    float4x4 bindposes[num_bones];

    for (int i = 0; i < num_bones; ++i) {
        char name[128];
        sprintf(name, "Bone%d", i);
        bones[i] = fbxeCreateNode(ctx, i == 0 ? root : bones[i - 1], name);
        fbxeSetTRS(ctx, bones[i], { 0.0f, 1.0f, 0.0f }, quatf::identity(), float3::one());

        bindposes[i] = float4x4::identity();
        bindposes[i][3].y = -1.0f * i;
    }

    std::vector<int> counts;
    std::vector<int> indices;
    std::vector<float3> points;
    std::vector<float2> uv;
    std::vector<Weights4> weights;
    GenerateCylinderMeshWithSkinning(counts, indices, points, uv, weights, 0.2f, 5.0f, 32, 128, false);

    auto mesh = fbxeCreateNode(ctx, root, "SkinnedMesh");
    fbxeAddMesh(ctx, mesh,
        fbxe::Topology::Quads, indices.size(), points.size(), indices.data(), points.data(), nullptr, nullptr, uv.data(), nullptr);
    fbxeAddSkin(ctx, mesh, weights.data(), bones, bindposes, num_bones);

    fbxeWrite(ctx, "skinnedmesh_binary.fbx", fbxe::Format::FBXBinary);
    fbxeWrite(ctx, "skinnedmesh_ascii.fbx", fbxe::Format::FBXAscii);
    fbxeReleaseContext(ctx);
}
