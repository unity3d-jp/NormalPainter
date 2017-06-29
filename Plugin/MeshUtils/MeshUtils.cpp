#include "pch.h"
#include "MeshUtils.h"
#include "mikktspace.h"

#ifdef muEnableHalf
#ifdef _WIN32
    #pragma comment(lib, "half.lib")
#endif
#endif // muEnableHalf

namespace mu {



bool GenerateNormals(
    IArray<float3> dst, const IArray<float3> points,
    const IArray<int> counts, const IArray<int> offsets, const IArray<int> indices)
{
    if (dst.size() != points.size()) {
        return false;
    }
    dst.zeroclear();

    size_t num_faces = counts.size();
    for (size_t fi = 0; fi < num_faces; ++fi)
    {
        int count = counts[fi];
        const int *face = &indices[offsets[fi]];
        float3 p0 = points[face[0]];
        float3 p1 = points[face[1]];
        float3 p2 = points[face[2]];
        float3 n = cross(p1 - p0, p2 - p0);
        for (int ci = 0; ci < count; ++ci) {
            dst[face[ci]] += n;
        }
    }
    Normalize(dst.data(), dst.size());
    return true;
}

struct TSpaceContext
{
    IArray<float4> dst;
    const IArray<float3> points;
    const IArray<float3> normals;
    const IArray<float2> uv;
    const IArray<int> counts;
    const IArray<int> offsets;
    const IArray<int> indices;

    static int getNumFaces(const SMikkTSpaceContext *tctx)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        return (int)_this->counts.size();
    }

    static int getCount(const SMikkTSpaceContext *tctx, int i)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        return (int)_this->counts[i];
    }

    static void getPosition(const SMikkTSpaceContext *tctx, float *o_pos, int iface, int ivtx)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        const int *face = &_this->indices[_this->offsets[iface]];
        (float3&)*o_pos = _this->points[face[ivtx]];
    }

    static void getPositionFlattened(const SMikkTSpaceContext *tctx, float *o_pos, int iface, int ivtx)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        (float3&)*o_pos = _this->points[_this->offsets[iface] + ivtx];
    }

    static void getNormal(const SMikkTSpaceContext *tctx, float *o_normal, int iface, int ivtx)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        const int *face = &_this->indices[_this->offsets[iface]];
        (float3&)*o_normal = _this->normals[face[ivtx]];
    }

    static void getNormalFlattened(const SMikkTSpaceContext *tctx, float *o_normal, int iface, int ivtx)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        (float3&)*o_normal = _this->normals[_this->offsets[iface] + ivtx];
    }

    static void getTexCoord(const SMikkTSpaceContext *tctx, float *o_tcoord, int iface, int ivtx)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        const int *face = &_this->indices[_this->offsets[iface]];
        (float2&)*o_tcoord = _this->uv[face[ivtx]];
    }

    static void getTexCoordFlattened(const SMikkTSpaceContext *tctx, float *o_tcoord, int iface, int ivtx)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        (float2&)*o_tcoord = _this->uv[_this->offsets[iface] + ivtx];
    }

    static void setTangent(const SMikkTSpaceContext *tctx, const float* tangent, const float* /*bitangent*/,
        float /*fMagS*/, float /*fMagT*/, tbool IsOrientationPreserving, int iface, int ivtx)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        const int *face = &_this->indices[_this->offsets[iface]];
        float sign = (IsOrientationPreserving != 0) ? 1.0f : -1.0f;
        _this->dst[face[ivtx]] = { tangent[0], tangent[1], tangent[2], sign };
    }

    static void setTangentFlattened(const SMikkTSpaceContext *tctx, const float* tangent, const float* /*bitangent*/,
        float /*fMagS*/, float /*fMagT*/, tbool IsOrientationPreserving, int iface, int ivtx)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        float sign = (IsOrientationPreserving != 0) ? 1.0f : -1.0f;
        _this->dst[_this->offsets[iface] + ivtx] = { tangent[0], tangent[1], tangent[2], sign };
    }
};

bool GenerateTangents(
    IArray<float4> dst, const IArray<float3> points, const IArray<float3> normals, const IArray<float2> uv,
    const IArray<int> counts, const IArray<int> offsets, const IArray<int> indices)
{
    TSpaceContext ctx = {dst, points, normals, uv, counts, offsets, indices};

    SMikkTSpaceInterface iface;
    memset(&iface, 0, sizeof(iface));
    iface.m_getNumFaces = TSpaceContext::getNumFaces;
    iface.m_getNumVerticesOfFace = TSpaceContext::getCount;
    iface.m_getPosition = points.size()  == indices.size() ? TSpaceContext::getPositionFlattened : TSpaceContext::getPosition;
    iface.m_getNormal   = normals.size() == indices.size() ? TSpaceContext::getNormalFlattened : TSpaceContext::getNormal;
    iface.m_getTexCoord = uv.size()      == indices.size() ? TSpaceContext::getTexCoordFlattened : TSpaceContext::getTexCoord;
    iface.m_setTSpace   = dst.size()     == indices.size() ? TSpaceContext::setTangentFlattened : TSpaceContext::setTangent;

    SMikkTSpaceContext tctx;
    memset(&tctx, 0, sizeof(tctx));
    tctx.m_pInterface = &iface;
    tctx.m_pUserData = &ctx;

    return genTangSpaceDefault(&tctx) != 0;
}



template<int N>
bool GenerateWeightsN(RawVector<Weights<N>>& dst, IArray<int> bone_indices, IArray<float> bone_weights, int bones_per_vertex)
{
    if (bone_indices.size() != bone_weights.size()) {
        return false;
    }

    int num_weightsN = (int)bone_indices.size() / bones_per_vertex;
    dst.resize(num_weightsN);

    if (bones_per_vertex <= N) {
        dst.zeroclear();
        int bpvN = std::min<int>(N, bones_per_vertex);
        for (int wi = 0; wi < num_weightsN; ++wi) {
            auto *bindices = &bone_indices[bones_per_vertex * wi];
            auto *bweights = &bone_weights[bones_per_vertex * wi];

            // copy (up to) N elements
            auto& w4 = dst[wi];
            for (int oi = 0; oi < bpvN; ++oi) {
                w4.indices[oi] = bindices[oi];
                w4.weights[oi] = bweights[oi];
            }
        }
    }
    else {
        int *order = (int*)alloca(sizeof(int) * bones_per_vertex);
        for (int wi = 0; wi < num_weightsN; ++wi) {
            auto *bindices = &bone_indices[bones_per_vertex * wi];
            auto *bweights = &bone_weights[bones_per_vertex * wi];

            // sort order
            std::iota(order, order + bones_per_vertex, 0);
            std::nth_element(order, order + N, order + bones_per_vertex,
                [&](int a, int b) { return bweights[a] > bweights[b]; });

            // copy (up to) N elements
            auto& w4 = dst[wi];
            float w = 0.0f;
            for (int oi = 0; oi < N; ++oi) {
                int o = order[oi];
                w4.indices[oi] = bindices[o];
                w4.weights[oi] = bweights[o];
                w += bweights[o];
            }

            // normalize weights
            float rcpw = 1.0f / w;
            for (int oi = 0; oi < N; ++oi) {
                w4.weights[oi] *= rcpw;
            }
        }
    }
    return true;
}
template bool GenerateWeightsN(RawVector<Weights<4>>& dst, IArray<int> bone_indices, IArray<float> bone_weights, int bones_per_vertex);
template bool GenerateWeightsN(RawVector<Weights<8>>& dst, IArray<int> bone_indices, IArray<float> bone_weights, int bones_per_vertex);


void ConnectionData::clear()
{
    counts.clear();
    offsets.clear();
    faces.clear();
    indices.clear();
    weld_map.clear();
}

namespace {

// "welded" indices
struct IndicesW
{
    IArray<int> m_indices;
    IArray<int> m_weld_map;

    size_t size() const { return m_indices.size(); }
    int operator[](size_t i) const { return m_weld_map[m_indices[i]]; }
};

struct CountsC
{
    int m_ngon;
    size_t m_size;

    size_t size() const { return m_size; }
    int operator[](size_t /*i*/) const { return m_ngon; }
};

struct OffsetsC
{
    int m_ngon;
    size_t m_size;

    size_t size() const { return m_size; }
    int operator[](size_t i) const { return m_ngon * (int)i; }
};

} // namespace

template<class Index, class Counts>
static void BuildConnectionDataImpl(
    ConnectionData& connection, const Index& indices, const Counts& counts, const IArray<float3>& vertices)
{
    size_t num_points = vertices.size();
    size_t num_faces = counts.size();
    size_t num_indices = indices.size();

    connection.offsets.resize(num_points);
    connection.faces.resize(num_indices);
    connection.indices.resize(num_indices);

    connection.counts.resize(num_points);
    connection.counts.zeroclear();
    {
        int ii = 0;
        for (size_t fi = 0; fi < num_faces; ++fi) {
            int c = counts[fi];
            for (int ci = 0; ci < c; ++ci) {
                connection.counts[indices[ii + ci]]++;
            }
            ii += c;
        }

        int offset = 0;
        for (size_t i = 0; i < num_points; ++i) {
            connection.offsets[i] = offset;
            offset += connection.counts[i];
        }
    }

    connection.counts.zeroclear();
    {
        int i = 0;
        for (int fi = 0; fi < (int)num_faces; ++fi) {
            int c = counts[fi];
            for (int ci = 0; ci < c; ++ci) {
                int vi = indices[i + ci];
                int ti = connection.offsets[vi] + connection.counts[vi]++;
                connection.faces[ti] = fi;
                connection.indices[ti] = i + ci;
            }
            i += c;
        }
    }
}

void ConnectionData::buildConnection(
    const IArray<int>& indices_, int ngon_, const IArray<float3>& vertices_, bool welding)
{
    if (welding) {
        buildIdenticalPositionMap(vertices_);

        IndicesW indices__{ indices_, weld_map };
        CountsC counts_{ ngon_, indices_.size()/ngon_ };
        BuildConnectionDataImpl(*this, indices__, counts_, vertices_);
    }
    else {
        CountsC counts_{ ngon_, indices_.size() / ngon_ };
        BuildConnectionDataImpl(*this, indices_, counts_, vertices_);
    }
}

void ConnectionData::buildConnection(
    const IArray<int>& indices_, const IArray<int>& counts_, const IArray<int>& /*offsets_*/, const IArray<float3>& vertices_, bool welding)
{
    if (welding) {
        buildIdenticalPositionMap(vertices_);

        IndicesW vi{ indices_, weld_map };
        BuildConnectionDataImpl(*this, vi, counts_, vertices_);
    }
    else {
        BuildConnectionDataImpl(*this, indices_, counts_, vertices_);
    }
}

void ConnectionData::buildIdenticalPositionMap(const IArray<float3>& vertices)
{
    int n = (int)vertices.size();
    weld_map.resize(n);
    parallel_for(0, n, [&](int vi) {
        int r = vi;
        float3 p = vertices[vi];
        for (int i = 0; i < vi; ++i) {
            if (near_equal(length_sq(vertices[i] - p), 0.0f)) {
                r = i;
                break;
            }
        }
        weld_map[vi] = r;
    });
}


template<class Index, class Counts, class Offsets>
bool OnEdgeImpl(const Index& indices, const Counts& counts, const Offsets& offsets, const IArray<float3>& vertices, const ConnectionData& connection, int vertex_index)
{
    int num_shared = connection.counts[vertex_index];
    int offset = connection.offsets[vertex_index];

    float angle = 0.0f;
    for (int si = 0; si < num_shared; ++si) {
        int fi = connection.faces[offset + si];
        int fo = offsets[fi];
        int c = counts[fi];
        if (c < 3) { continue; }
        int nth = connection.indices[offset + si] - fo;

        int f0 = nth;
        int f1 = f0 - 1; if (f1 < 0) { f1 = c - 1; }
        int f2 = f0 + 1; if (f2 == c) { f2 = 0; }
        float3 v0 = vertices[indices[c * fi + f0]];
        float3 v1 = vertices[indices[c * fi + f1]];
        float3 v2 = vertices[indices[c * fi + f2]];
        angle += angle_between(v1, v2, v0);
    }
    // angle_between's precision seems very low on Mac.. it can be like 357.9f on closed edge
    return angle < 357.0f * Deg2Rad;
}

bool OnEdge(const IArray<int>& indices, int ngon, const IArray<float3>& vertices, const ConnectionData& connection, int vertex_index)
{
    CountsC counts{ ngon, indices.size() / ngon };
    OffsetsC offsets{ ngon, indices.size() / ngon };
    return OnEdgeImpl(indices, counts, offsets, vertices, connection, vertex_index);
}

bool OnEdge(const IArray<int>& indices, const IArray<int>& counts, const IArray<int>& offsets, const IArray<float3>& vertices, const ConnectionData& connection, int vertex_index)
{
    return OnEdgeImpl(indices, counts, offsets, vertices, connection, vertex_index);
}


template<class Index, class Counts, class Offsets>
bool IsEdgeOpenedImpl(const Index& indices, const Counts& counts, const Offsets& offsets, const ConnectionData& connection, int i0, int i1)
{
    if (i1 < i0) { std::swap(i0, i1); }
    int edge[2]{ i0, i1 };

    int num_connection = 0;
    for (int e : edge) {
        int num_shared = connection.counts[e];
        int offset = connection.offsets[e];
        for (int si = 0; si < num_shared; ++si) {
            int fi = connection.faces[offset + si];
            int fo = offsets[fi];
            int c = counts[fi];
            if (c < 3) { continue; }
            for (int ci = 0; ci < c; ++ci) {
                int j0 = indices[fo + ci];
                int j1 = indices[fo + (ci + 1) % c];
                if (j1 < j0) { std::swap(j0, j1); }

                if (i0 == j0 && i1 == j1) { ++num_connection; }
            }
        }
    }
    return num_connection == 2;
}

bool IsEdgeOpened(const IArray<int>& indices, int ngon, const ConnectionData& connection, int i0, int i1)
{
    CountsC counts{ ngon, indices.size() / ngon };
    OffsetsC offsets{ ngon, indices.size() / ngon };
    return IsEdgeOpenedImpl(indices, counts, offsets, connection, i0, i1);
}

bool IsEdgeOpened(const IArray<int>& indices, const IArray<int>& counts, const IArray<int>& offsets, const ConnectionData& connection, int i0, int i1)
{
    return IsEdgeOpenedImpl(indices, counts, offsets, connection, i0, i1);
}



template<class Index, class Counts, class Offsets>
struct SelectEdgeImpl
{
    const Index& indices;
    const Counts& counts;
    const Offsets& offsets;
    const IArray<float3>& vertices;
    const ConnectionData& connection;
    RawVector<int>& dst;

    std::vector<bool> checked;
    std::vector<std::pair<int, int>> opened;

    SelectEdgeImpl(const Index& indices_, const Counts& counts_, const Offsets& offsets_, const IArray<float3>& vertices_,
        const ConnectionData& connection_, RawVector<int>& dst_)
        : indices(indices_)
        , counts(counts_)
        , offsets(offsets_)
        , vertices(vertices_)
        , connection(connection_)
        , dst(dst_)
    {
        checked.resize(vertices.size());
    }

    void select(int vertex_index)
    {
        if (checked[vertex_index]) { return; }

        {
            int num_shared = connection.counts[vertex_index];
            int offset = connection.offsets[vertex_index];
            for (int si = 0; si < num_shared; ++si) {
                int fi = connection.faces[offset + si];
                int fo = offsets[fi];
                int c = counts[fi];
                int nth = connection.indices[offset + si] - fo;

                int f0 = nth;
                int f1 = f0 - 1; if (f1 < 0) { f1 = c - 1; }
                int f2 = f0 + 1; if (f2 == c) { f2 = 0; }

                opened.push_back(std::make_pair(indices[fo + f0], indices[fo + f1]));
                opened.push_back(std::make_pair(indices[fo + f0], indices[fo + f2]));
            }
        }

        while (!opened.empty()) {
            int i0 = opened.back().first;
            int i1 = opened.back().second;
            opened.pop_back();

            if (checked[i0] && checked[i1]) { continue; }

            if (IsEdgeOpenedImpl(indices, counts, offsets, connection, i0, i1)) {
                if (!checked[i0]) { checked[i0] = true; dst.push_back(i0); }
                if (!checked[i1]) { checked[i1] = true; dst.push_back(i1); }

                int num_shared = connection.counts[i1];
                int offset = connection.offsets[i1];
                for (int si = 0; si < num_shared; ++si) {
                    int fi = connection.faces[offset + si];
                    int fo = offsets[fi];
                    int c = counts[fi];
                    int nth = connection.indices[offset + si] - fo;

                    int f0 = nth;
                    int f1 = f0 - 1; if (f1 < 0) { f1 = c - 1; }
                    int f2 = f0 + 1; if (f2 == c) { f2 = 0; }

                    opened.push_back(std::make_pair(indices[fo + f0], indices[fo + f1]));
                    opened.push_back(std::make_pair(indices[fo + f0], indices[fo + f2]));
                }
            }
        }
    }
};

void SelectEdge(const IArray<int>& indices, int ngon, const IArray<float3>& vertices, const ConnectionData& connection,
    const IArray<int>& vertex_indices, RawVector<int>& edge_indices)
{
    CountsC counts{ ngon, indices.size() / ngon };
    OffsetsC offsets{ ngon, indices.size() / ngon };
    SelectEdgeImpl<decltype(indices), decltype(counts), decltype(offsets)>
        impl(indices, counts, offsets, vertices, connection, edge_indices);

    for (int i : vertex_indices) {
        impl.select(i);
    }
}

void SelectEdge(const IArray<int>& indices, const IArray<int>& counts, const IArray<int>& offsets, const IArray<float3>& vertices, const ConnectionData& connection,
    const IArray<int>& vertex_indices, RawVector<int>& edge_indices)
{
    SelectEdgeImpl<decltype(indices), decltype(counts), decltype(offsets)>
        impl(indices, counts, offsets, vertices, connection, edge_indices);

    for (int i : vertex_indices) {
        impl.select(i);
    }
}

void SelectHole(const IArray<int>& indices_, int ngon, const IArray<float3>& vertices, const ConnectionData& connection,
    const IArray<int>& vertex_indices, RawVector<int>& edge_indices)
{
    IndicesW indices{ indices_, connection.weld_map };
    CountsC counts{ ngon, indices.size() / ngon };
    OffsetsC offsets{ ngon, indices.size() / ngon };
    SelectEdgeImpl<decltype(indices), decltype(counts), decltype(offsets)>
        impl(indices, counts, offsets, vertices, connection, edge_indices);

    for (int i : vertex_indices) {
        impl.select(i);
    }
}

void SelectHole(const IArray<int>& indices_, const IArray<int>& counts, const IArray<int>& offsets, const IArray<float3>& vertices, const ConnectionData& connection,
    const IArray<int>& vertex_indices, RawVector<int>& edge_indices)
{
    IndicesW indices{ indices_, connection.weld_map };
    SelectEdgeImpl<decltype(indices), decltype(counts), decltype(offsets)>
        impl(indices, counts, offsets, vertices, connection, edge_indices);

    for (int i : vertex_indices) {
        impl.select(i);
    }
}



} // namespace mu
