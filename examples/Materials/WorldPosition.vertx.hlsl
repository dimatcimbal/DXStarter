#include "WorldPosition.rsign.hlsl"

cbuffer MeshConstants : register(b0)
{
    float4x4 World;   // Object to world
};

[RootSignature(ROOTSIGN)]
float4 main(float3 pos: POSITION) : SV_POSITION
{
    float4 worldPos = mul(World, float4(pos, 1.0f));
    return worldPos;
}
