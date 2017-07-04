#define C programCount
#define I programIndex
#define FLT_MAX 3.402823466e+38F

typedef unsigned int16 half;

struct half2  { half x, y; };
struct half3  { half x, y, z; };
struct half4  { half x, y, z, w; };
struct quath  { half x, y, z, w; };
struct float2 { float x, y; };
struct float3 { float x, y, z; };
struct float4 { float x, y, z, w; };
struct quatf  { float x, y, z, w; };
struct float4x4 { float4 m[4]; };

inline float2 float2_(float x, float y) { float2 r={x,y}; return r; }
inline float3 float3_(float x, float y, float z) { float3 r={x,y,z }; return r; }
inline float4 float4_(float x, float y, float z, float w) { float4 r={x,y,z,w}; return r; }
inline uniform float2 float2_(uniform float x, uniform float y) { uniform float2 r = { x,y }; return r; }
inline uniform float3 float3_(uniform float x, uniform float y, uniform float z) { uniform float3 r = { x,y,z }; return r; }
inline uniform float4 float4_(uniform float x, uniform float y, uniform float z, uniform float w) { uniform float4 r = { x,y,z,w }; return r; }


static inline float2 half_to_float(half2 h)
{
    float2 ret = {half_to_float(h.x), half_to_float(h.y)};
    return ret;
}
static inline float3 half_to_float(half3 h)
{
    float3 ret = {half_to_float(h.x), half_to_float(h.y), half_to_float(h.z)};
    return ret;
}
static inline float4 half_to_float(half4 h)
{
    float4 ret = {half_to_float(h.x), half_to_float(h.y), half_to_float(h.z), half_to_float(h.w)};
    return ret;
}


static inline float2 operator+(float2 a, float2 b) { float2 r = { a.x + b.x, a.y + b.y }; return r; }
static inline float2 operator-(float2 a, float2 b) { float2 r = { a.x - b.x, a.y - b.y }; return r; }
static inline float2 operator*(float2 a, float2 b) { float2 r = { a.x * b.x, a.y * b.y }; return r; }
static inline float2 operator/(float2 a, float2 b) { float2 r = { a.x / b.x, a.y / b.y }; return r; }
static inline float2 operator+(float2 a, float b) { float2 r = { a.x + b, a.y + b }; return r; }
static inline float2 operator-(float2 a, float b) { float2 r = { a.x - b, a.y - b }; return r; }
static inline float2 operator*(float2 a, float b) { float2 r = { a.x * b, a.y * b }; return r; }
static inline float2 operator/(float2 a, float b) { float2 r = { a.x / b, a.y / b }; return r; }
static inline float2 operator+(float a, float2 b) { float2 r = { a + b.x, a + b.y }; return r; }
static inline float2 operator-(float a, float2 b) { float2 r = { a - b.x, a - b.y }; return r; }
static inline float2 operator*(float a, float2 b) { float2 r = { a * b.x, a * b.y }; return r; }
static inline float2 operator/(float a, float2 b) { float2 r = { a / b.x, a / b.y }; return r; }

static inline float3 operator+(float3 a, float3 b) { float3 r = { a.x + b.x, a.y + b.y, a.z + b.z }; return r; }
static inline float3 operator-(float3 a, float3 b) { float3 r = { a.x - b.x, a.y - b.y, a.z - b.z }; return r; }
static inline float3 operator*(float3 a, float3 b) { float3 r = { a.x * b.x, a.y * b.y, a.z * b.z }; return r; }
static inline float3 operator/(float3 a, float3 b) { float3 r = { a.x / b.x, a.y / b.y, a.z / b.z }; return r; }
static inline float3 operator+(float3 a, float b) { float3 r = { a.x + b, a.y + b, a.z + b }; return r; }
static inline float3 operator-(float3 a, float b) { float3 r = { a.x - b, a.y - b, a.z - b }; return r; }
static inline float3 operator*(float3 a, float b) { float3 r = { a.x * b, a.y * b, a.z * b }; return r; }
static inline float3 operator/(float3 a, float b) { float3 r = { a.x / b, a.y / b, a.z / b }; return r; }
static inline float3 operator+(float a, float3 b) { float3 r = { a + b.x, a + b.y, a + b.z }; return r; }
static inline float3 operator-(float a, float3 b) { float3 r = { a - b.x, a - b.y, a - b.z }; return r; }
static inline float3 operator*(float a, float3 b) { float3 r = { a * b.x, a * b.y, a * b.z }; return r; }
static inline float3 operator/(float a, float3 b) { float3 r = { a / b.x, a / b.y, a / b.z }; return r; }

static inline uniform float2 operator+(uniform float2 a, uniform float2 b) { uniform float2 r = { a.x + b.x, a.y + b.y }; return r; }
static inline uniform float2 operator-(uniform float2 a, uniform float2 b) { uniform float2 r = { a.x - b.x, a.y - b.y }; return r; }
static inline uniform float2 operator*(uniform float2 a, uniform float2 b) { uniform float2 r = { a.x * b.x, a.y * b.y }; return r; }
static inline uniform float2 operator/(uniform float2 a, uniform float2 b) { uniform float2 r = { a.x / b.x, a.y / b.y }; return r; }
static inline uniform float2 operator+(uniform float2 a, uniform float b) { uniform float2 r = { a.x + b, a.y + b }; return r; }
static inline uniform float2 operator-(uniform float2 a, uniform float b) { uniform float2 r = { a.x - b, a.y - b }; return r; }
static inline uniform float2 operator*(uniform float2 a, uniform float b) { uniform float2 r = { a.x * b, a.y * b }; return r; }
static inline uniform float2 operator/(uniform float2 a, uniform float b) { uniform float2 r = { a.x / b, a.y / b }; return r; }
static inline uniform float2 operator+(uniform float a, uniform float2 b) { uniform float2 r = { a + b.x, a + b.y }; return r; }
static inline uniform float2 operator-(uniform float a, uniform float2 b) { uniform float2 r = { a - b.x, a - b.y }; return r; }
static inline uniform float2 operator*(uniform float a, uniform float2 b) { uniform float2 r = { a * b.x, a * b.y }; return r; }
static inline uniform float2 operator/(uniform float a, uniform float2 b) { uniform float2 r = { a / b.x, a / b.y }; return r; }

static inline uniform float3 operator+(uniform float3 a, uniform float3 b) { uniform float3 r = { a.x + b.x, a.y + b.y, a.z + b.z }; return r; }
static inline uniform float3 operator-(uniform float3 a, uniform float3 b) { uniform float3 r = { a.x - b.x, a.y - b.y, a.z - b.z }; return r; }
static inline uniform float3 operator*(uniform float3 a, uniform float3 b) { uniform float3 r = { a.x * b.x, a.y * b.y, a.z * b.z }; return r; }
static inline uniform float3 operator/(uniform float3 a, uniform float3 b) { uniform float3 r = { a.x / b.x, a.y / b.y, a.z / b.z }; return r; }
static inline uniform float3 operator+(uniform float3 a, uniform float b) { uniform float3 r = { a.x + b, a.y + b, a.z + b }; return r; }
static inline uniform float3 operator-(uniform float3 a, uniform float b) { uniform float3 r = { a.x - b, a.y - b, a.z - b }; return r; }
static inline uniform float3 operator*(uniform float3 a, uniform float b) { uniform float3 r = { a.x * b, a.y * b, a.z * b }; return r; }
static inline uniform float3 operator/(uniform float3 a, uniform float b) { uniform float3 r = { a.x / b, a.y / b, a.z / b }; return r; }
static inline uniform float3 operator+(uniform float a, uniform float3 b) { uniform float3 r = { a + b.x, a + b.y, a + b.z }; return r; }
static inline uniform float3 operator-(uniform float a, uniform float3 b) { uniform float3 r = { a - b.x, a - b.y, a - b.z }; return r; }
static inline uniform float3 operator*(uniform float a, uniform float3 b) { uniform float3 r = { a * b.x, a * b.y, a * b.z }; return r; }
static inline uniform float3 operator/(uniform float a, uniform float3 b) { uniform float3 r = { a / b.x, a / b.y, a / b.z }; return r; }

static inline uniform float2 reduce_add(float2 v)
{
    uniform float2 r = { reduce_add(v.x), reduce_add(v.y) };
    return r;
}
static inline uniform float3 reduce_add(float3 v)
{
    uniform float3 r = {reduce_add(v.x), reduce_add(v.y), reduce_add(v.z)};
    return r;
}

static inline float mod(float a, float b)
{
    return a - b * floor(a / b);
}
static inline uniform float mod(uniform float a, uniform float b)
{
    return a - b * floor(a / b);
}

static inline float frac(float a)
{
    return mod(a, 1.0);
}
static inline uniform float frac(uniform float a)
{
    return mod(a, 1.0);
}


#define define_vmath1(f)\
    static inline float2 f(float2 a)\
    {\
        float2 r = { f(a.x), f(a.y) };\
        return r;\
    }\
    static inline uniform float2 f(uniform float2 a)\
    {\
        uniform float2 r = { f(a.x), f(a.y) };\
        return r;\
    }\
    static inline float3 f(float3 a)\
    {\
        float3 r = { f(a.x), f(a.y), f(a.z) };\
        return r;\
    }\
    static inline uniform float3 f(uniform float3 a)\
    {\
        uniform float3 r = { f(a.x), f(a.y), f(a.z) };\
        return r;\
    }

#define define_vmath2(f)\
    static inline float2 f(float2 a, float2 b)\
    {\
        float2 r = { f(a.x, b.x), f(a.y, b.y) };\
        return r;\
    }\
    static inline uniform float2 f(uniform float2 a, uniform float2 b)\
    {\
        uniform float2 r = { f(a.x, b.x), f(a.y, b.y) };\
        return r;\
    }\
    static inline float3 f(float3 a, float3 b)\
    {\
        float3 r = { f(a.x, b.x), f(a.y, b.y), f(a.z, b.z) };\
        return r;\
    }\
    static inline uniform float3 f(uniform float3 a, uniform float3 b)\
    {\
        uniform float3 r = { f(a.x, b.x), f(a.y, b.y), f(a.z, b.z) };\
        return r;\
    }

define_vmath1(abs)
define_vmath1(round)
define_vmath1(floor)
define_vmath1(ceil)
define_vmath2(min)
define_vmath2(max)
define_vmath1(rcp)
define_vmath1(sqrt)
define_vmath1(rsqrt)
define_vmath1(sin)
define_vmath1(cos)
define_vmath1(tan)
define_vmath1(asin)
define_vmath1(acos)
define_vmath1(atan)
define_vmath2(atan2)
define_vmath1(exp)
define_vmath1(log)
define_vmath2(pow)
define_vmath2(mod)
define_vmath1(frac)



static inline float dot(float2 a, float2 b)
{
    return a.x*b.x + a.y*b.y;
}
static inline uniform float dot(uniform float2 a, uniform float2 b)
{
    return a.x*b.x + a.y*b.y;
}
static inline float dot(float3 a, float3 b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
static inline uniform float dot(uniform float3 a, uniform float3 b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}


static inline float3 cross(float3 v0, float3 v1)
{
    float3 ret;
    ret.x = v0.y*v1.z - v0.z*v1.y;
    ret.y = v0.z*v1.x - v0.x*v1.z;
    ret.z = v0.x*v1.y - v0.y*v1.x;
    return ret;
}
static inline uniform float3 cross(uniform float3 v0, uniform float3 v1)
{
    uniform float3 ret;
    ret.x = v0.y*v1.z - v0.z*v1.y;
    ret.y = v0.z*v1.x - v0.x*v1.z;
    ret.z = v0.x*v1.y - v0.y*v1.x;
    return ret;
}



static inline float length_sq(float2 v)
{
    return dot(v, v);
}
static inline uniform float length_sq(uniform float2 v)
{
    return dot(v, v);
}
static inline float length_sq(float3 v)
{
    return dot(v, v);
}
static inline uniform float length_sq(uniform float3 v)
{
    return dot(v, v);
}

static inline float length(float2 v)
{
    return sqrt(length_sq(v));
}
static inline uniform float length(uniform float2 v)
{
    return sqrt(length_sq(v));
}
static inline float length(float3 v)
{
    return sqrt(length_sq(v));
}
static inline uniform float length(uniform float3 v)
{
    return sqrt(length_sq(v));
}

static inline float2 normalize(float2 v)
{
    return v * rsqrt(dot(v, v));
}
static inline uniform float2 normalize(uniform float2 v)
{
    return v * rsqrt(dot(v, v));
}
static inline float3 normalize(float3 v)
{
    return v * rsqrt(dot(v, v));
}
static inline uniform float3 normalize(uniform float3 v)
{
    return v * rsqrt(dot(v, v));
}

static inline float lerp(float a, float b, float t) {
    return (1.0f-t)*a + t*b;
}
static inline uniform float lerp(uniform float a, uniform float b, uniform float t) {
    return (1.0f-t)*a + t*b;
}

static inline float3 lerp(float3 a, float3 b, float t) {
    return (1.0f-t)*a + t*b;
}
static inline uniform float3 lerp(uniform float3 a, uniform float3 b, uniform float t) {
    return (1.0f-t)*a + t*b;
}

static inline float clamp_and_normalize(float v, float low, float high, float rcp_range) {
    float r = (v - low)*rcp_range;
    return clamp(r, 0.0f, 1.0f);
}



static inline bool ray_triangle_intersection(uniform float3 pos, uniform float3 dir, float3 p1, float3 p2, float3 p3, float& distance)
{
    const float epsdet = 1e-10f;
    const float eps = 1e-4f;

    float3 e1 = p2 - p1;
    float3 e2 = p3 - p1;
    float3 p = cross(dir, e2);
    float det = dot(e1, p);
    float inv_det = 1.0f / det;
    float3 t = pos - p1;
    float u = dot(t, p) * inv_det;
    float3 q = cross(t, e1);
    float v = dot(dir, q) * inv_det;

    distance = dot(e2, q) * inv_det;
    return
        abs(det) > 1e-10f &&
        u > -eps && u < 1 + eps &&
        v > -eps && u + v < 1 + eps &&
        distance >= 0.0f;
}

// uniform variant
static inline uniform bool ray_triangle_intersection(uniform float3 pos, uniform float3 dir, uniform float3 p1, uniform float3 p2, uniform float3 p3, uniform float& distance)
{
    uniform const float epsdet = 1e-10f;
    uniform const float eps = 1e-4f;

    uniform float3 e1 = p2 - p1;
    uniform float3 e2 = p3 - p1;
    uniform float3 p = cross(dir, e2);
    uniform float det = dot(e1, p);
    uniform float inv_det = 1.0f / det;
    uniform float3 t = pos - p1;
    uniform float u = dot(t, p) * inv_det;
    uniform float3 q = cross(t, e1);
    uniform float v = dot(dir, q) * inv_det;

    distance = dot(e2, q) * inv_det;
    return
        abs(det) > epsdet &&
        u > -eps && u < 1 + eps &&
        v > -eps && u + v < 1 + eps &&
        distance >= 0.0f;
}


static inline float ray_point_distance(float3 pos, float3 dir, float3 p)
{
    return length(cross(dir, p - pos));
}

static inline uniform float ray_point_distance(uniform float3 pos, uniform float3 dir, uniform float3 p)
{
    return length(cross(dir, p - pos));
}


static inline void compute_triangle_tangents(
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

static inline float4 orthogonalize_tangent(float3 tangent, float3 binormal, float3 normal)
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
