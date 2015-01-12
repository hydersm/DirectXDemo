#pragma once

using namespace Microsoft::WRL;
using namespace Windows::UI::Core;
using namespace Platform;
using namespace DirectX;

class CGame
{
private:
	ComPtr<ID3D11Device1> dev;
	ComPtr<ID3D11DeviceContext1> devcon;
	ComPtr<IDXGISwapChain1> swapchain;
	ComPtr<ID3D11RenderTargetView> renderTarget;
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11VertexShader> vertexshader;
	ComPtr<ID3D11PixelShader> pixelshader;
	ComPtr<ID3D11InputLayout> inputlayout;
	ComPtr<ID3D11Buffer> constantBuffer;

public:
	void Initialize();
	void Update();
	void Render();
	void InitGraphics();
	void InitPipeline();
};

