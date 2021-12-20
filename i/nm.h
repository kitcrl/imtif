#ifndef __NM_X_H_F2DBDC40_6196_4E67_A689_D31A9310BEC0__
#define __NM_X_H_F2DBDC40_6196_4E67_A689_D31A9310BEC0__

#include <stdint.h>
enum
{
	CM_CODEX_PATH = 0,
	CM_EXTDLL_PATH,
	CM_LOG_PATH,
	CM_DISPLAY_NAME,
	CM_SERVICE_NAME,
	CM_WSS_PORT,
	CM_HTTPD_PORT,
	CM_HTTPD_HOME,
	CM_HTTPD_INDEX,
	CM_VECTOR_PATH,
	CM_KEY_DAT,
	CM_KEY_IDX,
	CM_MAX
};



#if defined __cplusplus
extern "C"
#endif
#if __WIN32__
__declspec(dllexport)
#endif
void __nmain(int32_t argc, int8_t** argv, void* (*f0)(void*), void* (*f1)(void*), void* o);

#if defined __cplusplus
extern "C"
#endif
#if __WIN32__
__declspec(dllexport)
#endif
void readArgs(int32_t argc, int8_t** argv, int8_t* key, int8_t* value);

#endif