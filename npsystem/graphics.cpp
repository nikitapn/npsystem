// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory
// 
//#define _USE_MATH_DEFINES
#include "stdafx.h"
#include "graphics.h"
#include "constants.h"


#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dxguid.lib")

#include <boost/math/constants/constants.hpp>
constexpr auto PI = boost::math::constants::pi<float>();

dis::dis() {
	D2D1_FACTORY_OPTIONS options;
//	
#ifdef DEBUG
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#else 
	options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
#endif // DEBUG

	HR(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory2), &options, (void**)&d2dFactory));

	static const wchar_t msc_fontName[] = L"Verdana";
	static const FLOAT msc_fontSize = 12;
	
	HR(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(pDWriteFactory), reinterpret_cast<IUnknown **>(&pDWriteFactory)));

	HR(pDWriteFactory->CreateTextFormat(msc_fontName,
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		8,
		L"", //locale
		&text_centered[0]));

	// Center the text horizontally and vertically.
	text_centered[0]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	text_centered[0]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	HR(pDWriteFactory->CreateTextFormat(msc_fontName,
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		12,
		L"", //locale
		&text_centered[1]));

	// Center the text horizontally and vertically.
	text_centered[1]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	text_centered[1]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	HR(pDWriteFactory->CreateTextFormat(msc_fontName,
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		18,
		L"", //locale
		&text_centered[2]));

	// Center the text horizontally and vertically.
	text_centered[2]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	text_centered[2]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	// Left Alignment
	HR(pDWriteFactory->CreateTextFormat(
		msc_fontName,
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		msc_fontSize,
		L"", //locale
		&pTextFormatLeft));
		
	// Center the text horizontally and vertically.
	pTextFormatLeft->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	pTextFormatLeft->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	// Create a DirectWrite text format object.
	HR(pDWriteFactory->CreateTextFormat(
		msc_fontName,
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		msc_fontSize,
		L"", //locale
		&pTextFormatRight));
		
	// Center the text horizontally and vertically.
	pTextFormatRight->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
	pTextFormatRight->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	HR(pDWriteFactory->CreateTextFormat(
		msc_fontName,
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		16,
		L"", //locale
		&pTextOnline));
	
	// Center the text horizontally and vertically.
	pTextOnline->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	pTextOnline->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	// slots
	HR(pDWriteFactory->CreateTextFormat(
		msc_fontName,
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		10,
		L"", //locale
		&pTextFormatSlotLeft));
		
	// Center the text horizontally and vertically.
	pTextFormatSlotLeft->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	pTextFormatSlotLeft->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	// Create a DirectWrite text format object.
	HR(pDWriteFactory->CreateTextFormat(
		msc_fontName,
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		10,
		L"", //locale
		&pTextFormatSlotRight));
		
	// Center the text horizontally and vertically.
	pTextFormatSlotRight->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
	pTextFormatSlotRight->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	CreateGeometries();

	HR(pDWriteFactory->CreateTextLayout(L"unk", 3, pTextOnline, 100, 18, &text_layout_unknown));
	HR(pDWriteFactory->CreateTextLayout(L"x", 1, pTextOnline, 100, 18, &text_layout_bad_quality));
}

dis::~dis() {
	d2dFactory->Release();
	pDWriteFactory->Release();
	for (auto text : text_centered) text->Release();
	pTextFormatLeft->Release();
	pTextFormatRight->Release();
	pTextFormatSlotLeft->Release();
	pTextFormatSlotRight->Release();
	pTextOnline->Release();
}

// CGraphics

// This array defines the set of DirectX hardware feature levels this app  supports.
// The ordering is important and you should  preserve it.
// Don't forget to declare your app's minimum required feature level in its
// description.  All apps are assumed to support 9.1 unless otherwise stated.
D3D_FEATURE_LEVEL CGraphics::m_featureLevels[] = 
{
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0,
	D3D_FEATURE_LEVEL_9_3,
	D3D_FEATURE_LEVEL_9_2,
	D3D_FEATURE_LEVEL_9_1
};

CGraphics::CGraphics(HWND hwnd) 
	: m_hwnd(hwnd)
	, m_dirty{ 0 }
	, id_(++last_id_) {
	// This flag adds support for surfaces with a different color channel ordering than the API default.
	// You need it for compatibility with Direct2D.
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	// Create the DX11 API device object, and get a corresponding context.

	HR(D3D11CreateDevice(
			nullptr,                    // specify null to use the default adapter
			D3D_DRIVER_TYPE_HARDWARE,
			0,
			creationFlags,              // optionally set debug and Direct2D compatibility flags
			m_featureLevels,              // list of feature levels this app can support
			sizeof(m_featureLevels) / sizeof(D3D_FEATURE_LEVEL),   // number of possible feature levels
			D3D11_SDK_VERSION,
			m_d3dDevice.GetAddressOf(),               // returns the Direct3D device created
			&m_featureLevel,            // returns feature level of device created
			m_d3dContext.GetAddressOf()                    // returns the device immediate context
		));
	wrl::ComPtr<IDXGIDevice1> dxgiDevice;
	// Obtain the underlying DXGI device of the Direct3D11 device.
	HR(m_d3dDevice.As(&dxgiDevice));
	
	// Obtain the Direct2D device for 2-D rendering.
	HR(dis::Get().d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice));

	// Get Direct2D device's corresponding device context object.
	HR(m_d2dDevice->CreateDeviceContext(
			D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
			&m_d2dContext));
	

	// obtain dpi and geometry flattering tolerance
	m_d2dContext->GetDpi(&m_dpi_x, &m_dpi_x);
	m_flatteningTolerance = D2D1::ComputeFlatteningTolerance(
		D2D1::Matrix3x2F::Identity(),   // apply no additional scaling transform
		m_dpi_x,                           // horizontal DPI
		m_dpi_x,                            // vertical DPI
		5.0f
	);

	// Allocate a descriptor.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
	swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // this is the most common swapchain format
	swapChainDesc.SampleDesc.Count = 1;                // don't use multi-sampling
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;                     // use double buffering to enable flip
//	swapChainDesc.Scaling = DXGI_SCALING_NONE;
//	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // all apps must use this SwapEffect
//	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH /*| DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE*/;

	// Identify the physical adapter (GPU or card) this device is runs on.
	wrl::ComPtr<IDXGIAdapter> dxgiAdapter;
	HR(dxgiDevice->GetAdapter(&dxgiAdapter));
	
	// Get the factory object that created the DXGI device.
	wrl::ComPtr<IDXGIFactory2> dxgiFactory;
	HR(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)));

	// Get the final swap chain for this window from the DXGI factory.
	HR(dxgiFactory->CreateSwapChainForHwnd(
			m_d3dDevice.Get(),
			m_hwnd,
			&swapChainDesc,
			nullptr,    // allow on all displays
			nullptr,
			&m_swapChain
		));

	// Ensure that DXGI doesn't queue more than one frame at a time.
	HR(dxgiDevice->SetMaximumFrameLatency(1));

	CreateBrushes();
	CreateDeviceSwapChainBitmap();

	RECT rc;
	::GetClientRect(hwnd, &rc);
	CreateDeviceSizeResourses(rc.right - rc.left, rc.bottom - rc.top);

	m_lineX = m_lineY = 0;

	// effects
	m_d2dContext->CreateEffect(CLSID_D2D1GaussianBlur, &m_gaussianBlurEffect);
	m_d2dContext->CreateEffect(CLSID_D2D1Saturation, &m_saturationEffect);
}
void CGraphics::CreateDeviceSwapChainBitmap() {
	// Direct2D needs the dxgi version of the backbuffer surface pointer.
	wrl::ComPtr<IDXGISurface> dxgiBackBuffer;
	HR(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer)));
	
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = 
		D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
			m_dpi_x, m_dpi_y
		);

	wrl::ComPtr<ID2D1Bitmap1> bitmap;
	// Get a D2D surface from the DXGI back buffer to use as the D2D render target.
	HR(m_d2dContext->CreateBitmapFromDxgiSurface(
			dxgiBackBuffer.Get(),
			&bitmapProperties,
			bitmap.GetAddressOf()
		));
	// Now we set up the Direct2D render target bitmap linked to the swapchain. 
	// Whenever we render to this bitmap, it is directly rendered to the 
	// swap chain associated with the window.
	m_d2dContext->SetTarget(bitmap.Get());
}

void CGraphics::CreateDeviceSizeResourses(UINT width, UINT height) {
	D2D1_BITMAP_PROPERTIES1 bitmapProperties =
		D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
			m_dpi_x, m_dpi_y
		);
	
	HR(m_d2dContext->CreateBitmap(D2D1::SizeU(width, height),
		nullptr, 0,
		bitmapProperties,
		m_bitmap.ReleaseAndGetAddressOf()
	));

	HR(m_d2dContext->CreateBitmap(D2D1::SizeU(width, height),
		nullptr, 0, 
		bitmapProperties,
		m_shadowBitmap.ReleaseAndGetAddressOf()
	));

//	wrl::ComPtr<ID2D1Effect> shadowEffect;
//	m_d2dContext->CreateEffect(CLSID_D2D1Shadow, &shadowEffect);
	
//	shadowEffect->SetInput(0, m_blockBitmap.Get());

	// Shadow is composited on top of a white surface to show opacity.
//	wrl::ComPtr<ID2D1Effect> floodEffect;
//	m_d2dContext->CreateEffect(CLSID_D2D1Flood, &floodEffect);

//	floodEffect->SetValue(D2D1_FLOOD_PROP_COLOR, D2D1::Vector4F(1.0f, 1.0f, 1.0f, 1.0f));

//	wrl::ComPtr<ID2D1Effect> affineTransformEffect;
//	m_d2dContext->CreateEffect(CLSID_D2D12DAffineTransform, &affineTransformEffect);

//	affineTransformEffect->SetInputEffect(0, shadowEffect.Get());

//	D2D1_MATRIX_3X2_F matrix = D2D1::Matrix3x2F::Translation(20, 20);
//
//	affineTransformEffect->SetValue(D2D1_2DAFFINETRANSFORM_PROP_TRANSFORM_MATRIX, matrix);

//	wrl::ComPtr<ID2D1Effect> compositeEffect;
//	m_d2dContext->CreateEffect(CLSID_D2D1Composite, &m_shadow);

//	m_shadow->SetInputEffect(0, floodEffect.Get());
//	m_shadow->SetInputEffect(0, affineTransformEffect.Get());
	m_d2dContext->CreateEffect(CLSID_D2D1Shadow, m_shadow.ReleaseAndGetAddressOf());
	m_shadow->SetInput(0, m_shadowBitmap.Get());
}

void CGraphics::Resize(UINT width, UINT height) {
	if (width == 0 || height == 0)
		return;

	m_dimArea.width = static_cast<FLOAT>(width);
	m_dimArea.height = static_cast<FLOAT>(height);
	
	CalcVisibleArea();

	m_d2dContext->SetTarget(nullptr);
	m_dirty.center = false;

	if (S_OK == m_swapChain->ResizeBuffers(0,
		0, 0,
		DXGI_FORMAT_UNKNOWN, //, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH /*| DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE*/
		0
	)) {
		CreateDeviceSwapChainBitmap();
		CreateDeviceSizeResourses(width, height);
	} else
	{

	}
}

CGraphics::~CGraphics() {
	for (size_t i = 0; i < nSolidBrushes; i++)
		m_brushes[i].Reset();

	m_LGWhiteBrush.Reset();

	for (size_t i = 0; i < BlockColorCount; ++i) {
		m_gradientBlockBrush[i].Reset();
		m_gradientBlockHeaderBrush[i].Reset();
	}

	m_RGBrush1.Reset();
	m_RGBrushSl1.Reset();
	m_RGBrushSlotConnected.Reset();
	m_RGBrushSl3.Reset();
	m_RGBrushSlotActive.Reset();
	//
	m_d2dContext->SetTarget(NULL);
	m_d2dContext.Reset();
	m_swapChain.Reset();
	m_d2dDevice.Reset();
	m_shadow.Reset();
	m_bitmap.Reset();
	m_shadowBitmap.Reset();
}

void CGraphics::MoveOrgDxDy(float dx, float dy) {
	auto dxdy = D2D1::Matrix3x2F::Translation(dx, dy);
	m_Matrix = m_Matrix * dxdy;
	m_shadowMatrix = m_shadowMatrix * dxdy;
	m_d2dContext->SetTransform(m_Matrix);
	CalcVisibleArea();
	m_dirty.center = false;
}

void CGraphics::SetMatrix(const D2D1::Matrix3x2F& mat) {
	m_Matrix = mat;
	float offset = mat._11 * 5.0f;
	m_shadowMatrix = m_Matrix * D2D1::Matrix3x2F::Translation(offset, offset);
	m_d2dContext->SetTransform(m_Matrix);
	CalcVisibleArea();
	m_dirty.center = false;
}

const D2D1_POINT_2F& CGraphics::GetCenterOfView() noexcept {
	if (m_dirty.center == false) {
		const D2D1::Matrix3x2F& invM = GetInverseMatrix();
		m_ptCenter.x = m_dimArea.width / 2.0f;
		m_ptCenter.y = m_dimArea.height / 2.0f;
		m_ptCenter = m_ptCenter * invM;
		m_dirty.center = true;
	}
	return m_ptCenter;
}

void CGraphics::DrawGrid() {
	if (m_bGridEnabled == false)
		return;

	const D2D1::Matrix3x2F& invM = GetInverseMatrix();

	D2D1::MyPoint2F org(0, 0);
	org = org * invM;

	D2D1::MyPoint2F end = (D2D1::MyPoint2F&)m_d2dContext->GetSize();
	end = end * invM;
	
	for (float i = (float)(((int)org.y / constants::block::GRIDSZ_L) * constants::block::GRIDSZ_L); i < end.y; i += constants::block::GRIDSZ_F) {
		m_d2dContext->DrawLine(
			D2D1::Point2F(org.x, i),
			D2D1::Point2F(end.x, i),
			m_brushes[static_cast<size_t>(SolidColor::Gray)].Get(),
			0.1f);
	}
	for (float i = (float)(((int)org.x / constants::block::GRIDSZ_L) * constants::block::GRIDSZ_L); i < end.x; i += constants::block::GRIDSZ_F) {
		m_d2dContext->DrawLine(
			D2D1::Point2F(i, org.y),
			D2D1::Point2F(i, end.y),
			m_brushes[static_cast<size_t>(SolidColor::Gray)].Get(),
			0.1f);
	}
}


void CGraphics::CreateBrushes() {
	m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), m_brushes[static_cast<size_t>(SolidColor::White)].GetAddressOf());
	m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), m_brushes[static_cast<size_t>(SolidColor::Black)].GetAddressOf());
	m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), m_brushes[static_cast<size_t>(SolidColor::Red)].GetAddressOf());
	m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), m_brushes[static_cast<size_t>(SolidColor::Green)].GetAddressOf());
	m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue), m_brushes[static_cast<size_t>(SolidColor::Blue)].GetAddressOf());
	m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray), m_brushes[static_cast<size_t>(SolidColor::Gray)].GetAddressOf());
	m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightGray), m_brushes[static_cast<size_t>(SolidColor::LightGray)].GetAddressOf());
	m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::SteelBlue), m_brushes[static_cast<size_t>(SolidColor::Test)].GetAddressOf());
	m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(0.25f, 0.25f, 0.25f), m_brushes[static_cast<size_t>(SolidColor::DarkGray)].GetAddressOf());
	m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(0.95f, 0.10f, 0.10f), m_brushes[static_cast<size_t>(SolidColor::LineBadQuality)].GetAddressOf());
	m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(0.65f, 0.15f, 0.65f), m_brushes[static_cast<size_t>(SolidColor::LineDeviceNotResponded)].GetAddressOf());
	m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(0.95f, 0.8f, 0.8f), m_brushes[static_cast<size_t>(SolidColor::SlotValue)].GetAddressOf());
	m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray, 0.5f), m_brushes[static_cast<size_t>(SolidColor::EditColor)].GetAddressOf());
	m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkCyan, 1.0f), m_brushes[static_cast<size_t>(SolidColor::SelColor)].GetAddressOf());
	m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 0.55f), m_brushes[static_cast<size_t>(SolidColor::SliderThing)].GetAddressOf());
	m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkCyan, 0.85f), m_brushes[static_cast<size_t>(SolidColor::SliderThingHovered)].GetAddressOf());
	m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkSlateBlue, 0.20f), m_brushes[static_cast<size_t>(SolidColor::SliderBackground)].GetAddressOf());
	
	D2D1_GRADIENT_STOP gradientStops[10];
	
	auto  create_linear_gradient_brush = [&](wrl::ComPtr<ID2D1LinearGradientBrush>& brush, int n) {
		wrl::ComPtr<ID2D1GradientStopCollection> gradientStopCollection;
		HR(m_d2dContext->CreateGradientStopCollection(
			gradientStops,
			n,
			D2D1_GAMMA_2_2,
			D2D1_EXTEND_MODE_CLAMP,
			gradientStopCollection.GetAddressOf()
		));

		HR(m_d2dContext->CreateLinearGradientBrush(
			D2D1::LinearGradientBrushProperties(D2D1::Point2F(0, 0), D2D1::Point2F(0, 0)),
			gradientStopCollection.Get(), brush.GetAddressOf()
		));
	};

	auto create_radial_gradient_brush = [&](wrl::ComPtr<ID2D1RadialGradientBrush>& brush, int n, FLOAT radiusX, FLOAT radiusY) {
		wrl::ComPtr<ID2D1GradientStopCollection> gradientStopCollection;

		HR(m_d2dContext->CreateGradientStopCollection(
			gradientStops,
			n,
			D2D1_GAMMA_2_2,
			D2D1_EXTEND_MODE_CLAMP,
			gradientStopCollection.GetAddressOf()
		));

		HR(m_d2dContext->CreateRadialGradientBrush(
			D2D1::RadialGradientBrushProperties(D2D1::Point2F(0, 0),D2D1::Point2F(0, 0), radiusX, radiusY),
			gradientStopCollection.Get(),
			brush.GetAddressOf()
		));
	};

	auto create_block_brush = [&](BlockColor colorIndex, UINT32 color) {
		auto index = static_cast<size_t>(colorIndex);
		gradientStops[0].color = D2D1::ColorF(color, 0.85f);
		gradientStops[0].position = 0.0f;
		gradientStops[1].color = D2D1::ColorF(color - 0x202020, 0.85f);
		gradientStops[1].position = 1.0f;
		create_linear_gradient_brush(m_gradientBlockBrush[index], 2);
	};

	auto create_block_header_brush = [&](BlockColor colorIndex, UINT32 color) {
		auto index = static_cast<size_t>(colorIndex);
		gradientStops[0].color = D2D1::ColorF(color, 1.0f);
		gradientStops[0].position = 0.0f;
		gradientStops[1].color = D2D1::ColorF(color - 0x202020, 0.4f);
		gradientStops[1].position = 0.9f;
		gradientStops[2].color = D2D1::ColorF(color - 0x202020, 0.0f);
		gradientStops[2].position = 1.0f;
		create_radial_gradient_brush(m_gradientBlockHeaderBrush[index], 3, 400.0f, 12.0f);
	};

	// m_LGWhiteBrush
	gradientStops[0].color = D2D1::ColorF(D2D1::ColorF::White, 1.0f);
	gradientStops[0].position = 0.0f;
	gradientStops[1].color = D2D1::ColorF(D2D1::ColorF::White, 0.0f);
	gradientStops[1].position = 0.03f;
	gradientStops[2].color = D2D1::ColorF(D2D1::ColorF::White, 0.0f);
	gradientStops[2].position = 1.00f;
	create_linear_gradient_brush(m_LGWhiteBrush, 3);

	// m_GradientBlockBrush
	create_block_brush(BlockColor::Standart, D2D1::ColorF::GhostWhite);
	create_block_header_brush(BlockColor::Standart, D2D1::ColorF::LightSkyBlue);

	create_block_brush(BlockColor::Parameter, D2D1::ColorF::GhostWhite);
	create_block_header_brush(BlockColor::Parameter, D2D1::ColorF::SeaGreen);

	create_block_brush(BlockColor::ParameterQuality, D2D1::ColorF::NavajoWhite);
	create_block_header_brush(BlockColor::ParameterQuality, D2D1::ColorF::LimeGreen);

	create_block_brush(BlockColor::Math, D2D1::ColorF::GhostWhite);
	create_block_header_brush(BlockColor::Math, D2D1::ColorF::SeaGreen);

	create_block_brush(BlockColor::Compare, D2D1::ColorF::GhostWhite);
	create_block_header_brush(BlockColor::Compare, D2D1::ColorF::PaleGreen);

	create_block_brush(BlockColor::Control, D2D1::ColorF::GhostWhite);
	create_block_header_brush(BlockColor::Control, D2D1::ColorF::MediumPurple);
	// m_RGBrush1
	gradientStops[0].color = D2D1::ColorF(D2D1::ColorF::Gray, 0.6f);
	gradientStops[0].position = 0.0f;
	gradientStops[1].color = D2D1::ColorF(D2D1::ColorF::Gray, 0.6f);
	gradientStops[1].position = 0.9f;
	gradientStops[2].color = D2D1::ColorF(D2D1::ColorF::DarkGray, 1.0f);
	gradientStops[2].position = 1.0f;
	create_radial_gradient_brush(m_RGBrush1, 3, wheel_r, wheel_r);
	// Slot_1
	gradientStops[0].color = D2D1::ColorF(D2D1::ColorF::Azure, 1.0f);
	gradientStops[0].position = 0.5f;
	gradientStops[1].color = D2D1::ColorF(D2D1::ColorF::Azure, 0.0f);
	gradientStops[1].position = 1.0f;
	create_radial_gradient_brush(m_RGBrushSl1, 2, constants::block::SLOT_SIZE / 2.f, constants::block::SLOT_SIZE / 2.f);
	// m_RGBrushSlotConnected
	gradientStops[0].color = D2D1::ColorF(D2D1::ColorF::LightGray, 1.0f);
	gradientStops[0].position = 0.5f;
	gradientStops[1].color = D2D1::ColorF(D2D1::ColorF::SlateGray, 0.0f);
	gradientStops[1].position = 1.0f;
	create_radial_gradient_brush(m_RGBrushSlotConnected, 2, constants::block::SLOT_SIZE / 2.f, constants::block::SLOT_SIZE / 2.f);
	// Slot_3
	gradientStops[0].color = D2D1::ColorF(D2D1::ColorF::Cyan, 1.0f);
	gradientStops[0].position = 0.5f;
	gradientStops[1].color = D2D1::ColorF(D2D1::ColorF::Cyan, 0.0f);
	gradientStops[1].position = 1.0f;
	create_radial_gradient_brush(m_RGBrushSl3, 2, constants::block::SLOT_SIZE / 2.f, constants::block::SLOT_SIZE / 2.f);
	// Slot Activated
	gradientStops[0].color = D2D1::ColorF(D2D1::ColorF::LightGreen, 1.0f);
	gradientStops[0].position = 0.5f;
	gradientStops[1].color = D2D1::ColorF(D2D1::ColorF::SlateGray, 0.0f);
	gradientStops[1].position = 1.0f;
	create_radial_gradient_brush(m_RGBrushSlotActive, 2, constants::block::SLOT_SIZE / 2.f, constants::block::SLOT_SIZE / 2.f);

	// SFC block
	gradientStops[0].color = D2D1::ColorF(D2D1::ColorF::LightSteelBlue - 0x404040, 0.85f);
	gradientStops[0].position = 0.0f;
	gradientStops[1].color = D2D1::ColorF(D2D1::ColorF::LightSteelBlue, 0.85f);
	gradientStops[1].position = 1.0f;
	create_linear_gradient_brush(m_gradientBlockBrush[static_cast<size_t>(BlockColor::SfcBlockStep)], 2);
}

void CGraphics::FillRoundRect(const D2D1_ROUNDED_RECT& rect, SolidColor color) {
	m_d2dContext->FillRoundedRectangle(rect, m_brushes[static_cast<size_t>(color)].Get());
}

void CGraphics::FillBlock(const D2D1_ROUNDED_RECT& rect, BlockColor colorIndex, bool draw_header) {
	auto index = static_cast<size_t>(colorIndex);

	auto& brush = m_gradientBlockBrush[index];
	brush->SetStartPoint(D2D1::Point2F(rect.rect.left, rect.rect.top));
	brush->SetEndPoint(D2D1::Point2F(rect.rect.left, rect.rect.bottom));
	m_d2dContext->FillRoundedRectangle(rect, brush.Get());

	m_LGWhiteBrush->SetStartPoint(D2D1::Point2F(rect.rect.left, rect.rect.top));
	m_LGWhiteBrush->SetEndPoint(D2D1::Point2F(rect.rect.right, rect.rect.top + rect.rect.right - rect.rect.left));
	m_d2dContext->FillRoundedRectangle(rect, m_LGWhiteBrush.Get());

	wrl::ComPtr<ID2D1Image> previous;
	m_d2dContext->GetTarget(previous.GetAddressOf());
	m_d2dContext->SetTarget(m_shadowBitmap.Get());

	m_d2dContext->SetTransform(m_shadowMatrix);

	brush->SetStartPoint(D2D1::Point2F(rect.rect.left, rect.rect.top));
	brush->SetEndPoint(D2D1::Point2F(rect.rect.left, rect.rect.bottom));
	m_d2dContext->FillRoundedRectangle(rect, brush.Get());

	m_d2dContext->SetTransform(m_Matrix);

	m_d2dContext->SetTarget(previous.Get());

	if (!draw_header) return;

//	rect.rect.bottom = rect.rect.top + BlockGraphics::HEAD_HEIGHT;
	D2D1_POINT_2F center;
//	rect.rect.left += 10;
//	rect.rect.right -= 10;
	center.x = rect.rect.left +(rect.rect.right - rect.rect.left) / 2.0f;
	center.y = rect.rect.top + constants::block::HEAD_HEIGHT / 2.0f;
	m_gradientBlockHeaderBrush[index]->SetCenter(center);
	m_d2dContext->FillRoundedRectangle(rect, m_gradientBlockHeaderBrush[index].Get());
}

void CGraphics::DrawBlockGeometry(const D2D1_RECT_F& rect, ID2D1GeometryRealization* geometry, BlockColor colorIndex) {
	auto& brush = m_gradientBlockBrush[static_cast<size_t>(colorIndex)];

	brush->SetStartPoint(D2D1::Point2F(rect.left, rect.top));
	brush->SetEndPoint(D2D1::Point2F(rect.right, rect.bottom));
	m_d2dContext->DrawGeometryRealization(geometry, brush.Get());


	wrl::ComPtr<ID2D1Image> previous;
	m_d2dContext->GetTarget(previous.GetAddressOf());
	m_d2dContext->SetTarget(m_shadowBitmap.Get());

	m_d2dContext->SetTransform(m_shadowMatrix);

	brush->SetStartPoint(D2D1::Point2F(rect.left, rect.top));
	brush->SetEndPoint(D2D1::Point2F(rect.right, rect.bottom));
	m_d2dContext->DrawGeometryRealization(geometry, brush.Get());

	m_d2dContext->SetTransform(m_Matrix);

	m_d2dContext->SetTarget(previous.Get());
}

void CGraphics::DrawShadowGeometryRealization(ID2D1GeometryRealization* geometryRealization, SolidColor color) {
	// draw geometry
	m_d2dContext->DrawGeometryRealization(geometryRealization, m_brushes[static_cast<size_t>(color)].Get());
	wrl::ComPtr<ID2D1Image> previous;
	m_d2dContext->GetTarget(previous.GetAddressOf());
	m_d2dContext->SetTarget(m_shadowBitmap.Get());
	m_d2dContext->SetTransform(m_shadowMatrix);
	// draw geometry shadow
	m_d2dContext->DrawGeometryRealization(geometryRealization, m_brushes[static_cast<size_t>(color)].Get());
	// restore previous render target
	m_d2dContext->SetTransform(m_Matrix);
	m_d2dContext->SetTarget(previous.Get());
}

void CGraphics::FillBlock_2(const D2D1_ROUNDED_RECT& rect) {
	m_LGWhiteBrush->SetStartPoint(D2D1::Point2F(rect.rect.left, rect.rect.top));
	m_LGWhiteBrush->SetEndPoint(D2D1::Point2F(rect.rect.left, rect.rect.bottom));
	m_d2dContext->FillRoundedRectangle(rect, m_LGWhiteBrush.Get());
}

void CGraphics::FillEllipse(const D2D1_ELLIPSE& ellipse, SolidColor color) {
	m_d2dContext->FillEllipse(ellipse, m_brushes[static_cast<size_t>(color)].Get());
}
void CGraphics::FillSlot(const D2D1_ELLIPSE& ellipse) {
	m_RGBrushSl1->SetCenter(ellipse.point);
	m_d2dContext->FillEllipse(ellipse, m_RGBrushSl1.Get());
}
void CGraphics::FillSlotConnected(const D2D1_ELLIPSE& ellipse) {
	m_RGBrushSlotConnected->SetCenter(ellipse.point);
	m_d2dContext->FillEllipse(ellipse, m_RGBrushSlotConnected.Get());
}
void CGraphics::FillSlotFocused(const D2D1_ELLIPSE& ellipse) {
	m_RGBrushSl3->SetCenter(ellipse.point);
	m_d2dContext->FillEllipse(ellipse, m_RGBrushSl3.Get());
}
void CGraphics::FillSlotActivated(const D2D1_ELLIPSE& ellipse) {
	m_RGBrushSlotActive->SetCenter(ellipse.point);
	m_d2dContext->FillEllipse(ellipse, m_RGBrushSlotActive.Get());
}
void CGraphics::DrawOrigin() {
	m_d2dContext->DrawLine(D2D1::Point2F(-5, 0), D2D1::Point2F(5, 0), m_brushes[static_cast<size_t>(SolidColor::Black)].Get());
	m_d2dContext->DrawLine(D2D1::Point2F(0, -5), D2D1::Point2F(0, 5), m_brushes[static_cast<size_t>(SolidColor::Black)].Get());
}

void CGraphics::BeginDraw() {
	m_d2dContext->BeginDraw();
}

void CGraphics::BeginNewBufferedDraw() {
	m_d2dContext->BeginDraw();
	m_d2dContext->Clear(D2D1::ColorF(.9f, .9f, .9f));
	DrawOrigin();

	m_d2dContext->GetTarget(&m_surface);
	
	m_d2dContext->SetTarget(m_shadowBitmap.Get());
	m_d2dContext->Clear();

	m_d2dContext->SetTarget(m_bitmap.Get());
	m_d2dContext->Clear();
}
void CGraphics::RestoreSurface() {
	m_d2dContext->SetTarget(m_surface);
	m_surface->Release();
	m_d2dContext->SetTransform(D2D1::IdentityMatrix());
}
void CGraphics::EndDraw() {
//	m_saturationEffect->SetInput(0, m_bitmap.Get());
//	m_saturationEffect->SetValue(D2D1_SATURATION_PROP_SATURATION, 0.0f);
	m_d2dContext->SetTransform(m_Matrix);
	m_d2dContext->EndDraw();
	m_swapChain->Present(1, 0);
}
void CGraphics::DrawMainBuffer() {
	m_d2dContext->DrawImage(m_bitmap.Get());
}
void CGraphics::DrawShadow() {
	m_d2dContext->DrawImage(m_shadow.Get());
}

void CGraphics::PostEffectBlur() {
	m_gaussianBlurEffect->SetInput(0, m_bitmap.Get());
	m_gaussianBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 3.0f);
	m_d2dContext->DrawImage(m_gaussianBlurEffect.Get());
}

void dis::CreateCircle() {
	constexpr auto r5 = CGraphics::wheel_r;
	constexpr auto r4 = r5;// *0.9f;
	constexpr auto r3 = 60.f;
	constexpr auto r2 = 40.f;
	constexpr auto r1 = 20.f;
	constexpr auto r0 = 10.f;

	auto to_cart = [](float alpha, float r) {
		float rad = (alpha - 90.0f) * PI / 180.0f;
		return D2D1::MyPoint2F(r * cos(rad), r * sin(rad));
	};

	auto draw_arc = [&](int _a, float r, ID2D1GeometrySink* sink) {
		float a = static_cast<float>(_a);
		sink->AddArc(D2D1::ArcSegment(to_cart(a, r), D2D1::SizeF(r, r), 0, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
	};

	auto draw_arc_2 = [&](int _a, float r, ID2D1GeometrySink* sink) {
		float a = static_cast<float>(_a);
		sink->AddArc(D2D1::ArcSegment(to_cart(a, r), D2D1::SizeF(r, r), 0, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
	};

	auto draw_line = [&](int _a, float r, ID2D1GeometrySink* sink) {
		float a = static_cast<float>(_a);
		sink->AddLine(to_cart(a, r));
	};

	d2dFactory->CreateEllipseGeometry(
		D2D1::Ellipse(D2D1::Point2F(), r5, r5), back.GetAddressOf());

	d2dFactory->CreatePathGeometry(circle.GetAddressOf());
	d2dFactory->CreatePathGeometry(circle_i1.GetAddressOf());
	d2dFactory->CreatePathGeometry(circle_i2.GetAddressOf());
	
	auto draw_sector = [&](int _a1, int _a2, float r1, float r2, ID2D1GeometrySink* sink) {
		float a1 = static_cast<float>(_a1);
		float a2 = static_cast<float>(_a2);
		sink->BeginFigure(to_cart(a1, r1), D2D1_FIGURE_BEGIN_FILLED);
		sink->AddLine(to_cart(a1, r2));
		sink->AddArc(D2D1::ArcSegment(to_cart(a2, r2), D2D1::SizeF(r2, r2), 0, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
		sink->AddLine(to_cart(a2, r1));
		sink->AddArc(D2D1::ArcSegment(to_cart(a1, r1), D2D1::SizeF(r1, r1), 0, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
	};

	{
		wrl::ComPtr<ID2D1PathGeometry> geometry;
		d2dFactory->CreatePathGeometry(geometry.GetAddressOf());

		wrl::ComPtr<ID2D1GeometrySink> sink;

		geometry->Open(sink.GetAddressOf());

		sink->BeginFigure(to_cart(0, r4), D2D1_FIGURE_BEGIN_FILLED);
		draw_arc(60, r4, sink.Get());

		draw_line(60, r3, sink.Get());
		draw_arc(140, r3, sink.Get());

		draw_line(140, r2, sink.Get());
		draw_arc(200, r2, sink.Get());

		draw_line(200, r3, sink.Get());
		draw_arc(240, r3, sink.Get());

		draw_line(240, r4, sink.Get());
		draw_arc(300, r4, sink.Get());

		draw_line(300, r3, sink.Get());
		draw_arc(330, r3, sink.Get());

		draw_line(330, r4, sink.Get());
		draw_arc(360, r4, sink.Get());

		sink->EndFigure(D2D1_FIGURE_END_CLOSED);
		sink->Close();
		sink.Reset();

		wrl::ComPtr<ID2D1EllipseGeometry> hollow;
		d2dFactory->CreateEllipseGeometry(
			D2D1::Ellipse(D2D1::Point2F(), r0, r0), hollow.GetAddressOf());

		circle->Open(sink.GetAddressOf());
		geometry->CombineWithGeometry(hollow.Get(), D2D1_COMBINE_MODE_XOR, NULL, NULL, sink.Get());
		sink->Close();
	}
	{
		wrl::ComPtr<ID2D1GeometrySink> sink;

		circle_i1->Open(sink.GetAddressOf());

	//	draw_sector(0, 60, r3, r4 * 0.9f, sink.Get());
	//	sink->EndFigure(D2D1_FIGURE_END_OPEN);
	//	draw_sector(120, 180, r3, r4 * 0.9f, sink.Get());
	//	sink->EndFigure(D2D1_FIGURE_END_OPEN);
	//	draw_sector(240, 300, r3, r4 * 0.9f, sink.Get());
	//	sink->EndFigure(D2D1_FIGURE_END_OPEN);

	//	draw_sector(90, 130, r2, r3, sink.Get());
	//	sink->EndFigure(D2D1_FIGURE_END_OPEN);

	//	draw_sector(200, 280, r2, r3, sink.Get());
	//	sink->EndFigure(D2D1_FIGURE_END_OPEN);


		draw_sector(60, 120, r1, r2, sink.Get());
		sink->EndFigure(D2D1_FIGURE_END_OPEN);
		draw_sector(0, 180, r0, r1, sink.Get());
		sink->EndFigure(D2D1_FIGURE_END_OPEN);

		draw_sector(180, 240, r1, r2, sink.Get());
		sink->EndFigure(D2D1_FIGURE_END_OPEN);
		draw_sector(150, 280, r0, r1, sink.Get());
		sink->EndFigure(D2D1_FIGURE_END_CLOSED);
		
		//

		sink->Close();
	}
	{
		wrl::ComPtr<ID2D1GeometrySink> sink;

		circle_i2->Open(sink.GetAddressOf());

		draw_sector(20, 80, r2, r3, sink.Get());
		sink->EndFigure(D2D1_FIGURE_END_OPEN);

		draw_sector(0, 140, r3, r4, sink.Get());
		sink->EndFigure(D2D1_FIGURE_END_OPEN);

	//	draw_sector(240, 300, r2, r3, sink.Get());
	//	sink->EndFigure(D2D1_FIGURE_END_OPEN);

		//	draw_sector(90, 130, r2, r3, sink.Get());
		//	sink->EndFigure(D2D1_FIGURE_END_OPEN);

		//	draw_sector(200, 280, r2, r3, sink.Get());
		//	sink->EndFigure(D2D1_FIGURE_END_OPEN);

		sink->Close();
	}
}

void dis::CreateSliderThing() {
	wrl::ComPtr<ID2D1GeometrySink> sink;
	d2dFactory->CreatePathGeometry(slider_thing.GetAddressOf());
	slider_thing->Open(sink.GetAddressOf());

	sink->BeginFigure({0, 0}, D2D1_FIGURE_BEGIN_FILLED);
	
	sink->AddLine({ constants::slider::thing::width, 0 });
	sink->AddLine({ constants::slider::thing::width, constants::slider::thing::h1 });
	sink->AddLine({ constants::slider::thing::width / 2 + 0.35f, constants::slider::thing::height });
	sink->AddLine({ constants::slider::thing::width / 2 - 0.35f, constants::slider::thing::height });
	sink->AddLine({ 0, constants::slider::thing::h1 });
	sink->AddLine({ 0, 0 });

	sink->EndFigure(D2D1_FIGURE_END_CLOSED);

	sink->Close();
}

void dis::CreateSliderTimeChart() {
	wrl::ComPtr<ID2D1GeometrySink> sink;
	d2dFactory->CreatePathGeometry(slider_time_chart.GetAddressOf());
	slider_time_chart->Open(sink.GetAddressOf());

	size_t cnt = 0;
	for (auto x = 0.0f; 
		x < constants::slider::SCHEDULE_BLOCK_TIME_CHART_WIDTH + 0.0001f; 
		x += constants::slider::SLIDER_STEP, ++cnt) {
		sink->BeginFigure({ x, cnt % 4 != 0 ? 8.0f : 0.0f }, D2D1_FIGURE_BEGIN_HOLLOW);
		sink->AddLine({ x, 16.0f });
		sink->EndFigure(D2D1_FIGURE_END_OPEN);
	}

	sink->Close();
}

void dis::CreateSizeTool() {
	wrl::ComPtr<ID2D1GeometrySink> sink;
	d2dFactory->CreatePathGeometry(size_tool.GetAddressOf());
	size_tool->Open(sink.GetAddressOf());

	constexpr auto spacer = constants::size_tool_spacer;

	// top triangle
	sink->BeginFigure({0, 0}, D2D1_FIGURE_BEGIN_FILLED);
	sink->AddLine({ constants::size_tool_size - spacer, 0 });
	sink->AddLine({ 0, constants::size_tool_size - spacer });
	sink->AddLine({ 0, 0 });
	sink->EndFigure(D2D1_FIGURE_END_OPEN);

	// bottom triangle
	sink->BeginFigure({spacer, constants::size_tool_size}, D2D1_FIGURE_BEGIN_FILLED);
	sink->AddLine({ constants::size_tool_size, constants::size_tool_size });
	sink->AddLine({ constants::size_tool_size, spacer });
	sink->AddLine({spacer, constants::size_tool_size});
	sink->EndFigure(D2D1_FIGURE_END_CLOSED);

	sink->Close();
}

wrl::ComPtr<ID2D1PathGeometry> dis::CreateSFCBeginBlock(const D2D1::MySize2F& size) const {
	wrl::ComPtr<ID2D1PathGeometry> geometry;
	wrl::ComPtr<ID2D1GeometrySink> sink;

	d2dFactory->CreatePathGeometry(geometry.GetAddressOf());
	geometry->Open(sink.GetAddressOf());

	constexpr auto corner_radius = constants::block::RADIUS_X2;
	constexpr auto slot_radius = constants::block::SFC_SLOT_CIRCLE_RADIUS;
	
	const auto width = size.width;
	const auto heigth = size.height;
	const auto slot_arc_x1 = (width / 2.0f) - slot_radius;
	const auto slot_arc_x2 = (width / 2.0f) + slot_radius;

	sink->BeginFigure({0, corner_radius}, D2D1_FIGURE_BEGIN_FILLED);
	sink->AddArc(D2D1::ArcSegment({corner_radius, 0}, {corner_radius, corner_radius}, 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
	sink->AddLine({width - corner_radius, 0 });
	sink->AddArc(D2D1::ArcSegment({width, corner_radius}, {corner_radius, corner_radius}, 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
	sink->AddLine({width, heigth - corner_radius - slot_radius });
	sink->AddArc(D2D1::ArcSegment({width - corner_radius, heigth - slot_radius}, {corner_radius, corner_radius}, 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
	sink->AddLine({slot_arc_x2, heigth-slot_radius });
	sink->AddArc(D2D1::ArcSegment({slot_arc_x1, heigth - slot_radius}, {slot_radius, slot_radius}, 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
	sink->AddLine({corner_radius, heigth - slot_radius });
	sink->AddArc(D2D1::ArcSegment({0, heigth - corner_radius - slot_radius}, {corner_radius, corner_radius}, 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
	sink->AddLine({ 0, corner_radius });
	sink->EndFigure(D2D1_FIGURE_END_CLOSED);

	sink->Close();

	return geometry;
}

wrl::ComPtr<ID2D1PathGeometry> dis::CreateSFCTermBlock(const D2D1::MySize2F& size) const {
	wrl::ComPtr<ID2D1PathGeometry> geometry;
	wrl::ComPtr<ID2D1GeometrySink> sink;

	d2dFactory->CreatePathGeometry(geometry.GetAddressOf());
	geometry->Open(sink.GetAddressOf());

	constexpr auto corner_radius = constants::block::RADIUS_X2;
	constexpr auto slot_radius = constants::block::SFC_SLOT_CIRCLE_RADIUS;
	
	const auto width = size.width;
	const auto heigth = size.height;
	const auto slot_arc_x1 = (width / 2.0f) - slot_radius;
	const auto slot_arc_x2 = (width / 2.0f) + slot_radius;
	const auto width2 = size.width / 2.0f;

	const auto xh = (width - width2) / 2.0f;

	const auto x2 = xh;
	const auto x3 = x2 + width2;

	sink->BeginFigure({0, corner_radius + slot_radius}, D2D1_FIGURE_BEGIN_FILLED);
	sink->AddArc(D2D1::ArcSegment({corner_radius, slot_radius}, {corner_radius, corner_radius}, 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
	sink->AddLine({slot_arc_x1, slot_radius });
	sink->AddArc(D2D1::ArcSegment({slot_arc_x2, slot_radius}, {slot_radius, slot_radius}, 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
	sink->AddLine({width - corner_radius, slot_radius });
	sink->AddArc(D2D1::ArcSegment({width, slot_radius + corner_radius}, {corner_radius, corner_radius}, 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
	
	sink->AddLine({x3, heigth - 3.0f });
	sink->AddArc(D2D1::ArcSegment({x3 - 3.0f, heigth}, {3.0f, 4.0f}, 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
	sink->AddLine({x2+3.0f, heigth });
	sink->AddArc(D2D1::ArcSegment({x2, heigth - 3.0f}, {3.0f, 4.0f}, 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
	sink->AddLine({ 0, corner_radius + slot_radius });
	sink->EndFigure(D2D1_FIGURE_END_CLOSED);

	sink->Close();

	return geometry;
}

wrl::ComPtr<ID2D1PathGeometry> dis::CreateSFCStepBlock(const D2D1::MySize2F& size) const {
	wrl::ComPtr<ID2D1PathGeometry> geometry;
	wrl::ComPtr<ID2D1GeometrySink> sink;

	d2dFactory->CreatePathGeometry(geometry.GetAddressOf());
	geometry->Open(sink.GetAddressOf());

	constexpr auto corner_radius = constants::block::RADIUS_X2;
	constexpr auto slot_radius = constants::block::SFC_SLOT_CIRCLE_RADIUS;
	
	const auto width = size.width;
	const auto heigth = size.height - slot_radius * 1.0f;
	const auto slot_arc_x1 = (width / 2.0f) - slot_radius;
	const auto slot_arc_x2 = (width / 2.0f) + slot_radius;

	sink->BeginFigure({0, corner_radius + slot_radius}, D2D1_FIGURE_BEGIN_FILLED);
	sink->AddArc(D2D1::ArcSegment({corner_radius, slot_radius}, {corner_radius, corner_radius}, 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
	sink->AddLine({slot_arc_x1, slot_radius });
	sink->AddArc(D2D1::ArcSegment({slot_arc_x2, slot_radius}, {slot_radius, slot_radius}, 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
	sink->AddLine({width - corner_radius, slot_radius });
	sink->AddArc(D2D1::ArcSegment({width, corner_radius + slot_radius}, {corner_radius, corner_radius}, 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
	sink->AddLine({width, heigth - corner_radius });
	sink->AddArc(D2D1::ArcSegment({width - corner_radius, heigth}, {corner_radius, corner_radius}, 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
	sink->AddLine({slot_arc_x2, heigth });
	sink->AddArc(D2D1::ArcSegment({slot_arc_x1, heigth}, {slot_radius, slot_radius}, 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
	sink->AddLine({corner_radius, heigth });
	sink->AddArc(D2D1::ArcSegment({0, heigth - corner_radius}, {corner_radius, corner_radius}, 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
	sink->AddLine({ 0, corner_radius + slot_radius });
	sink->EndFigure(D2D1_FIGURE_END_CLOSED);

	sink->Close();

	return geometry;
}

void dis::CreateSlot() {
	d2dFactory->CreateRoundedRectangleGeometry(D2D1_ROUNDED_RECT{
		D2D1_RECT_F{0, 0, constants::block::SLOT_SIZE, constants::block::SLOT_SIZE},
			constants::block::SLOT_SIZE,
			constants::block::SLOT_SIZE
	}, (ID2D1RoundedRectangleGeometry**)geometry_slot.GetAddressOf());
}

void dis::CreateGeometries() {
	CreateSlot();
	CreateCircle();
	CreateSliderTimeChart();
	CreateSliderThing();
	CreateSizeTool();
	
	sfc_begin_block = CreateSFCBeginBlock(constants::block::SFC_BLOCK_STEP_SIZE);
	sfc_term_block = CreateSFCTermBlock(constants::block::SFC_BLOCK_TERM_SIZE);
	sfc_step_block = CreateSFCStepBlock(constants::block::SFC_BLOCK_STEP_SIZE);
	sfc_transition_block = CreateSFCStepBlock(constants::block::SFC_BLOCK_TRANSITION_SIZE);
}

void CGraphics::DrawWaitCircle(float angle) {
//	angle = 0;
	const auto& r = dis::Get();

	D2D1::MyPoint2F center(m_dimArea.width - wheel_r, 
		m_dimArea.height - wheel_r);

	wrl::ComPtr<ID2D1TransformedGeometry> transformed_back;
	wrl::ComPtr<ID2D1TransformedGeometry> transformed;
	wrl::ComPtr<ID2D1TransformedGeometry> transformed_i_1;

	{
		D2D1::Matrix3x2F m = D2D1::Matrix3x2F::Translation(center.x, center.y);
		r.d2dFactory->CreateTransformedGeometry(r.back.Get(),
			m, transformed_back.GetAddressOf());
	}
	{
		D2D1::Matrix3x2F m = D2D1::Matrix3x2F::Rotation(angle) * D2D1::Matrix3x2F::Translation(center.x, center.y);
		r.d2dFactory->CreateTransformedGeometry(r.circle.Get(),
			m, transformed.GetAddressOf());
	}
	{
		D2D1::Matrix3x2F m = D2D1::Matrix3x2F::Rotation(-angle) * D2D1::Matrix3x2F::Translation(center.x, center.y);
		r.d2dFactory->CreateTransformedGeometry(r.circle_i1.Get(),
			m, transformed_i_1.GetAddressOf());
	}

	m_RGBrush1->SetCenter(center);

//	m_d2dContext->FillGeometry(transformed_back.Get(), m_RGBrush1.Get());
	m_d2dContext->FillGeometry(transformed.Get(), m_RGBrush1.Get());
	m_d2dContext->FillGeometry(transformed_i_1.Get(), m_RGBrush1.Get());
}
