// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "dlgloadingmanager.h"

extern CProgressBarCtrl* g_progress;

LRESULT CDlg_LoadingManager::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	CenterWindow(GetParent());

	m_buttonOk.SubclassWindow(GetDlgItem(IDOK));
	m_output.SubclassWindow(GetDlgItem(IDC_CUSTOM1));
	m_progress.Attach(GetDlgItem(IDC_PROGRESS1));
	
	m_output.SetReadOnly();
	m_output.SetBackgroundColor(RGB(235, 235, 235));
	
	old_cout_ = std::cout.rdbuf();
	old_cerr_ = std::cerr.rdbuf();
	std::cout.set_rdbuf(&m_output);
	std::cerr.set_rdbuf(&m_output);
	standart_progress_ = g_progress;
	g_progress = &m_progress;

	std::thread(&CTask::Exec, task_.get(), this).detach();

	return 0;
}

BOOL CDlg_LoadingManager::EndDialog(_In_ int nRetCode) {
	std::cout.set_rdbuf(old_cout_);
	std::cerr.set_rdbuf(old_cerr_);
	g_progress = standart_progress_;
	return base::EndDialog(nRetCode);
}

LRESULT CDlg_LoadingManager::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (m_buttonOk.IsWindowEnabled()) EndDialog(wID);
	return 0;
}

LRESULT CDlg_LoadingManager::OnOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	EndDialog(wID);
	return 0;
}

#include "avrconfigurator.h"
#include "network_ext.h"
#include "algext.h"
#include "module.h"

static void DynamicTranslateResult(CDlg_LoadingManager* dlg, CDynamicLinker::Result code) noexcept {
	switch (code)
	{
	case CDynamicLinker::R_CRITICAL_ERROR:
		dlg->m_progress.SetPos(0);
		std::cout << ">Critical error. Operation has been interrupted." << std::endl;
		break;
	case CDynamicLinker::R_ERROR:
		dlg->m_progress.SetPos(100);
		std::cout << ">Operation has been completed with errors." << std::endl;
		break;
	case CDynamicLinker::E_CANCELED:
		dlg->m_progress.SetPos(0);
		std::cout << ">Operation was canceled---" << std::endl;
		break;
	case CDynamicLinker::R_OK:
		dlg->m_progress.SetPos(100);
		std::cout << ">Operation has been completed." << std::endl;
		break;
	default:
		std::cout << ">Unknown result code." << std::endl;
		ASSERT(FALSE);
		break;
	}

	dlg->m_buttonOk.EnableWindow();
}

void CDlg_LoadingManager::CTaskLoadOneAlgorithm::Exec(CDlg_LoadingManager* dlg) noexcept {
	std::cout << ">---Upload started---"
		<< "\n>\tdevice: \"" << dev_->get_name() << "\""
		<< "\n>\ttarget: \"" << alg_->get_name() << '\"'
		<< std::endl;
	auto linker = dev_->CreateLinker();
	auto result = linker->UploadAlgorithm(alg_);
	if (result == CDynamicLinker::Result::R_OK) {
		dev_->item->CalcAndUpdateHardwareStatus();
		dev_->item->m_tabview->Invalidate();
	}
	DynamicTranslateResult(dlg, result);
}

void CDlg_LoadingManager::CTaskUnloadOneAlgorithm::Exec(CDlg_LoadingManager* dlg) noexcept {
	std::cout << ">---Unload started---"
		<< "\n>\tdevice: \"" << dev_->get_name() << "\""
		<< "\n>\ttarget: \"" << alg_->get_name() << '\"'
		<< std::endl;
	auto linker = dev_->CreateLinker();
	auto result = linker->UnloadAlgorithm(alg_);
	if (result == CDynamicLinker::Result::R_OK) {
		dev_->item->CalcAndUpdateHardwareStatus();
		dev_->item->m_tabview->Invalidate();
	}
	DynamicTranslateResult(dlg, result);
}

void CDlg_LoadingManager::CTaskUploadOneI2CModule::Exec(CDlg_LoadingManager* dlg) noexcept {
	std::cout << ">---Unload started---"
		<< "\n>\tdevice: \"" << ctrl_->get_name() << "\""
		<< "\n>\ttarget: \"" << mod_->get_name() << '\"'
		<< std::endl;
	auto linker = ctrl_->CreateLinker();
	auto result = linker->UploadI2CModule(mod_);
	if (result == CDynamicLinker::Result::R_OK) {
		ctrl_->item->CalcAndUpdateHardwareStatus();
		ctrl_->item->m_tabview->Invalidate();
	}
	DynamicTranslateResult(dlg, result);
}
void CDlg_LoadingManager::CTaskUploadControllerFirmware::Exec(CDlg_LoadingManager* dlg) noexcept {
	std::cout << ">---Unload started---"
		<< "\n>\tdevice: \"" << ctrl_->get_name() << "\""
		<< std::endl;
	if (ctrl_->LoadFirmware()) {
		dlg->m_progress.SetPos(100); 
		std::cout << "Firmware has been uploaded." << std::endl;
	} else {
		dlg->m_progress.SetPos(0);
		std::cout << "Error: Firmware upload failed." << std::endl;
	}
	dlg->m_buttonOk.EnableWindow();
}
