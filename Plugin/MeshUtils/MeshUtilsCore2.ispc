#include "ispcmath.h"

// functions that have gather/scatter operations need to be written in this compile unit.
// (--opt=force-aligned-memory is disabled)

export void MulVectors3(uniform const float4x4& m_, uniform const float3 src[], uniform float3 dst[], uniform int num_data)
{
    uniform float4x4 m = m_;

    uniform int num_data_simd = num_data & ~(C - 1);
    for (uniform int bi = 0; bi < num_data_simd; bi += C) {
        float3 v;
        aos_to_soa3((uniform float*)&src[bi], &v.x, &v.y, &v.z);

        float3 r = {
            m.m[0].x * v.x + m.m[1].x * v.y + m.m[2].x * v.z,
            m.m[0].y * v.x + m.m[1].y * v.y + m.m[2].y * v.z,
            m.m[0].z * v.x + m.m[1].z * v.y + m.m[2].z * v.z,
        };
        soa_to_aos3(r.x, r.y, r.z, (uniform float*)&dst[bi]);
    }

    for(uniform int i = num_data_simd; i < num_data; ++i) {
        uniform float3 v = src[i];
        uniform float3 r = {
            m.m[0].x * v.x + m.m[1].x * v.y + m.m[2].x * v.z,
            m.m[0].y * v.x + m.m[1].y * v.y + m.m[2].y * v.z,
            m.m[0].z * v.x + m.m[1].z * v.y + m.m[2].z * v.z,
        };
        dst[i] = r;
    }
}

export void MulPoints3(uniform const float4x4& m_, uniform const float3 src[], uniform float3 dst[], uniform int num_data)
{
    uniform float4x4 m = m_;

    uniform int num_data_simd = num_data & ~(C - 1);
    for (uniform int bi = 0; bi < num_data_simd; bi += C) {
        float3 v;
        aos_to_soa3((uniform float*)&src[bi], &v.x, &v.y, &v.z);

        float3 r = {
            m.m[0].x * v.x + m.m[1].x * v.y + m.m[2].x * v.z + m.m[3].x,
            m.m[0].y * v.x + m.m[1].y * v.y + m.m[2].y * v.z + m.m[3].y,
            m.m[0].z * v.x + m.m[1].z * v.y + m.m[2].z * v.z + m.m[3].z,
        };
        soa_to_aos3(r.x, r.y, r.z, (uniform float*)&dst[bi]);
    }

    for(uniform int i = num_data_simd; i < num_data; ++i) {
        uniform float3 v = src[i];
        uniform float3 r = {
            m.m[0].x * v.x + m.m[1].x * v.y + m.m[2].x * v.z + m.m[3].x,
            m.m[0].y * v.x + m.m[1].y * v.y + m.m[2].y * v.z + m.m[3].y,
            m.m[0].z * v.x + m.m[1].z * v.y + m.m[2].z * v.z + m.m[3].z,
        };
        dst[i] = r;
    }
}

export void MinMax3(
    uniform const float3 src[], uniform const int num,
    uniform float3& dst_min, uniform float3& dst_max)
{
    if(num == 0) { return; }

    uniform float3 rmin = src[0], rmax = src[0];

    const uniform int block_size = C;
    const uniform int num_loops = num / block_size;
    if(num_loops > 1) {
        const uniform float * uniform fv = (const uniform float * uniform)src;
        uniform float tmin[3][C];
        uniform float tmax[3][C];
        tmin[0][I] = tmax[0][I] = fv[C*0 + I];
        tmin[1][I] = tmax[1][I] = fv[C*1 + I];
        tmin[2][I] = tmax[2][I] = fv[C*2 + I];

        for(uniform int i=1; i < num_loops; ++i) {
            uniform const int i3 = i*3;

            float _0 = fv[C*(i3+0) + I];
            tmin[0][I] = min(tmin[0][I], _0);
            tmax[0][I] = max(tmax[0][I], _0);

            float _1 = fv[C*(i3+1) + I];
            tmin[1][I] = min(tmin[1][I], _1);
            tmax[1][I] = max(tmax[1][I], _1);

            float _2 = fv[C*(i3+2) + I];
            tmin[2][I] = min(tmin[2][I], _2);
            tmax[2][I] = max(tmax[2][I], _2);
        }

        float x,y,z;
        aos_to_soa3((uniform float*)&tmin[0], &x, &y, &z);
        rmin.x = reduce_min(x);
        rmin.y = reduce_min(y);
        rmin.z = reduce_min(z);

        aos_to_soa3((uniform float*)&tmax[0], &x, &y, &z);
        rmax.x = reduce_max(x);
        rmax.y = reduce_max(y);
        rmax.z = reduce_max(z);
    }

    for(uniform int i=num_loops*block_size; i < num; ++i) {
        uniform float3 t = src[i];
        rmin.x = min(rmin.x, t.x);
        rmin.y = min(rmin.y, t.y);
        rmin.z = min(rmin.z, t.z);
        rmax.x = max(rmax.x, t.x);
        rmax.y = max(rmax.y, t.y);
        rmax.z = max(rmax.z, t.z);
    }

    dst_min = rmin;
    dst_max = rmax;
}

export void MinMax2(
    uniform const float2 src[], uniform const int num,
    uniform float2& dst_min, uniform float2& dst_max)
{
    if(num == 0) { return; }

    uniform float2 rmin = src[0];
    uniform float2 rmax = src[0];

    const uniform int block_size = C;
    const uniform int num_loops = num / block_size;
    if(num_loops > 1) {
        const uniform float * uniform fv = (const uniform float * uniform)src;

        float tmin[2], tmax[2];
        tmin[0] = tmax[0] = fv[  I];
        tmin[1] = tmax[1] = fv[C+I];
        for(uniform int i=1; i < num_loops; ++i) {
            uniform const int i2 = i*2;
            tmin[0] = min(tmin[0], fv[C*(i2+0) + I]);
            tmax[0] = max(tmax[0], fv[C*(i2+0) + I]);
            tmin[1] = min(tmin[1], fv[C*(i2+1) + I]);
            tmax[1] = max(tmax[1], fv[C*(i2+1) + I]);
        }

        rmin.x = reduce_min(shuffle(tmin[0], tmin[1], I*2 + 0));
        rmin.y = reduce_min(shuffle(tmin[0], tmin[1], I*2 + 1));
        rmax.x = reduce_max(shuffle(tmax[0], tmax[1], I*2 + 0));
        rmax.y = reduce_max(shuffle(tmax[0], tmax[1], I*2 + 1));
    }

    for(uniform int i=num_loops*block_size; i < num; ++i) {
        uniform float2 t = src[i];
        rmin.x = min(rmin.x, t.x);
        rmin.y = min(rmin.y, t.y);
        rmax.x = max(rmax.x, t.x);
        rmax.y = max(rmax.y, t.y);
    }

    dst_min = rmin;
    dst_max = rmax;
}


export uniform int RayTrianglesIntersectionIndexed(
    uniform const float3& pos, uniform const float3& dir,
    uniform const float3 vertices[], uniform const int indices[], uniform const int num_triangles,
    uniform int& tindex, uniform float& distance)
{
    uniform int total_hit = 0;
    distance = FLT_MAX;

    // SIMD pass
    uniform int num_triangles_simd = num_triangles & ~(C - 1);
    for(uniform int bi=0; bi < num_triangles_simd; bi += C) {
        float3 p1, p2, p3;
        // this emits warnings but performace is acceptable.
        p1 = vertices[indices[(bi + I) * 3  + 0]];
        p2 = vertices[indices[(bi + I) * 3  + 1]];
        p3 = vertices[indices[(bi + I) * 3  + 2]];

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
        uniform float3 p1 = vertices[indices[ti * 3 + 0]];
        uniform float3 p2 = vertices[indices[ti * 3 + 1]];
        uniform float3 p3 = vertices[indices[ti * 3 + 2]];

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

export uniform int RayTrianglesIntersectionArray(
    uniform const float3& pos, uniform const float3& dir,
    uniform const float3 vertices[], uniform const int num_triangles,
    uniform int& tindex, uniform float& distance)
{
    uniform int total_hit = 0;
    distance = FLT_MAX;

    // SIMD pass
    uniform int num_triangles_simd = num_triangles & ~(C - 1);
    for(uniform int bi=0; bi < num_triangles_simd; bi += C) {
        float3 p1, p2, p3;
        // this emits warnings but performace is acceptable.
        p1 = vertices[(bi + I) * 3  + 0];
        p2 = vertices[(bi + I) * 3  + 1];
        p3 = vertices[(bi + I) * 3  + 2];

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
        uniform float3 p1 = vertices[ti * 3 + 0];
        uniform float3 p2 = vertices[ti * 3 + 1];
        uniform float3 p3 = vertices[ti * 3 + 2];

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


export uniform int PolyInsideImpl(
    uniform const float2 points[], uniform int ngon, uniform float2& minp, uniform float2& maxp, uniform float2& pos,
    uniform float xc[], uniform int maxxc)
{
    if (pos.x < minp.x || pos.x > maxp.x ||
        pos.y < minp.y || pos.y > maxp.y)
    {
        return 0;
    }

    uniform int c = 0;
    uniform int ngon_simd = (ngon - 1) & ~(C - 1);

    // SIMD pass
    for (uniform int bi = 0; bi < ngon_simd; bi += C) {
        float2 p1, p2;
        {
            float t1 = ((uniform const float*)points)[bi*2 + I    ];
            float t2 = ((uniform const float*)points)[bi*2 + C+I  ];
            float t3 = ((uniform const float*)points)[bi*2 + I  +2];
            float t4 = ((uniform const float*)points)[bi*2 + C+I+2];

            float2 tp1 = {
                shuffle(t1, t2, I*2 + 0),
                shuffle(t1, t2, I*2 + 1) };
            float2 tp2 = {
                shuffle(t3, t4, I*2 + 0),
                shuffle(t3, t4, I*2 + 1) };

            bool needs_swap = tp1.y > tp2.y;
            p1.x = select(needs_swap, tp2.x, tp1.x);
            p1.y = select(needs_swap, tp2.y, tp1.y);
            p2.x = select(needs_swap, tp1.x, tp2.x);
            p2.y = select(needs_swap, tp1.y, tp2.y);
        }

        bool intersect =
            (p1.y != p2.y) &&
            ((pos.y >= p1.y && pos.y < p2.y) ||
             (pos.y == maxp.y && pos.y > p1.y && pos.y <= p2.y));

        if (any(intersect)) {
            float x = (pos.y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y) + p1.x;

            uniform int intersecta[C]; intersecta[I] = intersect;
            uniform float xa[C]; xa[I] = x;
            for (uniform int ci = 0; ci < C; ++ci) {
                if (intersecta[ci]) {
                    xc[c++] = xa[ci];
                    if (c == maxxc) return c;
                }
            }
        }
    }

    // non-SIMD pass
    for (uniform int i = ngon_simd; i < ngon; ++i) {
        uniform int j = i + 1;
        if (j == ngon) { j = 0; }

        uniform float2 p1 = points[i];
        uniform float2 p2 = points[j];
        if(p1.y == p2.y) { continue; }
        else if(p1.y > p2.y) {
            uniform float2 tmp = p1;
            p1 = p2;
            p2 = tmp;
        }

        uniform bool intersect =
            (pos.y >= p1.y && pos.y < p2.y) ||
            (pos.y == maxp.y && pos.y > p1.y && pos.y <= p2.y);

        if (intersect) {
            xc[c++] = (pos.y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y) + p1.x;
            if (c == maxxc) return c;
        }
    }
    return c;
}

export uniform int PolyInsideSoAImpl(
    uniform const float px[], uniform const float py[], uniform int ngon, uniform float2& minp, uniform float2& maxp, uniform float2& pos,
    uniform float xc[], uniform int maxxc)
{
    if (pos.x < minp.x || pos.x > maxp.x ||
        pos.y < minp.y || pos.y > maxp.y)
    {
        return 0;
    }

    uniform int c = 0;
    uniform int ngon_simd = (ngon - 1) & ~(C - 1);

    // SIMD pass
    for (uniform int bi = 0; bi < ngon_simd; bi += C) {
        float2 p1, p2;
        {
            float2 tp1 = { px[bi+I+0], py[bi+I+0] };
            float2 tp2 = { px[bi+I+1], py[bi+I+1] };

            bool needs_swap = tp1.y > tp2.y;
            p1.x = select(needs_swap, tp2.x, tp1.x);
            p1.y = select(needs_swap, tp2.y, tp1.y);
            p2.x = select(needs_swap, tp1.x, tp2.x);
            p2.y = select(needs_swap, tp1.y, tp2.y);
        }

        bool intersect =
            (p1.y != p2.y) &&
            ((pos.y >= p1.y && pos.y < p2.y) ||
             (pos.y == maxp.y && pos.y > p1.y && pos.y <= p2.y));

        if (any(intersect)) {
            float x = (pos.y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y) + p1.x;

            uniform int intersecta[C]; intersecta[I] = intersect;
            uniform float xa[C]; xa[I] = x;
            for (uniform int ci = 0; ci < C; ++ci) {
                if (intersecta[ci]) {
                    xc[c++] = xa[ci];
                    if (c == maxxc) return c;
                }
            }
        }
    }

    // non-SIMD pass
    for (uniform int i = ngon_simd; i < ngon; ++i) {
        uniform int j = i + 1;
        if (j == ngon) { j = 0; }

        uniform float2 p1 = {px[i], py[i]};
        uniform float2 p2 = {px[j], py[j]};
        if(p1.y == p2.y) { continue; }
        else if(p1.y > p2.y) {
            uniform float2 tmp = p1;
            p1 = p2;
            p2 = tmp;
        }

        uniform bool intersect =
            (pos.y >= p1.y && pos.y < p2.y) ||
            (pos.y == maxp.y && pos.y > p1.y && pos.y <= p2.y);

        if (intersect) {
            xc[c++] = (pos.y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y) + p1.x;
            if (c == maxxc) return c;
        }
    }
    return c;
}


export void GenerateNormalsTriangleIndexed(uniform float3 dst[],
    uniform const float3 vertices[], uniform const int indices[],
    uniform const int num_triangles, uniform const int num_vertices)
{
    for(uniform int bi=0; bi < num_triangles; bi += C) {
        int ti = bi+I;

        float3 v[3] = {
             vertices[indices[ti * 3  + 0]],
             vertices[indices[ti * 3  + 1]],
             vertices[indices[ti * 3  + 2]],
        };
        float3 n = cross(v[1] - v[0], v[2] - v[0]);

        uniform float3 n_aos[C];
        soa_to_aos3(n.x, n.y, n.z, (uniform float*)&n_aos[bi]);

        uniform int nt = min(C, num_triangles - bi);
        for(uniform int i=0; i<nt; ++i) {
            int i0 = indices[(bi + i) * 3 + 0];
            int i1 = indices[(bi + i) * 3 + 1];
            int i2 = indices[(bi + i) * 3 + 2];
            dst[i0] = dst[i0] + n_aos[i];
            dst[i1] = dst[i1] + n_aos[i];
            dst[i2] = dst[i2] + n_aos[i];
        }
    }
    NormalizeImpl(dst, num_vertices);
}

export void GenerateNormalsTriangleSoA(uniform float3 dst[],
    uniform const float v1x[], uniform const float v1y[], uniform const float v1z[],
    uniform const float v2x[], uniform const float v2y[], uniform const float v2z[],
    uniform const float v3x[], uniform const float v3y[], uniform const float v3z[],
    uniform const int indices[],
    uniform const int num_triangles, uniform const int num_vertices)
{
    for(uniform int bi=0; bi < num_triangles; bi += C) {
        int ti = bi+I;

        float3 v[3] = {
             {v1x[ti], v1y[ti], v1z[ti]},
             {v2x[ti], v2y[ti], v2z[ti]},
             {v3x[ti], v3y[ti], v3z[ti]},
        };
        float3 n = cross(v[1] - v[0], v[2] - v[0]);

        uniform float3 n_aos[C];
        soa_to_aos3(n.x, n.y, n.z, (uniform float*)&n_aos[bi]);

        uniform int nt = min(C, num_triangles - bi);
        for(uniform int i=0; i<nt; ++i) {
            int i0 = indices[(bi + i) * 3 + 0];
            int i1 = indices[(bi + i) * 3 + 1];
            int i2 = indices[(bi + i) * 3 + 2];
            dst[i0] = dst[i0] + n_aos[i];
            dst[i1] = dst[i1] + n_aos[i];
            dst[i2] = dst[i2] + n_aos[i];
        }
    }
    NormalizeImpl(dst, num_vertices);
}


export void GenerateTangentsIndexed(
    uniform const float3 vertices[],
    uniform const float2 uv[],
    uniform const float3 normals[],
    uniform const int indices[],

    uniform const int num_triangles,
    uniform const int num_vertices,

    uniform float3 tmp_tangents[],
    uniform float3 tmp_binormals[],
    uniform float4 dst_tangents[])
{
    for(uniform int bi=0; bi < num_triangles; bi += C) {
        int ti = bi+I;
        float3 v[3] = {
             vertices[indices[ti * 3  + 0]],
             vertices[indices[ti * 3  + 1]],
             vertices[indices[ti * 3  + 2]],
        };
        float2 u[3] = {
             uv[indices[ti * 3  + 0]],
             uv[indices[ti * 3  + 1]],
             uv[indices[ti * 3  + 2]],
        };
        float3 t[3];
        float3 b[3];
        compute_triangle_tangents(v, u, t, b);

        uniform float3 tagents_aos[C*3];
        uniform float3 binormals_aos[C*3];
        for(uniform int i=0; i<3; ++i) {
            soa_to_aos3(t[i].x, t[i].y, t[i].z, (uniform float*)&tagents_aos[C*i]);
            soa_to_aos3(b[i].x, b[i].y, b[i].z, (uniform float*)&binormals_aos[C*i]);
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

        float4 result = orthogonalize_tangent(t, b, n);
        soa_to_aos4(result.x, result.y, result.z, result.w, (uniform float*)&dst_tangents[bi]);
    }
}

export void GenerateTangentsSoA(
    uniform float4 dst_tangents[],
    uniform const float v1x[], uniform const float v1y[], uniform const float v1z[],
    uniform const float v2x[], uniform const float v2y[], uniform const float v2z[],
    uniform const float v3x[], uniform const float v3y[], uniform const float v3z[],

    uniform const float u1x[], uniform const float u1y[],
    uniform const float u2x[], uniform const float u2y[],
    uniform const float u3x[], uniform const float u3y[],

    uniform const float3 normals[],
    uniform const int num_triangles,
    uniform const int num_vertices,

    uniform const int indices[],
    uniform float3 tmp_tangents[],
    uniform float3 tmp_binormals[])
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
        compute_triangle_tangents(vertices, uv, tangents, binormals);

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

        float4 result = orthogonalize_tangent(t, b, n);
        soa_to_aos4(result.x, result.y, result.z, result.w, (uniform float*)&dst_tangents[bi]);
    }
}
