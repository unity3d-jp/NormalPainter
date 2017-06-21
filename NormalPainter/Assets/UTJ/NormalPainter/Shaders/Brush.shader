Shader "Hidden/NormalPainter/Brush"
{
CGINCLUDE
#include "UnityCG.cginc"


StructuredBuffer<float> _BrushSamples;

struct appdata
{
    float4 vertex : POSITION;
};

struct v2f
{
    float4 vertex : SV_POSITION;
    float4 uv : TEXCOORD0;
};


v2f vert(appdata v)
{
    v2f o;
    o.vertex = float4(v.vertex.xy, 0.0, 1.0);
    //o.vertex.y *= -1;
    o.uv = float4(abs(v.vertex.x * 0.5 + 0.5), 1.0f - (v.vertex.y * 0.5f + 0.5f), 0, 0);
    return o;
}

float4 frag(v2f i) : SV_Target
{
    uint n, s;
    _BrushSamples.GetDimensions(n, s);

    float u = 1.0 - abs(i.uv.x * 2.0 - 1.0);
    float v = _BrushSamples[(int)(u * (n-1))];
    return i.uv.y < v ? float4(1, 1, 1, 1) : float4(0, 0, 0, 0);
    //return float4(i.uv.xxx, 1);
}
ENDCG

    SubShader
    {
        Tags{ "RenderType" = "Transparent" "Queue" = "Transparent+1" }

        Pass
        {
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag
            #pragma target 4.5
            ENDCG
        }
    }
}
