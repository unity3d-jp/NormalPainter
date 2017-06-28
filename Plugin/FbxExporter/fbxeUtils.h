#pragma once

namespace fbxe {

inline FbxVector2 ToV2(float2 v) { return { v.x, v.y }; }
inline FbxDouble3 ToP3(float3 v) { return { v.x, v.y, v.z }; }
inline FbxDouble4 ToP4(float3 v) { return { v.x, v.y, v.z, 1.0f }; }
inline FbxVector4 ToV4(float3 v) { return { v.x, v.y, v.z, 0.0f }; }
inline FbxDouble4 ToP4(float4 v) { return { v.x, v.y, v.z, v.w }; }
inline FbxVector4 ToV4(float4 v) { return { v.x, v.y, v.z, v.w }; }
inline FbxColor   ToC4(float4 v) { return { v.x, v.y, v.z, v.w }; }
inline FbxAMatrix ToAM44(float4x4 v)
{
    FbxAMatrix ret;
    auto src = &v[0][0];
    auto dst = (double*)ret;
    for (int i = 0; i < 16; ++i) {
        dst[i] = src[i];
    }
    return ret;
}

inline void FlipHandedness(FbxDouble4 *v, int n)
{
    for (int i = 0; i < n; ++i) { v[i].mData[0] *= -1.0; }
}
inline void FlipHandedness(FbxVector4 *v, int n)
{
    for (int i = 0; i < n; ++i) { v[i].mData[0] *= -1.0; }
}



template<int N>
inline int GetInfluence(Weights<N> weights[], int num_vertices, int bone_index, RawVector<int>& dindices, RawVector<double>& dweights)
{
    dindices.clear();
    dweights.clear();

    for (int vi = 0; vi < num_vertices; ++vi) {
        for (int i = 0; i < N; ++i) {
            if (weights[vi].indices[i] == bone_index) {
                float w = weights[vi].weights[i];
                if (w > 0.0f) {
                    dindices.push_back(vi);
                    dweights.push_back(w);
                }
                break;
            }
        }
    }
    return (int)dindices.size();
}

} // namespace fbxe
