#include "Instance.h"

const char cfgFileName[] = "instance.cfg";

bool IsValidInstance(fs::path rootDir)
{
	return fs::exists(rootDir) && fs::is_directory(rootDir) &&
		fs::exists(rootDir / cfgFileName) && fs::is_regular_file(rootDir / cfgFileName);
}

Instance::Instance(fs::path rootDir, wxString name)
{
	this->rootDir = rootDir;

	if (!name.IsNull() && !name.IsEmpty())
		this->SetName(name);

	Load();
}

Instance::~Instance(void)
{

}

void Instance::Save()
{
	config.Save(rootDir / cfgFileName);
}

void Instance::Load()
{
	fs::path cfgPath = rootDir / cfgFileName;
	fs::path oldCfgPath = rootDir / "instance.xml";

	try
	{
		if (exists(cfgPath) && is_regular_file(cfgPath))
		{
			config.Load(cfgPath);
		}
		else if (exists(oldCfgPath) && is_regular_file(oldCfgPath))
		{
			config.LoadXML(oldCfgPath);
		}
	}
	catch (boost::property_tree::ini_parser_error e)
	{

	}
	catch (boost::property_tree::xml_parser_error e)
	{

	}
}

fs::path Instance::GetRootDir()
{
	return rootDir;
}


// InstConfig
void InstConfig::Load(fs::path &filename)
{
	using boost::property_tree::ptree;

//#ifdef WIN32
//	boost::property_tree::basic_ptree<TCHAR, std::string> pt;
//#else
	ptree pt;
//#endif

	read_ini((char *)filename.native().c_str(), pt);

	name = pt.get<std::string>("name", "Unnamed Instance");
	iconKey = pt.get<std::string>("iconKey", "default");
	notes = pt.get<std::string>("notes", "");
	needsRebuild = pt.get<bool>("NeedsRebuild", false);
	askUpdate = pt.get<bool>("AskUpdate", true);
}

void InstConfig::Save(fs::path &filename)
{
	using boost::property_tree::ptree;
	ptree pt;

	pt.put<std::string>("name", name);
	pt.put<std::string>("iconKey", iconKey);
	pt.put<std::string>("notes", notes);
	pt.put<bool>("NeedsRebuild", needsRebuild);
	pt.put<bool>("AskUpdate", askUpdate);

	write_ini((char *)filename.native().c_str(), pt);
}

void InstConfig::LoadXML(fs::path &filename)
{
	using boost::property_tree::ptree;
	ptree pt;

	read_xml((char *)filename.native().c_str(), pt);

	name = pt.get<std::string>("instance.name", "Unnamed Instance");
	iconKey = pt.get<std::string>("instance.iconKey", "default");
	notes = pt.get<std::string>("instance.notes", "");
	needsRebuild = pt.get<bool>("instance.NeedsRebuild", false);
	askUpdate = pt.get<bool>("instance.AskUpdate", true);
}