#include "pch.h"
#include "muMath.h"
#include "muSIMD.h"
#include "muRawVector.h"

namespace mu {


#ifdef muEnableISPC
#include "MeshUtilsCore.h"
#include "MeshUtilsCore2.h"

#ifdef muEnableHalf
void FloatToHalf_ISPC(half *dst, const float *src, size_t num)
{
    ispc::FloatToHalf((uint16_t*)dst, src, (int)num);
}
void HalfToFloat_ISPC(float *dst, const half *src, size_t num)
{
    ispc::HalfToFloat(dst, (const uint16_t*)src, (int)num);
}
#endif // muEnableHalf

void InvertX_ISPC(float3 *dst, size_t num)
{
    ispc::InvertXF3((ispc::float3*)dst, (int)num);
}
void InvertX_ISPC(float4 *dst, size_t num)
{
    ispc::InvertXF4((ispc::float4*)dst, (int)num);
}

void Scale_ISPC(float *dst, float s, size_t num)
{
    ispc::ScaleF((float*)dst, s, (int)num * 1);
}
void Scale_ISPC(float3 *dst, float s, size_t num)
{
    ispc::ScaleF((float*)dst, s, (int)num * 3);
}

void Normalize_ISPC(float3 *dst, size_t num)
{
    ispc::Normalize((ispc::float3*)dst, (int)num);
}

void GenerateNormals_ISPC(
    float3 *dst, const float3 *p,
    const int *counts, const int *offsets, const int *indices, size_t num_points, size_t num_faces)
{
    memset(dst, 0, sizeof(float3)*num_points);

    for (size_t fi = 0; fi < num_faces; ++fi)
    {
        int count = counts[fi];
        const int *face = &indices[offsets[fi]];
        int i0 = face[0];
        int i1 = face[1];
        int i2 = face[2];
        float3 p0 = p[i0];
        float3 p1 = p[i1];
        float3 p2 = p[i2];
        float3 n = cross(p1 - p0, p2 - p0);
        for (int ci = 0; ci < count; ++ci) {
            dst[face[ci]] += n;
        }
    }

    ispc::Normalize((ispc::float3*)dst, (int)num_points);
}

void Lerp_ISPC(float *dst, const float *src1, const float *src2, size_t num, float w)
{
    ispc::Lerp(dst, src1, src2, (int)num, w);
}

void MinMax_ISPC(const float2 *src, size_t num, float2& dst_min, float2& dst_max)
{
    if (num == 0) { return; }
    ispc::MinMax2((ispc::float2*)src, (int)num, (ispc::float2&)dst_min, (ispc::float2&)dst_max);
}
void MinMax_ISPC(const float3 *src, size_t num, float3& dst_min, float3& dst_max)
{
    if (num == 0) { return; }
    ispc::MinMax3((ispc::float3*)src, (int)num, (ispc::float3&)dst_min, (ispc::float3&)dst_max);
}

bool NearEqual_ISPC(const float *src1, const float *src2, size_t num, float eps)
{
    return ispc::NearEqual(src1, src2, (int)num, eps);
}

void MulPoints_ISPC(const float4x4& m, const float3 src[], float3 dst[], size_t num_data)
{
    ispc::MulPoints3((ispc::float4x4&)m, (ispc::float3*)src, (ispc::float3*)dst, (int)num_data);
}
void MulVectors_ISPC(const float4x4& m, const float3 src[], float3 dst[], size_t num_data)
{
    ispc::MulVectors3((ispc::float4x4&)m, (ispc::float3*)src, (ispc::float3*)dst, (int)num_data);
}

int RayTrianglesIntersectionIndexed_ISPC(
    float3 pos, float3 dir, const float3 *vertices, const int *indices, int num_triangles, int& tindex, float& distance)
{
    return ispc::RayTrianglesIntersectionIndexed(
        (ispc::float3&)pos, (ispc::float3&)dir, (ispc::float3*)vertices, indices, num_triangles, tindex, distance);
}
int RayTrianglesIntersectionFlattened_ISPC(
    float3 pos, float3 dir, const float3 *vertices, int num_triangles, int& tindex, float& distance)
{
    return ispc::RayTrianglesIntersectionFlattened(
        (ispc::float3&)pos, (ispc::float3&)dir, (ispc::float3*)vertices, num_triangles, tindex, distance);
}
int RayTrianglesIntersectionSoA_ISPC(float3 pos, float3 dir,
    const float *v1x, const float *v1y, const float *v1z,
    const float *v2x, const float *v2y, const float *v2z,
    const float *v3x, const float *v3y, const float *v3z,
    int num_triangles, int& tindex, float& distance)
{
    return ispc::RayTrianglesIntersectionSoA(
        (ispc::float3&)pos, (ispc::float3&)dir, v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z, num_triangles, tindex, distance);
}


bool PolyInside_ISPC(const float px[], const float py[], int ngon, const float2 minp, const float2 maxp, const float2 pos)
{
    const int MaxXC = 64;
    float xc[MaxXC];

    int c = ispc::PolyInsideSoAImpl(px, py, ngon, (ispc::float2&)minp, (ispc::float2&)maxp, (ispc::float2&)pos, xc, MaxXC);
    std::sort(xc, xc + c);
    for (int i = 0; i < c; i += 2) {
        if (pos.x >= xc[i] && pos.x < xc[i + 1]) {
            return true;
        }
    }
    return false;
}

bool PolyInside_ISPC(const float2 poly[], int ngon, const float2 minp, const float2 maxp, const float2 pos)
{
    const int MaxXC = 64;
    float xc[MaxXC];

    int c = ispc::PolyInsideImpl((ispc::float2*)poly, ngon, (ispc::float2&)minp, (ispc::float2&)maxp, (ispc::float2&)pos, xc, MaxXC);
    std::sort(xc, xc + c);
    for (int i = 0; i < c; i += 2) {
        if (pos.x >= xc[i] && pos.x < xc[i + 1]) {
            return true;
        }
    }
    return false;
}

bool PolyInside_ISPC(const float2 poly[], int ngon, const float2 pos)
{
    float2 minp, maxp;
    MinMax(poly, ngon, minp, maxp);
    return PolyInside_ISPC(poly, ngon, minp, maxp, pos);
}

void GenerateNormalsTriangleIndexed_ISPC(float3 *dst,
    const float3 *vertices, const int *indices, int num_triangles, int num_vertices)
{
    memset(dst, 0, sizeof(float3)*num_vertices);
    ispc::GenerateNormalsTriangleIndexed((ispc::float3*)dst, (ispc::float3*)vertices, indices, num_triangles, num_vertices);
}

void GenerateNormalsTriangleFlattened_ISPC(float3 *dst,
    const float3 *vertices, const int *indices, int num_triangles, int num_vertices)
{
    memset(dst, 0, sizeof(float3)*num_vertices);
    ispc::GenerateNormalsTriangleFlattened((ispc::float3*)dst, (ispc::float3*)vertices, indices, num_triangles, num_vertices);
}

void GenerateNormalsTriangleSoA_ISPC(float3 *dst,
    const float *v1x, const float *v1y, const float *v1z,
    const float *v2x, const float *v2y, const float *v2z,
    const float *v3x, const float *v3y, const float *v3z,
    const int *indices, int num_triangles, int num_vertices)
{
    memset(dst, 0, sizeof(float3)*num_vertices);
    ispc::GenerateNormalsTriangleSoA((ispc::float3*)dst,
        v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z,
        indices, num_triangles, num_vertices);
}

void GenerateTangentsTriangleIndexed_ISPC(float4 *dst,
    const float3 *vertices, const float2 *uv, const float3 *normals, const int *indices, int num_triangles, int num_vertices)
{
    RawVector<float3> tmp_tangents, tmp_binormals;
    tmp_tangents.resize_with_zeroclear(num_vertices);
    tmp_binormals.resize_with_zeroclear(num_vertices);

    ispc::GenerateTangentsIndexed((ispc::float4*)dst,
        (ispc::float3*)vertices, (ispc::float2*)uv, (ispc::float3*)normals, indices,
        num_triangles, num_vertices,
        (ispc::float3*)tmp_tangents.data(),
        (ispc::float3*)tmp_binormals.data());
}

void GenerateTangentsTriangleFlattened_ISPC(float4 *dst,
    const float3 *vertices, const float2 *uv, const float3 *normals, const int *indices, int num_triangles, int num_vertices)
{
    RawVector<float3> tmp_tangents, tmp_binormals;
    tmp_tangents.resize_with_zeroclear(num_vertices);
    tmp_binormals.resize_with_zeroclear(num_vertices);

    ispc::GenerateTangentsFlattened((ispc::float4*)dst,
        (ispc::float3*)vertices, (ispc::float2*)uv, (ispc::float3*)normals, indices,
        num_triangles, num_vertices,
        (ispc::float3*)tmp_tangents.data(),
        (ispc::float3*)tmp_binormals.data());
}

void GenerateTangentsTriangleSoA_ISPC(float4 *dst,
    const float *v1x, const float *v1y, const float *v1z,
    const float *v2x, const float *v2y, const float *v2z,
    const float *v3x, const float *v3y, const float *v3z,
    const float *u1x, const float *u1y,
    const float *u2x, const float *u2y,
    const float *u3x, const float *u3y,
    const float3 *normals,
    const int *indices, int num_triangles, int num_vertices)
{
    RawVector<float3> tmp_tangents, tmp_binormals;
    tmp_tangents.resize_with_zeroclear(num_vertices);
    tmp_binormals.resize_with_zeroclear(num_vertices);

    ispc::GenerateTangentsSoA(
        (ispc::float4*)dst,
        v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z,
        u1x, u1y, u2x, u2y, u3x, u3y,
        (ispc::float3*)normals,
        indices,
        num_triangles, num_vertices,
        (ispc::float3*)tmp_tangents.data(),
        (ispc::float3*)tmp_binormals.data());
}
#endif



#ifdef muEnableISPC
    #define Forward(Name, ...) Name##_ISPC(__VA_ARGS__)
#else
    #define Forward(Name, ...) Name##_Generic(__VA_ARGS__)
#endif

#ifdef muEnableHalf
void FloatToHalf(half *dst, const float *src, size_t num)
{
    Forward(FloatToHalf, dst, src, num);
}
void HalfToFloat(float *dst, const half *src, size_t num)
{
    Forward(HalfToFloat, dst, src, num);
}
#endif // muEnableHalf

void InvertX(float3 *dst, size_t num)
{
    Forward(InvertX, dst, num);
}
void InvertX(float4 *dst, size_t num)
{
    Forward(InvertX, dst, num);
}

void Scale(float *dst, float s, size_t num)
{
    Forward(Scale, dst, s, num);
}
void Scale(float3 *dst, float s, size_t num)
{
    Forward(Scale, dst, s, num);
}

void Normalize(float3 *dst, size_t num)
{
    Forward(Normalize, dst, num);
}

void Lerp(float *dst, const float *src1, const float *src2, size_t num, float w)
{
    Forward(Lerp, dst, src1, src2, num, w);
}
void Lerp(float2 *dst, const float2 *src1, const float2 *src2, size_t num, float w)
{
    Lerp((float*)dst, (const float*)src1, (const float*)src2, num * 2, w);
}
void Lerp(float3 *dst, const float3 *src1, const float3 *src2, size_t num, float w)
{
    Lerp((float*)dst, (const float*)src1, (const float*)src2, num * 3, w);
}

void MinMax(const float2 *p, size_t num, float2& dst_min, float2& dst_max)
{
    Forward(MinMax, p, num, dst_min, dst_max);
}
void MinMax(const float3 *p, size_t num, float3& dst_min, float3& dst_max)
{
    Forward(MinMax, p, num, dst_min, dst_max);
}

bool NearEqual(const float *src1, const float *src2, size_t num, float eps)
{
    return Forward(NearEqual, src1, src2, num, eps);
}
bool NearEqual(const float2 *src1, const float2 *src2, size_t num, float eps)
{
    return NearEqual((const float*)src1, (const float*)src2, num * 2, eps);
}
bool NearEqual(const float3 *src1, const float3 *src2, size_t num, float eps)
{
    return NearEqual((const float*)src1, (const float*)src2, num * 3, eps);
}
bool NearEqual(const float4 *src1, const float4 *src2, size_t num, float eps)
{
    return NearEqual((const float*)src1, (const float*)src2, num * 4, eps);
}

void MulPoints(const float4x4& m, const float3 src[], float3 dst[], size_t num_data)
{
    Forward(MulPoints, m, src, dst, num_data);
}
void MulVectors(const float4x4& m, const float3 src[], float3 dst[], size_t num_data)
{
    Forward(MulVectors, m, src, dst, num_data);
}

int RayTrianglesIntersectionIndexed(float3 pos, float3 dir, const float3 *vertices, const int *indices, int num_triangles, int& tindex, float& result)
{
    return Forward(RayTrianglesIntersectionIndexed, pos, dir, vertices, indices, num_triangles, tindex, result);
}
int RayTrianglesIntersectionFlattened(float3 pos, float3 dir, const float3 *vertices, int num_triangles, int& tindex, float& result)
{
    return Forward(RayTrianglesIntersectionFlattened, pos, dir, vertices, num_triangles, tindex, result);
}
int RayTrianglesIntersectionSoA(float3 pos, float3 dir,
    const float *v1x, const float *v1y, const float *v1z,
    const float *v2x, const float *v2y, const float *v2z,
    const float *v3x, const float *v3y, const float *v3z,
    int num_triangles, int& tindex, float& result)
{
    return Forward(RayTrianglesIntersectionSoA, pos, dir, v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z, num_triangles, tindex, result);
}

bool PolyInside(const float px[], const float py[], int ngon, const float2 minp, const float2 maxp, const float2 pos)
{
    return Forward(PolyInside, px, py, ngon, minp, maxp, pos);
}
bool PolyInside(const float2 poly[], int ngon, const float2 minp, const float2 maxp, const float2 pos)
{
    return Forward(PolyInside, poly, ngon, minp, maxp, pos);
}
bool PolyInside(const float2 poly[], int ngon, const float2 pos)
{
    return Forward(PolyInside, poly, ngon, pos);
}


void GenerateNormalsTriangleIndexed(float3 *dst,
    const float3 *vertices, const int *indices, int num_triangles, int num_vertices)
{
    return Forward(GenerateNormalsTriangleIndexed, dst, vertices, indices, num_triangles, num_vertices);
}
void GenerateNormalsTriangleFlattened(float3 *dst,
    const float3 *vertices, const int *indices,
    int num_triangles, int num_vertices)
{
    return Forward(GenerateNormalsTriangleFlattened, dst, vertices, indices, num_triangles, num_vertices);
}
void GenerateNormalsTriangleSoA(float3 *dst,
    const float *v1x, const float *v1y, const float *v1z,
    const float *v2x, const float *v2y, const float *v2z,
    const float *v3x, const float *v3y, const float *v3z,
    const int *indices, int num_triangles, int num_vertices)
{
    return Forward(GenerateNormalsTriangleSoA, dst,
        v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z,
        indices, num_triangles, num_vertices);
}


void GenerateTangentsTriangleIndexed(float4 *dst,
    const float3 *vertices, const float2 *uv, const float3 *normals, const int *indices,
    int num_triangles, int num_vertices)
{
    return Forward(GenerateTangentsTriangleIndexed, dst, vertices, uv, normals, indices, num_triangles, num_vertices);
}
void GenerateTangentsTriangleFlattened(float4 *dst,
    const float3 *vertices, const float2 *uv, const float3 *normals, const int *indices,
    int num_triangles, int num_vertices)
{
    return Forward(GenerateTangentsTriangleFlattened, dst, vertices, uv, normals, indices, num_triangles, num_vertices);
}
void GenerateTangentsTriangleSoA(float4 *dst,
    const float *v1x, const float *v1y, const float *v1z,
    const float *v2x, const float *v2y, const float *v2z,
    const float *v3x, const float *v3y, const float *v3z,
    const float *u1x, const float *u1y,
    const float *u2x, const float *u2y,
    const float *u3x, const float *u3y,
    const float3 *normals,
    const int *indices, int num_triangles, int num_vertices)
{
    return Forward(GenerateTangentsTriangleSoA, dst,
        v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z,
        u1x, u1y, u2x, u2y, u3x, u3y,
        normals, indices, num_triangles, num_vertices);
}

#undef Forward
} // namespace mu
