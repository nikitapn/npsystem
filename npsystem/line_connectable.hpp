#pragma once

template<typename TLine>
class CLineConnectableT {
public:
	virtual bool HitTestWhileConnectingLine(const D2D1_POINT_2F& pt, UINT& flags) noexcept = 0;
	virtual bool ConnectToLine(TLine* line, bool checking) = 0;
};