#pragma once

#include <wx/process.h>

class InstConsoleWindow;
class Instance;

class MinecraftProcess : public wxProcess
{
public:
	static wxProcess *Launch(Instance * source, InstConsoleWindow* parent, wxString username, wxString sessionID);

public:
	bool ProcessInput();
	void KillMinecraft();
	bool AlreadyKilled() const
	{
		return m_wasKilled;
	}
protected:
	MinecraftProcess(Instance * source, InstConsoleWindow* parent);
	void OnTerminate ( int pid, int status );
	bool m_wasKilled;
	InstConsoleWindow* m_parent;
	Instance * m_source;
};
