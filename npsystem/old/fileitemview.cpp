#include "stdafx.h"
#include "ScintillaView.h"
#include "FileItemView.h"
#include "FilesContainer.h"

/*

IMPLEMENT_DYNCREATE(CFileItemView,CScintillaView)

BEGIN_MESSAGE_MAP(CFileItemView, CScintillaView)
	ON_WM_CREATE()
	ON_COMMAND(ID_FILE_SAVE, &OnFileSave)
END_MESSAGE_MAP()


int CFileItemView::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CScintillaView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	CItemDocument* pDoc = static_cast<CItemDocument*>(GetDocument());
	ASSERT_KINDOF(CItemDocument, pDoc);
	pDoc->GetItem()->SetWindow(this);
	
	m_pFileItem = (CFileItem*)pDoc->GetItem();
	char* lpszText = m_pFileItem->OpenFile();
	scWnd.SetText(lpszText);
	delete []lpszText;

	return 0;
}
void CFileItemView::OnFileSave() {
	Save();
}
void CFileItemView::Save() {
	char* lpszText = scWnd.GetText();
	m_pFileItem->SaveFile(lpszText, scWnd.GetLenght() - 1);
	delete[] lpszText;
}
*/