#pragma once
#include <wx/string.h>
#include <wx/filename.h>
namespace javautils
{
	/*
	 * Get the version from a minecraft.jar by parsing its class files. Expensive!
	 */
	wxString GetMinecraftJarVersion(wxFileName jar);
}