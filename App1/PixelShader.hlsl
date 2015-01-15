float4 main(float4 position : POSITION, float4 color : COLOR) : SV_TARGET
{
	float4 newColor = color;
	newColor.a = 0.5f; //for some reason, the blending does not work if alpha is less than or equal to 0.5
	return newColor;
}