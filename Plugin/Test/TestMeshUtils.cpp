#include "pch.h"
#include "Test.h"
#include "MeshGenerator.h"
using namespace mu;

#ifdef EnableFbxExport
#include "FbxExporter/FbxExporter.h"
#pragma comment(lib, "FbxExporterCore.lib")

void ExportFbxImpl(const char *path,
    const RawVector<int>& indices, int ngon,
    const RawVector<float3>& points, const RawVector<float3>& normals, const RawVector<float4>& tangents,
    const RawVector<float2>& uv, const RawVector<float4>& colors)
{
    fbxe::ExportOptions opt;
    auto ctx = fbxeCreateContext(&opt);
    fbxeCreateScene(ctx, "TestScene");
    auto node = fbxeCreateNode(ctx, nullptr, "Mesh");
    fbxeAddMesh(ctx, node, points.size(),
        (fbxe::float3*)points.data(), (fbxe::float3*)normals.data(), (fbxe::float4*)tangents.data(),
        (fbxe::float2*)uv.data(), (fbxe::float4*)colors.data());
    fbxeAddMeshSubmesh(ctx, node, (fbxe::Topology)(ngon-1), indices.size(), indices.data(), -1);
    fbxeWrite(ctx, path, fbxe::Format::FbxAscii);
    fbxeReleaseContext(ctx);
}
#define ExportFbx(...) ExportFbxImpl(__VA_ARGS__)
#else // EnableFbxExport
#define ExportFbx(...)
#endif // EnableFbxExport


template<class T> struct StrideIterator
{
    T *data;
    size_t stride;
};


void Test_IndexedArrays()
{
    std::vector<uint16_t> indices16 = { 0,1,0,2,1,3,2 };
    std::vector<uint32_t> indices32 = { 0,1,2,1,2,3 };
    std::vector<float> values = { 0.0f, 10.0f, 20.0f, 30.0f };

    IIArray<uint16_t, float> iia1 = { indices16, values };
    IIArray<uint32_t, float> iia2 = { indices32, values };

    Print("    iia1: ");
    for(auto& v : iia1) { Print("%.f ", v); }
    Print("\n");

    Print("    iia2: ");
    for (auto& v : iia2) { Print("%.f ", v); }
    Print("\n");
}
RegisterTestEntry(Test_IndexedArrays)


void TestMeshRefiner()
{
    RawVector<float3> points = {
        { 0.0f, 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 2.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f, 1.0f },{ 2.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 2.0f },{ 1.0f, 0.0f, 2.0f },{ 2.0f, 1.0f, 2.0f },
    };
    RawVector<float2> uv = {
        { 0.0f, 0.0f },{ 0.5f, 0.0f },{ 1.0f, 0.0f },
        { 0.0f, 0.5f },{ 0.5f, 0.5f },{ 1.0f, 0.5f },
        { 0.0f, 1.0f },{ 0.5f, 1.0f },{ 1.0f, 1.0f },
    };
    RawVector<int> indices = {
        0, 1, 4, 3,
        1, 2, 5, 4,
        3, 4, 7, 6,
        4, 5, 8, 7,
    };
    RawVector<int> counts = {
        4, 4, 4, 4
    };
    RawVector<int> materialIDs = {
        0, 1, 2, 3
    };

    RawVector<float2> uv_flattened(indices.size());
    for (int i = 0; i < indices.size(); ++i) {
        uv_flattened[i] = uv[indices[i]];
    }

    mu::MeshRefiner refiner;
    refiner.prepare(counts, indices, points);
    refiner.uv = uv_flattened;
    refiner.split_unit = 8;
    refiner.genNormalsWithSmoothAngle(40.0f, false);
    refiner.genTangents();
    refiner.refine(false);
    refiner.genSubmesh(materialIDs);
}
RegisterTestEntry(TestMeshRefiner)


void TestNormalsAndTangents()
{
    RawVector<int> indices, counts;
    RawVector<float3> points;
    RawVector<float2> uv;
    GenerateWaveMesh(counts, indices, points, uv, 10.0f, 0.25f, 250, 0.0f, true);

    int num_try = 1;
    int num_points = (int)points.size();
    int num_triangles = (int)indices.size() / 3;
    RawVector<float3> points_f;
    RawVector<float2> uv_f;
    RawVector<float3> normals[6];
    RawVector<float4> tangents[7];
    RawVector<float> psoa[9], usoa[6];


    // generate flattened data
    points_f.resize(indices.size());
    uv_f.resize(indices.size());
    for (size_t i = 0; i < indices.size(); ++i) {
        points_f[i] = points[indices[i]];
        uv_f[i] = uv[indices[i]];
    }

    // generate soa data
    for (auto& v : normals) { v.resize(points.size()); }
    for (auto& v : tangents) { v.resize(points.size()); }
    for (auto& v : psoa) { v.resize(num_triangles); }
    for (auto& v : usoa) { v.resize(num_triangles); }
    for (int ti = 0; ti < num_triangles; ++ti) {
        float3 p[3] = {
            points[indices[ti * 3 + 0]],
            points[indices[ti * 3 + 1]],
            points[indices[ti * 3 + 2]],
        };
        float2 u[3] = {
            uv[indices[ti * 3 + 0]],
            uv[indices[ti * 3 + 1]],
            uv[indices[ti * 3 + 2]],
        };

        for (int i = 0; i < 9; ++i) {
            psoa[i][ti] = ((float*)p)[i];
        }
        for (int i = 0; i < 6; ++i) {
            usoa[i][ti] = ((float*)u)[i];
        }
    }


    Print(
        "    num_vertices: %d\n"
        "    num_triangles: %d\n"
        ,
        (int)points.size(),
        (int)indices.size() / 3);

#define SoAPointsArgs\
    psoa[0].data(), psoa[1].data(), psoa[2].data(),\
    psoa[3].data(), psoa[4].data(), psoa[5].data(),\
    psoa[6].data(), psoa[7].data(), psoa[8].data()

#define SoAUVArgs\
    usoa[0].data(), usoa[1].data(),\
    usoa[2].data(), usoa[3].data(),\
    usoa[4].data(), usoa[5].data()

    {
        auto s1b = Now();
        for (int i = 0; i < num_try; ++i)
            GenerateNormalsTriangleIndexed_Generic(normals[0].data(), points.data(), indices.data(), num_triangles, num_points);
        auto s1e = Now();

        auto s2b = Now();
#ifdef muSIMD_GenerateNormalsTriangleIndexed
        for (int i = 0; i < num_try; ++i)
            GenerateNormalsTriangleIndexed_ISPC(normals[1].data(), points.data(), indices.data(), num_triangles, num_points);
#endif
        auto s2e = Now();

        auto s3b = Now();
        for (int i = 0; i < num_try; ++i)
            GenerateNormalsTriangleFlattened_Generic(normals[2].data(), points_f.data(), indices.data(), num_triangles, num_points);
        auto s3e = Now();

        auto s4b = Now();
#ifdef muSIMD_GenerateNormalsTriangleFlattened
        for (int i = 0; i < num_try; ++i)
            GenerateNormalsTriangleFlattened_ISPC(normals[3].data(), points_f.data(), indices.data(), num_triangles, num_points);
#endif
        auto s4e = Now();

        auto s5b = Now();
        for (int i = 0; i < num_try; ++i)
            GenerateNormalsTriangleSoA_Generic(normals[4].data(), SoAPointsArgs, indices.data(), num_triangles, num_points);
        auto s5e = Now();

        auto s6b = Now();
#ifdef muSIMD_GenerateNormalsTriangleSoA
        for (int i = 0; i < num_try; ++i)
            GenerateNormalsTriangleSoA_ISPC(normals[5].data(), SoAPointsArgs, indices.data(), num_triangles, num_points);
#endif
        auto s6e = Now();

        Print(
            "    GenerateNormals indexed C++: %.2fms\n"
            "    GenerateNormals indexed ISPC: %.2fms\n"
            "    GenerateNormals flattened C++: %.2fms\n"
            "    GenerateNormals flattened ISPC: %.2fms\n"
            "    GenerateNormals SoA C++: %.2fms\n"
            "    GenerateNormals SoA ISPC: %.2fms\n"
            ,
            NS2MS(s1e - s1b) / num_try,
            NS2MS(s2e - s2b) / num_try,
            NS2MS(s3e - s3b) / num_try,
            NS2MS(s4e - s4b) / num_try,
            NS2MS(s5e - s5b) / num_try,
            NS2MS(s6e - s6b) / num_try);
    }

    {
        auto s1b = Now();
        for (int i = 0; i < num_try; ++i)
            GenerateTangentsTriangleIndexed_Generic(tangents[0].data(),
                points.data(), uv.data(), normals[0].data(), indices.data(), num_triangles, num_points);
        auto s1e = Now();

        auto s2b = Now();
#ifdef muSIMD_GenerateTangentsTriangleIndexed
        for (int i = 0; i < num_try; ++i)
            GenerateTangentsTriangleIndexed_ISPC(tangents[1].data(),
                points.data(), uv.data(), normals[1].data(), indices.data(), num_triangles, num_points);
#endif
        auto s2e = Now();

        auto s3b = Now();
        for (int i = 0; i < num_try; ++i)
            GenerateTangentsTriangleFlattened_Generic(tangents[2].data(),
                points_f.data(), uv_f.data(), normals[2].data(), indices.data(), num_triangles, num_points);
        auto s3e = Now();

        auto s4b = Now();
#ifdef muSIMD_GenerateTangentsTriangleFlattened
        for (int i = 0; i < num_try; ++i)
            GenerateTangentsTriangleFlattened_ISPC(tangents[3].data(),
                points_f.data(), uv_f.data(), normals[3].data(), indices.data(), num_triangles, num_points);
#endif
        auto s4e = Now();

        auto s5b = Now();
        for (int i = 0; i < num_try; ++i)
            GenerateTangentsTriangleSoA_Generic(tangents[4].data(),
                SoAPointsArgs, SoAUVArgs, normals[4].data(), indices.data(), num_triangles, num_points);
        auto s5e = Now();

        auto s6b = Now();
#ifdef muSIMD_GenerateTangentsTriangleSoA
        for (int i = 0; i < num_try; ++i)
            GenerateTangentsTriangleSoA_ISPC(tangents[5].data(),
                SoAPointsArgs, SoAUVArgs, normals[5].data(), indices.data(), num_triangles, num_points);
#endif
        auto s6e = Now();

        Print(
            "    GenerateTangents indexed C++: %.2fms\n"
            "    GenerateTangents indexed ISPC: %.2fms\n"
            "    GenerateTangents flattened C++: %.2fms\n"
            "    GenerateTangents flattened ISPC: %.2fms\n"
            "    GenerateTangents SoA C++: %.2fms\n"
            "    GenerateTangents SoA ISPC: %.2fms\n"
            ,
            NS2MS(s1e - s1b) / num_try,
            NS2MS(s2e - s2b) / num_try,
            NS2MS(s3e - s3b) / num_try,
            NS2MS(s4e - s4b) / num_try,
            NS2MS(s5e - s5b) / num_try,
            NS2MS(s6e - s6b) / num_try);
    }
    {
        auto unity_exe = GetModule("Unity.exe");
        if (unity_exe) {
            InitializeSymbols();

            using CalculateTangentsT = void(*)(
                StrideIterator<float3> vertices, StrideIterator<float3> normals, StrideIterator<float2> uv, const int *indices,
                int num_vertices, int num_triangles, StrideIterator<float4> dst);

            auto CalculateTangents = (CalculateTangentsT)FindSymbolByName("?CalculateTangents@@YAXV?$StrideIterator@VVector3f@@@@0V?$StrideIterator@VVector2f@@@@PEBIHHV?$StrideIterator@VVector4f@@@@@Z");
            if (CalculateTangents) {
                StrideIterator<float3> viter = { points.data(), sizeof(float3) };
                StrideIterator<float3> niter = { normals[0].data(), sizeof(float3) };
                StrideIterator<float2> uiter = { uv.data(), sizeof(float2) };
                StrideIterator<float4> titer = { tangents[6].data(), sizeof(float4) };

                auto s7b = Now();
                for (int i = 0; i < num_try; ++i)
                    CalculateTangents(viter, niter, uiter, indices.data(), num_points, num_triangles, titer);
                auto s7e = Now();
                Print("    GenerateTangents Unity: %.2fms\n", NS2MS(s7e - s7b) / num_try);
            }
        }
        else {
            tangents[6].clear();
        }
    }

    for (int i = 1; i < countof(normals); ++i) {
        if (normals[i].size() != normals[0].size()) { continue; }
        if (!NearEqual(normals[0].data(), normals[i].data(), normals[0].size(), 0.01f)) {
            Print("    *** validation failed: normals %d ***\n", i);
        }
    }
    for (int i = 1; i < countof(tangents); ++i) {
        if (tangents[i].size() != tangents[0].size()) { continue; }
        if (!NearEqual(tangents[0].data(), tangents[i].data(), tangents[0].size(), 0.01f)) {
            Print("    *** validation failed: tangents %d ***\n", i);
        }
    }

    //ExportFbx("Wave_IndexedCpp.fbx", indices, 3, points, normals[0], tangents[0], uv, {});
    //ExportFbx("Wave_IndexedISPC.fbx", indices, 3, points, normals[1], tangents[1], uv, {});
    //ExportFbx("Wave_FlattenedCpp.fbx", indices, 3, points, normals[2], tangents[2], uv, {});
    //ExportFbx("Wave_FlattenedISPC.fbx", indices, 3, points, normals[3], tangents[3], uv, {});
    //ExportFbx("Wave_SoACpp.fbx", indices, 3, points, normals[4], tangents[4], uv, {});
    //ExportFbx("Wave_SoAISPC.fbx", indices, 3, points, normals[5], tangents[5], uv, {});
    //if (!tangents[6].empty()) {
    //    ExportFbx("Wave_Unity.fbx", indices, 3, points, normals[0], tangents[6], uv, {});
    //}

#undef SoAUVArgs
#undef SoAPointsArgs
}
RegisterTestEntry(TestNormalsAndTangents)

#ifdef _WIN32
testExport void TestNormalsAndTangentsEx()
{
    TestNormalsAndTangents();
}
#endif


void TestMatrixSwapHandedness()
{
    quatf rot1 = rotate(normalize(float3{0.15f, 0.3f, 0.6f}), 60.0f);
    quatf rot2 = swap_handedness(rot1);
    float4x4 mat1 = to_float4x4(rot1);
    float4x4 mat2 = to_float4x4(rot2);
    float4x4 mat3 = swap_handedness(mat1);
    float4x4 imat1 = invert(mat1);
    float4x4 imat2 = invert(mat2);
    float4x4 imat3 = swap_handedness(imat1);

    bool r1 = near_equal(mat2, mat3);
    bool r2 = near_equal(imat2, imat3);
    Print("    %d, %d\n", (int)r1, (int)r2);
}
RegisterTestEntry(TestMatrixSwapHandedness)


void TestMulPoints()
{
    const int num_data = 65536;
    const int num_try = 128;

    RawVector<float3> src, dst1, dst2;
    src.resize(num_data);
    dst1.resize(num_data);
    dst2.resize(num_data);

    float4x4 matrix = transform({ 1.0f, 2.0f, 4.0f }, rotateY(45.0f), {2.0f, 2.0f, 2.0f});

    for (int i = 0; i < num_data; ++i) {
        src[i] = { (float)i*0.1f, (float)i*0.05f, (float)i*0.025f };
    }


    auto s1_begin = Now();
    for (int i = 0; i < num_try; ++i) {
        MulPoints_Generic(matrix, src.data(), dst1.data(), num_data);
    }
    auto s1_end = Now();

    auto s2_begin = Now();
#ifdef muSIMD_MulPoints3
    for (int i = 0; i < num_try; ++i) {
        MulPoints_ISPC(matrix, src.data(), dst2.data(), num_data);
    }
#endif
    auto s2_end = Now();

    int eq1 = NearEqual(dst1.data(), dst2.data(), num_data);


    auto s3_begin = Now();
    for (int i = 0; i < num_try; ++i) {
        MulVectors_Generic(matrix, src.data(), dst1.data(), num_data);
    }
    auto s3_end = Now();

    auto s4_begin = Now();
#ifdef muSIMD_MulVectors3
    for (int i = 0; i < num_try; ++i) {
        MulVectors_ISPC(matrix, src.data(), dst2.data(), num_data);
    }
#endif
    auto s4_end = Now();

    int eq2 = NearEqual(dst1.data(), dst2.data(), num_data);


    Print(
        "    num_data: %d\n"
        "    num_try: %d\n"
        "    MulPoints  : Generic %.2fms, ISPC %.2fms\n"
        "    MulVectors : Generic %.2fms, ISPC %.2fms\n"
        "    %d %d\n",
        num_data,
        num_try,
        NS2MS(s1_end - s1_begin),
        NS2MS(s2_end - s2_begin),
        NS2MS(s3_end - s3_begin),
        NS2MS(s4_end - s4_begin),
        eq1, eq2);
}
RegisterTestEntry(TestMulPoints)


void TestRayTrianglesIntersection()
{
    RawVector<float3> vertices;
    RawVector<float3> vertices_flattened;
    RawVector<float> v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z;
    RawVector<int> indices;

    const int seg = 70;
    const int num_try = 4000;

    vertices.resize(seg * seg);
    for (int yi = 0; yi < seg; ++yi) {
        for (int xi = 0; xi < seg; ++xi) {
            float3 v = { xi - (float)seg*0.5f, 0.0f, yi - (float)seg*0.5f};
            vertices[yi * seg + xi] = v;
        }
    }

    indices.resize((seg - 1) * (seg - 1) * 6);
    for (int yi = 0; yi < seg - 1; ++yi) {
        for (int xi = 0; xi < seg - 1; ++xi) {
            int i = yi * (seg - 1) + xi;
            indices[i * 6 + 0] = seg* yi + xi;
            indices[i * 6 + 1] = seg* (yi + 1) + xi;
            indices[i * 6 + 2] = seg* (yi + 1) + (xi + 1);
            indices[i * 6 + 3] = seg* yi + xi;
            indices[i * 6 + 4] = seg* (yi + 1) + (xi + 1);
            indices[i * 6 + 5] = seg * yi + (xi + 1);
        }
    }

    int num_triangles = indices.size() / 3;
    vertices_flattened.resize(indices.size());
    v1x.resize(num_triangles); v1y.resize(num_triangles); v1z.resize(num_triangles);
    v2x.resize(num_triangles); v2y.resize(num_triangles); v2z.resize(num_triangles);
    v3x.resize(num_triangles); v3y.resize(num_triangles); v3z.resize(num_triangles);
    for (int ti = 0; ti < num_triangles; ++ti) {
        auto p1 = vertices[indices[ti * 3 + 0]];
        auto p2 = vertices[indices[ti * 3 + 1]];
        auto p3 = vertices[indices[ti * 3 + 2]];
        vertices_flattened[ti * 3 + 0] = p1;
        vertices_flattened[ti * 3 + 1] = p2;
        vertices_flattened[ti * 3 + 2] = p3;
        v1x[ti] = p1.x; v1y[ti] = p1.y; v1z[ti] = p1.z;
        v2x[ti] = p2.x; v2y[ti] = p2.y; v2z[ti] = p2.z;
        v3x[ti] = p3.x; v3y[ti] = p3.y; v3z[ti] = p3.z;
    }

    float3 ray_pos = { 0.0f, 10.0f, 0.0f };
    float3 ray_dir = { 0.0f, -1.0f, 0.0f };

    int num_hits;
    int tindex;
    float distance;

    auto print = [&]() {
        Print("    %d hits: index %d, distance %f\n",
            num_hits, tindex, distance);
    };

    auto s1_begin = Now();
    for (int i = 0; i < num_try; ++i) {
        num_hits = RayTrianglesIntersectionIndexed_Generic(ray_pos, ray_dir, vertices.data(),
            indices.data(), num_triangles, tindex, distance);
    }
    auto s1_end = Now();

    print();

    auto s2_begin = Now();
#ifdef muSIMD_RayTrianglesIntersectionIndexed
    for (int i = 0; i < num_try; ++i) {
        num_hits = RayTrianglesIntersectionIndexed_ISPC(ray_pos, ray_dir, vertices.data(),
            indices.data(), num_triangles, tindex, distance);
    }
#endif
    auto s2_end = Now();

    print();

    auto s3_begin = Now();
    for (int i = 0; i < num_try; ++i) {
        num_hits = RayTrianglesIntersectionFlattened_Generic(ray_pos, ray_dir, vertices_flattened.data(), num_triangles, tindex, distance);
    }
    auto s3_end = Now();

    print();

    auto s4_begin = Now();
#ifdef muSIMD_RayTrianglesIntersectionFlattened
    for (int i = 0; i < num_try; ++i) {
        num_hits = RayTrianglesIntersectionFlattened_ISPC(ray_pos, ray_dir, vertices_flattened.data(), num_triangles, tindex, distance);
    }
#endif
    auto s4_end = Now();

    print();

    auto s5_begin = Now();
    for (int i = 0; i < num_try; ++i) {
        num_hits = RayTrianglesIntersectionSoA_Generic(ray_pos, ray_dir,
            v1x.data(), v1y.data(), v1z.data(),
            v2x.data(), v2y.data(), v2z.data(),
            v3x.data(), v3y.data(), v3z.data(), num_triangles, tindex, distance);
    }
    auto s5_end = Now();

    print();

    auto s6_begin = Now();
#ifdef muSIMD_RayTrianglesIntersectionSoA
    for (int i = 0; i < num_try; ++i) {
        num_hits = RayTrianglesIntersectionSoA_ISPC(ray_pos, ray_dir,
            v1x.data(), v1y.data(), v1z.data(),
            v2x.data(), v2y.data(), v2z.data(),
            v3x.data(), v3y.data(), v3z.data(), num_triangles, tindex, distance);
    }
#endif
    auto s6_end = Now();

    print();

    Print(
        "    triangle count: %d\n"
        "    ray count: %d\n"
        "    RayTrianglesIntersection (indexed):   Generic %.2fms, ISPC %.2fms\n"
        "    RayTrianglesIntersection (flattened): Generic %.2fms, ISPC %.2fms\n"
        "    RayTrianglesIntersection (SoA):       Generic %.2fms, ISPC %.2fms\n",
        num_triangles,
        num_try,
        NS2MS(s1_end - s1_begin),
        NS2MS(s2_end - s2_begin),
        NS2MS(s3_end - s3_begin),
        NS2MS(s4_end - s4_begin),
        NS2MS(s5_end - s5_begin),
        NS2MS(s6_end - s6_begin));
}
RegisterTestEntry(TestRayTrianglesIntersection)


void TestPolygonInside()
{
    const int num_try = 100;
    const int ngon = 80000;
    RawVector<float2> poly;
    RawVector<float> polyx, polyy;

    poly.resize(ngon); polyx.resize(ngon); polyy.resize(ngon);
    for (int i = 0; i < ngon; ++i) {
        float a = (360.0f / ngon) * i * Deg2Rad;
        poly[i] = { std::sin(a), std::cos(a) };
        polyx[i] = poly[i].x;
        polyy[i] = poly[i].y;
    }


    float2 points[]{
        float2{ 0.0f, 0.0f },
        float2{ 0.1f, 0.1f },
        float2{ 0.2f, 0.2f },
        float2{ 0.3f, 0.3f },
        float2{ 0.4f, 0.4f },
        float2{ 0.5f, 0.5f },
    };
    float2 pmin, pmax;
    int num_inside = 0;

    auto s1_begin = Now();
    num_inside = 0;
    for (int ti = 0; ti < num_try; ++ti) {
        MinMax_Generic(poly.data(), ngon, pmin, pmax);
        for (int pi = 0; pi < countof(points); ++pi) {
            if (PolyInside_Generic(poly.data(), ngon, pmin, pmax, points[pi])) {
                ++num_inside;
            }
        }
    }
    auto s1_end = Now();

    Print("    Generic: %d (%.2fms)\n", num_inside, NS2MS(s1_end - s1_begin));

    auto s2_begin = Now();
    num_inside = 0;
    pmin = pmax = float2::zero();
    for (int ti = 0; ti < num_try; ++ti) {
        MinMax_Generic(poly.data(), ngon, pmin, pmax);
        for (int pi = 0; pi < countof(points); ++pi) {
            if (PolyInside_Generic(polyx.data(), polyy.data(), ngon, pmin, pmax, points[pi])) {
                ++num_inside;
            }
        }
    }
    auto s2_end = Now();

    Print("    Generic SoA: %d (%.2fms)\n", num_inside, NS2MS(s2_end - s2_begin));

    auto s3_begin = Now();
#ifdef muSIMD_PolyInside
    num_inside = 0;
    pmin = pmax = float2::zero();
    for (int ti = 0; ti < num_try; ++ti) {
        MinMax_ISPC(poly.data(), ngon, pmin, pmax);
        for (int pi = 0; pi < countof(points); ++pi) {
            if (PolyInside_ISPC(poly.data(), ngon, pmin, pmax, points[pi])) {
                ++num_inside;
            }
        }
    }
#endif
    auto s3_end = Now();

    Print("    ISPC: %d (%.2fms)\n", num_inside, NS2MS(s3_end - s3_begin));

    auto s4_begin = Now();
#ifdef muSIMD_PolyInsideSoA
    num_inside = 0;
    pmin = pmax = float2::zero();
    for (int ti = 0; ti < num_try; ++ti) {
        MinMax_ISPC(poly.data(), ngon, pmin, pmax);
        for (int pi = 0; pi < countof(points); ++pi) {
            if (PolyInside_ISPC(polyx.data(), polyy.data(), ngon, pmin, pmax, points[pi])) {
                ++num_inside;
            }
        }
    }
#endif
    auto s4_end = Now();

    Print("    ISPC SoA: %d (%.2fms)\n", num_inside, NS2MS(s4_end - s4_begin));
    Print("\n");
}
RegisterTestEntry(TestPolygonInside)


void TestEdge()
{
    {
        float3 points[] = {
            { 0.0f, 0.5f, 0.0f },
            { 1.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f },
            { -1.0f, 0.0f, 0.0f },
        };
        int indices[] = {
            0, 1, 2,
            0, 2, 3,
            0, 3, 1,
        };

        ConnectionData connection;
        connection.buildConnection(indices, 3, points);

        for (int vi = 0; vi < 4; ++vi) {
            bool is_edge = OnEdge(indices, 3, points, connection, vi);
            Print("    IsEdge(): %d %d\n", vi, (int)is_edge);
        }

        RawVector<int> edges;
        int vi[] = { 1 };
        SelectEdge(indices, 3, points, vi, [&](int vi) { edges.push_back(vi); });

        Print("    SelectEdge (triangles):");
        for (int e : edges) {
            Print(" %d", e);
        }
        Print("\n");
    }
    Print("\n");

    {
        float3 points[4 * 4];
        RawVector<int> indices(3 * 3 * 4);

        for (int yi = 0; yi < 4; ++yi) {
            for (int xi = 0; xi < 4; ++xi) {
                points[4 * yi + xi] = {(float)xi, 0.0f, (float)yi};
            }
        }
        for (int yi = 0; yi < 3; ++yi) {
            for (int xi = 0; xi < 3; ++xi) {
                int i = 3 * yi + xi;
                indices[4 * i + 0] = 4 * (yi + 0) + (xi + 0);
                indices[4 * i + 1] = 4 * (yi + 0) + (xi + 1);
                indices[4 * i + 2] = 4 * (yi + 1) + (xi + 1);
                indices[4 * i + 3] = 4 * (yi + 1) + (xi + 0);
            }
        }
        indices.erase(indices.begin() + 4 * 4, indices.begin() + 5 * 4);

        int counts[8] = { 4,4,4,4,4,4,4,4 };
        int offsets[8] = { 0,4,8,12,16,20,24,28 };
        for (int i = 0; i < 8; ++i) {
            counts[i] = 4;
            offsets[i] = i * 4;
        }

        ConnectionData connection;
        connection.buildConnection(indices, counts, offsets, points);

        for (int vi = 0; vi < 16; ++vi) {
            bool is_edge = OnEdge(indices, 4, points, connection, vi);
            Print("    IsEdge(): %d %d\n", vi, (int)is_edge);
        }

        RawVector<int> edges;
        int vi[] = { 1 };

        SelectEdge(indices, counts, offsets, points, vi, [&](int vi) { edges.push_back(vi); });
        Print("    SelectEdge (quads):");
        for (int e : edges) {
            Print(" %d", e);
        }
        Print("\n");

        edges.clear();
        vi[0] = 5;
        SelectEdge(indices, counts, offsets, points, vi, [&](int vi) { edges.push_back(vi); });
        Print("    SelectEdge (quads):");
        for (int e : edges) {
            Print(" %d", e);
        }
        Print("\n");
    }
}
RegisterTestEntry(TestEdge)
