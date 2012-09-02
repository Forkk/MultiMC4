// 
//  Copyright 2012 Andrew Okin
// 
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
// 
//        http://www.apache.org/licenses/LICENSE-2.0
// 
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//

#pragma once
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/dnd.h>
#include <instance.h>

class MainWindow;

class ModEditWindow : public wxFrame
{
public:
	ModEditWindow(MainWindow *parent, Instance *inst);
	
	virtual bool Show(bool show = true);
	
	class ModListCtrl : public wxListCtrl
	{
	public:
		ModListCtrl(wxWindow *parent, int id, Instance *inst);
		
		virtual wxString OnGetItemText(long int item, long int column) const;
		
		virtual void UpdateItems();
		wxArrayInt GetSelectedItems();
		
		void DrawInsertMark(int index);
		virtual void SetInsertMark(const int index);

		virtual void OnCopyMod(wxCommandEvent &event);
		virtual void OnPasteMod(wxCommandEvent &event);
		virtual void OnDeleteMod(wxCommandEvent &event);

		virtual void CopyMod() = 0;
		virtual void PasteMod() = 0;
		virtual void DeleteMod() = 0;
		
	protected:
		Instance *m_inst;
		
		ModList *GetModList() const;
		
		int m_insMarkIndex;

		DECLARE_EVENT_TABLE()
	} *jarModList, *mlModList, *coreModList;
	
protected:
	class JarModListCtrl : public ModListCtrl
	{
	public:
		JarModListCtrl(wxWindow *parent, int id, Instance *inst)
			: ModListCtrl(parent, id, inst) {}

		virtual void CopyMod();
		virtual void PasteMod();
		virtual void DeleteMod();
	};

	class MLModListCtrl : public ModListCtrl
	{
	public:
		MLModListCtrl(wxWindow *parent, int id, Instance *inst)
			: ModListCtrl(parent, id, inst) {}

		virtual void CopyMod();
		virtual void PasteMod();
		virtual void DeleteMod();
	};

	class CoreModListCtrl : public ModListCtrl
	{
	public:
		CoreModListCtrl(wxWindow *parent, int id, Instance *inst)
			: ModListCtrl(parent, id, inst) {}

		virtual void CopyMod();
		virtual void PasteMod();
		virtual void DeleteMod();
	};
	
	wxButton *delJarModBtn;
	wxButton *jarModUpBtn;
	wxButton *jarModDownBtn;
	
	void LoadJarMods();
	void LoadMLMods();
	void LoadCoreMods();
	
	void UpdateColSizes();
	
	Instance *m_inst;
	
	void OnDeleteJarMod();
	void OnDeleteMLMod();
	void OnJarListKeyDown(wxListEvent &event);
	void OnMLListKeyDown(wxListEvent &event);
	void OnCoreListKeyDown(wxListEvent &event);
	
	void OnAddJarMod(wxCommandEvent &event);
	void OnDeleteJarMod(wxCommandEvent &event);
	void OnMoveJarModUp(wxCommandEvent &event);
	void OnMoveJarModDown(wxCommandEvent &event);
	void OnJarModSelChanged(wxListEvent &event);
	void OnDragJarMod(wxListEvent &event);
	
	void OnAddMLMod(wxCommandEvent &event);
	void OnDeleteMLMod(wxCommandEvent &event);
	
	void OnAddCoreMod(wxCommandEvent &event);
	void OnDeleteCoreMod(wxCommandEvent &event);
	
	void OnExportClicked(wxCommandEvent& event);
	void OnCloseClicked(wxCommandEvent &event);
	
	class JarModsDropTarget : public wxFileDropTarget
	{
	public:
		JarModsDropTarget(ModListCtrl *owner, Instance *inst);
		
		virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &filenames);
		
		virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def);
		virtual void OnLeave();
		
		ModListCtrl *m_owner;
		Instance *m_inst;
	};
	
	class MLModsDropTarget : public wxFileDropTarget
	{
	public:
		MLModsDropTarget(ModListCtrl *owner, Instance *inst);
		
		virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &filenames);
		virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def);
		
		ModListCtrl *m_owner;
		Instance *m_inst;
	};
	
	class CoreModsDropTarget : public wxFileDropTarget
	{
	public:
		CoreModsDropTarget(ModListCtrl *owner, Instance *inst);
		
		virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &filenames);
		virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def);
		
		ModListCtrl *m_owner;
		Instance *m_inst;
	};
	
	MainWindow *m_mainWin;
	
	DECLARE_EVENT_TABLE()
};

enum
{
	ID_JAR_MOD_LIST,
	ID_ML_MOD_LIST,
	ID_CORE_MOD_LIST,
	
	ID_ADD_JAR_MOD,
	ID_DEL_JAR_MOD,
	ID_MOVE_JAR_MOD_UP,
	ID_MOVE_JAR_MOD_DOWN,
	
	ID_ADD_ML_MOD,
	ID_DEL_ML_MOD,
	
	ID_ADD_CORE_MOD,
	ID_DEL_CORE_MOD,

	ID_EXPORT,
};
