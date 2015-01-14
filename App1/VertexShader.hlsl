cbuffer ConstantBuffer
{
	row_major float4x4 matFinal;
	row_major float4x4 matRotation;
	float4 lightvec;
	float4 lightcol;
	float4 ambientcol;
};

struct Vout
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

Vout main( float4 pos : POSITION, float4 normal: NORMAL )
{
	Vout output;

	output.position = mul(pos, matFinal); 
	output.color = ambientcol;

	float4 norm = normalize(mul(normal, matRotation));
	float diffuseBrightness = saturate(dot(norm, lightvec));
	output.color += lightcol * diffuseBrightness;

	return output;
}