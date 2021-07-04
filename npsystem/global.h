// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "dockable.h"

enum class AlphaIcon {
	Root,
	Folder_Open,
	Folder_Close,

	Modules,
	Module,
	I2C,
	Container,
	Network,
	Block,
	Controller_nl,
	Controller_good,
	Controller_bad,
	Hardware,
	Software,
	Algorithm,
	Exclam,
	Empty,
	Ctrl_Dev,
	Io,
	File_U,
	File_C,
	File_S,
	File_H,
	File_Delete,
	Di_Pin,
	Do_Pin,
	Ai_Pin,

	Library,
	Book,
	Fun_cat,
	Fun,
	Source,
	Object,
	PC,
	Var,
	Binary,

	Pinned,
	UnPinned,
	Question,

	nIcons,
};

enum class AlphaCursor {
	Arrow,
	LinkSelect,
	Block,
	Drop,
	NoDrop,
	Pen,
	Drag,
	Hand,
	SizeWE,

	nCursors,
};

enum class CurrentDirectory {
		ROOT,
		MODULE,
		ALGORITHMS,
		BUILD,
		LIBRARIES
};

struct TreeItemState {
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & state;
		ar & index;
	}
	union {
		struct {
			bool window : 1;
			bool expanded : 1;
			bool pinned : 1;
			bool active : 1;
		};
		uint32_t state;
	};
	size_t index;
	TreeItemState() { state = 0; index = 0; }
};

class CMyTabContainer;

class Global {
	class CControlsState {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int /*file_version*/) {
		ar & fullscreen;
		ar & rc_main_wnd;
		ar & tree_state;
		ar & opened_windows;
		
		// Dockable windows
		ar & CMyTabContainer::states;
		ar & CDockableWindow::dockable_windows;
	}
public:
	std::map<oid_t, TreeItemState> tree_state;
	size_t opened_windows;

	bool fullscreen;
	CRect rc_main_wnd;
	//
	void Load();
	void Save();
	void Default();
};

	std::string app_roaming_dir_;
	std::string controls_state_path_;
	std::string config_path_;
	std::string module_dir_;
	std::string alg_dir_;
	std::string lib_dir_;
	std::string build_dir_;
	CIcon m_icons[static_cast<size_t>(AlphaIcon::nIcons)];
	CCursor m_cursors[static_cast<size_t>(AlphaCursor::nCursors)];
public:
	void init() noexcept;

	const std::string& GetAppRoamingDir() const { return app_roaming_dir_; }
	const std::string& ControlsStatePath() const { return controls_state_path_; }
	const std::string& ConfigPath() const { return config_path_; }
	const std::string& GetModuleDir() const { return module_dir_; }
	const std::string& GetAlgorithmsDir() const { return alg_dir_; }
	const std::string& GetLibrariesDir() const { return lib_dir_; }
	const std::string& GetBuildDir() const { return build_dir_; }
	
	CIconHandle GetIcon(AlphaIcon icon) const noexcept { 
		auto ix = static_cast<size_t>(icon);
		assert(ix < static_cast<size_t>(AlphaIcon::nIcons));
		return m_icons[ix].m_hIcon; 
	}

	CCursorHandle GetCursor(AlphaCursor cursor) const noexcept {
		auto ix = static_cast<size_t>(cursor);
		assert(ix < static_cast<size_t>(AlphaCursor::nCursors));
		return m_cursors[ix].m_hCursor;
	}

	void ChangeCurrentDirectory(CurrentDirectory dir) const noexcept;
	CControlsState controls;
};

extern Global global;