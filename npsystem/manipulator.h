// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

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
	D2D1::MyPoint2F _offset;
	D2D1::MyPoint2F _oldpos;
	D2D1::MyPoint2F _prevpos;
	int drag;
};