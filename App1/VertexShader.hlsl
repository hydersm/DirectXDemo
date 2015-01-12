cbuffer ConstantBuffer
{
	float3 Offset;
};

struct Vout
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

Vout main( float4 pos : POSITION, float4 color: COLOR )
{
	Vout output;

	output.position = pos;
	output.position.x += Offset.x;
	output.position.y += Offset.y;
	output.position.xy *= Offset.z;
	output.color = color;

	return output;
}