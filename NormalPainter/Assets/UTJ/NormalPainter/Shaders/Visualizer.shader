Shader "Hidden/NormalPainter/Visualizer" {

CGINCLUDE
#include "UnityCG.cginc"

sampler2D _TmpDepth;

float _VertexSize;
float _NormalSize;
float _TangentSize;
float _BinormalSize;

float4 _VertexColor;
float4 _VertexColor2;
float4 _VertexColor3;
float4 _NormalColor;
float4 _TangentColor;
float4 _BinormalColor;
float4 _BrushPos;
int _OnlySelected = 0;

float4x4 _Transform;
float4x4 _InvViewProj;
StructuredBuffer<float3> _BaseNormals;
StructuredBuffer<float4> _BaseTangents;
StructuredBuffer<float3> _Points;
StructuredBuffer<float3> _Normals;
StructuredBuffer<float4> _Tangents;
StructuredBuffer<float> _Selection;

int _NumBrushSamples;
StructuredBuffer<float> _BrushSamples;

struct ia_out
{
    float4 vertex : POSITION;
    float4 normal : NORMAL;
    float4 uv : TEXCOORD0;
    float4 color : COLOR;
    uint vertexID : SV_VertexID;
    uint instanceID : SV_InstanceID;
};

struct vs_out
{
    float4 vertex : SV_POSITION;
    float4 color : TEXCOORD0;
};


vs_out vert_vertices(ia_out v)
{
    float3 pos = (mul(_Transform, float4(_Points[v.instanceID], 1.0))).xyz;

    float s = _Selection[v.instanceID];
    float4 vertex = v.vertex;
    vertex.xyz *= _VertexSize;
    vertex.xyz *= abs(UnityObjectToViewPos(pos).z);
    vertex.xyz += pos;
    vertex = mul(UNITY_MATRIX_VP, vertex);

    vs_out o;
    o.vertex = vertex;
    o.color = lerp(_VertexColor, _VertexColor2, s);


    float d = length(pos - _BrushPos.xyz);
    if (d < _BrushPos.w) {
        int bsi = clamp(1.0f - d / _BrushPos.w, 0, 1) * (_NumBrushSamples - 1);
        o.color.rgb += _VertexColor3.rgb * _BrushSamples[bsi];
    }
    return o;
}

vs_out vert_normals(ia_out v)
{
    float3 pos = (mul(_Transform, float4(_Points[v.instanceID], 1.0))).xyz;
    float3 dir = normalize((mul(_Transform, float4(_Normals[v.instanceID], 0.0))).xyz);

    float s = _OnlySelected ? _Selection[v.instanceID] : 1.0f;
    float4 vertex = v.vertex;
    vertex.xyz += pos + dir * v.uv.x * _NormalSize * s;
    vertex = mul(UNITY_MATRIX_VP, vertex);

    vs_out o;
    o.vertex = vertex;
    o.color = _NormalColor;
    o.color.a = 1.0 - v.uv.x;
    return o;
}

vs_out vert_tangents(ia_out v)
{
    float3 pos = (mul(_Transform, float4(_Points[v.instanceID], 1.0))).xyz;
    float3 dir = normalize((mul(_Transform, float4(_Tangents[v.instanceID].xyz, 0.0))).xyz);

    float s = _OnlySelected ? _Selection[v.instanceID] : 1.0f;
    float4 vertex = v.vertex;
    vertex.xyz += pos + dir * v.uv.x * _TangentSize * s;
    vertex = mul(UNITY_MATRIX_VP, vertex);

    vs_out o;
    o.vertex = vertex;
    o.color = _TangentColor;
    o.color.a = 1.0 - v.uv.x;
    return o;
}

vs_out vert_binormals(ia_out v)
{
    float3 pos = (mul(_Transform, float4(_Points[v.instanceID], 1.0))).xyz;
    float3 binormal = normalize(cross(_Normals[v.instanceID], _Tangents[v.instanceID].xyz) * _Tangents[v.instanceID].w);
    float3 dir = normalize((mul(_Transform, float4(binormal, 0.0))).xyz);

    float s = _OnlySelected ? _Selection[v.instanceID] : 1.0f;
    float4 vertex = v.vertex;
    vertex.xyz += pos + dir * v.uv.x * _BinormalSize * s;
    vertex = mul(UNITY_MATRIX_VP, vertex);

    vs_out o;
    o.vertex = vertex;
    o.color = _BinormalColor;
    o.color.a = 1.0 - v.uv.x;
    return o;
}


vs_out vert_local_space_normals(ia_out v)
{
    vs_out o;
    o.vertex = UnityObjectToClipPos(v.vertex);
    o.color.rgb = v.normal.xyz * 0.5 + 0.5;
    o.color.a = 1.0;
    return o;
}

float3 ToBaseTangentSpace(uint vid, float3 n)
{
    float3 base_normal = _BaseNormals[vid];
    float4 base_tangent = _BaseTangents[vid];
    float3 base_binormal = normalize(cross(base_normal, base_tangent.xyz) * base_tangent.w);
    float3x3 tbn = float3x3(base_tangent.xyz, base_binormal, base_normal);
    return normalize(mul(n, transpose(tbn)));
}

vs_out vert_tangent_space_normals(ia_out v)
{
    vs_out o;
    o.vertex = UnityObjectToClipPos(v.vertex);
    o.color.rgb = ToBaseTangentSpace(v.vertexID, v.normal.xyz) * 0.5 + 0.5;
    o.color.a = 1.0;
    return o;
}

vs_out vert_color(ia_out v)
{
    vs_out o;
    o.vertex = UnityObjectToClipPos(v.vertex);
    o.color = v.color;
    return o;
}

vs_out vert_lasso(ia_out v)
{
    vs_out o;
    o.vertex = float4(v.vertex.xy, 0.0, 1.0);
    o.vertex.y *= -1;
    o.color = float4(1.0, 0.0, 0.0, 1.0);
    return o;
}

vs_out vert_depth(ia_out v)
{
    vs_out o;
    o.vertex = UnityObjectToClipPos(v.vertex);
    //o.color = ComputeScreenPos(o.vertex);
    o.color = mul(UNITY_MATRIX_M, v.vertex);
    return o;
}

vs_out vert_brush_range(ia_out v)
{
    vs_out o;
    o.vertex = mul(UNITY_MATRIX_VP, float4(v.vertex.xyz * (_BrushPos.w * 2.0) + _BrushPos.xyz, 1.0));
    o.color = ComputeScreenPos(o.vertex);
    return o;
}



float4 frag(vs_out v) : SV_Target
{
    return v.color;
}


float3 GetWorldPosition(float2 uv, float depth)
{
    float4 pos4 = mul(_InvViewProj, float4(uv * 2.0 - 1.0, depth, 1.0));
    return pos4.xyz / pos4.w;
}

float ComputeDepth(float4 clippos)
{
#if defined(SHADER_TARGET_GLSL) || defined(SHADER_API_GLES) || defined(SHADER_API_GLES3)
    return (clippos.z / clippos.w) * 0.5 + 0.5;
#else
    return clippos.z / clippos.w;
#endif
}

float4 frag_depth(vs_out v) : SV_Target
{
    float depth = ComputeDepth(v.color);
    //return depth;

    return v.color;
}

float4 frag_brush_range(vs_out v) : SV_Target
{
    float2 uv = v.color.xy / v.color.w;
    float depth = tex2D(_TmpDepth, uv).r;

    //float3 pixel_pos = GetWorldPosition(uv, depth);
    float3 pixel_pos = tex2D(_TmpDepth, uv).xyz;
    float3 brush_pos = _BrushPos.xyz;
    float distance = length(pixel_pos - brush_pos);

    if (distance >= _BrushPos.w) { discard; }
    float range = clamp(1.0f - distance / _BrushPos.w, 0, 1);
    int bsi = range * (_NumBrushSamples - 1);
    float4 color = float4(_VertexColor3.rgb, _VertexColor3.a * _BrushSamples[bsi]);
    if (range < 0.01) {
        color = float4(1,0,0,1);
    }

    return color;
    //return float4(uv, depth, 1.0);
    //return float4(abs(pixel_pos.xyz * 0.02), 1.0);
    //return float4(abs(tex2D(_TmpDepth, uv).xyz), 1.0);
    //return float4(abs(brush_pos.xyz), 1.0);
    //return float4(distance, distance, distance, 1.0);
    //return float4(depth, depth, depth, 1.0);
}

ENDCG

    SubShader
    {
        Tags{ "RenderType" = "Transparent" "Queue" = "Transparent+100" }
        Blend SrcAlpha OneMinusSrcAlpha
        ZWrite Off

        // pass 0: visualize vertices
        Pass
        {
            ZTest LEqual

            CGPROGRAM
            #pragma vertex vert_vertices
            #pragma fragment frag
            #pragma target 4.5
            ENDCG
        }

        // pass 1: visualize normals
        Pass
        {
            ZTest LEqual

            CGPROGRAM
            #pragma vertex vert_normals
            #pragma fragment frag
            #pragma target 4.5
            ENDCG
        }

        // pass 2: visualize tangents
        Pass
        {
            ZTest LEqual

            CGPROGRAM
            #pragma vertex vert_tangents
            #pragma fragment frag
            #pragma target 4.5
            ENDCG
        }

        // pass 3: visualize binormals
        Pass
        {
            ZTest LEqual

            CGPROGRAM
            #pragma vertex vert_binormals
            #pragma fragment frag
            #pragma target 4.5
            ENDCG
        }
            
        // pass 4: local space normals overlay
        Pass
        {
            ZTest LEqual

            CGPROGRAM
            #pragma vertex vert_local_space_normals
            #pragma fragment frag
            #pragma target 4.5
            ENDCG
        }

        // pass 5: tangent space normals overlay
        Pass
        {
            ZTest LEqual

            CGPROGRAM
            #pragma vertex vert_tangent_space_normals
            #pragma fragment frag
            #pragma target 4.5
            ENDCG
        }

        // pass 6: vertex color overlay
        Pass
        {
            ZTest LEqual

            CGPROGRAM
            #pragma vertex vert_color
            #pragma fragment frag
            #pragma target 4.5
            ENDCG
        }

        // pass 7: lasso
        Pass
        {
            ZTest Always

            CGPROGRAM
            #pragma vertex vert_lasso
            #pragma fragment frag
            #pragma target 4.5
            ENDCG
        }

        // pass 8: depth
        Pass
        {
            ZTest LEqual

            CGPROGRAM
            #pragma vertex vert_depth
            #pragma fragment frag_depth
            #pragma target 4.5
            ENDCG
        }

        // pass 9: brush range
        Pass
        {
            ZTest LEqual

            CGPROGRAM
            #pragma vertex vert_brush_range
            #pragma fragment frag_brush_range
            #pragma target 4.5
            ENDCG
        }
    }
}
