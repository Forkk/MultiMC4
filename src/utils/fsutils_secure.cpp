#ifdef WIN32
#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <sddl.h>
#include <wchar.h>
// This constructs an ACL to give the current user file permissions.
BOOL CreateMyDACL(SECURITY_ATTRIBUTES * pSA)
{
	// Get process token
	HANDLE hToken = NULL;
	if (!OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken ))
	{
		//_tprintf( _T("OpenProcessToken failed. GetLastError returned: %d\n"), GetLastError());
		return false;
	}

	// Get the size of the memory buffer needed for the SID
	DWORD dwBufferSize = 0;
	if (!GetTokenInformation( hToken, TokenUser, NULL, 0, &dwBufferSize) && (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
	{
		//_tprintf( _T("GetTokenInformation failed. GetLastError returned: %d\n"), GetLastError());
		// Cleanup
		CloseHandle( hToken );
		hToken = NULL;
		return false;
	}
	
	// Allocate buffer for user token data
	PTOKEN_USER pTokenUser = (PTOKEN_USER) malloc(dwBufferSize);
	
	// Retrieve the token information in a TOKEN_USER structure
	if (!GetTokenInformation( hToken, TokenUser, pTokenUser, dwBufferSize, &dwBufferSize))
	{
		//_tprintf( _T("2 GetTokenInformation failed. GetLastError returned: %d\n"), GetLastError());
		// Cleanup
		CloseHandle( hToken );
		hToken = NULL;
		free(pTokenUser);
		return false;
	}
	
	// Check if SID is valid
	if (!IsValidSid(pTokenUser->User.Sid))
	{
		//_tprintf( _T("The owner SID is invalid.\n") );
		// Cleanup
		CloseHandle(hToken);
		hToken = NULL;
		free(pTokenUser);
		return false;
	}
	
	// Convert user SID to the equivalent string
	LPTSTR pszSID = NULL;
	if(!ConvertSidToStringSid( pTokenUser->User.Sid, &pszSID ))
	{
		CloseHandle(hToken);
		hToken = NULL;
		free(pTokenUser);
		return false;
	}

	// 1024 bytes is enough for everyone. This is final.
	wchar_t long_str[1024];
	// give full access to the file to the user running this process and nobody else
	swprintf(long_str, 1024 ,TEXT("D:P(A;OI;GA;;;%s)"),pszSID);
	if(pSA == NULL)
	{
		free(pTokenUser);
		return FALSE;
	}
	
	PULONG nSize = 0;
	// Do some verification
	/*
	wprintf(TEXT("The ACE strings: %s \n"), long_str);
	wprintf(TEXT("The converted string is at: %p \n"), &(pSA->lpSecurityDescriptor));
	*/

	// Convert the string to the security descriptor binary and return
	bool result = (bool) ConvertStringSecurityDescriptorToSecurityDescriptorW(
		long_str,                         // The ACE strings
		SDDL_REVISION_1,              // Standard revision level
		&(pSA->lpSecurityDescriptor), // Pointer to the converted security descriptor
		nSize);                // The size in byte the converted security descriptor
	free(pTokenUser);
	return result;
}
namespace fsutils
{
	// only works on NTFS volumes.
	bool SecureFile (const char * filename)
	{
		SECURITY_ATTRIBUTES  sa;
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.bInheritHandle = FALSE;
		if(!CreateMyDACL(&sa))
			return false;
		int result = SetFileSecurityA(filename,DACL_SECURITY_INFORMATION,sa.lpSecurityDescriptor);
		LocalFree(sa.lpSecurityDescriptor);
		return result != 0;
	}
}
#else
#include <sys/stat.h>
namespace fsutils
{
	// Windows developers, gaze upon the perfection of this solution and weep. Weep blood.
	bool SecureFile (const char * filename)
	{
		int result = chmod(filename,S_IRUSR | S_IWUSR);
		return result != -1;
	}
}
#endif

