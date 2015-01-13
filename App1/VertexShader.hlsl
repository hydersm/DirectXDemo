cbuffer ConstantBuffer
{
	row_major float4x4 matFinal;
};

struct Vout
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

Vout main( float4 pos : POSITION, float4 color: COLOR )
{
	Vout output;

	output.position = mul(pos, matFinal); 
	//output.position = pos;
	output.color = color;

	return output;
}