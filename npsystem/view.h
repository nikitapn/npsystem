// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

class CTreeItemAbstract;

class CView : public CWindow {
public:
	CView(HWND hwnd = NULL) : 
		CWindow(hwnd) {
	}
	virtual ~CView() = default;

	virtual void PostDestroy() = 0;
	virtual bool IsModified() = 0;
	virtual void Save() = 0;
	virtual bool IsFocused() const noexcept = 0;
	virtual std::optional<HBRUSH> CustomActiveColor() const noexcept { return {}; }
	virtual HICON GetIcon() const noexcept;

	virtual BOOL CanUndo() { return FALSE; }
	virtual BOOL CanRedo() { return FALSE; }
	virtual BOOL CanStartOnline() { return FALSE; }
	virtual BOOL CanStopOnline() { return FALSE; }
	virtual BOOL CanUpload() { return FALSE; }
};


class CItemView : public CView {
public:
	CItemView(HWND hwnd = NULL)
		: CView(hwnd)
		, item_(nullptr)
	{
	}
	CTreeItemAbstract* GetItem() noexcept { return item_; }
	const CTreeItemAbstract* GetItem() const noexcept { return item_; }
	virtual void PostDestroy() final;
protected:
	CTreeItemAbstract* item_;
};
