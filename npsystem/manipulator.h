#pragma once

class CAlgorithmView;
class CGraphics;

enum S_CURSOR {
	A_NOT_SELECTED,
	A_ONE_ELEM,
	A_GROUP,
	A_LINE,
	A_RECT,
	A_SELECTED,
};

class CManipulator {
public:
	virtual ~CManipulator() = default;
	virtual S_CURSOR MouseButtonDown(CPoint point, CAlgorithmView* pWnd) = 0;
	virtual void Drag(CPoint point, CAlgorithmView* pWnd) = 0;
	virtual void MouseButtonUP(CPoint point, CAlgorithmView* pWnd) = 0;
	virtual void Draw(CGraphics* pGraphics) {}
protected:
	D2D1_POINT_2F _p1;
};

class CMultiManipulator : public CManipulator {
public:
	virtual S_CURSOR MouseButtonDown(CPoint point, CAlgorithmView* pWnd) = 0;
	virtual void Drag(CPoint point, CAlgorithmView* pWnd) = 0;
	virtual void MouseButtonUP(CPoint point, CAlgorithmView* pWnd) = 0;
	virtual void Draw(CGraphics* pGraphics) = 0;
protected:
	/*Смещение от левого верхнего угла блока*/
	D2D1::MyPoint2F _offset;
	D2D1::MyPoint2F _oldpos;
	D2D1::MyPoint2F _prevpos;
	int drag;
};