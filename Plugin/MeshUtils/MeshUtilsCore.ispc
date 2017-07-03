#include "ispcmath.h"

export void FloatToHalf(
    uniform half dst[],
    uniform const float src[],
    uniform const int num)
{
    foreach(i=0 ... num) {
        dst[i] = float_to_half(src[i]);
    }
}

export void HalfToFloat(
    uniform float dst[],
    uniform const half src[],
    uniform const int num)
{
    foreach(i=0 ... num) {
        dst[i] = half_to_float(src[i]);
    }
}


// invert every x elements of float3 array
export void InvertXF3(uniform float3 dst[], uniform const int num)
{
    const uniform int num_loops = num / C;

    {
        uniform float _c[3][C];
        _c[0][I] = select((C*0 + I)%3==0, -1.0f, 1.0f);
        _c[1][I] = select((C*1 + I)%3==0, -1.0f, 1.0f);
        _c[2][I] = select((C*2 + I)%3==0, -1.0f, 1.0f);

        uniform float * uniform fv = (uniform float * uniform)dst;
        for(uniform int i=0; i < num_loops; ++i) {
            uniform int i3 = i*3;
            fv[C*(i3+0) + I] = fv[C*(i3+0) + I] * _c[0][I];
            fv[C*(i3+1) + I] = fv[C*(i3+1) + I] * _c[1][I];
            fv[C*(i3+2) + I] = fv[C*(i3+2) + I] * _c[2][I];
        }
    }

    for(uniform int i=num_loops*C; i < num; ++i) {
        dst[i].x *= -1.0f;
    }
}

// invert every x elements of float4 array
export void InvertXF4(uniform float4 dst[], uniform const int num)
{
    const uniform int num_loops = num / (C/4);

    {
        uniform float _c[C];
        _c[I] = select(I%4==0, -1.0f, 1.0f);

        uniform float * uniform fv = (uniform float * uniform)dst;
        for(uniform int i=0; i < num_loops; ++i) {
            fv[C*i + I] = fv[C*i + I] * _c[I];
        }
    }

    for(uniform int i=num_loops*C; i < num; ++i) {
        dst[i].x *= -1.0f;
    }
}


// apply scale to every elements
export void ScaleF(uniform float dst[], uniform const float scale, uniform const int num)
{
    const uniform int num_loops = num / (C*4);

    {
        uniform float * uniform fv = (uniform float * uniform)dst;
        for(uniform int i=0; i < num_loops; ++i) {
            uniform int i4 = i*4;
            fv[C*(i4+0) + I] = fv[C*(i4+0) + I] * scale;
            fv[C*(i4+1) + I] = fv[C*(i4+1) + I] * scale;
            fv[C*(i4+2) + I] = fv[C*(i4+2) + I] * scale;
            fv[C*(i4+3) + I] = fv[C*(i4+3) + I] * scale;
        }
    }

    for(uniform int i=num_loops*(C*4); i < num; ++i) {
        dst[i] *= scale;
    }
}

export void Normalize(
    uniform float3 dst[],
    uniform const int num)
{
    uniform const int num_loops = num / C;

    uniform float * uniform fv = (uniform float * uniform)dst;
    for(uniform int i=0; i < num_loops; ++i) {
        uniform const int i3 = i*3;
        float x,y,z;
        aos_to_soa3((uniform float*)&fv[C*i3], &x, &y, &z);
        float d = rsqrt(x*x + y*y + z*z);
        x *= d;
        y *= d;
        z *= d;
        soa_to_aos3(x, y, z, (uniform float*)&fv[C*i3]);
    }

    for(uniform int i=num_loops*C; i < num; ++i) {
        uniform float3 t = dst[i];
        uniform float d = rsqrt(t.x*t.x + t.y*t.y + t.z*t.z);
        t.x *= d;
        t.y *= d;
        t.z *= d;
        dst[i] = t;
    }
}


export void Lerp(uniform float dst[], uniform const float src1[], uniform const float src2[], uniform const int num, uniform float w)
{
    uniform float iw = 1.0f - w;
    foreach(i=0 ... num) {
        dst[i] = src1[i]*w + src2[i]*iw;
    }
}

export uniform bool NearEqual(
    uniform const float src1[], uniform const float src2[], uniform const int num, uniform const float eps)
{
    float tdiff = 0.0f;
    foreach(i=0 ... num) {
        tdiff = max(tdiff, abs(src1[i] - src2[i]));
    }
    return reduce_max(tdiff) < eps;
}



export uniform int RayTrianglesIntersectionSoA(
    uniform const float3& pos, uniform const float3& dir,
    uniform const float v1x[], uniform const float v1y[], uniform const float v1z[],
    uniform const float v2x[], uniform const float v2y[], uniform const float v2z[],
    uniform const float v3x[], uniform const float v3y[], uniform const float v3z[],
    uniform const int num_triangles,
    uniform int& tindex, uniform float& distance)
{
    uniform int total_hit = 0;
    distance = FLT_MAX;

    // SIMD pass
    uniform int num_triangles_simd = num_triangles & ~(C - 1);
    for(uniform int bi=0; bi < num_triangles_simd; bi += C) {
        float3 p1 = {v1x[bi+I], v1y[bi+I], v1z[bi+I]};
        float3 p2 = {v2x[bi+I], v2y[bi+I], v2z[bi+I]};
        float3 p3 = {v3x[bi+I], v3y[bi+I], v3z[bi+I]};

        float d;
        bool hit = ray_triangle_intersection(pos, dir, p1, p2, p3, d);
        if(any(hit)) {
            uniform int hita[C]; hita[I] = hit;
            uniform float da[C]; da[I] = d;
            for(uniform int i = 0; i < C; ++i) {
                if(hita[i]) {
                    total_hit++;
                    if(da[i] < distance) {
                        tindex = bi + i;
                        distance = da[i];
                    }
                }
            }
        }
    }

    // non-SIMD pass
    for(uniform int ti = num_triangles_simd; ti < num_triangles; ++ti) {
        uniform float3 p1 = {v1x[ti], v1y[ti], v1z[ti]};
        uniform float3 p2 = {v2x[ti], v2y[ti], v2z[ti]};
        uniform float3 p3 = {v3x[ti], v3y[ti], v3z[ti]};

        uniform float d;
        uniform bool hit = ray_triangle_intersection(pos, dir, p1, p2, p3, d);
        if(hit) {
            total_hit++;
            if(d < distance) {
                tindex = ti;
                distance = d;
            }
        }
    }

    return total_hit;
}


static inline void ComputeTriangleTangentBasis(
    const float3 vertices[3], const float2 uv[3], float3 dst_tangent[3], float3 dst_binormal[3])
{

    float p[] = { vertices[1].x - vertices[0].x, vertices[1].y - vertices[0].y, vertices[1].z - vertices[0].z };
    float q[] = { vertices[2].x - vertices[0].x, vertices[2].y - vertices[0].y, vertices[2].z - vertices[0].z };

    float s[] = { uv[1].x - uv[0].x, uv[2].x - uv[0].x };
    float t[] = { uv[1].y - uv[0].y, uv[2].y - uv[0].y };

    float div = s[0] * t[1] - s[1] * t[0];
    float areaMult = abs(div);

    float3 tangent, binormal;
    if (areaMult >= 1e-8)
    {
        float r = 1.0f / div;

        s[0] *= r;  t[0] *= r;
        s[1] *= r;  t[1] *= r;

        float3 tt = {
            t[1] * p[0] - t[0] * q[0],
            t[1] * p[1] - t[0] * q[1],
            t[1] * p[2] - t[0] * q[2]
        };
        tangent = normalize(tt) * areaMult;

        float3 tb = {
            s[0] * q[0] - s[1] * p[0],
            s[0] * q[1] - s[1] * p[1],
            s[0] * q[2] - s[1] * p[2]
        };
        binormal = normalize(tb) * areaMult;
    }
    else {
        tangent = binormal = float3_(0.0f, 0.0f, 0.0f);
    }

    float3 edge1[3] = {
        vertices[2] - vertices[0],
        vertices[0] - vertices[1],
        vertices[1] - vertices[2],
    };
    float3 edge2[3] = {
        vertices[1] - vertices[0],
        vertices[2] - vertices[1],
        vertices[0] - vertices[2],
    };
    for (int v = 0; v < 3; ++v)
    {
        float angle = dot(normalize(edge1[v]), normalize(edge2[v]));
        float w = acos(clamp(angle, -1.0f, 1.0f));

        dst_tangent[v] = tangent * w;
        dst_binormal[v] = binormal * w;
    }
}

static inline float4 OrthogonalizeTangent(float3 tangent, float3 binormal, float3 normal)
{
    float NdotT = dot(normal, tangent);
    tangent = float3_(
        tangent.x - NdotT * normal.x,
        tangent.y - NdotT * normal.y,
        tangent.z - NdotT * normal.z
    );
    float magT = length(tangent);
    tangent = tangent / magT;

    float NdotB = dot(normal, binormal);
    float TdotB = dot(tangent, binormal) * magT;
    binormal = float3_(
        binormal.x - NdotB * normal.x - TdotB * tangent.x,
        binormal.y - NdotB * normal.y - TdotB * tangent.y,
        binormal.z - NdotB * normal.z - TdotB * tangent.z
    );
    float magB = length(binormal);
    binormal = binormal / magB;


    const float kNormalizeEpsilon = 1e-6;

    if (magT <= kNormalizeEpsilon || magB <= kNormalizeEpsilon)
    {
        float3 axis1, axis2;

        float dpXN = abs(dot(float3_(1.0f, 0.0f, 0.0f), normal));
        float dpYN = abs(dot(float3_(0.0f, 1.0f, 0.0f), normal));
        float dpZN = abs(dot(float3_(0.0f, 0.0f, 1.0f), normal));

        if (dpXN <= dpYN && dpXN <= dpZN)
        {
            axis1 = float3_(1.0f, 0.0f, 0.0f);
            if (dpYN <= dpZN)
                axis2 = float3_(0.0f, 1.0f, 0.0f);
            else
                axis2 = float3_(0.0f, 0.0f, 1.0f);
        }
        else if (dpYN <= dpXN && dpYN <= dpZN)
        {
            axis1 = float3_(0.0f, 1.0f, 0.0f);
            if (dpXN <= dpZN)
                axis2 = float3_(1.0f, 0.0f, 0.0f);
            else
                axis2 = float3_(0.0f, 0.0f, 1.0f);
        }
        else
        {
            axis1 = float3_(0.0f, 0.0f, 1.0f);
            if (dpXN <= dpYN)
                axis2 = float3_(1.0f, 0.0f, 0.0f);
            else
                axis2 = float3_(0.0f, 1.0f, 0.0f);
        }

        tangent = normalize(axis1 - normal * dot(normal, axis1));
        binormal = normalize(axis2 - normal * dot(normal, axis2) - normalize(tangent) * dot(tangent, axis2));
    }

    return float4_(tangent.x, tangent.y, tangent.z,
        dot(cross(normal, tangent), binormal) > 0.0f ? 1.0f : -1.0f);
}

export void GenerateTangentsSoA(
    uniform const float v1x[], uniform const float v1y[], uniform const float v1z[],
    uniform const float v2x[], uniform const float v2y[], uniform const float v2z[],
    uniform const float v3x[], uniform const float v3y[], uniform const float v3z[],

    uniform const float u1x[], uniform const float u1y[],
    uniform const float u2x[], uniform const float u2y[],
    uniform const float u3x[], uniform const float u3y[],

    uniform const int num_triangles,
    uniform const int num_vertices,

    uniform const int indices[],
    uniform float3 tmp_tangents[],
    uniform float3 tmp_binormals[],
    uniform const float3 normals[],
    uniform float4 dst_tangents[])
{
    for(uniform int bi=0; bi < num_triangles; bi += C) {
        int ti = bi+I;
        float3 vertices[3] = {
            {v1x[ti], v1y[ti], v1z[ti]},
            {v2x[ti], v2y[ti], v2z[ti]},
            {v3x[ti], v3y[ti], v3z[ti]}
        };
        float2 uv[3] = {
            {u1x[ti], u1y[ti]},
            {u2x[ti], u2y[ti]},
            {u3x[ti], u3y[ti]}
        };
        float3 tangents[3];
        float3 binormals[3];
        ComputeTriangleTangentBasis(vertices, uv, tangents, binormals);

        uniform float3 tagents_aos[C*3];
        uniform float3 binormals_aos[C*3];
        for(uniform int i=0; i<3; ++i) {
            soa_to_aos3(tangents[i].x, tangents[i].y, tangents[i].z, (uniform float*)&tagents_aos[C*i]);
            soa_to_aos3(binormals[i].x, binormals[i].y, binormals[i].z, (uniform float*)&binormals_aos[C*i]);
        }
        int ni = min(C*3, num_triangles * 3 - bi * 3);
        for(uniform int i=0; i<ni; ++i) {
            uniform int ati = bi * 3 + i;
            tmp_tangents[indices[ati]] = tmp_tangents[indices[ati]] + tagents_aos[i];
            tmp_binormals[indices[ati]] = tmp_binormals[indices[ati]] + binormals_aos[i];
        }
    }

    for(uniform int bi=0; bi < num_vertices; bi += C) {
        float3 t, b, n;
        aos_to_soa3((uniform float*)&tmp_tangents[bi], &t.x, &t.y, &t.z);
        aos_to_soa3((uniform float*)&tmp_binormals[bi], &b.x, &b.y, &b.z);
        aos_to_soa3((uniform float*)&normals[bi], &n.x, &n.y, &n.z);

        float4 result = OrthogonalizeTangent(t, b, n);
        soa_to_aos4(result.x, result.y, result.z, result.w, (uniform float*)&dst_tangents[bi]);
    }
}
