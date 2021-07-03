#include "stdafx.h"
#include "ScintillaView.h"
/*
#define IDC_SCINTILLA 200

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CScintillaView

IMPLEMENT_DYNCREATE(CScintillaView, CView)

BEGIN_MESSAGE_MAP(CScintillaView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CScintillaView::OnFilePrintPreview)
	ON_EN_CHANGE(IDC_SCINTILLA, &CScintillaView::TextWasChanged)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_CREATE()
	ON_WM_SIZE()

END_MESSAGE_MAP()

// CScintillaView construction/destruction

CScintillaView::CScintillaView() {
	// TODO: add construction code here
}

CScintillaView::~CScintillaView() {
	
}
// CScintillaView drawing

void CScintillaView::OnDraw(CDC*) {
	CDocument* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	// TODO: add draw code for native data here
}

// CScintillaView printing

void CScintillaView::OnFilePrintPreview() {
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CScintillaView::OnPreparePrinting(CPrintInfo* pInfo) {
	// default preparation
	return DoPreparePrinting(pInfo);
}


void CScintillaView::OnRButtonUp(UINT , CPoint point) {
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CScintillaView::OnContextMenu(CWnd* , CPoint point) {
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}
// CScintillaView diagnostics
#ifdef _DEBUG
void CScintillaView::AssertValid() const {
	CView::AssertValid();
}

void CScintillaView::Dump(CDumpContext& dc) const {
	CView::Dump(dc);
}

CDocument* CScintillaView::GetDocument() const // non-debug version is inline
{
	return m_pDocument;
}
#endif //_DEBUG

// CScintillaView message handlers
int CScintillaView::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	// TODO:  Add your specialized creation code here
	scWnd.Create(m_hWnd, IDC_SCINTILLA);
	
	CString stExt = GetDocument()->GetTitle();
	stExt.Delete(0, stExt.ReverseFind('.'));
	
	if (stExt == ".asm" || stExt == ".S")
		scWnd.SetLexer(CScintillaWindow::lexer::ASM);
	else if (stExt == ".c")
		scWnd.SetLexer(CScintillaWindow::lexer::C);
	else
		scWnd.SetLexer(CScintillaWindow::lexer::NONE);

	return 0;
}
void CScintillaView::OnSize(UINT nType, int cx, int cy) {
	CView::OnSize(nType, cx, cy);
	// TODO: Add your message handler code here
	scWnd.OnSize(nType, 0, 0, cx, cy);
}
BOOL CScintillaView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) {
	// TODO: Add your specialized code here and/or call the base class
	if (scWnd.OnNotify(wParam,  lParam, pResult)) {
		return TRUE;
	}
	return CView::OnNotify(wParam, lParam, pResult);
}
void CScintillaView::TextWasChanged() {
	scWnd.TextWasChanged();
}
BOOL CScintillaView::PreCreateWindow(CREATESTRUCT& cs) {
	cs.style &= ~WS_BORDER;
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;

	return CView::PreCreateWindow(cs);
}

*/