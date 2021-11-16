#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#if defined XWIN32
#include <Windows.h>
#endif

#if defined LINUX
#include <dlfcn.h>
#endif

#include <nm.h>
#include <nutil.h>
#include <zio.x.h>

typedef struct
{
	void* (*f[2])(void*);
	void* o;
}ServiceArgs;



static ServiceArgs sa = {0};


#if defined XWIN32
static uint32_t status_service;
static uint32_t hsvc = SERVICE_STOPPED;
#endif

#if defined XWIN32
VOID SET_SERVICE_STATE(SERVICE_STATUS_HANDLE hd, DWORD dwState)
{
	SERVICE_STATUS ss;
	ss.dwServiceType=SERVICE_WIN32_OWN_PROCESS;
	ss.dwCurrentState=dwState;
	ss.dwControlsAccepted=SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
	ss.dwWin32ExitCode=0;
	ss.dwServiceSpecificExitCode=0;
	ss.dwCheckPoint=0;
	ss.dwWaitHint=0;
        
	status_service = dwState;
	SetServiceStatus(hd, &ss);
}
#endif

#if defined XWIN32
DWORD GET_SERVICE_STATE()
{
	return status_service;
}
#endif

#if defined XWIN32
uint32_t WINAPI __ServiceHandler(uint32_t ctrl, uint32_t evnt, void* p, void* ctx)
{
	switch(ctrl)
	{
	case SERVICE_CONTROL_PAUSE:
		SET_SERVICE_STATE(hsvc, SERVICE_PAUSE_PENDING, 0);
		SET_SERVICE_STATE(hsvc, SERVICE_PAUSED);
		break;

	case SERVICE_CONTROL_CONTINUE:
		SET_SERVICE_STATE(hsvc, SERVICE_CONTINUE_PENDING, 0);
		SET_SERVICE_STATE(hsvc, SERVICE_RUNNING);
		break;


	case SERVICE_CONTROL_STOP:
		//StopProcess(&cx);
		SET_SERVICE_STATE(hsvc, SERVICE_STOP_PENDING, 0);
		SET_SERVICE_STATE(hsvc, SERVICE_STOPPED);
		break;

	default:
		break;
	}

	return NO_ERROR;
}
#endif

#if 0
void argsParsing(int32_t argc, int8_t** argv, ServiceArgs* sa)
{
	int32_t i = 0;
	for ( i=1 ; i<argc ; i++ )
	{
		if ( argv[i][0]=='-' )
		{
			if ( argv[i][1] == '-' ) 
			{
				if ( argv[i][2] == 'd' ) ///// display
				{
					sprintf(sa->args[CM_DISPLAY_NAME], "%s", argv[i+1]);
					i++;
				}
				else if (argv[i][2] == 's' ) //// service
				{
					sprintf(sa->args[CM_SERVICE_NAME], "%s", argv[i+1]);
					i++;
				}
				else if ( (argv[i][2] == 'X') || (argv[i][2] =='x') ) //// codex
				{
					sprintf(sa->args[CM_CODEX_PATH], "%s", argv[i+1]);
					i++;
				}
				else if ( (argv[i][2] == 'P') || (argv[i][2] =='p') ) //// extern dll path
				{
					sprintf(sa->args[CM_EXTDLL_PATH], "%s", argv[i+1]);
					i++;
				}
				else if ( argv[i][2] == 'l' ) //// log
				{
					sprintf(sa->args[CM_LOG_PATH], "%s", argv[i+1]);
					i++;
				}
				else if ( argv[i][2] == 'e' ) //// log_expire
				{
					//cx._logFlag = argv[i+1][0];
					//cx._logExpire = atoi(&argv[i+1][1]);
					i++;
				}
				else if ( (argv[i][2] == 'M') || (argv[i][2] == 'm') ) //// mode
				{
					sa->mode = argv[i+1][0];
					i++;
				}
			}
			else
			{
				if ( strncmp(&argv[i][1], "wssport", 7) == 0 )
				{
					sprintf(sa->args[CM_WSS_PORT], "%s", argv[i+1]);
					i++;
				}
				else if ( strncmp(&argv[i][1], "httpport", 8) == 0 )
				{
					sprintf(sa->args[CM_HTTPD_PORT], "%s", argv[i+1]);
					i++;
				}
				else if ( strncmp(&argv[i][1], "home", 4) == 0 )
				{
					sprintf(sa->args[CM_HTTPD_HOME], "%s", argv[i+1]);
					i++;
				}
				else if ( strncmp(&argv[i][1], "index", 5) == 0 )
				{
					sprintf(sa->args[CM_HTTPD_INDEX], "%s", argv[i+1]);
					i++;
				}
				else if ( strncmp(&argv[i][1], "vector", 5) == 0 )
				{
					sprintf(sa->args[CM_VECTOR_PATH], "%s", argv[i+1]);
					i++;
				}
				else if ( strncmp(&argv[i][1], "key0", 5) == 0 )
				{
					sprintf(sa->args[CM_KEY_DAT], "%s", argv[i+1]);
					i++;
				}
				else if ( strncmp(&argv[i][1], "key1", 5) == 0 )
				{
					sprintf(sa->args[CM_KEY_IDX], "%s", argv[i+1]);
					i++;
				}

			}
		}
	}
}
#endif
void readArgs(int32_t argc, int8_t** argv, int8_t* key, int8_t* value)
{
	int32_t i = 0;
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == *(key+0))
		{
			if (argv[i][1] == *(key+1))
			{
				if (strncmp(&argv[i][2], key+2, strlen(&argv[i][2])) == 0)
				{
					sprintf(value, "%s", argv[i + 1]);
					i++;
				}
			}
			else
			{
				if (strncmp(&argv[i][1], key+1, strlen(&argv[i][1])) == 0)
				{
					sprintf(value, "%s", argv[i + 1]);
					i++;
				}
			}
		}
	}
}


void* __cmain(int argc, char** argv)
{
  int8_t  _argv[512] = {0};

  memset(_argv, 0, 512);
  readArgs(argc, argv, "--m", _argv); 


	if ( (_argv[0]=='s') || (_argv[0]=='S') )
	{
		#if defined XWIN32
    memset(_argv, 0, 512);
    readArgs(argc, argv, "--s", _argv); 
		hsvc = RegisterServiceCtrlHandlerEx(_argv, __ServiceHandler, 0);
		if ( hsvc == 0 )
		{
		}

		sa.f[0](sa.o);

		SET_SERVICE_STATE(hsvc, SERVICE_RUNNING);

		for ( ; GET_SERVICE_STATE() != SERVICE_STOPPED; )
		{
			sa.f[1](sa.o);
			if ( GET_SERVICE_STATE() == SERVICE_PAUSED )
			{
				continue;
			}
			Sleep(1);
		}
		#endif
	}
	else
	{
		sa.f[0](sa.o);

		//StartProcess(&cx);
		while ( 1 )
		{
			sa.f[1](sa.o);
			#if defined XWIN32
			Sleep(1);
			#endif
			#if defined LINUX
			usleep(1);
			#endif
		}
	}
	return 0;
}


int32_t __box_icode_x(int32_t argc, int8_t** argv, ServiceArgs* p)
{
	int32_t e = 0;
  int8_t  _argv[512] = {0};
  int8_t  _ver[512] = {0};
  int8_t  _authr[512] = {0};
	void* hmodule = 0;
  int32_t (*getMessage)(void*,void*,void*,void*) = 0;

  memset(_argv, 0, 512);
  readArgs(argc, argv, "--X", _argv); 

  #if __STATIC_LIB__==0
  #if defined LINUX
  hmodule = dlopen(_argv, RTLD_LAZY);
  #endif

  #if defined XWIN32
  hmodule = LoadLibrary(_argv);
  #endif

  if ( hmodule == 0 ) return 0xEFFFFFFF;

  #if defined LINUX
  if ( getMessage == 0 )	getMessage   = dlsym(hmodule, "mtifGetMessage");
  #endif

  #if defined XWIN32
  if ( getMessage == 0 )	*(FARPROC*)&getMessage   = GetProcAddress(hmodule, "mtifGetMessage");
  #endif
  #endif


	e = getMessage(0, (void*)MAKELONG(SYSTEM_VERSION, MTIF), _ver, 512);
	e = getMessage(0, (void*)MAKELONG(SYSTEM_AUTHOR, MTIF), _authr, 512);


  memset(_argv, 0, 512);
  readArgs(argc, argv, "--s", _argv); 
	BOX(_argv);
  printf("+------------------------+-----------------------------------------------------+\r\n");
	printf("| %22s | %51s |\r\n", "version", _ver);
	printf("| %22s | %51s |\r\n", "author", _authr);
  printf("+------------------------+-----------------------------------------------------+\r\n");

  #if __STATIC_LIB__==0
  #if defined LINUX
  dlclose(hmodule);
  #endif

  #if defined XWIN32
	FreeLibrary(hmodule);
  #endif
	#endif

	return 0;
}



void __nmain(int32_t argc, int8_t** argv, void* (*f0)(void*), void* (*f1)(void*), void* o)
{
  int8_t _argv[512] = {0};
	//LOG(">>>> %d %s %s %s %s %s %s %s %s %s %s\r\n",
	//				argc, argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9], argv[10]);

	//argsParsing(argc, argv, &sa);
	sa.f[0] = f0;
	sa.f[1] = f1;
	sa.o = o;

	__box_icode_x(argc, argv,&sa);
	

  memset(_argv, 0, 512);
  readArgs(argc, argv, "--m", _argv);
  


	#if defined XWIN32
	if ( (_argv[0]=='s') || (_argv[0]=='S') )
	{
    memset(_argv, 0, 512);
    readArgs(argc, argv, "--s", _argv);
		SERVICE_TABLE_ENTRY STE[] = 
		{
			{_argv, (LPSERVICE_MAIN_FUNCTION)__cmain},
			{0, 0}
		};
		StartServiceCtrlDispatcher(STE);
	}
	else
	if ( (_argv[0]=='c') || (_argv[0]=='C') )
	{
		__cmain(argc, argv);
	}
	#endif


	#if defined LINUX
	__cmain(argc, argv);
	#endif
}

