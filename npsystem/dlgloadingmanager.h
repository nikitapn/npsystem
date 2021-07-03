#pragma once

#include "mybutton.h"
#include "dlgbase.h"
#include <npsys/header.h>
#include "outputwnd.h"

class CDlg_LoadingManager 
	: public CMyDlgBase<CDlg_LoadingManager>
{
	using base = CMyDlgBase<CDlg_LoadingManager>;
	enum TARGER { ALL, ALGORITHM, MODULE, IO };
public:
	class CTask {
	public:
		virtual ~CTask() = default;
		virtual void Exec(CDlg_LoadingManager* dlg) noexcept = 0;
	};

	class CTaskLoadOneAlgorithm : public CTask {
	public:
		CTaskLoadOneAlgorithm(npsys::device_n& dev, npsys::algorithm_n& alg)
			: dev_(dev)
			, alg_(alg) 
		{
		}
		virtual void Exec(CDlg_LoadingManager* dlg) noexcept final;
	private:
		npsys::device_n dev_;
		npsys::algorithm_n alg_;
	};

	class CTaskUnloadOneAlgorithm : public CTask {
	public:
		CTaskUnloadOneAlgorithm(npsys::device_n& dev, npsys::algorithm_n& alg)
			: dev_(dev)
			, alg_(alg)
		{
		}
		virtual void Exec(CDlg_LoadingManager* dlg) noexcept final;
	private:
		npsys::device_n dev_;
		npsys::algorithm_n alg_;
	};

	class CTaskUploadOneI2CModule : public CTask {
	public:
		CTaskUploadOneI2CModule(npsys::controller_n& ctrl, npsys::i2c_assigned_module_n& mod)
			: ctrl_(ctrl)
			, mod_(mod)
		{
		}
		virtual void Exec(CDlg_LoadingManager* dlg) noexcept final;
	private:
		npsys::controller_n ctrl_;
		npsys::i2c_assigned_module_n mod_;
	};

	class CTaskUploadControllerFirmware : public CTask {
	public:
		CTaskUploadControllerFirmware(npsys::controller_n& ctrl)
			: ctrl_(ctrl)
		{
		}
		virtual void Exec(CDlg_LoadingManager* dlg) noexcept final;
	private:
		npsys::controller_n ctrl_;
	};


	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	CDlg_LoadingManager(std::unique_ptr<CTask>&& task)
		: task_(std::move(task)) 
	{
	}

	enum { IDD = IDD_DLG_LOADING_MANAGER };

	BEGIN_MSG_MAP(CDlg_LoadingManager)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColor)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		CHAIN_MSG_MAP(base)
	END_MSG_MAP()

	COutputEdit m_output;
	CProgressBarCtrl m_progress;
	CMyButton m_buttonOk;

	BOOL EndDialog(_In_ int nRetCode);
protected:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCtlColor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		return 0;
	}
	std::unique_ptr<CTask> task_;
	std::streambuf* old_cout_;
	std::streambuf* old_cerr_;
	CProgressBarCtrl* standart_progress_;
};
