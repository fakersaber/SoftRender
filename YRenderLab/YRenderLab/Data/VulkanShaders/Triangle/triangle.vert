// Copyright 2020 Google LLC

struct VSInput
{
    [[vk::location(0)]] float3 Pos : POSITION0;
    [[vk::location(1)]] float3 Color : COLOR0;
};

struct ViewBuffers
{
	float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

cbuffer ubo : register(b0)
{
    ViewBuffers ubo;
}

struct VSOutput
{
	float4 Pos : SV_POSITION;
    [[vk::location(0)]] float3 Color : COLOR0;
};

VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;
	output.Color = input.Color;
    output.Pos = mul(ubo.projectionMatrix, mul(ubo.viewMatrix, float4(input.Pos.xyz, 1.0)));
	return output;
}
