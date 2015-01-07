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
public:
	void Initialize();
	void Update();
	void Render();
};

