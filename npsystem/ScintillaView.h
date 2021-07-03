#pragma once

#include "ScintillaWindow.h"
#include "View.h"

class CScintillaView : public CView
{
	/*
protected: // create from serialization only
	CScintillaView();
	DECLARE_DYNCREATE(CScintillaView)
	
// Attributes
public:
	CDocument* GetDocument() const;
// Operations
public:
	CScintillaWindow scWnd;
// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
// Implementation
public:
	virtual ~CScintillaView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void TextWasChanged();
	//
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	*/
};
