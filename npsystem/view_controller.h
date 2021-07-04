// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <npsys/header.h>
#include "mytabview.h"
#include "myonlinetreectrl.h"
#include "myedit.h"

class CTreeController;

class CControllerView 
	: public CViewItemDialogImpl<CControllerView> {
	using base = CViewItemDialogImpl<CControllerView>;
public:
	enum { IDD = IDD_DLG_CONTROLLER };

	BEGIN_MSG_MAP(CControllerView)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
//		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
//		MESSAGE_HANDLER(WM_SIZE, OnSize)
		COMMAND_HANDLER(IDC_BUTTON1, BN_CLICKED, OnBnClickedButtonAdvise)
		CHAIN_MSG_MAP(base)
	END_MSG_MAP()

protected:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

//	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { return TRUE; }
//	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

//  Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
	// CView
	virtual bool IsModified();
	virtual void Save();
//	virtual BOOL CanUndo();
//	virtual BOOL CanRedo();
//	virtual BOOL CanStartOnline();
//	virtual BOOL CanStopOnline();
//	virtual BOOL CanUpload();
	LRESULT OnBnClickedButtonAdvise(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
public:
	CControllerView(CTreeController* item, CMyTabView& tabview);
	CMyOnlineTreeView tree_;
	CListViewCtrl m_listLibs;
	CMyEdit m_editModel;
	CMyEdit m_editAddress;
	CMyEdit m_editVersion;
	CButton m_cbAdvise;
};

