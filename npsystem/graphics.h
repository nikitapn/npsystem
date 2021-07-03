#pragma once

#include "Color.h"
#include <array>

class dis {
	dis();
	dis(const dis&) = delete;
	dis& operator = (const dis&) = delete;
	void CreateCircle();
	void CreateSliderThing();
	void CreateSliderTimeChart();
public:
	~dis();

	static const dis& Get() {
		static dis m_instance;
		return m_instance; 
	}

	ID2D1Factory2* d2dFactory;
	IDWriteFactory* pDWriteFactory;

	std::array<IDWriteTextFormat*, 2> text_centered;

	IDWriteTextFormat* pTextFormatLeft;
	IDWriteTextFormat* pTextFormatRight;

	IDWriteTextFormat* pTextOnline;
	IDWriteTextFormat* pTextFormatSlotLeft;
	IDWriteTextFormat* pTextFormatSlotRight;
	
	wrl::ComPtr<ID2D1EllipseGeometry> back;
	wrl::ComPtr<ID2D1PathGeometry> circle;
	wrl::ComPtr<ID2D1PathGeometry> circle_i1;
	wrl::ComPtr<ID2D1PathGeometry> circle_i2;
	wrl::ComPtr<ID2D1PathGeometry> slider_thing;
	wrl::ComPtr<ID2D1PathGeometry> slider_time_chart;
	wrl::ComPtr<IDWriteTextLayout> text_layout_unknown;
	wrl::ComPtr<IDWriteTextLayout> text_layout_bad_quality;
	
};

class CGraphics {
	HWND m_hwnd;

	static constexpr size_t nSolidBrushes = 20;

	wrl::ComPtr<ID2D1SolidColorBrush> m_brushes[nSolidBrushes];
	wrl::ComPtr<ID2D1LinearGradientBrush> m_gradientBlockBrushParameter[2];
	
	wrl::ComPtr<ID2D1RadialGradientBrush> m_gradientBlockHeaderBrush[BlockColorCount];

	wrl::ComPtr<ID2D1RadialGradientBrush> m_RGBrush1;
	wrl::ComPtr<ID2D1LinearGradientBrush> m_LGWhiteBrush;
	wrl::ComPtr<ID2D1RadialGradientBrush> m_RGBrushSl1;
	wrl::ComPtr<ID2D1RadialGradientBrush> m_RGBrushSlotConnected;
	wrl::ComPtr<ID2D1RadialGradientBrush> m_RGBrushSl3;
	wrl::ComPtr<ID2D1RadialGradientBrush> m_RGBrushSlotActive;

	float m_lineX;
	float m_lineY;

	D2D1::Matrix3x2F m_Matrix;
	D2D1::Matrix3x2F m_shadowMatrix;
	D2D1::Matrix3x2F m_invMatrix;
	D2D1_POINT_2F m_ptCenter;
	D2D1_SIZE_F m_dimArea;
	struct {
		bool center : 1;
	} m_dirty;
	union 
	{
		struct
		{
			D2D1_POINT_2F m_ptArea_1;
			D2D1_POINT_2F m_ptArea_2;
		};
		D2D1_RECT_F m_rectArea;
	};
	bool m_bGridEnabled = true;
	float m_dpi_x = 96.0f;
	float m_dpi_y = 96.0f;
	float m_flatteningTolerance;
	bool m_editingMode = false;
	// Windows 10
	wrl::ComPtr<ID3D11Device> m_d3dDevice;
	wrl::ComPtr<ID3D11DeviceContext> m_d3dContext;
	//
	wrl::ComPtr<ID2D1Device1> m_d2dDevice;
	
	wrl::ComPtr<IDXGISwapChain1> m_swapChain;
	ID2D1Image* m_surface;
	wrl::ComPtr<ID2D1Bitmap1> m_bitmap;
	wrl::ComPtr<ID2D1Bitmap1> m_shadowBitmap;
	wrl::ComPtr<ID2D1Effect> m_shadow;
	wrl::ComPtr<ID2D1Effect> m_gaussianBlurEffect;
	wrl::ComPtr<ID2D1Effect> m_saturationEffect;

	static D3D_FEATURE_LEVEL m_featureLevels[];
	D3D_FEATURE_LEVEL m_featureLevel;
	
	void CreateDeviceSwapChainBitmap();
	void CreateDeviceSizeResourses(UINT width, UINT height);
	void CreateBrushes();
	void DrawOrigin();
	D2D1_SIZE_F m_size;
public:
	static constexpr auto wheel_r = 80.f;

	wrl::ComPtr<ID2D1DeviceContext1> m_d2dContext;
	wrl::ComPtr<ID2D1LinearGradientBrush> m_gradientBlockBrush[BlockColorCount];

	void BeginNewBufferedDraw();
	void BeginDraw();
	void RestoreSurface();
	void EndDraw();
	void DrawMainBuffer();
	void DrawShadow();
	void PostEffectBlur();
	void FillRoundRect(const D2D1_ROUNDED_RECT& rect, SolidColor color);
	void FillBlock(const D2D1_ROUNDED_RECT& rect, BlockColor colorIndex, bool draw_header = true);
	void FillBlock_2(const D2D1_ROUNDED_RECT& rect);
	void FillSlotConnected(const D2D1_ELLIPSE& ellipse);
	void FillSlotFocused(const D2D1_ELLIPSE& ellipse);
	void FillSlot(const D2D1_ELLIPSE& ellipse);
	void FillSlotActivated(const D2D1_ELLIPSE& ellipse);
	void FillEllipse(const D2D1_ELLIPSE& ellipse, SolidColor color);
	void Resize(UINT width, UINT height);
	void DrawGrid();
	void MoveOrgDxDy(float dx, float dy);
	void SetMatrix(const D2D1::Matrix3x2F& m);
	void DrawWaitCircle(float angle);
	const D2D1_POINT_2F& GetCenterOfView() noexcept;
	void SetEditingMode(bool mode) { m_editingMode = mode; }
	bool IsEditingMode() { return m_editingMode; }
	void ClearScreen(D2D1::ColorF color) {
		m_d2dContext->Clear(color);
	}
	void EnableGrid(bool bEnable) {
		m_bGridEnabled = bEnable;
	}
	const D2D1::Matrix3x2F& GetMatrix() {
		return m_Matrix;
	}
	const D2D1::Matrix3x2F& GetInverseMatrix() {
		return m_invMatrix;
	}
	const D2D1_RECT_F& GetVisibleRect() {
		return m_rectArea;
	}
	void CalcVisibleArea() {
		m_invMatrix = D2D1::Invert(m_Matrix);
		m_rectArea = { 0, 0, m_dimArea.width, m_dimArea.height };
		m_ptArea_1 = m_ptArea_1 * m_invMatrix;
		m_ptArea_2 = m_ptArea_2 * m_invMatrix;
	}
	void DrawTextLayout(const D2D1::MyPoint2F& pt, IDWriteTextLayout* pTextLayout, SolidColor color) {
		m_d2dContext->DrawTextLayout(pt, pTextLayout, m_brushes[static_cast<size_t>(color)].Get());
	}
	void MoveTo(float x, float y) {
		m_lineX = x;
		m_lineY = y;
	}
	void LineTo(float x, float y, SolidColor color, float strokeWidth = 1.0f) {
		m_d2dContext->DrawLine(D2D1::Point2F(m_lineX, m_lineY), D2D1::Point2F(x, y), m_brushes[static_cast<size_t>(color)].Get(), strokeWidth);
		m_lineX = x;
		m_lineY = y;
	}
	void DrawTextLeft(const wchar_t* str, int len, D2D1_RECT_F rc, SolidColor color) {
		m_d2dContext->DrawTextW(str, len, dis::Get().pTextFormatLeft, rc, m_brushes[static_cast<size_t>(color)].Get());
	}
	void DrawTextRight(const wchar_t* str, int len, D2D1_RECT_F rc, SolidColor color) {
		m_d2dContext->DrawTextW(str, len, dis::Get().pTextFormatRight, rc, m_brushes[static_cast<size_t>(color)].Get());
	}
	void DrawValueBk(const D2D1_ROUNDED_RECT& rc) {
		m_d2dContext->DrawRoundedRectangle(rc, m_brushes[static_cast<size_t>(SolidColor::Black)].Get());
		m_d2dContext->FillRoundedRectangle(rc, m_brushes[static_cast<size_t>(SolidColor::SlotValue)].Get());
	}
	void FillRect(const D2D1_RECT_F& rect, SolidColor color) {
		m_d2dContext->FillRectangle(rect, m_brushes[static_cast<size_t>(color)].Get());
	}
	void DrawSolidRect(float x, float y, float width, float height, SolidColor color) {
		m_d2dContext->FillRectangle(D2D1::RectF(x, y, x + width, y + height), m_brushes[static_cast<size_t>(color)].Get());
	}
	void DrawGeometry(ID2D1Geometry* pGeometry, SolidColor color, float strokeWidth = 1.0f) {
		m_d2dContext->DrawGeometry(pGeometry, m_brushes[static_cast<size_t>(color)].Get(), strokeWidth);
	}
	void FillGeometry(ID2D1Geometry* pGeometry, SolidColor color) {
		m_d2dContext->FillGeometry(pGeometry, m_brushes[static_cast<size_t>(color)].Get());
	}
	void DrawRect(const D2D1_RECT_F& rect, SolidColor color, float strokeWidth = 1.0f) {
		m_d2dContext->DrawRectangle(rect, m_brushes[static_cast<size_t>(color)].Get(), strokeWidth);
	}
	void DrawRoundRect(const D2D1_ROUNDED_RECT& rect, SolidColor color, float strokeWidth = 1.0f) {
		m_d2dContext->DrawRoundedRectangle(rect, m_brushes[static_cast<size_t>(color)].Get(), strokeWidth);
	}
	void PushClip(const D2D1_RECT_F& clipRect) {
		m_d2dContext->PushAxisAlignedClip(clipRect, D2D1_ANTIALIAS_MODE_ALIASED);
	}
	void PopClip() {
		m_d2dContext->PopAxisAlignedClip();
	}
	HRESULT CreateFilledGeometryRealization(ID2D1Geometry* geometry, ID2D1GeometryRealization** geometryRealization) {
		return m_d2dContext->CreateFilledGeometryRealization(geometry, m_flatteningTolerance, geometryRealization);
	}
	HRESULT CreateStrokedGeometryRealization(ID2D1Geometry* geometry, float strokeWidth, ID2D1GeometryRealization** geometryRealization) {
		return m_d2dContext->CreateStrokedGeometryRealization(geometry, m_flatteningTolerance, strokeWidth, NULL, geometryRealization);
	}


	void DrawGeometryRealization(ID2D1GeometryRealization* geometryRealization, SolidColor color) {
		m_d2dContext->DrawGeometryRealization(geometryRealization, m_brushes[static_cast<size_t>(color)].Get());
	}
	
	void DrawShadowGeometryRealization(ID2D1GeometryRealization* pGeometry,  SolidColor color);


	CGraphics(HWND hwnd);
	~CGraphics();
};
