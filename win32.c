/*
 * Copyright (c) 1996 The Regents of the University of California.
 * All rights reserved.
 * 
 *
 * This module contributed by John Brezak <brezak@apollo.hp.com>.
 * January 31, 1996
 *
 * Additional contributions Orion Hodson (UCL) 1996-98.
 *
 */

#ifdef WIN32

#ifndef lint
static char rcsid[] =
    "@(#) $Header$ (LBL)";
#endif

#include "config_win32.h"
#include "debug.h"
#include "tcl.h"
#include "tk.h"
#include "util.h"

int
uname(struct utsname *ub)
{
    char *ptr;
    DWORD version;
    SYSTEM_INFO sysinfo;
    char hostname[MAXGETHOSTSTRUCT];
    
    version = GetVersion();
    GetSystemInfo(&sysinfo);
    
    switch (sysinfo.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_INTEL:
	(void)strcpy(ub->machine, "ix86");
	break;
    case PROCESSOR_ARCHITECTURE_MIPS :
	(void)strcpy(ub->machine, "mips");
	break;
    case PROCESSOR_ARCHITECTURE_ALPHA:
	(void)strcpy(ub->machine, "alpha");
	break;
    case PROCESSOR_ARCHITECTURE_PPC:
	(void)strcpy(ub->machine, "ppc");
	break;
    default:
	(void)strcpy(ub->machine, "unknown");
	break;
    }
    
    if (version < 0x80000000) {
	(void)strcpy(ub->version, "NT");
    }
    else if (LOBYTE(LOWORD(version))<4) {
	(void)strcpy(ub->version, "Win32s");
    }
    else				/* Win95 */ {
	(void)strcpy(ub->version, "Win95");
    }
    (void)sprintf(ub->release, "%u.%u",
		  (DWORD)(LOBYTE(LOWORD(version))),
		  (DWORD)(HIBYTE(LOWORD(version))));
    (void)strcpy(ub->sysname, "Windows");
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        ptr = strchr(hostname, '.');
            if (ptr)
	    *ptr = '\0';
    }
    else {
	perror("uname: gethostname failed");
	strcpy(hostname, "FAILURE");
    }
    strncpy(ub->nodename, hostname, sizeof(ub->nodename));
    ub->nodename[_SYS_NMLN - 1] = '\0';
    return 0;
}

uid_t
getuid(void) 
{ 
    return 0;
    
}

gid_t
getgid(void)
{
    return 0;
}

unsigned long
gethostid(void)
{
    char   hostname[WSADESCRIPTION_LEN];
    struct hostent *he;

    if ((gethostname(hostname, WSADESCRIPTION_LEN) == 0) &&
        (he = gethostbyname(hostname)) != NULL) {
            return *((unsigned long*)he->h_addr_list[0]);        
    }

    /*XXX*/
    return 0;
}

int
nice(int pri)
{
    return 0;
}

extern void TkWinXInit(HINSTANCE hInstance);
extern int main(int argc, const char *argv[]);
extern int __argc;
extern char **__argv;

static char argv0[255];		/* Buffer used to hold argv0. */

char *__progname = "main";

#define WS_VERSION_ONE MAKEWORD(1,1)
#define WS_VERSION_TWO MAKEWORD(2,2)

int APIENTRY
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpszCmdLine,
    int nCmdShow)
{
    char *p;
    WSADATA WSAdata;
    int r;
    if (WSAStartup(WS_VERSION_TWO, &WSAdata) != 0 &&
        WSAStartup(WS_VERSION_ONE, &WSAdata) != 0) {
    	MessageBox(NULL, "Windows Socket initialization failed. TCP/IP stack\nis not installed or is damaged.", "Network Error", MB_OK | MB_ICONERROR);
        exit(-1);
    }

    debug_msg("WSAStartup OK: %sz\nStatus:%s\n", WSAdata.szDescription, WSAdata.szSystemStatus);

    TkWinXInit(hInstance);

    /*
     * Increase the application queue size from default value of 8.
     * At the default value, cross application SendMessage of WM_KILLFOCUS
     * will fail because the handler will not be able to do a PostMessage!
     * This is only needed for Windows 3.x, since NT dynamically expands
     * the queue.
     */
    SetMessageQueue(64);

    GetModuleFileName(NULL, argv0, 255);
    p = argv0;
    __progname = strrchr(p, '/');
    if (__progname != NULL) {
	__progname++;
    }
    else {
	__progname = strrchr(p, '\\');
	if (__progname != NULL) {
	    __progname++;
	} else {
	    __progname = p;
	}
    }
    
    r = main(__argc, (const char**)__argv);
    WSACleanup();
    return r;
}

void
ShowMessage(int level, char *msg)
{
    MessageBeep(level);
    MessageBox(NULL, msg, __progname,
	       level | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
}

static char szTemp[256];

int
printf(const char *fmt, ...)
{
	int retval;
	
	va_list ap;
	va_start(ap, fmt);
	retval = _vsnprintf(szTemp, 256, fmt, ap);
	OutputDebugString(szTemp);
	va_end(ap);

	return retval;
}

int
fprintf(FILE *f, const char *fmt, ...)
{
	int 	retval;
	va_list	ap;

	va_start(ap, fmt);
	if (f == stderr) {
		retval = _vsnprintf(szTemp, 256, fmt, ap);
		OutputDebugString(szTemp);
	} else {
		retval = vfprintf(f, fmt, ap);
	}
	va_end(ap);
	return retval;
}

void
perror(const char *msg)
{
    DWORD cMsgLen;
    CHAR *msgBuf;
    DWORD dwError = GetLastError();
    
    cMsgLen = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
			    FORMAT_MESSAGE_ALLOCATE_BUFFER | 40, NULL,
			    dwError,
			    MAKELANGID(0, SUBLANG_ENGLISH_US),
			    (LPTSTR) &msgBuf, 512,
			    NULL);
    if (!cMsgLen)
	fprintf(stderr, "%s%sError code %lu\n",
		msg?msg:"", msg?": ":"", dwError);
    else {
	fprintf(stderr, "%s%s%s\n", msg?msg:"", msg?": ":"", msgBuf);
	LocalFree((HLOCAL)msgBuf);
    }
}

int
WinPutsCmd(ClientData clientData, Tcl_Interp *interp, int argc, char **argv)
{
    FILE *f;
    int i, newline;
    char *fileId;

    i = 1;
    newline = 1;
    if ((argc >= 2) && (strcmp(argv[1], "-nonewline") == 0)) {
	newline = 0;
	i++;
    }
    if ((i < (argc-3)) || (i >= argc)) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" ?-nonewline? ?fileId? string\"", (char *) NULL);
	return TCL_ERROR;
    }

    /*
     * The code below provides backwards compatibility with an old
     * form of the command that is no longer recommended or documented.
     */

    if (i == (argc-3)) {
	if (strncmp(argv[i+2], "nonewline", strlen(argv[i+2])) != 0) {
	    Tcl_AppendResult(interp, "bad argument \"", argv[i+2],
		    "\": should be \"nonewline\"", (char *) NULL);
	    return TCL_ERROR;
	}
	newline = 0;
    }
    if (i == (argc-1)) {
	fileId = "stdout";
    } else {
	fileId = argv[i];
	i++;
    }

    if (strcmp(fileId, "stdout") == 0 || strcmp(fileId, "stderr") == 0) {
	char *result;

	if (newline) {
	    int len = strlen(argv[i]);
	    result = ckalloc(len+2);
	    memcpy(result, argv[i], len);
	    result[len] = '\n';
	    result[len+1] = 0;
	} else {
	    result = argv[i];
	}
	OutputDebugString(result);
	if (newline)
	    ckfree(result);
    } else {
	return TCL_OK;
	clearerr(f);
	fputs(argv[i], f);
	if (newline) {
	    fputc('\n', f);
	}
	if (ferror(f)) {
	    Tcl_AppendResult(interp, "error writing \"", fileId,
		    "\": ", Tcl_PosixError(interp), (char *) NULL);
	    return TCL_ERROR;
	}
    }
    return TCL_OK;
}

int
WinGetUserName(ClientData clientData, Tcl_Interp *interp, int argc, char **argv)
{
    char user[256];
    int size = sizeof(user);
    
    if (!GetUserName(user, &size)) {
	Tcl_AppendResult(interp, "GetUserName failed", NULL);
	return TCL_ERROR;
    }
    purge_chars(user, " \"`'![]");
    Tcl_AppendResult(interp, user, NULL);
    return TCL_OK;
}

static HKEY
regroot(root)
    char *root;
{
    if (strcasecmp(root, "HKEY_LOCAL_MACHINE") == 0)
	return HKEY_LOCAL_MACHINE;
    else if (strcasecmp(root, "HKEY_CURRENT_USER") == 0)
	return HKEY_CURRENT_USER;
    else if (strcasecmp(root, "HKEY_USERS") == 0)
	return HKEY_USERS;
    else if (strcasecmp(root, "HKEY_CLASSES_ROOT") == 0)
	return HKEY_CLASSES_ROOT;
    else
	return (HKEY)-1;
}

int 
WinReg(ClientData clientdata, Tcl_Interp *interp, int argc, char **argv)
{
	static char szBuf[255], szOutBuf[255];
        char *szRegRoot = NULL, *szRegPath = NULL, *szValueName;
        int cbOutBuf = 255;
        HKEY hKey, hKeyResult;
        DWORD dwDisp;

        if (argc < 4 || argc > 5) {
                Tcl_AppendResult(interp, "wrong number of args\n", szBuf, NULL);
                return TCL_ERROR;
        }
	
        strcpy(szBuf, argv[2]);
        szValueName = argv[3];
        szRegRoot   = szBuf;
        szRegPath   = strchr(szBuf, '\\');

        if (szRegPath == NULL || szValueName == NULL) {
                Tcl_AppendResult(interp, "registry path is wrongly written\n", szBuf, NULL);
                return TCL_ERROR;
        }
        
        *szRegPath = '\x0';
        szRegPath++;

        hKey = regroot(szRegRoot);
        
        if (hKey == (HKEY)-1) {
                Tcl_AppendResult(interp, "root not found %s", szRegRoot, NULL);
                return TCL_ERROR;
        }

        if (ERROR_SUCCESS != RegCreateKeyEx(hKey, szRegPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyResult, &dwDisp)) {
                Tcl_AppendResult(interp, "Could not open key", szRegRoot, szRegPath, NULL);
                return TCL_ERROR;
        }

	if (argc == 4 && !strcmp(argv[1],"get")) {
                DWORD dwType = REG_SZ;
                if (ERROR_SUCCESS != RegQueryValueEx(hKeyResult, szValueName, 0, &dwType, szOutBuf, &cbOutBuf)) {
                        RegCloseKey(hKeyResult);
                        Tcl_AppendResult(interp, "Could not set value", szValueName, NULL);
                        return TCL_ERROR;       
                }
                Tcl_SetResult(interp, szOutBuf, TCL_STATIC);	
        } else if (argc == 5 && !strcmp(argv[1], "set")) {
                if (ERROR_SUCCESS != RegSetValueEx(hKeyResult, szValueName, 0, REG_SZ, argv[4], strlen(argv[4]))) {
                        RegCloseKey(hKeyResult);
                        Tcl_AppendResult(interp, "Could not set value", szValueName, argv[4], NULL);
                        return TCL_ERROR;
                }
	}
        RegCloseKey(hKeyResult);
        return TCL_OK;
}

int
RegGetValue(HKEY* key, char *subkey, char *value, char *dst, int dlen)
{
        HKEY lkey;      
        LONG r;
        LONG len;
        DWORD type;
 
        r = RegOpenKeyEx(*key, subkey, 0, KEY_READ, &lkey);
 
        if (ERROR_SUCCESS == r) {
                r = RegQueryValueEx(lkey, value, 0, &type, NULL, &len);
                if (ERROR_SUCCESS == r && len <= dlen && type == REG_SZ) {
                        type = REG_SZ;
                        r = RegQueryValueEx(lkey, value, 0, &type, dst, &len);
                } else {
                        SetLastError(r);
                        perror("");
                }
        } else {
                SetLastError(r);
                perror("");
                return FALSE;
        }
        RegCloseKey(lkey);
        return TRUE;
}

#define MAX_VERSION_STRING_LEN 256
const char*
w32_make_version_info(char *szRatVer) 
{
	char		platform[64];
        static char	szVer[MAX_VERSION_STRING_LEN];
        OSVERSIONINFO	oi;

        oi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        GetVersionEx(&oi);

        switch(oi.dwPlatformId) {
	        case VER_PLATFORM_WIN32_NT:
			sprintf(platform, "Windows NT");
			break;
		case VER_PLATFORM_WIN32_WINDOWS:
			if (oi.dwMinorVersion > 0) {
				sprintf(platform, "Windows 98");
			} else {
				sprintf(platform, "Windows 95");
			}
			break;
		case  VER_PLATFORM_WIN32s:
			sprintf(platform, "Win32s");
		        break;
		default:
			sprintf(platform, "Windows (unknown)");
        }

	sprintf(szVer, "%s %s %d.%d %s", 
		szRatVer, 
		platform, 
		oi.dwMajorVersion, 
		oi.dwMinorVersion, 
		oi.szCSDVersion);
        return szVer;
}

#endif /* WIN32 */
