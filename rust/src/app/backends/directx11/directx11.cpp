#include "../../overlay.hpp"
#include "directx11.hpp"
#include <utils/vmp.hpp>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dcomp.lib")

#include <print>
#include <utils/debug.hpp>

#if defined(APP_BACKEND_USE_DX11)
namespace winapp::backends::directx11
{
	bool create_device(HWND hwnd)
	{
		VMP_START("create_device");
		DBG("[dx] dx1: entered create_device hwnd={:#x}", (uintptr_t)hwnd);
		std::uint32_t device_flags{ 0 };
		D3D_FEATURE_LEVEL feature_level{};
		const std::array<D3D_FEATURE_LEVEL, 2> feature_levels{ D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

		DBG("[dx] dx2: D3D11CreateDevice (HARDWARE)");
		HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, device_flags, feature_levels.data(), feature_levels.size(), D3D11_SDK_VERSION, &impl::device, &feature_level, &impl::device_context);
		DBG("[dx] dx3: D3D11CreateDevice returned hr={:x}", (std::uint32_t)hr);

		if (hr == DXGI_ERROR_UNSUPPORTED)
		{
			DBG("failed to create hardware accelerated device, forwarding with WARP");
			hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, device_flags, feature_levels.data(), feature_levels.size(), D3D11_SDK_VERSION, &impl::device, &feature_level, &impl::device_context);
		}

		if (FAILED(hr))
		{
			DBG("failed to create device (HRESULT: {:x})", (std::uint32_t)hr);
			return false;
		}

		DBG("[dx] dx4: QueryInterface IDXGIDevice");
		IDXGIDevice* dxgi_device{ nullptr };
		impl::device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgi_device);
		DBG("[dx] dx5: dxgi_device={:#x}", (uintptr_t)dxgi_device);

		DBG("[dx] dx6: GetAdapter");
		IDXGIAdapter* dxgi_adapter{ nullptr };
		dxgi_device->GetAdapter(&dxgi_adapter);
		DBG("[dx] dx7: dxgi_adapter={:#x}", (uintptr_t)dxgi_adapter);

		DBG("[dx] dx8: GetParent IDXGIFactory2");
		IDXGIFactory2* dxgi_factory{ nullptr };
		dxgi_adapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgi_factory);
		DBG("[dx] dx9: dxgi_factory={:#x}", (uintptr_t)dxgi_factory);

		RECT rect{};
		::GetClientRect(hwnd, &rect);
		DBG("[dx] dx10: GetClientRect = {}x{}", rect.right - rect.left, rect.bottom - rect.top);

#ifdef HIJACK_OVERLAY
		DXGI_SWAP_CHAIN_DESC sd{};
		sd.BufferCount = 2;
		sd.BufferDesc.Width = 0;
		sd.BufferDesc.Height = 0;
		sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 0;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hwnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		sd.Flags = 0;

		hr = dxgi_factory->CreateSwapChain(impl::device, &sd, reinterpret_cast<IDXGISwapChain**>(&impl::swap_chain));
#else
		DXGI_SWAP_CHAIN_DESC1 swap_chain_desc1{};
		swap_chain_desc1.Width = rect.right - rect.left;
		swap_chain_desc1.Height = rect.bottom - rect.top;
		swap_chain_desc1.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swap_chain_desc1.Stereo = FALSE;
		swap_chain_desc1.SampleDesc.Count = 1;
		swap_chain_desc1.SampleDesc.Quality = 0;
		swap_chain_desc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swap_chain_desc1.BufferCount = 2;
		swap_chain_desc1.Scaling = DXGI_SCALING_STRETCH;
		swap_chain_desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swap_chain_desc1.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
		swap_chain_desc1.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		DBG("[dx] dx11: CreateSwapChainForComposition");
		hr = dxgi_factory->CreateSwapChainForComposition(impl::device, &swap_chain_desc1, nullptr, &impl::swap_chain);
		DBG("[dx] dx12: CreateSwapChainForComposition returned hr={:x}", (std::uint32_t)hr);
#endif
		if (FAILED(hr))
		{
			DBG("failed to create swapchain (HRESULT: {:x})", (std::uint32_t)hr);
			dxgi_factory->Release();
			dxgi_adapter->Release();
			dxgi_device->Release();
			return false;
		}

		{
			IDXGISwapChain2* swap_chain2{ nullptr };
			if (SUCCEEDED(impl::swap_chain->QueryInterface(&swap_chain2)))
			{
				swap_chain2->SetMaximumFrameLatency(1);
				swap_chain2->Release();
			}
		}

#ifndef HIJACK_OVERLAY
		DBG("[dx] dx13: DCompositionCreateDevice");
		hr = DCompositionCreateDevice(dxgi_device, __uuidof(IDCompositionDevice), (void**)&impl::dcomp_device);
		DBG("[dx] dx14: DCompositionCreateDevice returned hr={:x}", (std::uint32_t)hr);
		if (FAILED(hr))
		{
			DBG("failed to create DComp device (HRESULT: {:x})", (std::uint32_t)hr);
			dxgi_factory->Release();
			dxgi_adapter->Release();
			dxgi_device->Release();
			return false;
		}

		DBG("[dx] dx15: CreateTargetForHwnd");
		hr = impl::dcomp_device->CreateTargetForHwnd(hwnd, TRUE, &impl::dcomp_target);
		DBG("[dx] dx16: CreateTargetForHwnd returned hr={:x}", (std::uint32_t)hr);
		if (FAILED(hr))
		{
			DBG("failed to create DComp target (HRESULT: {:x})", (std::uint32_t)hr);
			dxgi_factory->Release();
			dxgi_adapter->Release();
			dxgi_device->Release();
			return false;
		}

		DBG("[dx] dx17: CreateVisual + SetContent + SetRoot + Commit");
		impl::dcomp_device->CreateVisual(&impl::dcomp_visual);
		impl::dcomp_visual->SetContent(impl::swap_chain);
		impl::dcomp_target->SetRoot(impl::dcomp_visual);
		impl::dcomp_device->Commit();
		DBG("[dx] dx18: Commit returned");
#endif

		dxgi_factory->Release();
		dxgi_adapter->Release();
		dxgi_device->Release();

		if (!create_blend_state())
			DBG("failed to create blend state");

		if (!update_refresh_rate(hwnd))
			DBG("failed to query monitor refresh rate, defaulting to 60hz");

		VMP_END;
		return create_render_target();
	}

	bool create_render_target()
	{
		ID3D11Texture2D* back_buffer{ nullptr };
		impl::swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
		if (!back_buffer)
		{
			DBG("failed to get back buffer from swapchain");
			return false;
		}

		D3D11_RENDER_TARGET_VIEW_DESC rtv_desc{};
		rtv_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		impl::device->CreateRenderTargetView(back_buffer, &rtv_desc, &impl::render_target);

		back_buffer->Release();

		if (!impl::render_target)
		{
			DBG("failed to create render target");
			return false;
		}

		return true;
	}

	void cleanup_render_target()
	{
		if (impl::render_target)
		{
			impl::render_target->Release();
			impl::render_target = nullptr;
		}
	}

	bool update_refresh_rate(HWND hwnd)
	{
		const HMONITOR hmonitor = ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
		if (!hmonitor)
		{
			DBG("failed to get monitor from window");
			return false;
		}

		MONITORINFOEXW mi{};
		mi.cbSize = sizeof(mi);
		if (!::GetMonitorInfoW(hmonitor, &mi))
		{
			DBG("failed to get monitor info");
			return false;
		}

		DEVMODEW dm{};
		dm.dmSize = sizeof(dm);
		if (!::EnumDisplaySettingsW(mi.szDevice, ENUM_CURRENT_SETTINGS, &dm))
		{
			DBG("failed to enumerate display settings");
			return false;
		}

		impl::refresh_rate = static_cast<double>(dm.dmDisplayFrequency);

		DBG("monitor refresh rate: {}hz", dm.dmDisplayFrequency);
		return true;
	}

	bool create_blend_state()
	{
		D3D11_BLEND_DESC blend_desc{};
		blend_desc.AlphaToCoverageEnable = FALSE;
		blend_desc.IndependentBlendEnable = FALSE;
		blend_desc.RenderTarget[0].BlendEnable = TRUE;
		blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		HRESULT hr = impl::device->CreateBlendState(&blend_desc, &impl::blend_state);
		if (FAILED(hr))
		{
			DBG("failed to create blend state (HRESULT: {:x})", (std::uint32_t)hr);
			return false;
		}
		return true;
	}

	void destroy_device()
	{
		cleanup_render_target();

		if (impl::blend_state)
		{
			impl::blend_state->Release();
			impl::blend_state = nullptr;
		}

		if (impl::dcomp_visual)
		{
			impl::dcomp_visual->Release();
			impl::dcomp_visual = nullptr;
		}

		if (impl::dcomp_target)
		{
			impl::dcomp_target->Release();
			impl::dcomp_target = nullptr;
		}

		if (impl::dcomp_device)
		{
			impl::dcomp_device->Release();
			impl::dcomp_device = nullptr;
		}

		if (impl::swap_chain)
		{
			impl::swap_chain->Release();
			impl::swap_chain = nullptr;
		}

		if (impl::device_context)
		{
			impl::device_context->Release();
			impl::device_context = nullptr;
		}

		if (impl::device)
		{
			impl::device->Release();
			impl::device = nullptr;
		}
	}
}
#endif