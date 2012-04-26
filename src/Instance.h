#pragma once
#include <wx/wx.h>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace fs = boost::filesystem;

bool IsValidInstance(fs::path rootDir);

struct InstConfig
{
	std::string name;
	std::string iconKey;
	std::string notes;
	bool needsRebuild;
	bool askUpdate;

	void Load(fs::path &filename);
	void Save(fs::path &filename);

	void LoadXML(fs::path &filename);
};

class Instance
{
public:
	Instance(fs::path rootDir, wxString name = _T(""));
	~Instance(void);

	void Save();
	void Load();

	fs::path GetRootDir();

	wxString GetName() { return config.name.c_str(); }
	void SetName(wxString name) { config.name = name.c_str(); }

	wxString GetIconKey();
	void SetIconKey(wxString iconKey);

protected:
	InstConfig config;

	fs::path rootDir;
};
