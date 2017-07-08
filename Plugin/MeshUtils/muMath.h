#pragma once

#include <cmath>
#include <cstring>
#include <algorithm>
#ifdef muEnableHalf
    #include "half.h"
#endif // muEnableHalf
#include "muIntrusiveArray.h"

#define muEpsilon 1e-4f
#define muMath_AddNamespace

#ifdef muMath_AddNamespace
namespace mu {
#endif

extern const float PI;
extern const float Deg2Rad;
extern const float Rad2Deg;

template<class T>
struct tvec2
{
    T x, y;
    T& operator[](int i) { return ((T*)this)[i]; }
    const T& operator[](int i) const { return ((T*)this)[i]; }
    bool operator==(const tvec2& v) const { return x == v.x && y == v.y; }
    bool operator!=(const tvec2& v) const { return !((*this)==v); }

    template<class U> void assign(const U *v)
    {
        *this = { (T)v[0], (T)v[1] };
    }

    static tvec2 zero() { return{ (T)0, (T)0 }; }
    static tvec2 one() { return{ (T)1, (T)1 }; }
};
using float2 = tvec2<float>;
using double2 = tvec2<double>;

template<class T>
struct tvec3
{
    T x, y, z;
    T& operator[](int i) { return ((T*)this)[i]; }
    const T& operator[](int i) const { return ((T*)this)[i]; }
    bool operator==(const tvec3& v) const { return x == v.x && y == v.y && z == v.z; }
    bool operator!=(const tvec3& v) const { return !((*this) == v); }

    template<class U> void assign(const U *v)
    {
        *this = { (T)v[0], (T)v[1], (T)v[2] };
    }

    static tvec3 zero() { return{ (T)0, (T)0, (T)0 }; }
    static tvec3 one() { return{ (T)1, (T)1, (T)1 }; }
};
using float3 = tvec3<float>;
using double3 = tvec3<double>;

template<class T>
struct tvec4
{
    T x, y, z, w;
    T& operator[](int i) { return ((T*)this)[i]; }
    const T& operator[](int i) const { return ((T*)this)[i]; }
    bool operator==(const tvec4& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
    bool operator!=(const tvec4& v) const { return !((*this) == v); }

    template<class U> void assign(const U *v)
    {
        *this = { (T)v[0], (T)v[1], (T)v[2], (T)v[3] };
    }

    static tvec4 zero() { return{ (T)0, (T)0, (T)0, (T)0 }; }
    static tvec4 one() { return{ (T)1, (T)1, (T)1, (T)1 }; }
};
using float4 = tvec4<float>;
using double4 = tvec4<double>;

template<class T>
struct tquat
{
    T x, y, z, w;
    T& operator[](int i) { return ((T*)this)[i]; }
    const T& operator[](int i) const { return ((T*)this)[i]; }
    bool operator==(const tquat& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
    bool operator!=(const tquat& v) const { return !((*this) == v); }

    template<class U> void assign(const U *v)
    {
        *this = { (T)v[0], (T)v[1], (T)v[2], (T)v[3] };
    }

    static tquat identity() { return{ (T)0, (T)0, (T)0, (T)1 }; }
};
using quatf = tquat<float>;
using quatd = tquat<double>;

template<class T>
struct tmat3x3
{
    tvec3<T> m[3];
    tvec3<T>& operator[](int i) { return m[i]; }
    const float3& operator[](int i) const { return m[i]; }
    bool operator==(const tmat3x3& v) const { return memcmp(m, v.m, sizeof(*this)) == 0; }
    bool operator!=(const tmat3x3& v) const { return !((*this) == v); }

    template<class U> void assign(const U *v)
    {
        *this = { {
            { (T)v[0], (T)v[1], (T)v[2] },
            { (T)v[3], (T)v[4], (T)v[5] },
            { (T)v[6], (T)v[7], (T)v[8] }
        } };
    }

    static tmat3x3 identity()
    {
        return{ {
            { (T)1, (T)0, (T)0 },
            { (T)0, (T)1, (T)0 },
            { (T)0, (T)0, (T)1 },
        } };
    }
};
using float3x3 = tmat3x3<float>;
using double3x3 = tmat3x3<double>;

template<class T>
struct tmat4x4
{
    tvec4<T> m[4];
    tvec4<T>& operator[](int i) { return m[i]; }
    const tvec4<T>& operator[](int i) const { return m[i]; }
    bool operator==(const tmat4x4& v) const { return memcmp(m, v.m, sizeof(*this)) == 0; }
    bool operator!=(const tmat4x4& v) const { return !((*this) == v); }

    void assign(const T *v)
    {
        memcpy(this, v, sizeof(*this));
    }
    template<class U> void assign(const U *v)
    {
        *this = { {
            { (T)v[0], (T)v[1], (T)v[2], (T)v[3] },
            { (T)v[4], (T)v[5], (T)v[6], (T)v[7] },
            { (T)v[8], (T)v[9], (T)v[10],(T)v[11]},
            { (T)v[12],(T)v[13],(T)v[14],(T)v[15]}
        } };
    }

    static tmat4x4 identity()
    {
        return{ {
            { (T)1, (T)0, (T)0, (T)0 },
            { (T)0, (T)1, (T)0, (T)0 },
            { (T)0, (T)0, (T)1, (T)0 },
            { (T)0, (T)0, (T)0, (T)1 },
        } };
    }
};
using float4x4 = tmat4x4<float>;
using double4x4 = tmat4x4<double>;


template<class T> inline tvec2<T> operator-(const tvec2<T>& v) { return{ -v.x, -v.y }; }
template<class T> inline tvec2<T> operator+(const tvec2<T>& l, const tvec2<T>& r) { return{ l.x + r.x, l.y + r.y }; }
template<class T> inline tvec2<T> operator-(const tvec2<T>& l, const tvec2<T>& r) { return{ l.x - r.x, l.y - r.y }; }
template<class T> inline tvec2<T> operator*(const tvec2<T>& l, const tvec2<T>& r) { return{ l.x * r.x, l.y * r.y }; }
template<class T> inline tvec2<T> operator/(const tvec2<T>& l, const tvec2<T>& r) { return{ l.x / r.x, l.y / r.y }; }
template<class T> inline tvec2<T> operator+(T l, const tvec2<T>& r) { return{ l + r.x, l + r.y }; }
template<class T> inline tvec2<T> operator-(T l, const tvec2<T>& r) { return{ l - r.x, l - r.y }; }
template<class T> inline tvec2<T> operator*(T l, const tvec2<T>& r) { return{ l * r.x, l * r.y }; }
template<class T> inline tvec2<T> operator/(T l, const tvec2<T>& r) { return{ l / r.x, l / r.y }; }
template<class T> inline tvec2<T> operator+(const tvec2<T>& l, T r) { return{ l.x + r, l.y + r }; }
template<class T> inline tvec2<T> operator-(const tvec2<T>& l, T r) { return{ l.x - r, l.y - r }; }
template<class T> inline tvec2<T> operator*(const tvec2<T>& l, T r) { return{ l.x * r, l.y * r }; }
template<class T> inline tvec2<T> operator/(const tvec2<T>& l, T r) { return{ l.x / r, l.y / r }; }
template<class T> inline tvec2<T>& operator+=(tvec2<T>& l, const tvec2<T>& r) { l.x += r.x; l.y += r.y; return l; }
template<class T> inline tvec2<T>& operator-=(tvec2<T>& l, const tvec2<T>& r) { l.x -= r.x; l.y -= r.y; return l; }
template<class T> inline tvec2<T>& operator*=(tvec2<T>& l, const tvec2<T>& r) { l.x *= r.x; l.y *= r.y; return l; }
template<class T> inline tvec2<T>& operator/=(tvec2<T>& l, const tvec2<T>& r) { l.x /= r.x; l.y /= r.y; return l; }
template<class T> inline tvec2<T>& operator+=(tvec2<T>& l, T r) { l.x += r; l.y += r; return l; }
template<class T> inline tvec2<T>& operator-=(tvec2<T>& l, T r) { l.x -= r; l.y -= r; return l; }
template<class T> inline tvec2<T>& operator*=(tvec2<T>& l, T r) { l.x *= r; l.y *= r; return l; }
template<class T> inline tvec2<T>& operator/=(tvec2<T>& l, T r) { l.x /= r; l.y /= r; return l; }

template<class T> inline tvec3<T> operator-(const tvec3<T>& v) { return{ -v.x, -v.y, -v.z }; }
template<class T> inline tvec3<T> operator+(const tvec3<T>& l, const tvec3<T>& r) { return{ l.x + r.x, l.y + r.y, l.z + r.z }; }
template<class T> inline tvec3<T> operator-(const tvec3<T>& l, const tvec3<T>& r) { return{ l.x - r.x, l.y - r.y, l.z - r.z }; }
template<class T> inline tvec3<T> operator*(const tvec3<T>& l, const tvec3<T>& r) { return{ l.x * r.x, l.y * r.y, l.z * r.z }; }
template<class T> inline tvec3<T> operator/(const tvec3<T>& l, const tvec3<T>& r) { return{ l.x / r.x, l.y / r.y, l.z / r.z }; }
template<class T> inline tvec3<T> operator+(T l, const tvec3<T>& r) { return{ l + r.x, l + r.y, l + r.z }; }
template<class T> inline tvec3<T> operator-(T l, const tvec3<T>& r) { return{ l - r.x, l - r.y, l - r.z }; }
template<class T> inline tvec3<T> operator*(T l, const tvec3<T>& r) { return{ l * r.x, l * r.y, l * r.z }; }
template<class T> inline tvec3<T> operator/(T l, const tvec3<T>& r) { return{ l / r.x, l / r.y, l / r.z }; }
template<class T> inline tvec3<T> operator+(const tvec3<T>& l, T r) { return{ l.x + r, l.y + r, l.z + r }; }
template<class T> inline tvec3<T> operator-(const tvec3<T>& l, T r) { return{ l.x - r, l.y - r, l.z - r }; }
template<class T> inline tvec3<T> operator*(const tvec3<T>& l, T r) { return{ l.x * r, l.y * r, l.z * r }; }
template<class T> inline tvec3<T> operator/(const tvec3<T>& l, T r) { return{ l.x / r, l.y / r, l.z / r }; }
template<class T> inline tvec3<T>& operator+=(tvec3<T>& l, const tvec3<T>& r) { l.x += r.x; l.y += r.y; l.z += r.z; return l; }
template<class T> inline tvec3<T>& operator-=(tvec3<T>& l, const tvec3<T>& r) { l.x -= r.x; l.y -= r.y; l.z -= r.z; return l; }
template<class T> inline tvec3<T>& operator*=(tvec3<T>& l, const tvec3<T>& r) { l.x *= r.x; l.y *= r.y; l.z *= r.z; return l; }
template<class T> inline tvec3<T>& operator/=(tvec3<T>& l, const tvec3<T>& r) { l.x /= r.x; l.y /= r.y; l.z /= r.z; return l; }
template<class T> inline tvec3<T>& operator+=(tvec3<T>& l, T r) { l.x += r; l.y += r; l.z += r; return l; }
template<class T> inline tvec3<T>& operator-=(tvec3<T>& l, T r) { l.x -= r; l.y -= r; l.z -= r; return l; }
template<class T> inline tvec3<T>& operator*=(tvec3<T>& l, T r) { l.x *= r; l.y *= r; l.z *= r; return l; }
template<class T> inline tvec3<T>& operator/=(tvec3<T>& l, T r) { l.x /= r; l.y /= r; l.z /= r; return l; }

template<class T> inline tvec4<T> operator-(const tvec4<T>& v) { return{ -v.x, -v.y, -v.z, -v.w }; }
template<class T> inline tvec4<T> operator+(const tvec4<T>& l, const tvec4<T>& r) { return{ l.x + r.x, l.y + r.y, l.z + r.z, l.w + r.w }; }
template<class T> inline tvec4<T> operator-(const tvec4<T>& l, const tvec4<T>& r) { return{ l.x - r.x, l.y - r.y, l.z - r.z, l.w - r.w }; }
template<class T> inline tvec4<T> operator*(const tvec4<T>& l, const tvec4<T>& r) { return{ l.x * r.x, l.y * r.y, l.z * r.z, l.w * r.w }; }
template<class T> inline tvec4<T> operator/(const tvec4<T>& l, const tvec4<T>& r) { return{ l.x / r.x, l.y / r.y, l.z / r.z, l.w / r.w }; }
template<class T> inline tvec4<T> operator+(T l, const tvec4<T>& r) { return{ l + r.x, l + r.y, l + r.z, l + r.w }; }
template<class T> inline tvec4<T> operator-(T l, const tvec4<T>& r) { return{ l - r.x, l - r.y, l - r.z, l - r.w }; }
template<class T> inline tvec4<T> operator*(T l, const tvec4<T>& r) { return{ l * r.x, l * r.y, l * r.z, l * r.w }; }
template<class T> inline tvec4<T> operator/(T l, const tvec4<T>& r) { return{ l / r.x, l / r.y, l / r.z, l / r.w }; }
template<class T> inline tvec4<T> operator+(const tvec4<T>& l, T r) { return{ l.x + r, l.y + r, l.z + r, l.w + r }; }
template<class T> inline tvec4<T> operator-(const tvec4<T>& l, T r) { return{ l.x - r, l.y - r, l.z - r, l.w - r }; }
template<class T> inline tvec4<T> operator*(const tvec4<T>& l, T r) { return{ l.x * r, l.y * r, l.z * r, l.w * r }; }
template<class T> inline tvec4<T> operator/(const tvec4<T>& l, T r) { return{ l.x / r, l.y / r, l.z / r, l.w / r }; }
template<class T> inline tvec4<T>& operator+=(tvec4<T>& l, const tvec4<T>& r) { l.x += r.x; l.y += r.y; l.z += r.z; l.w += r.w; return l; }
template<class T> inline tvec4<T>& operator-=(tvec4<T>& l, const tvec4<T>& r) { l.x -= r.x; l.y -= r.y; l.z -= r.z; l.w -= r.w; return l; }
template<class T> inline tvec4<T>& operator*=(tvec4<T>& l, const tvec4<T>& r) { l.x *= r.x; l.y *= r.y; l.z *= r.z; l.w *= r.w; return l; }
template<class T> inline tvec4<T>& operator/=(tvec4<T>& l, const tvec4<T>& r) { l.x /= r.x; l.y /= r.y; l.z /= r.z; l.w /= r.w; return l; }
template<class T> inline tvec4<T>& operator+=(tvec4<T>& l, T r) { l.x += r; l.y += r; l.z += r; l.w += r; return l; }
template<class T> inline tvec4<T>& operator-=(tvec4<T>& l, T r) { l.x -= r; l.y -= r; l.z -= r; l.w -= r; return l; }
template<class T> inline tvec4<T>& operator*=(tvec4<T>& l, T r) { l.x *= r; l.y *= r; l.z *= r; l.w *= r; return l; }
template<class T> inline tvec4<T>& operator/=(tvec4<T>& l, T r) { l.x /= r; l.y /= r; l.z /= r; l.w /= r; return l; }


template<class T> inline tquat<T> operator*(const tquat<T>& l, float r) { return{ l.x*r, l.y*r, l.z*r, l.w*r }; }
template<class T> inline tquat<T> operator*(const tquat<T>& l, const tquat<T>& r)
{
    return{
        l.w*r.x + l.x*r.w + l.y*r.z - l.z*r.y,
        l.w*r.y + l.y*r.w + l.z*r.x - l.x*r.z,
        l.w*r.z + l.z*r.w + l.x*r.y - l.y*r.x,
        l.w*r.w - l.x*r.x - l.y*r.y - l.z*r.z,
    };
}
template<class T> inline tquat<T>& operator*=(tquat<T>& l, float r)
{
    l = l * r;
    return l;
}
template<class T> inline tquat<T>& operator*=(tquat<T>& l, const tquat<T>& r)
{
    l = l * r;
    return l;
}

template<class T> inline tvec3<T> operator*(const tmat3x3<T>& m, const tvec3<T>& v)
{
    return{
        m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2],
        m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2],
        m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2],
    };
}
template<class T> inline tvec3<T> operator*(const tmat4x4<T>& m, const tvec3<T>& v)
{
    return{
        m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2],
        m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2],
        m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2],
    };
}
template<class T> inline tvec4<T> operator*(const tmat4x4<T>& m, const tvec4<T>& v)
{
    return{
        m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2] + m[3][0] * v[3],
        m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2] + m[3][1] * v[3],
        m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2] + m[3][2] * v[3],
        m[0][3] * v[0] + m[1][3] * v[1] + m[2][3] * v[2] + m[3][3] * v[3],
    };
}
template<class T> inline tmat4x4<T> operator*(const tmat4x4<T> &a, const tmat4x4<T> &b)
{
    tmat4x4<T> c;
    const float *ap = &a[0][0];
    const float *bp = &b[0][0];
    float *cp = &c[0][0];
    float a0, a1, a2, a3;

    a0 = ap[0];
    a1 = ap[1];
    a2 = ap[2];
    a3 = ap[3];

    cp[0] = a0 * bp[0] + a1 * bp[4] + a2 * bp[8] + a3 * bp[12];
    cp[1] = a0 * bp[1] + a1 * bp[5] + a2 * bp[9] + a3 * bp[13];
    cp[2] = a0 * bp[2] + a1 * bp[6] + a2 * bp[10] + a3 * bp[14];
    cp[3] = a0 * bp[3] + a1 * bp[7] + a2 * bp[11] + a3 * bp[15];

    a0 = ap[4];
    a1 = ap[5];
    a2 = ap[6];
    a3 = ap[7];

    cp[4] = a0 * bp[0] + a1 * bp[4] + a2 * bp[8] + a3 * bp[12];
    cp[5] = a0 * bp[1] + a1 * bp[5] + a2 * bp[9] + a3 * bp[13];
    cp[6] = a0 * bp[2] + a1 * bp[6] + a2 * bp[10] + a3 * bp[14];
    cp[7] = a0 * bp[3] + a1 * bp[7] + a2 * bp[11] + a3 * bp[15];

    a0 = ap[8];
    a1 = ap[9];
    a2 = ap[10];
    a3 = ap[11];

    cp[8] = a0 * bp[0] + a1 * bp[4] + a2 * bp[8] + a3 * bp[12];
    cp[9] = a0 * bp[1] + a1 * bp[5] + a2 * bp[9] + a3 * bp[13];
    cp[10] = a0 * bp[2] + a1 * bp[6] + a2 * bp[10] + a3 * bp[14];
    cp[11] = a0 * bp[3] + a1 * bp[7] + a2 * bp[11] + a3 * bp[15];

    a0 = ap[12];
    a1 = ap[13];
    a2 = ap[14];
    a3 = ap[15];

    cp[12] = a0 * bp[0] + a1 * bp[4] + a2 * bp[8] + a3 * bp[12];
    cp[13] = a0 * bp[1] + a1 * bp[5] + a2 * bp[9] + a3 * bp[13];
    cp[14] = a0 * bp[2] + a1 * bp[6] + a2 * bp[10] + a3 * bp[14];
    cp[15] = a0 * bp[3] + a1 * bp[7] + a2 * bp[11] + a3 * bp[15];
    return c;
}
template<class T> inline tmat4x4<T>& operator*=(tmat4x4<T>& a, const tmat4x4<T> &b)
{
    a = a * b;
    return a;
}

inline bool near_equal(float a, float b, float epsilon = muEpsilon)
{
    return std::abs(a - b) < epsilon;
}
inline bool near_equal(const float2& a, const float2& b, float e = muEpsilon)
{
    return near_equal(a.x, b.x, e) && near_equal(a.y, b.y, e);
}
inline bool near_equal(const float3& a, const float3& b, float e = muEpsilon)
{
    return near_equal(a.x, b.x, e) && near_equal(a.y, b.y, e) && near_equal(a.z, b.z, e);
}
inline bool near_equal(const float4& a, const float4& b, float e = muEpsilon)
{
    return near_equal(a.x, b.x, e) && near_equal(a.y, b.y, e) && near_equal(a.z, b.z, e) && near_equal(a.w, b.w, e);
}
inline bool near_equal(const quatf& a, const quatf& b, float e = muEpsilon)
{
    return near_equal(a.x, b.x, e) && near_equal(a.y, b.y, e) && near_equal(a.z, b.z, e) && near_equal(a.w, b.w, e);
}
inline bool near_equal(const float3x3& a, const float3x3& b, float e = muEpsilon)
{
    return near_equal(a[0], b[0], e) && near_equal(a[1], b[1], e) && near_equal(a[2], b[2], e);
}
inline bool near_equal(const float4x4& a, const float4x4& b, float e = muEpsilon)
{
    return near_equal(a[0], b[0], e) && near_equal(a[1], b[1], e) && near_equal(a[2], b[2], e) && near_equal(a[3], b[3], e);
}

template<class Int>
inline Int ceildiv(Int v, Int d)
{
    return v / d + (v % d == 0 ? 0 : 1);
}

inline float3 swap_handedness(const float3& v) { return { -v.x, v.y, v.z }; }
inline float4 swap_handedness(const float4& v) { return { -v.x, v.y, v.z, v.w }; }

inline static float clamp01(float v) { return std::max<float>(std::min<float>(v, 1.0f), 0.0f); }
inline static float clamp11(float v) { return std::max<float>(std::min<float>(v, 1.0f), -1.0f); }
inline static int clamp01(int v) { return std::max<int>(std::min<int>(v, 1), 0); }
inline static int clamp11(int v) { return std::max<int>(std::min<int>(v, 1), -1); }


inline static float3 mul_v(const float4x4& m, const float3& v)
{
    return {
        m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2],
        m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2],
        m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2],
    };
}
inline static float4 mul_v(const float4x4& m, const float4& v)
{
    return {
        m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2],
        m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2],
        m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2],
        v[3],
    };
}
inline static float3 mul_p(const float4x4& m, const float3& v)
{
    return {
        m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2] + m[3][0],
        m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2] + m[3][1],
        m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2] + m[3][2],
    };
}
inline static float4 mul4(const float4x4& m, const float3& v)
{
    return {
        m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2] + m[3][0],
        m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2] + m[3][1],
        m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2] + m[3][2],
        m[0][3] * v[0] + m[1][3] * v[1] + m[2][3] * v[2] + m[3][3],
    };
}

inline float2 min(float2 a, float2 b) { return{ std::min<float>(a.x, b.x), std::min<float>(a.y, b.y) }; }
inline float3 min(float3 a, float3 b) { return{ std::min<float>(a.x, b.x), std::min<float>(a.y, b.y), std::min<float>(a.z, b.z) }; }
inline float2 max(float2 a, float2 b) { return{ std::max<float>(a.x, b.x), std::max<float>(a.y, b.y) }; }
inline float3 max(float3 a, float3 b) { return{ std::max<float>(a.x, b.x), std::max<float>(a.y, b.y), std::max<float>(a.z, b.z) }; }

inline float  lerp(float  a, float  b, float t) { return a*(1.0f - t) + b*t; }
inline float2 lerp(float2 a, float2 b, float t) { return a*(1.0f - t) + b*t; }
inline float3 lerp(float3 a, float3 b, float t) { return a*(1.0f - t) + b*t; }

inline float dot(const float2& l, const float2& r) { return l.x*r.x + l.y*r.y; }
inline float dot(const float3& l, const float3& r) { return l.x*r.x + l.y*r.y + l.z*r.z; }
inline float length_sq(float2 v) { return dot(v, v); }
inline float length_sq(float3 v) { return dot(v, v); }
inline float length(float2 v) { return sqrt(length_sq(v)); }
inline float length(float3 v) { return sqrt(length_sq(v)); }

inline float3 normalize(const float3& v)
{
    return v / length(v);
}

inline float3 cross(const float3& l, const float3& r)
{
    return{
        l.y * r.z - l.z * r.y,
        l.z * r.x - l.x * r.z,
        l.x * r.y - l.y * r.x };
}

// a & b must be normalized
inline float angle_between(float3 a, float3 b)
{
    return std::acos(dot(a, b));
}
inline float angle_between2(float3 pos1, float3 pos2, float3 center)
{
    return angle_between(
        normalize(pos1 - center),
        normalize(pos2 - center));
}

inline float3 apply_rotation(const quatf& q, const float3& p)
{
    float3 a = cross((float3&)q, p);
    float3 b = cross((float3&)q, a);
    return p + (a * q.w + b) * 2.0f;
}

inline quatf invert(const quatf& v)
{
    return{ -v.x, -v.y, -v.z, v.w };
}

inline quatf flipY(const quatf& v)
{
    return{ -v.z, v.w, v.x, -v.y, };
}

inline quatf rotateX(float angle)
{
    float c = std::cos(angle * 0.5f);
    float s = std::sin(angle * 0.5f);
    return{ s, 0.0f, 0.0f, c };
}
inline quatf rotateY(float angle)
{
    float c = std::cos(angle * 0.5f);
    float s = std::sin(angle * 0.5f);
    return{ 0.0f, s, 0.0f, c };
}
inline quatf rotateZ(float angle)
{
    float c = std::cos(angle * 0.5f);
    float s = std::sin(angle * 0.5f);
    return{ 0.0f, 0.0f, s, c };
}

// eular -> quaternion
inline quatf rotateXYZ(const float3& euler)
{
    quatf qX = rotateX(euler.x);
    quatf qY = rotateY(euler.y);
    quatf qZ = rotateZ(euler.z);
    return (qZ * qY) * qX;
}
inline quatf rotateXZY(const float3& euler)
{
    quatf qX = rotateX(euler.x);
    quatf qY = rotateY(euler.y);
    quatf qZ = rotateZ(euler.z);
    return (qY * qZ) * qX;
}
inline quatf rotateYXZ(const float3& euler)
{
    quatf qX = rotateX(euler.x);
    quatf qY = rotateY(euler.y);
    quatf qZ = rotateZ(euler.z);
    return (qZ * qX) * qY;
}
inline quatf rotateYZX(const float3& euler)
{
    quatf qX = rotateX(euler.x);
    quatf qY = rotateY(euler.y);
    quatf qZ = rotateZ(euler.z);
    return (qX * qZ) * qY;
}
inline quatf rotateZXY(const float3& euler)
{
    quatf qX = rotateX(euler.x);
    quatf qY = rotateY(euler.y);
    quatf qZ = rotateZ(euler.z);
    return (qY * qX) * qZ;
}
inline quatf rotateZYX(const float3& euler)
{
    quatf qX = rotateX(euler.x);
    quatf qY = rotateY(euler.y);
    quatf qZ = rotateZ(euler.z);
    return (qX * qY) * qZ;
}

inline quatf rotate(const float3& axis, float angle)
{
    return{
        axis.x * std::sin(angle * 0.5f),
        axis.y * std::sin(angle * 0.5f),
        axis.z * std::sin(angle * 0.5f),
        std::cos(angle * 0.5f)
    };
}

template<class T> inline float clamp(T v, T vmin, T vmax) { return std::min<T>(std::max<T>(v, vmin), vmax); }
inline float saturate(float v) { return clamp(v, -1.0f, 1.0f); }

inline float3 to_eularZXY(const quatf& q)
{
    float d[] = {
        q.x*q.x, q.x*q.y, q.x*q.z, q.x*q.w,
        q.y*q.y, q.y*q.z, q.y*q.w,
        q.z*q.z, q.z*q.w,
        q.w*q.w
    };

    float v0 = d[5] - d[3];
    float v1 = 2.0f*(d[1] + d[8]);
    float v2 = d[4] - d[7] - d[0] + d[9];
    float v3 = -1.0f;
    float v4 = 2.0f * v0;

    const float SINGULARITY_CUTOFF = 0.499999f;
    if (std::abs(v0) < SINGULARITY_CUTOFF)
    {
        float v5 = 2.0f * (d[2] + d[6]);
        float v6 = d[7] - d[0] - d[4] + d[9];

        return{
            v3 * std::asin(saturate(v4)),
            std::atan2(v5, v6),
            std::atan2(v1, v2)
        };
    }
    else
    {
        float a = d[1] + d[8];
        float b =-d[5] + d[3];
        float c = d[1] - d[8];
        float e = d[5] + d[3];

        float v5 = a*e + b*c;
        float v6 = b*e - a*c;

        return{
            v3 * std::asin(saturate(v4)),
            std::atan2(v5, v6),
            0.0f
        };
    }
}

inline void to_axis_angle(const quatf& q, float3& axis, float& angle)
{
    angle = 2.0f * std::acos(q.w);
    axis = {
        q.x / std::sqrt(1.0f - q.w*q.w),
        q.y / std::sqrt(1.0f - q.w*q.w),
        q.z / std::sqrt(1.0f - q.w*q.w)
    };
}

inline quatf swap_handedness(const quatf& q)
{
    return { q.x, -q.y, -q.z, q.w };
}

inline float3x3 look33(float3 dir, float3 up)
{
    float3 z = dir;
    float3 x = normalize(cross(up, z));
    float3 y = cross(z, x);
    return{ {
        { x.x, y.x, z.x },
        { x.y, y.y, z.y },
        { x.z, y.z, z.z },
    } };
}

inline float3x3 swap_handedness(const float3x3& m)
{
    return{ {
        { m[0].x,-m[0].y,-m[0].z },
        {-m[1].x, m[1].y, m[1].z },
        {-m[2].x, m[2].y, m[2].z },
    } };
}

inline float4x4 swap_handedness(const float4x4& m)
{
    return{ {
        { m[0].x,-m[0].y,-m[0].z, m[0].w },
        {-m[1].x, m[1].y, m[1].z, m[1].w },
        {-m[2].x, m[2].y, m[2].z, m[2].w },
        {-m[3].x, m[3].y, m[3].z, m[3].w },
    } };
}

inline float3x3 to_float3x3(const quatf& q)
{
    return {{
        {1.0f-2.0f*q.y*q.y - 2.0f*q.z*q.z, 2.0f*q.x*q.y - 2.0f*q.z*q.w,        2.0f*q.x*q.z + 2.0f*q.y*q.w,      },
        {2.0f*q.x*q.y + 2.0f*q.z*q.w,      1.0f - 2.0f*q.x*q.x - 2.0f*q.z*q.z, 2.0f*q.y*q.z - 2.0f*q.x*q.w,      },
        {2.0f*q.x*q.z - 2.0f*q.y*q.w,      2.0f*q.y*q.z + 2.0f*q.x*q.w,        1.0f - 2.0f*q.x*q.x - 2.0f*q.y*q.y}
    }};
}
inline float4x4 to_float4x4(const quatf& q)
{
    return {{
        {1.0f-2.0f*q.y*q.y - 2.0f*q.z*q.z, 2.0f*q.x*q.y - 2.0f*q.z*q.w,        2.0f*q.x*q.z + 2.0f*q.y*q.w,        0.0},
        {2.0f*q.x*q.y + 2.0f*q.z*q.w,      1.0f - 2.0f*q.x*q.x - 2.0f*q.z*q.z, 2.0f*q.y*q.z - 2.0f*q.x*q.w,        0.0},
        {2.0f*q.x*q.z - 2.0f*q.y*q.w,      2.0f*q.y*q.z + 2.0f*q.x*q.w,        1.0f - 2.0f*q.x*q.x - 2.0f*q.y*q.y, 0.0},
        {0.0,                              0.0,                                0.0,                                1.0}
    }};
}

inline float4x4 translate(const float3& t)
{
    return {{
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, 0.0f },
        {  t.x,  t.y,  t.z, 1.0f }
    }};
}

inline float3x3 scale33(const float3& t)
{
    return{{
        {  t.x, 0.0f, 0.0f },
        { 0.0f,  t.y, 0.0f },
        { 0.0f, 0.0f,  t.z },
    }};
}
inline float4x4 scale44(const float3& t)
{
    return{{
        {  t.x, 0.0f, 0.0f, 0.0f },
        { 0.0f,  t.y, 0.0f, 0.0f },
        { 0.0f, 0.0f,  t.z, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    }};
}

inline float4x4 transform(const float3& t, const quatf& r, const float3& s)
{
    auto ret = scale44(s);
    ret *= to_float4x4(r);
    ret *= translate(t);
    return ret;
}

inline float3x3 invert(const float3x3& x)
{
    if (x[0][2] != 0 || x[1][2] != 0 || x[2][2] != 1) {
        float3x3 s = {
            x[1][1] * x[2][2] - x[2][1] * x[1][2],
            x[2][1] * x[0][2] - x[0][1] * x[2][2],
            x[0][1] * x[1][2] - x[1][1] * x[0][2],

            x[2][0] * x[1][2] - x[1][0] * x[2][2],
            x[0][0] * x[2][2] - x[2][0] * x[0][2],
            x[1][0] * x[0][2] - x[0][0] * x[1][2],

            x[1][0] * x[2][1] - x[2][0] * x[1][1],
            x[2][0] * x[0][1] - x[0][0] * x[2][1],
            x[0][0] * x[1][1] - x[1][0] * x[0][1] };

        float r = x[0][0] * s[0][0] + x[0][1] * s[1][0] + x[0][2] * s[2][0];

        if (std::abs(r) >= 1) {
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    s[i][j] /= r;
                }
            }
        }
        else {
            float mr = std::abs(r) / std::numeric_limits<float>::min();

            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    if (mr > std::abs(s[i][j])) {
                        s[i][j] /= r;
                    }
                    else {
                        // singular
                        return float3x3::identity();
                    }
                }
            }
        }

        return s;
    }
    else {
        float3x3 s = {
             x[1][1], -x[0][1], 0,
            -x[1][0],  x[0][0], 0,
                   0,        0, 1 };

        float r = x[0][0] * x[1][1] - x[1][0] * x[0][1];

        if (std::abs(r) >= 1) {
            for (int i = 0; i < 2; ++i) {
                for (int j = 0; j < 2; ++j) {
                    s[i][j] /= r;
                }
            }
        }
        else {
            float mr = std::abs(r) / std::numeric_limits<float>::min();

            for (int i = 0; i < 2; ++i) {
                for (int j = 0; j < 2; ++j) {
                    if (mr > std::abs(s[i][j])) {
                        s[i][j] /= r;
                    }
                    else
                    {
                        
                        // singular
                        return float3x3::identity();
                    }
                }
            }
        }

        s[2][0] = -x[2][0] * s[0][0] - x[2][1] * s[1][0];
        s[2][1] = -x[2][0] * s[0][1] - x[2][1] * s[1][1];

        return s;
    }
}

inline float4x4 invert(const float4x4& x)
{
    float4x4 s = {
        x[1][1] * x[2][2] - x[2][1] * x[1][2],
        x[2][1] * x[0][2] - x[0][1] * x[2][2],
        x[0][1] * x[1][2] - x[1][1] * x[0][2],
        0,

        x[2][0] * x[1][2] - x[1][0] * x[2][2],
        x[0][0] * x[2][2] - x[2][0] * x[0][2],
        x[1][0] * x[0][2] - x[0][0] * x[1][2],
        0,

        x[1][0] * x[2][1] - x[2][0] * x[1][1],
        x[2][0] * x[0][1] - x[0][0] * x[2][1],
        x[0][0] * x[1][1] - x[1][0] * x[0][1],
        0,

        0, 0, 0, 1,
    };

    auto r = x[0][0] * s[0][0] + x[0][1] * s[1][0] + x[0][2] * s[2][0];

    if (std::abs(r) >= 1) {
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                s[i][j] /= r;
            }
        }
    }
    else {
        auto mr = std::abs(r) / std::numeric_limits<float>::min();

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                if (mr > std::abs(s[i][j])) {
                    s[i][j] /= r;
                }
                else {
                    // error
                    return float4x4::identity();
                }
            }
        }
    }

    s[3][0] = -x[3][0] * s[0][0] - x[3][1] * s[1][0] - x[3][2] * s[2][0];
    s[3][1] = -x[3][0] * s[0][1] - x[3][1] * s[1][1] - x[3][2] * s[2][1];
    s[3][2] = -x[3][0] * s[0][2] - x[3][1] * s[1][2] - x[3][2] * s[2][2];
    return s;
}

template<class T>
inline quatf to_quat_impl(const T& m)
{
    float tr, s;
    float q[4];
    int i, j, k;
    quatf quat;

    int nxt[3] = { 1, 2, 0 };
    tr = m[0][0] + m[1][1] + m[2][2];

    // check the diagonal
    if (tr > 0.0) {
        s = std::sqrt(tr + 1.0f);
        quat.w = s / 2.0f;
        s = 0.5f / s;

        quat.x = (m[1][2] - m[2][1]) * s;
        quat.y = (m[2][0] - m[0][2]) * s;
        quat.z = (m[0][1] - m[1][0]) * s;
    }
    else {
        // diagonal is negative
        i = 0;
        if (m[1][1] > m[0][0])
            i = 1;
        if (m[2][2] > m[i][i])
            i = 2;

        j = nxt[i];
        k = nxt[j];
        s = std::sqrt((m[i][i] - (m[j][j] + m[k][k])) + 1.0f);

        q[i] = s * 0.5f;
        if (s != 0.0f)
            s = 0.5f / s;

        q[3] = (m[j][k] - m[k][j]) * s;
        q[j] = (m[i][j] + m[j][i]) * s;
        q[k] = (m[i][k] + m[k][i]) * s;

        quat.x = q[0];
        quat.y = q[1];
        quat.z = q[2];
        quat.w = q[3];
    }

    return quat;
}

inline quatf to_quat(const float3x3& m)
{
    return to_quat_impl(m);
}
inline quatf to_quat(const float4x4& m)
{
    return to_quat_impl(m);
}

// aperture and focal_length must be millimeter. return fov in degree
inline float compute_fov(float aperture, float focal_length)
{
    return 2.0f * std::atan(aperture / (2.0f * focal_length)) * Rad2Deg;
}

// aperture: millimeter
// fov: degree
inline float compute_focal_length(float aperture, float fov)
{
    return aperture / std::tan(fov * Deg2Rad / 2.0f) / 2.0f;
}


inline bool ray_triangle_intersection(float3 pos, float3 dir, float3 p1, float3 p2, float3 p3, float& distance)
{
    const float epsdet = 1e-10f;
    const float eps = 1e-4f;

    float3 e1 = p2 - p1;
    float3 e2 = p3 - p1;
    float3 p = cross(dir, e2);
    float det = dot(e1, p);
    if (std::abs(det) < epsdet) return false;
    float inv_det = 1.0f / det;
    float3 t = pos - p1;
    float u = dot(t, p) * inv_det;
    if (u < -eps || u  > 1 + eps) return false;
    float3 q = cross(t, e1);
    float v = dot(dir, q) * inv_det;
    if (v < -eps || u + v > 1 + eps) return false;

    distance = dot(e2, q) * inv_det;
    return distance >= 0.0f;
}

// pos must be on the triangle
template<class T>
inline T triangle_interpolation(float3 pos, float3 p1, float3 p2, float3 p3, T x1, T x2, T x3)
{
    auto f1 = p1 - pos;
    auto f2 = p2 - pos;
    auto f3 = p3 - pos;
    float a = 1.0f / length(cross(p1 - p2, p1 - p3));
    float a1 = length(cross(f2, f3)) * a;
    float a2 = length(cross(f3, f1)) * a;
    float a3 = length(cross(f1, f2)) * a;
    return x1 * a1 + x2 * a2 + x3 * a3;
}

inline float ray_point_distance(float3 pos, float3 dir, float3 p)
{
    return length(cross(dir, p - pos));
}

inline float plane_distance(float3 p, float3 pn, float pd)  { return dot(p, pn) - pd; }
inline float plane_distance(float3 p, float3 pn)            { return dot(p, pn); }
inline float3 plane_mirror(float3 p, float3 pn, float pd)   { return p - pn * (plane_distance(p, pn, pd) * 2.0f); }
inline float3 plane_mirror(float3 p, float3 pn)             { return p - pn * (plane_distance(p, pn) * 2.0f); }


inline void compute_triangle_tangent(
    const float3(&vertices)[3], const float2(&uv)[3], float3(&dst_tangent)[3], float3(&dst_binormal)[3])
{
    float3 p = vertices[1] - vertices[0];
    float3 q = vertices[2] - vertices[0];
    float2 s = { uv[1].x - uv[0].x, uv[2].x - uv[0].x };
    float2 t = { uv[1].y - uv[0].y, uv[2].y - uv[0].y };

    float div = s.x * t.y - s.y * t.x;
    float area = std::abs(div);
    float rdiv = 1.0f / div;
    s *= rdiv;
    t *= rdiv;

    float3 tangent = normalize({
        t.y * p.x - t.x * q.x,
        t.y * p.y - t.x * q.y,
        t.y * p.z - t.x * q.z
    }) * area;

    float3 binormal = normalize({
        s.x * q.x - s.y * p.x,
        s.x * q.y - s.y * p.y,
        s.x * q.z - s.y * p.z
    }) * area;

    float angles[3] = {
        angle_between2(vertices[2], vertices[1], vertices[0]),
        angle_between2(vertices[0], vertices[2], vertices[1]),
        angle_between2(vertices[1], vertices[0], vertices[2]),
    };
    for (int v = 0; v < 3; ++v)
    {
        dst_tangent[v] = tangent * angles[v];
        dst_binormal[v] = binormal * angles[v];
    }
}

inline float4 orthogonalize_tangent(float3 tangent, float3 binormal, float3 normal)
{
    float NdotT = dot(normal, tangent);
    tangent -= normal * NdotT;
    float magT = length(tangent);
    tangent = tangent / magT;

    float NdotB = dot(normal, binormal);
    float TdotB = dot(tangent, binormal) * magT;
    binormal -= normal * NdotB - tangent * TdotB;;
    float magB = length(binormal);
    binormal = binormal / magB;

#if 0
    const float epsilon = 1e-6f;
    if (magT <= epsilon || magB <= epsilon)
    {
        float3 axis1, axis2;

        float dpXN = std::abs(dot({ 1.0f, 0.0f, 0.0f }, normal));
        float dpYN = std::abs(dot({ 0.0f, 1.0f, 0.0f }, normal));
        float dpZN = std::abs(dot({ 0.0f, 0.0f, 1.0f }, normal));

        if (dpXN <= dpYN && dpXN <= dpZN)
        {
            axis1 = { 1.0f, 0.0f, 0.0f };
            if (dpYN <= dpZN)
                axis2 = { 0.0f, 1.0f, 0.0f };
            else
                axis2 = { 0.0f, 0.0f, 1.0f };
        }
        else if (dpYN <= dpXN && dpYN <= dpZN)
        {
            axis1 = { 0.0f, 1.0f, 0.0f };
            if (dpXN <= dpZN)
                axis2 = { 1.0f, 0.0f, 0.0f };
            else
                axis2 = { 0.0f, 0.0f, 1.0f };
        }
        else
        {
            axis1 = { 0.0f, 0.0f, 1.0f };
            if (dpXN <= dpYN)
                axis2 = { 1.0f, 0.0f, 0.0f };
            else
                axis2 = { 0.0f, 1.0f, 0.0f };
        }
        tangent = normalize(axis1 - normal * dot(normal, axis1));
        binormal = normalize(axis2 - normal * dot(normal, axis2) - normalize(tangent) * dot(tangent, axis2));
    }
#endif

    return { tangent.x, tangent.y, tangent.z,
        dot(cross(normal, tangent), binormal) > 0.0f ? 1.0f : -1.0f };
}

#ifdef muMath_AddNamespace
} // namespace mu
#endif
