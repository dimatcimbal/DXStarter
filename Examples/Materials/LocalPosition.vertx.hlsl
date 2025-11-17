#include "Base.rsign.hlsl"

[RootSignature(ROOTSIGN)]
float4 main(float3 pos: POSITION) : SV_POSITION
{
    return float4(pos, 1.0f);
}