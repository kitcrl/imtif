#include <common.h>
#include <zio.x.h>
#include <imtif.h>
#include <njson.h>
#include <nutil.h>
#if __SSL_TLS__
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/engine.h>
#include <openssl/conf.h>

#endif


#if defined XWIN32
#include <Windows.h>
#endif

#if defined LINUX
#include <dlfcn.h>
#endif


#if __STATIC_LIB__==1
extern int32_t mtifGetMessage(void* h, void* m, void* w, void* l);
extern int32_t mtifSetMessage(void* h, void* m, void* w, void* l);
extern int32_t mtifPutMessage(void* h, void* m, void* w, void* l);
#endif

#if defined LINUX
#define __SYSTEM_SERIAL_PORT__             ""
//#define __SYSTEM_LIBRARY                   "code.X.x32.so"
//#define __SYSTEM_LIBRARY_SIFR              "code.X.sifr.x32.so"
#endif

#if defined XWIN32
#define __SYSTEM_SERIAL_PORT__             "COM1"
//#define __SYSTEM_LIBRARY                   "code.X.x86.dll"
//#define __SYSTEM_LIBRARY_SIFR              "code.X.sifr.x86.dll"
#endif


#define __BUF_SZ                           1024


#define MODULE_INDEX(a)                   (a-XMODULES)



#if __SSL_TLS__
typedef struct
{
  SSL*         ssl;
  BIO*         bio;
  BIO*         abio;
  BIO*         rbio;
  BIO*         wbio;
  BIO*         fd;
}oBIO;
#endif



#if __SSL_TLS__
typedef struct
{
  SSL_METHOD*  mthd;
  SSL_CTX*     ctx;
  X509*        cert;
  oBIO         obio[32];
}oSSL;
#endif



typedef struct
{
  int32_t fd;
  int32_t ip_port[2];
}CallbackInfo;

typedef struct
{
  void* hmodule;
  uint32_t hmodule_idx;
  void* hdl;
  void* h;
  int8_t* pb;

  int32_t (*getMessage)(void*,void*,void*,void*);
  int32_t (*setMessage)(void*,void*,void*,void*);
  int32_t (*putMessage)(void*,void*,void*,void*);

  int32_t (*callback[IMTIF_CALLBACK])(void*,int32_t,int8_t*,int32_t,void*,void*);
  void* obj;

  int32_t fd;
  CallbackInfo cbi;

  imtif icd;


  #if __CODE_LIMITER__==1
  uint32_t SR;
  int32_t _tid;
  int32_t _thr;
  #endif

  #if __SSL_TLS__
  oSSL   ossl;
  #endif



}IMTIF;



int32_t _ARGS(int8_t* s, int32_t idx, int8_t* v)
{
  int32_t i = 0;
  int32_t c = idx;

  for ( i=0 ;*s; s++, i++ )
  {
    *(v+i) = *s;
    *(v+i+1) = 0;

    if ( *(v+i) == ':' )
    {
      *(v+i) = 0;
      if ( c == 0 ) break;
      c--;
      i = -1;
    }
  }



  return i;
}


#if __CODE_LIMITER__==1
#define  TIME_LIMITER        3000
#endif


#if __CODE_LIMITER__==1
void* __code_limiter(void* o)
{
  int32_t e = 0;
  int32_t r = 0;
  uint32_t msg = 0;
  IMTIF* p = (IMTIF*)o;
  xSET_SEMAPHORE(p->SR, 0x80000000, 0x80000000);
  for ( ; ; )
  {
    xGET_SEMAPHORE(p->SR, 0x00000001, 0x00000001, 1, r);

    e++;
    if ( e == TIME_LIMITER ) exit(0);

    delay(1000);
  }
  xSET_SEMAPHORE(p->SR, 0x80000001, 0x7FFFFFFE);

  xTHREAD_EXIT(0, p->_thr);
  return 0;
}
#endif



int32_t modulus(IMTIF* imtif, int8_t* libpath)
{
  int32_t r = 0;

  #if __STATIC_LIB__==0
  if ( imtif->hmodule == 0 )
  {
    #if defined LINUX
    imtif->hmodule = dlopen(libpath, RTLD_LAZY);
    #endif

    #if defined XWIN32
    imtif->hmodule = LoadLibrary(libpath);
    #endif
  }
  if ( imtif->hmodule == 0 ) return 0xEFFFFFFF;

  #if defined LINUX
  if ( imtif->setMessage == 0 )	imtif->setMessage   = dlsym(imtif->hmodule, "mtifSetMessage");
  if ( imtif->getMessage == 0 )	imtif->getMessage   = dlsym(imtif->hmodule, "mtifGetMessage");
  if ( imtif->putMessage == 0 )	imtif->putMessage   = dlsym(imtif->hmodule, "mtifPutMessage");
  #endif

  #if defined XWIN32
  if ( imtif->getMessage == 0 )	*(FARPROC*)&imtif->getMessage   = GetProcAddress(imtif->hmodule, "mtifGetMessage");
  if ( imtif->setMessage == 0 )	*(FARPROC*)&imtif->setMessage   = GetProcAddress(imtif->hmodule, "mtifSetMessage");
  if ( imtif->putMessage == 0 )	*(FARPROC*)&imtif->putMessage   = GetProcAddress(imtif->hmodule, "mtifPutMessage");
  #endif
  #endif


  #if __CODE_LIMITER__==1
  if (imtif->hmodule_idx == 0 )
  {
    xTHREAD_CREATE(__code_limiter, imtif, (void*)&imtif->_tid, imtif->_thr);
    xGET_SEMAPHORE(imtif->SR, 0x80000000, 0x80000000, 0x00002000, r);
  }
  #endif

  imtif->hmodule_idx++;

  return 0;
}



int32_t unmodulus(IMTIF* imtif)
{
  int32_t r = 0;
  imtif->hmodule_idx --;

  if ( imtif->hmodule_idx > 0 ) return imtif->hmodule_idx;


  #if __CODE_LIMITER__==1
  if (imtif->hmodule_idx == 0 )
  {
    if ( xCHECK_SEMAPHORE(imtif->SR,0x80000000,0x80000000) )
    {
      xSET_SEMAPHORE(imtif->SR, 0x00000001, 0x00000001);
      xGET_SEMAPHORE(imtif->SR, 0x80000001, 0x00000000,  0x00002000,r);
    }
  }
  #endif

  #if __STATIC_LIB__==0
  #if defined LINUX
  if ( imtif->hmodule ) dlclose(imtif->hmodule);
  #endif
  #if defined XWIN32
  if ( imtif->hmodule ) FreeLibrary(imtif->hmodule);
  #endif
  #endif

  return 0;
}





///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////                                                                   //////
//////                                                                   //////
//////                            Socket                                 //////
//////                                                                   //////
//////                                                                   //////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*****************************************************************************/
/*****************************************************************************/
/*********************************        ************************************/
/*********************************        ************************************/
/*********************************        ************************************/
/*****************************                ********************************/
/*******************************            **********************************/
/*********************************        ************************************/
/************************************   **************************************/
/************************************* ***************************************/
/*****************************************************************************/
#if __CUSTOM_X_SOCKET__
void* __socket_callback(void* h, void* msg, void* wparam, void* lparam)
{
  IMTIF* imtif = (IMTIF*)h;
  int32_t fd=0;
  int32_t e = 0;

  switch(LOWORD((uint32_t)msg))
  {
  case READ:
    imtif->icd.fdset.max = imtif->getMessage(imtif->h, (void*)MAKELONG(FDSCRPTRS, XSOCKET), &imtif->icd.fdset.fds, sizeof(imtif->icd.fdset.fds));
    if ( imtif->callback[IMTIF_CALLBACK_READ] ) e = imtif->callback[IMTIF_CALLBACK_READ](imtif->h, imtif->cbi.fd, (int8_t*)wparam, (int32_t)lparam, &imtif->icd.fdset, imtif->obj);
    break;

  case READFROM:
    imtif->icd.fdset.max = imtif->getMessage(imtif->h, (void*)MAKELONG(FDSCRPTRS, XSOCKET), &imtif->icd.fdset.fds, sizeof(imtif->icd.fdset.fds));
    if ( imtif->callback[IMTIF_CALLBACK_READFROM] ) e = imtif->callback[IMTIF_CALLBACK_READFROM](imtif->h, imtif->cbi.fd, (int8_t*)wparam, (int32_t)lparam, imtif->cbi.ip_port, imtif->obj);
    break;

  case IP_PORT:
    memcpy(imtif->cbi.ip_port, wparam, (int32_t)lparam);
    break;

  case FDSCRPTR:
    imtif->cbi.fd = (int32_t)wparam;
    break;

  default:
    switch ( LOWORD((uint32_t)msg)|0xE0000000 )
    {
    case 0xE000FD0B:
      break;
    case 0xE000FD0F:
      if ( imtif->callback[IMTIF_CALLBACK_DISCONNECTED] ) e = imtif->callback[IMTIF_CALLBACK_DISCONNECTED](imtif->h, (int32_t)wparam, 0, 0, lparam, imtif->obj);
      break;
    case 0xE000FD0A:
      if ( imtif->callback[IMTIF_CALLBACK_CONNECTED] ) e = imtif->callback[IMTIF_CALLBACK_CONNECTED](imtif->h, (int32_t)wparam, 0, 0, lparam, imtif->obj);
      break;
    case 0xE000100B:
      break;
    case 0xE000100A:
      break;
    case 0xE000100F:
      break;
    case 0xE000FD1B:
      break;
    case 0xE000FD1A:
      if ( imtif->callback[IMTIF_CALLBACK_CONNECTED] ) e = imtif->callback[IMTIF_CALLBACK_CONNECTED](imtif->h, (int32_t)wparam, 0, 0, lparam, imtif->obj);
      break;
    case 0xE000101B:
      break;
    case 0xE000101A:
      break;
    case 0xE000101F:
      if ( imtif->callback[IMTIF_CALLBACK_DISCONNECTED] ) e = imtif->callback[IMTIF_CALLBACK_DISCONNECTED](imtif->h, (int32_t)wparam, 0, 0, lparam, imtif->obj);
      break;
    case 0xE000FDFB:
      break;
    case 0xE000FDFA:
      if ( imtif->callback[IMTIF_CALLBACK_DISCONNECTED] ) e = imtif->callback[IMTIF_CALLBACK_DISCONNECTED](imtif->h, (int32_t)wparam, 0, 0, lparam, imtif->obj);
      break;
    case 0xE000FDFF:
      break;
    case 0xE00010B0:
      break;
    case 0xE00010F0: //IO Function Output
      break;
    case 0xE00010F1: //IO Function Input
      break;


    }
    break;
  }
  return e;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __socket_fdset(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  IMTIF* imtif = (IMTIF*)h;
	return imtif->getMessage(imtif->h, MAKELONG(FDSCRPTRS, XSOCKET), (void*)moreinfo,0);
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __socket_fd(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  IMTIF* imtif = (IMTIF*)h;
	imtif->getMessage(imtif->h, MAKELONG(FDSCRPTR, XSOCKET), (void*)moreinfo,0);
  return *(int32_t*)(moreinfo);
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __socket_read(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  int32_t e = 0;
  IMTIF* imtif = (IMTIF*)h;

  e = imtif->setMessage(imtif->h, (void*)MAKELONG(FDSCRPTR, XSOCKET), (void*)fd, (void*)0);
  e = imtif->getMessage(imtif->h, (void*)MAKELONG(FDSCRPTRS, XSOCKET), &imtif->icd.fdset, sizeof(imtif->icd.fdset));
  e = imtif->setMessage(imtif->h, (void*)MAKELONG(READ, XSOCKET), (void*)b, (void*)sz);
  return e;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __socket_readfrom(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  int32_t e = 0;
  IMTIF* imtif = (IMTIF*)h;

  e = imtif->setMessage(imtif->h, (void*)MAKELONG(FDSCRPTR, XSOCKET), (void*)fd, (void*)0);
  e = imtif->setMessage(imtif->h, (void*)MAKELONG(READFROM, XSOCKET), (void*)b, (void*)sz);
  return e;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __socket_write(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  int32_t e = 0;
  IMTIF* imtif = (IMTIF*)h;

  e = imtif->setMessage(imtif->h, (void*)MAKELONG(FDSCRPTR, XSOCKET), (void*)fd, (void*)0);
  e = imtif->setMessage(imtif->h, (void*)MAKELONG(WRITE, XSOCKET), (void*)b, (void*)sz);
  return e;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __socket_writeto(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  int32_t e = 0;
  IMTIF* imtif = (IMTIF*)h;
  int8_t argv[2][32];

  _ARGS((int8_t*)moreinfo, 0, argv[0]);
  _ARGS((int8_t*)moreinfo, 1, argv[1]);

  e = imtif->setMessage(imtif->h, (void*)MAKELONG(XM_DST_IP, XSOCKET),   (void*)argv[0], (void*)strlen(argv[0]));
  e = imtif->setMessage(imtif->h, (void*)MAKELONG(XM_DST_PORT, XSOCKET), (void*)argv[1], (void*)strlen(argv[1]));

  e = imtif->setMessage(imtif->h, (void*)MAKELONG(FDSCRPTR, XSOCKET), (void*)fd, (void*)0);
  e = imtif->setMessage(imtif->h, (void*)MAKELONG(WRITETO, XSOCKET), (void*)b, (void*)sz);

  return e;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __socket_open(void** h, int8_t* argv, int32_t (*f[])(void*,int32_t,int8_t*,int32_t,void*,void*), void* o)
{
  int8_t  _argv[128] = {0};
  int32_t e = 0;
  IMTIF* imtif = 0;
  int8_t  _iargv[4096] = {0};
  int8_t  _iap = 0;
  if ( *h ) return 0xEFFFFFFF;
  *h = imtif = (IMTIF*)calloc(1, sizeof(IMTIF));

  for ( e=0 ; e<IMTIF_CALLBACK ; e++ )
  {
    if ( *f[e] ) imtif->callback[e] = f[e];
  }
  imtif->obj = o;

  e = NJSON_STR(argv, "SYSTEM_LIBRARY", _argv);
  if ( e > 0 ) sprintf(&_iargv[_iap], "%s\n", _argv);
  else sprintf(&_iargv[_iap], "\n", _argv);
  _iap = strlen(_iargv);

  if ( modulus(imtif, _argv) < 0 ) return 0xEF000001;

  e = NJSON_STR(argv, "SECRET_KEY_0", _argv);
  if ( e > 0 ) sprintf(&_iargv[_iap], "%s\n", _argv);
  else sprintf(&_iargv[_iap], "\n", _argv);
  _iap = strlen(_iargv);

  e = NJSON_STR(argv, "SECRET_KEY_1", _argv);
  if ( e > 0 ) sprintf(&_iargv[_iap], "%s\n", _argv);
  else sprintf(&_iargv[_iap], "\n", _argv);
  _iap = strlen(_iargv);

  e = imtif->setMessage(&imtif->hdl, (void*)MAKELONG(INIT,MTIF), _iargv, 0);
  e = imtif->setMessage(&imtif->h, (void*)MAKELONG(INIT,XSOCKET), (void*)imtif->hdl,0);
  e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_DELAY_0,XSOCKET), (void*)"3000",(void*)strlen("XXX"));
  e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_ECHOMODE,XSOCKET), (void*)"0",(void*)strlen("X"));
  e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_KEY,XSOCKET), (void*)"shinbaad@gmail.com",(void*)strlen("XXXXXXXXXXXXXXXXXX"));

  ///////   {"IP":"192.168.0.2","PORT":"7887","CSTYPE":"CLIENT","PROTOCOL":"TCP","CASTTYPE":"UNICAST"}
  if ( NJSON_STR(argv, "BUF_SZ", _argv)<0 )
  {
    imtif->pb = (int8_t*)calloc(__BUF_SZ, sizeof(int8_t));
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_BUFFER_PTR,XSOCKET), (void*)imtif->pb,(void*)__BUF_SZ);
  }
  else
  {
    imtif->pb = (int8_t*)calloc(atoi(_argv), sizeof(int8_t));
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_BUFFER_PTR,XSOCKET), (void*)imtif->pb,(void*)atoi(_argv));
  }

  if ( NJSON_STR(argv, "IP", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_IP,XSOCKET), (void*)"127.0.0.1",(void*)strlen("XXXXXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_IP,XSOCKET), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "PORT", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_PORT, XSOCKET), (void*)"7810",(void*)strlen("XXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_PORT, XSOCKET), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "CSTYPE", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_CSTYPE, XSOCKET), (void*)"SERVER",(void*)strlen("XXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_CSTYPE, XSOCKET), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "MAX_FD", _argv)<0 )
  {
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_MAX_FD, XSOCKET), (void*)"32",(void*)strlen("XX"));
    imtif->icd.fdset.max = atoi("32");
  }
  else
  {
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_MAX_FD, XSOCKET), (void*)_argv,(void*)strlen(_argv));
    imtif->icd.fdset.max = atoi(_argv);
  }

  if ( NJSON_STR(argv, "SYNC", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_SYNC, XSOCKET), (void*)"DISABLE",(void*)strlen("XXXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_SYNC, XSOCKET), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "BLOCK", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_BLOCK, XSOCKET), (void*)"DISABLE",(void*)strlen("XXXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_BLOCK, XSOCKET), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "TIMEOUT", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_TIMEOUT, XSOCKET), (void*)"0",(void*)strlen("X"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_TIMEOUT, XSOCKET), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "PROTOCOL", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_PROTO, XSOCKET), (void*)"TCP",(void*)strlen("XXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_PROTO, XSOCKET), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "CASTTYPE", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_CASTTYPE, XSOCKET), (void*)"UNICAST",(void*)strlen("XXXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_CASTTYPE, XSOCKET), (void*)_argv,(void*)strlen(_argv));

  e = imtif->setMessage( imtif->h, (void*)MAKELONG(SYSTEM_CALLBACK, XSOCKET),__socket_callback, imtif);
  imtif->fd = e = imtif->setMessage( imtif->h, (void*)MAKELONG(ENABLE,XSOCKET),0,0);

  return e;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __socket_close(void** h)
{
  int32_t e = 0;
  IMTIF* imtif = 0;

  imtif = *h;
  if ( *h )
  {
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(DISABLE,XSOCKET),0,0);
    if ( imtif->pb )
    {
      free(imtif->pb);
      imtif->pb = 0;
    }
    e = imtif->setMessage(&imtif->h, (void*)MAKELONG(RELEASE,XSOCKET),0,0);
    e = imtif->setMessage(&imtif->hdl, (void*)MAKELONG(RELEASE,MTIF), 0, 0);
    imtif->hdl = 0;
    unmodulus(imtif);
    free(*h);
    *h = 0;
  }

  return e;
}
#endif
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////                                                                   //////
//////                                                                   //////
//////                              HTTP                                 //////
//////                                                                   //////
//////                                                                   //////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*****************************************************************************/
/*****************************************************************************/
/*********************************        ************************************/
/*********************************        ************************************/
/*********************************        ************************************/
/*****************************                ********************************/
/*******************************            **********************************/
/*********************************        ************************************/
/************************************   **************************************/
/************************************* ***************************************/
/*****************************************************************************/
#if __CUSTOM_X_HTTP__
void* __http_callback(void* h, void* msg, void* wparam, void* lparam)
{
  IMTIF* imtif = (IMTIF*)h;
  int32_t fd=0;
  int32_t e=0;

  switch(LOWORD((uint32_t)msg))
  {
  case READ:
    if ( imtif->callback[IMTIF_CALLBACK_READ] ) e = imtif->callback[IMTIF_CALLBACK_READ](imtif->h, imtif->cbi.fd, (int8_t*)wparam, (int32_t)lparam, 0, imtif->obj);
    break;
  case FDSCRPTR:
    imtif->cbi.fd = (int32_t)wparam;
    break;
  case HTTP_URI:
    if ( imtif->callback[IMTIF_CALLBACK_URI] ) e = imtif->callback[IMTIF_CALLBACK_URI](imtif->h, imtif->cbi.fd, (int8_t*)wparam, (int32_t)lparam, 0, imtif->obj);
		break;
	case METHOD_GET:
    if ( imtif->callback[IMTIF_CALLBACK_GET] ) e = imtif->callback[IMTIF_CALLBACK_GET](imtif->h, imtif->cbi.fd, (int8_t*)wparam, (int32_t)lparam, 0, imtif->obj);
		break;
	case METHOD_POST:
    if ( imtif->callback[IMTIF_CALLBACK_POST] ) e = imtif->callback[IMTIF_CALLBACK_POST](imtif->h, imtif->cbi.fd, (int8_t*)wparam, (int32_t)lparam, 0, imtif->obj);
		break;

  default:
    switch ( LOWORD((uint32_t)msg)|0xE0000000 )
    {
    case 0xE000FD0B:
      break;
    case 0xE000FD0F:
      if ( imtif->callback[IMTIF_CALLBACK_DISCONNECTED] ) e = imtif->callback[IMTIF_CALLBACK_DISCONNECTED](imtif->h, (int32_t)wparam, 0, 0, 0, imtif->obj);
      break;
    case 0xE000FD0A:
      if ( imtif->callback[IMTIF_CALLBACK_CONNECTED] ) e = imtif->callback[IMTIF_CALLBACK_CONNECTED](imtif->h, (int32_t)wparam, 0, 0, 0, imtif->obj);
      break;
    case 0xE000100B:
      break;
    case 0xE000100A:
      break;
    case 0xE000100F:
      break;
    case 0xE000FD1B:
      break;
    case 0xE000FD1A:
      break;
    case 0xE000101B:
      break;
    case 0xE000101A:
      break;
    case 0xE000101F:
      if ( imtif->callback[IMTIF_CALLBACK_DISCONNECTED] ) e = imtif->callback[IMTIF_CALLBACK_DISCONNECTED](imtif->h, (int32_t)wparam, 0, 0, 0, imtif->obj);
      break;
    case 0xE000FDFB:
      break;
    case 0xE000FDFA:
      if ( imtif->callback[IMTIF_CALLBACK_DISCONNECTED] ) e = imtif->callback[IMTIF_CALLBACK_DISCONNECTED](imtif->h, (int32_t)wparam, 0, 0, 0, imtif->obj);
      break;
    case 0xE000FDFF:
      break;
    case 0xE00010B0:
      break;
    case 0xE00010F0: //IO Function Output
      break;
    case 0xE00010F1: //IO Function Input
      break;
    }
    break;
  }
  return (void*)e;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __http_fdset(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  IMTIF* imtif = (IMTIF*)h;
	return imtif->getMessage(imtif->h, MAKELONG(FDSCRPTRS, XHTTP), (void*)moreinfo,0);
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __http_fd(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  IMTIF* imtif = (IMTIF*)h;
	imtif->getMessage(imtif->h, MAKELONG(FDSCRPTR, XHTTP), (void*)moreinfo,0);
  return *(int32_t*)(moreinfo);
}


#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __http_read(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  int32_t e = 0;
  IMTIF* imtif = (IMTIF*)h;

  e = imtif->setMessage(imtif->h, (void*)MAKELONG(FDSCRPTR, XHTTP), (void*)fd, (void*)0);
  e = imtif->setMessage(imtif->h, (void*)MAKELONG(READ, XHTTP), (void*)b, (void*)sz);
  return e;
}


#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __http_write(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  int32_t e = 0;
  IMTIF* imtif = (IMTIF*)h;

  e = imtif->setMessage(imtif->h, (void*)MAKELONG(FDSCRPTR, XHTTP), (void*)fd, (void*)0);
  e = imtif->setMessage(imtif->h, (void*)MAKELONG(WRITE, XHTTP), (void*)b, (void*)sz);
  return e;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __http_open(void** h, int8_t* argv, int32_t (*f[])(void*,int32_t,int8_t*,int32_t,void*,void*), void* o)
{
  int8_t  _argv[2048] = {0};
  int32_t s = 0;
  int32_t e = 0;
  IMTIF* imtif = 0;
  int8_t  _iargv[4096] = {0};
  int8_t  _iap = 0;

  if ( *h ) return 0xEFFFFFFF;
  *h = imtif = (IMTIF*)calloc(1, sizeof(IMTIF));


  for ( e=0 ; e<IMTIF_CALLBACK ; e++ )
  {
    if ( *f[e] ) imtif->callback[e] = f[e];
  }
  imtif->obj = o;

  e = NJSON_STR(argv, "SYSTEM_LIBRARY", _argv);
  if ( e > 0 ) sprintf(&_iargv[_iap], "%s\n", _argv);
  else sprintf(&_iargv[_iap], "\n", _argv);
  _iap = strlen(_iargv);

  if ( modulus(imtif, _argv) < 0 ) return 0xEF000001;


  e = NJSON_STR(argv, "SECRET_KEY_0", _argv);
  if ( e > 0 ) sprintf(&_iargv[_iap], "%s\n", _argv);
  else sprintf(&_iargv[_iap], "\n", _argv);
  _iap = strlen(_iargv);

  e = NJSON_STR(argv, "SECRET_KEY_1", _argv);
  if ( e > 0 ) sprintf(&_iargv[_iap], "%s\n", _argv);
  else sprintf(&_iargv[_iap], "\n", _argv);
  _iap = strlen(_iargv);

  e = imtif->setMessage(&imtif->hdl, (void*)MAKELONG(INIT,MTIF), _iargv, 0);

  e = imtif->setMessage(&imtif->h, (void*)MAKELONG(INIT           , XHTTP), (void*)imtif->hdl,0);
  e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_DELAY_0     , XHTTP), (void*)"3000",(void*)strlen("XXX"));
  e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_ECHOMODE    , XHTTP), (void*)"0",(void*)strlen("X"));
  e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_KEY,XHTTP), (void*)"shinbaad@gmail.com",(void*)strlen("XXXXXXXXXXXXXXXXXX"));

  ///////   {"IP":"192.168.0.2","PORT":"7887","CSTYPE":"CLIENT","PROTOCOL":"TCP","CASTTYPE":"UNICAST"}
  if ( NJSON_STR(argv, "BUF_SZ", _argv)<0 )
  {
    imtif->pb = (int8_t*)calloc(__BUF_SZ, sizeof(int8_t));
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_BUFFER_PTR,XHTTP), (void*)imtif->pb,(void*)__BUF_SZ);
  }
  else
  {
    imtif->pb = (int8_t*)calloc(atoi(_argv), sizeof(int8_t));
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_BUFFER_PTR,XHTTP), (void*)imtif->pb,(void*)atoi(_argv));
  }

  if ( NJSON_STR(argv, "IP", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_IP,XHTTP), (void*)"127.0.0.1",(void*)strlen("XXXXXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_IP,XHTTP), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "PORT", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_PORT, XHTTP), (void*)"80",(void*)strlen("XX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_PORT, XHTTP), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "CSTYPE", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_CSTYPE, XHTTP), (void*)"CLIENT",(void*)strlen("XXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_CSTYPE, XHTTP), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "MAX_FD", _argv)<0 )
  {
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_MAX_FD, XHTTP), (void*)"32",(void*)strlen("XX"));
    imtif->icd.fdset.max = atoi("32");
  }
  else
  {
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_MAX_FD, XHTTP), (void*)_argv,(void*)strlen(_argv));
    imtif->icd.fdset.max = atoi(_argv);
  }

  if ( NJSON_STR(argv, "SYNC", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_SYNC, XHTTP), (void*)"DISABLE",(void*)strlen("XXXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_SYNC, XHTTP), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "BLOCK", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_BLOCK, XHTTP), (void*)"DISABLE",(void*)strlen("XXXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_BLOCK, XHTTP), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "TIMEOUT", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_TIMEOUT, XHTTP), (void*)"0",(void*)strlen("X"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_TIMEOUT, XHTTP), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "PROTOCOL", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_PROTO, XHTTP), (void*)"TCP",(void*)strlen("XXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_PROTO, XHTTP), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "CASTTYPE", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_CASTTYPE, XHTTP), (void*)"UNICAST",(void*)strlen("XXXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_CASTTYPE, XHTTP), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "URL", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_URL         , XHTTP), (void*)"http://127.0.0.1",(void*)strlen("XXXXXXXXXXXXXXXX"));
  else 
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_URL         , XHTTP), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "URI", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_URI         , XHTTP), (void*)"/",(void*)strlen("X"));
  else 
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_URI         , XHTTP), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "CONTENTS", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_CONTENTS    , XHTTP), (void*)0,(void*)0);
  else 
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_CONTENTS    , XHTTP), (void*)_argv,(void*)strlen(_argv));

  e = imtif->setMessage( imtif->h, (void*)MAKELONG(SYSTEM_CALLBACK, XHTTP),__http_callback, imtif);
  e = imtif->setMessage( imtif->h, (void*)MAKELONG(REQUEST,XHTTP), (void*)0,(void*)0);
  return e;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __http_close(void** h)
{
  int32_t e = 0;
  IMTIF* imtif = 0;

  imtif = *h;

  if ( *h )
  {
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(DISABLE,XHTTP),0,0);
    if ( imtif->pb )
    {
      free(imtif->pb);
      imtif->pb = 0;
    }
    e = imtif->setMessage(&imtif->h, (void*)MAKELONG(RELEASE,XHTTP),0,0);
    e = imtif->setMessage(&imtif->hdl, (void*)MAKELONG(RELEASE,MTIF), 0, 0);
    unmodulus(imtif);
    free(*h);
    *h = 0;
  }

  return e;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////                                                                   //////
//////                                                                   //////
//////                              HTTPD                                //////
//////                                                                   //////
//////                                                                   //////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*****************************************************************************/
/*****************************************************************************/
/*********************************        ************************************/
/*********************************        ************************************/
/*********************************        ************************************/
/*****************************                ********************************/
/*******************************            **********************************/
/*********************************        ************************************/
/************************************   **************************************/
/************************************* ***************************************/
/*****************************************************************************/
#if __CUSTOM_X_HTTPD__
void* __httpd_callback(void* h, void* msg, void* wparam, void* lparam)
{
  int32_t e = 0;
  IMTIF* imtif = (IMTIF*)h;
  int32_t fd=0;

  switch(LOWORD((uint32_t)msg))
  {
  case READ:
    if(imtif->callback[IMTIF_CALLBACK_READ]) e = imtif->callback[IMTIF_CALLBACK_READ](imtif->h, imtif->cbi.fd, (int8_t*)wparam, (int32_t)lparam, 0, imtif->obj);
    break;
  case FDSCRPTR:
    imtif->cbi.fd = (int32_t)wparam;
    break;
  case HTTP_URI:
    if ( imtif->callback[IMTIF_CALLBACK_URI] ) e = imtif->callback[IMTIF_CALLBACK_URI](imtif->h, imtif->cbi.fd, (int8_t*)wparam, (int32_t)lparam, 0, imtif->obj);
    break;
	case METHOD_GET:
    if ( imtif->callback[IMTIF_CALLBACK_GET] ) e = imtif->callback[IMTIF_CALLBACK_GET](imtif->h, imtif->cbi.fd, (int8_t*)wparam, (int32_t)lparam, 0, imtif->obj);
		break;
	case METHOD_POST:
    if ( imtif->callback[IMTIF_CALLBACK_POST] ) e = imtif->callback[IMTIF_CALLBACK_POST](imtif->h, imtif->cbi.fd, (int8_t*)wparam, (int32_t)lparam, 0, imtif->obj);
		break;

  default:
    switch ( LOWORD((uint32_t)msg)|0xE0000000 )
    {
    case 0xE000FD0B:
      break;
    case 0xE000FD0F:
      if(imtif->callback[IMTIF_CALLBACK_DISCONNECTED]) e = imtif->callback[IMTIF_CALLBACK_DISCONNECTED](imtif->h, imtif->cbi.fd, (int8_t*)wparam, (int32_t)lparam, 0, imtif->obj);
      break;
    case 0xE000FD0A:
      if(imtif->callback[IMTIF_CALLBACK_CONNECTED]) e = imtif->callback[IMTIF_CALLBACK_CONNECTED](imtif->h, imtif->cbi.fd, (int8_t*)wparam, (int32_t)lparam, 0, imtif->obj);
      break;
    case 0xE000100B:
      break;
    case 0xE000100A:
      break;
    case 0xE000100F:
      break;
    case 0xE000FD1B:
      break;
    case 0xE000FD1A:
      break;
    case 0xE000101B:
      break;
    case 0xE000101A:
      break;
    case 0xE000101F:
      break;
    case 0xE000FDFB:
      break;
    case 0xE000FDFA:
      if(imtif->callback[IMTIF_CALLBACK_DISCONNECTED]) imtif->callback[IMTIF_CALLBACK_DISCONNECTED](imtif->h, imtif->cbi.fd, (int8_t*)wparam, (int32_t)lparam, 0, imtif->obj);
      break;
    case 0xE000FDFF:
      break;
    case 0xE00010B0:
      break;
    case 0xE00010F0: //IO Function Output
      break;
    case 0xE00010F1: //IO Function Input
      break;
    }
    break;
  }
  return e;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __httpd_fdset(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  IMTIF* imtif = (IMTIF*)h;
	return imtif->getMessage(imtif->h, MAKELONG(FDSCRPTRS, XHTTPD), (void*)moreinfo,0);
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __httpd_fd(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  IMTIF* imtif = (IMTIF*)h;
	imtif->getMessage(imtif->h, MAKELONG(FDSCRPTR, XHTTPD), (void*)moreinfo,0);
  return *(int32_t*)(moreinfo);
}


#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __httpd_read(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  int32_t e = 0;
  IMTIF* imtif = (IMTIF*)h;

  e = imtif->setMessage(imtif->h, (void*)MAKELONG(FDSCRPTR, XHTTPD), (void*)fd, (void*)0);
  e = imtif->setMessage(imtif->h, (void*)MAKELONG(READ, XHTTPD), (void*)b, (void*)sz);
  return e;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __httpd_write(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  int32_t e = 0;
  IMTIF* imtif = (IMTIF*)h;

  e = imtif->setMessage(imtif->h, (void*)MAKELONG(FDSCRPTR, XHTTPD), (void*)fd, (void*)0);
  e = imtif->setMessage(imtif->h, (void*)MAKELONG(WRITE, XHTTPD), (void*)b, (void*)sz);
  return e;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __httpd_open(void** h, int8_t* argv, int32_t (*f[])(void*,int32_t,int8_t*,int32_t,void*,void*), void* o)
{
  int8_t  _argv[128] = {0};
  int32_t e = 0;
  IMTIF* imtif = 0;
  int8_t  _iargv[4096] = {0};
  int8_t  _iap = 0;

  if ( *h ) return 0xEFFFFFFF;
  *h = imtif = (IMTIF*)calloc(1, sizeof(IMTIF));

  for ( e=0 ; e<IMTIF_CALLBACK ; e++ )
  {
    if ( *f[e] ) imtif->callback[e] = f[e];
  }
  imtif->obj = o;

  e = NJSON_STR(argv, "SYSTEM_LIBRARY", _argv);
  if ( e > 0 ) sprintf(&_iargv[_iap], "%s\n", _argv);
  else sprintf(&_iargv[_iap], "\n", _argv);
  _iap = strlen(_iargv);

  if ( modulus(imtif, _argv) < 0 ) return 0xEF000001;

  e = NJSON_STR(argv, "SECRET_KEY_0", _argv);
  if ( e > 0 ) sprintf(&_iargv[_iap], "%s\n", _argv);
  else sprintf(&_iargv[_iap], "\n", _argv);
  _iap = strlen(_iargv);

  e = NJSON_STR(argv, "SECRET_KEY_1", _argv);
  if ( e > 0 ) sprintf(&_iargv[_iap], "%s\n", _argv);
  else sprintf(&_iargv[_iap], "\n", _argv);
  _iap = strlen(_iargv);

  e = imtif->setMessage(&imtif->hdl, (void*)MAKELONG(INIT,MTIF), _iargv, 0);

  e = imtif->setMessage(&imtif->h, (void*)MAKELONG(INIT           , XHTTPD), (void*)imtif->hdl, 0);
  e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_DELAY_0     , XHTTPD), (void*)"3000",(void*)strlen("XXX"));
  e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_ECHOMODE    , XHTTPD), (void*)"0",(void*)strlen("X"));
  e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_KEY,XHTTPD), (void*)"shinbaad@gmail.com",(void*)strlen("XXXXXXXXXXXXXXXXXX"));

  ///////   {"IP":"192.168.0.2","PORT":"7887","CSTYPE":"CLIENT","PROTOCOL":"TCP","CASTTYPE":"UNICAST"}
  if ( NJSON_STR(argv, "BUF_SZ", _argv)<0 )
  {
    imtif->pb = (int8_t*)calloc(__BUF_SZ, sizeof(int8_t));
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_BUFFER_PTR,XHTTPD), (void*)imtif->pb,(void*)__BUF_SZ);
  }
  else
  {
    imtif->pb = (int8_t*)calloc(atoi(_argv), sizeof(int8_t));
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_BUFFER_PTR,XHTTPD), (void*)imtif->pb,(void*)atoi(_argv));
  }

  if ( NJSON_STR(argv, "IP", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_IP,XHTTPD), (void*)"127.0.0.1",(void*)strlen("XXXXXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_IP,XHTTPD), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "PORT", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_PORT, XHTTPD), (void*)"80",(void*)strlen("XXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_PORT, XHTTPD), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "CSTYPE", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_CSTYPE, XHTTPD), (void*)"SERVER",(void*)strlen("XXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_CSTYPE, XHTTPD), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "MAX_FD", _argv)<0 )
  {
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_MAX_FD, XHTTPD), (void*)"32",(void*)strlen("XX"));
    imtif->icd.fdset.max = atoi("32");
  }
  else
  {
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_MAX_FD, XHTTPD), (void*)_argv,(void*)strlen(_argv));
    imtif->icd.fdset.max = atoi(_argv);
  }

  if ( NJSON_STR(argv, "SYNC", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_SYNC, XHTTPD), (void*)"DISABLE",(void*)strlen("XXXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_SYNC, XHTTPD), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "BLOCK", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_BLOCK, XHTTPD), (void*)"DISABLE",(void*)strlen("XXXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_BLOCK, XHTTPD), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "TIMEOUT", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_TIMEOUT, XHTTPD), (void*)"0",(void*)strlen("X"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_TIMEOUT, XHTTPD), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "PROTOCOL", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_PROTO, XHTTPD), (void*)"TCP",(void*)strlen("XXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_PROTO, XHTTPD), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "CASTTYPE", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_CASTTYPE, XHTTPD), (void*)"UNICAST",(void*)strlen("XXXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_CASTTYPE, XHTTPD), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "HOME", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_HOME, XHTTPD), (void*)"/",(void*)strlen("X"));
  else 
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_HOME, XHTTPD), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "INDEX", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_INDEX, XHTTPD), (void*)"index.html",(void*)strlen("XXXXXXXXXX"));
  else 
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_INDEX, XHTTPD), (void*)_argv,(void*)strlen(_argv));

  e = imtif->setMessage( imtif->h, (void*)MAKELONG(SYSTEM_CALLBACK,XHTTPD),__httpd_callback, imtif);
  imtif->fd = e = imtif->setMessage( imtif->h, (void*)MAKELONG(ENABLE,XHTTPD), 0,0);

  return e;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __httpd_close(void** h)
{
  int32_t e = 0;
  IMTIF* imtif = 0;

  imtif = *h;

  if ( *h )
  {
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(DISABLE,XHTTPD),0,0);
    if ( imtif->pb )
    {
      free(imtif->pb);
      imtif->pb = 0;
    }
    e = imtif->setMessage(&imtif->h, (void*)MAKELONG(RELEASE,XHTTPD),0,0);
    e = imtif->setMessage(&imtif->hdl, (void*)MAKELONG(RELEASE,MTIF), 0, 0);
    unmodulus(imtif);
    free(*h);
    *h = 0;
  }

  return e;
}
#endif
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////                                                                   //////
//////                                                                   //////
//////                              HTTPSD                               //////
//////                                                                   //////
//////                                                                   //////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*****************************************************************************/
/*****************************************************************************/
/*********************************        ************************************/
/*********************************        ************************************/
/*********************************        ************************************/
/*****************************                ********************************/
/*******************************            **********************************/
/*********************************        ************************************/
/************************************   **************************************/
/************************************* ***************************************/
/*****************************************************************************/
#if __CUSTOM_X_HTTPSD__
#if __SSL_TLS__
#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __httpsd_write(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  int32_t e =0;
  IMTIF* imtif = (IMTIF*)h;
  int32_t idx = (moreinfo==0)?0:(*(int32_t*)moreinfo);

  e = SSL_write(imtif->ossl.obio[idx].ssl, b, (int32_t)sz);

  return e;
}

void* __httpsd_accepter(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  IMTIF* imtif = (IMTIF*)h;
  int32_t e = 0;
  int32_t i = 0;
  int32_t n = 0;
  int32_t sts = 0;
  int32_t sfd = 0;

  return e;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
void* __httpsd_closer(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  int32_t e =0;
  IMTIF* imtif = (IMTIF*)h;
  int32_t idx = (moreinfo==0)?0:(*(int32_t*)moreinfo);
  int32_t i = 0;

  if ( imtif->ossl.obio[idx].ssl )
  {
    SSL_shutdown(imtif->ossl.obio[idx].ssl);
    SSL_free(imtif->ossl.obio[idx].ssl);
    imtif->ossl.obio[idx].ssl = 0;
  }

  #if DEBUG
  e = imtif->getMessage(imtif->h, (void*)MAKELONG(FDSCRPTRS, XHTTPD), &imtif->icd.fdset.fds, (void*)sizeof(imtif->icd.fdset.fds));
  printf("--------------------------------------------------------------------------------\r\n");
  printf("-                                                                              -\r\n");
  printf("-                   closer                                                     -\r\n");
  printf("-                                                                              -\r\n");
  printf("--------------------------------------------------------------------------------\r\n");
  for ( i=0 ; i<imtif->icd.fdset.max ; i++ )
  {
    printf("%8X", (imtif->icd.fdset.fds+i)->a[0]);
  }
  printf("\r\n--------------------------------------------------------------------------------\r\n\r\n");
  #endif
  return e;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
void* __httpsd_writer(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  int32_t e =0;
  IMTIF* imtif = (IMTIF*)h;
  int32_t idx = (moreinfo==0)?0:(*(int32_t*)moreinfo);

  if ( imtif->ossl.obio[idx].ssl == 0 ) return 0xEFFFFFFF;

  e = SSL_write(imtif->ossl.obio[idx].ssl, b, (int32_t)sz);
  #if DEBUG
  printf("\r\n\r\n\r\nSSL_write (%08X -> %d) \r\n%s\r\n\r\n\r\n\r\n", fd, e, b);
  #endif
  return e;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
void* __httpsd_reader(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  IMTIF* imtif = (IMTIF*)h;
  int32_t e = 0;
  int32_t idx = (moreinfo==0)?0:(*(int32_t*)moreinfo);
  int32_t sfd = 0;
  //int8_t  buf[4096] = {0};
  int8_t  raw[4096] = {0};
  int32_t n = 0;
  int32_t w=0,r=0;
  int32_t sts = 0;


  //printf("__httpsd_reader ----> \r\n");


  if ( sz <=0 ) return sz;
  if ( imtif->ossl.obio[idx].ssl == 0 ) return 0xEFFFFFFF;


  for ( ; ; )
  {
    e = SSL_read(imtif->ossl.obio[idx].ssl, b, sz);
    if ( e > 0 )
    {
      *(b+e) = 0;
      #if DEBUG
      printf("%s\r\n", b);
      #endif
      break;
    }
    else
    {
      r = SSL_get_error(imtif->ossl.obio[idx].ssl, e);// == SSL_ERROR_WANT_READ )

      #if DEBUG
      printf("SSL_read --> %4d, %4d, %4d, %4d, %s\r\n", e, r, ERR_get_error(), errno, strerror(errno));
      #endif

      if ( (r==SSL_ERROR_WANT_READ) || (r==SSL_ERROR_WANT_WRITE) )
      {
        #if DEBUG
        printf("continue \r\n");
        #endif
        continue;
      }
      else
      //if ( (r==SSL_ERROR_SSL) || (r==SSL_ERROR_SYSCALL) )
      {
        e = -1;
        #if DEBUG
        printf("break; \r\n");
        #endif
        break;
      }

    }
  }
  return e;
}


#if 0//defined XWIN32
#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
void* __httpsd_accept(void* h, void* msg, void* wparam, void* lparam)
{
  IMTIF* imtif = (IMTIF*)h;
  int32_t e = 0;
  int32_t i = 0;
  int32_t n = 0;
  int32_t sts = 0;
  int32_t sfd = 0;
  int32_t idx = (lparam==0)?0:(*(int32_t*)lparam);

  int8_t buf[4096] = {0};


  #if DEBUG
  e = imtif->getMessage(imtif->h, (void*)MAKELONG(FDSCRPTRS, XHTTPD), &imtif->icd.fdset.fds, (void*)sizeof(imtif->icd.fdset.fds));
  printf("--------------------------------------------------------------------------------\r\n");
  printf("-                                                                              -\r\n");
  printf("-                   accept (%8X)                                          -\r\n", (int32_t)wparam);
  printf("-                                                                              -\r\n");
  printf("--------------------------------------------------------------------------------\r\n");
  for ( i=0 ; i<imtif->icd.fdset.max ; i++ )
  {
    if ( i && (i%4==0) ) printf("\r\n");
    printf("%8X", (imtif->icd.fdset.fds+i)->a[0]);
  }
  printf("\r\n--------------------------------------------------------------------------------\r\n\r\n");
  #endif

  imtif->ossl.obio[idx].ssl = SSL_new(imtif->ossl.ctx);
  SSL_set_fd(imtif->ossl.obio[idx].ssl, (int32_t)wparam);

  //SSL_set_accept_state(imtif->ossl.obio[idx].ssl);

  e = SSL_accept(imtif->ossl.obio[idx].ssl);

  return e;
}
#endif
#if 1// defined LINUX
#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
void* __httpsd_accept(void* h, void* msg, void* wparam, void* lparam)
{
  IMTIF* imtif = (IMTIF*)h;
  int32_t e = 0;
  int32_t i = 0;
  int32_t n = 0;
  int32_t sts = 0;
  int32_t sfd = 0;
  int32_t idx = (lparam==0)?0:(*(int32_t*)lparam);


  #if DEBUG
  e = imtif->getMessage(imtif->h, (void*)MAKELONG(FDSCRPTRS, XHTTPD), &imtif->icd.fdset.fds, (void*)sizeof(imtif->icd.fdset.fds));
  printf("--------------------------------------------------------------------------------\r\n");
  printf("-                                                                              -\r\n");
  printf("-                   accept (%8X)                                          -\r\n", (int32_t)wparam);
  printf("-                                                                              -\r\n");
  printf("--------------------------------------------------------------------------------\r\n");
  for ( i=0 ; i<imtif->icd.fdset.max ; i++ )
  {
    if ( i && (i%4==0) ) printf("\r\n");
    printf("%8X", (imtif->icd.fdset.fds+i)->a[0]);
  }
  printf("\r\n--------------------------------------------------------------------------------\r\n\r\n");
  #endif

  imtif->ossl.obio[idx].ssl = SSL_new(imtif->ossl.ctx);
  SSL_set_fd(imtif->ossl.obio[idx].ssl, (int32_t)wparam);

  SSL_set_accept_state(imtif->ossl.obio[idx].ssl);

  e = SSL_accept(imtif->ossl.obio[idx].ssl);
  //printf("SSL_accept -----> %d, %d, %s\r\n", e, SSL_get_error(imtif->ossl.obio[idx].ssl, e), ERR_error_string(e, 0));

  return e;
}
#endif


#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __httpsd_pre_open(void* h, int8_t* argv)
{
  int32_t e = 0;
  IMTIF* imtif = (IMTIF*)h;
  int8_t  _argv[2][2048] = {0};

  //#if defined LINUX
  //if( !SSL_library_init() ) return 0xEFFFFFFF;
  //SSL_load_error_strings();
  //OpenSSL_add_all_algorithms();
  //#endif

  imtif->ossl.mthd = TLS_server_method();
                  //TLSv1_client_method();//SSLv1_client_method();
  imtif->ossl.ctx = SSL_CTX_new(imtif->ossl.mthd);
  SSL_CTX_set_verify(imtif->ossl.ctx, SSL_VERIFY_NONE, 0);
  SSL_CTX_set_cipher_list(imtif->ossl.ctx, "ALL:eNULL");


  if ( NJSON_STR(argv, "CERTFILE", _argv[0])<0 ) return 0xEFFFFFFF;
  if ( NJSON_STR(argv, "KEYFILE", _argv[1])<0 ) return 0xEFFFFFFF;
  e = SSL_CTX_load_verify_locations(imtif->ossl.ctx, _argv[0], _argv[1]);
  if ( e != 1 ) return 0xEFFFFFFF;
  e = SSL_CTX_set_default_verify_paths(imtif->ossl.ctx);
  if ( e != 1 ) return 0xEFFFFFFF;
  e = SSL_CTX_use_certificate_file(imtif->ossl.ctx, _argv[0], SSL_FILETYPE_PEM);
  if ( e <= 0 ) return 0xEFFFFFFF;
  e = SSL_CTX_use_PrivateKey_file(imtif->ossl.ctx, _argv[1], SSL_FILETYPE_PEM);
  if ( e <= 0 ) return 0xEFFFFFFF;

  return e;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __httpsd_pre_close(void* h)
{
  int32_t e = 0;
  IMTIF* imtif = (IMTIF*)h;

  SSL_CTX_free(imtif->ossl.ctx);
  //ERR_remove_state(0);
  //EVP_cleanup();
  sk_SSL_COMP_free(SSL_COMP_get_compression_methods());
  //CRYPTO_cleanup_all_ex_data();
  return e;
}

void ShowCerts(SSL* ssl)
{
  X509 *cert;
  char *line;

  cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
  if ( cert != NULL )
  {
    //printf("Server certificates:\n");
    line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
    //printf("Subject: %s\n", line);
    free(line);
    line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
    //printf("Issuer: %s\n", line);
    free(line);
    X509_free(cert);
  }
  else
  {
    //printf("No certificates.\n");
  }
}


void* __httpsd_callback(void* h, void* msg, void* wparam, void* lparam)
{
  IMTIF* imtif = (IMTIF*)h;
  int32_t fd=0;
  int32_t e = 0;
  int32_t i = 0;

  switch(LOWORD((uint32_t)msg))
  {
  case READ:
    if ( imtif->callback[IMTIF_CALLBACK_READ] ) e = imtif->callback[IMTIF_CALLBACK_READ](imtif->h, imtif->cbi.fd, (int8_t*)wparam, (int32_t)lparam, 0, imtif->obj);
    break;
  case FDSCRPTR:
    imtif->cbi.fd = (int32_t)wparam;
    break;
	case METHOD_GET:
    if ( imtif->callback[IMTIF_CALLBACK_GET] ) e = imtif->callback[IMTIF_CALLBACK_GET](imtif->h, imtif->cbi.fd, (int8_t*)wparam, (int32_t)lparam, 0, imtif->obj);
		break;
	case METHOD_POST:
    if ( imtif->callback[IMTIF_CALLBACK_POST] ) e = imtif->callback[IMTIF_CALLBACK_POST](imtif->h, imtif->cbi.fd, (int8_t*)wparam, (int32_t)lparam, 0, imtif->obj);
		break;

  default:
    switch ( LOWORD((uint32_t)msg)|0xE0000000 )
    {
    case 0xE000FD0B:
      break;
    case 0xE000FD0F:
      break;
    case 0xE000FD0A:
      break;
    case 0xE000100B:
      break;
    case 0xE000100A:
      break;
    case 0xE000100F:
      break;
    case 0xE000FD1B:
      e = -1;
      break;
    case 0xE000FD1A:
      __httpsd_accept(h,msg,wparam,lparam);
      if ( imtif->callback[IMTIF_CALLBACK_CONNECTED] ) e = imtif->callback[IMTIF_CALLBACK_CONNECTED](imtif->h, (int32_t)wparam, 0, 0, 0, imtif->obj);
      break;
    case 0xE000101B:
      break;
    case 0xE000101A:
      break;
    case 0xE000101F:
      if ( imtif->callback[IMTIF_CALLBACK_DISCONNECTED] ) e = imtif->callback[IMTIF_CALLBACK_DISCONNECTED](imtif->h, (int32_t)wparam, 0, 0, 0, imtif->obj);
      #if DEBUG
      e = imtif->getMessage(imtif->h, (void*)MAKELONG(FDSCRPTRS, XHTTPD), &imtif->icd.fdset.fds, (void*)sizeof(imtif->icd.fdset.fds));
      printf("--------------------------------------------------------------------------------\r\n");
      printf("-                                                                              -\r\n");
      printf("-                   disconnected (%8X)                                    -\r\n", (int32_t)wparam);
      printf("-                                                                              -\r\n");
      printf("--------------------------------------------------------------------------------\r\n");
      for ( i=0 ; i<imtif->icd.fdset.max ; i++ )
      {
        printf("%8X", (imtif->icd.fdset.fds+i)->a[0]);
      }
      printf("\r\n--------------------------------------------------------------------------------\r\n\r\n");
      #endif
      break;

    case 0xE000FDFB:
      break;
    case 0xE000FDFA:
      break;
    case 0xE000FDFF:
      break;
    case 0xE00010B0:
      break;
    case 0xE00010F0: //IO Function Output
      break;
    case 0xE00010F1: //IO Function Input
      break;
    }
    break;
  }
  return (void*)e;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __httpsd_fdset(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  IMTIF* imtif = (IMTIF*)h;
	return imtif->getMessage(imtif->h, MAKELONG(FDSCRPTRS, XHTTPD), (void*)moreinfo,0);
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __httpsd_fd(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  IMTIF* imtif = (IMTIF*)h;
	imtif->getMessage(imtif->h, MAKELONG(FDSCRPTR, XHTTPD), (void*)moreinfo,0);
  return *(int32_t*)(moreinfo);
}


#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __httpsd_read(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  int32_t e = 0;
  IMTIF* imtif = (IMTIF*)h;

  e = imtif->setMessage(imtif->h, (void*)MAKELONG(FDSCRPTR, XHTTPD), (void*)fd, (void*)0);
  e = imtif->setMessage(imtif->h, (void*)MAKELONG(READ, XHTTPD), (void*)b, (void*)sz);
  return e;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __httpsd_open(void** h, int8_t* argv, int32_t (*f[])(void*,int32_t,int8_t*,int32_t,void*,void*), void* o)
{
  int8_t  _argv[2048] = {0};
  int32_t s = 0;
  int32_t e = 0;
  int32_t i = 0;
  IMTIF* imtif = 0;
  int8_t  _iargv[4096] = {0};
  int8_t  _iap = 0;

  if ( *h ) return 0xEFFFFFFF;
  *h = imtif = (IMTIF*)calloc(1, sizeof(IMTIF));

  for ( e=0 ; e<IMTIF_CALLBACK ; e++ )
  {
    if ( *f[e] ) imtif->callback[e] = f[e];
  }
  imtif->obj = o;

  e = NJSON_STR(argv, "SYSTEM_LIBRARY", _argv);
  if ( e > 0 ) sprintf(&_iargv[_iap], "%s\n", _argv);
  else sprintf(&_iargv[_iap], "\n", _argv);
  _iap = strlen(_iargv);

  #if DEBUG
  printf("%s\r\n", _argv);
  #endif

  if ( modulus(imtif, _argv) < 0 ) return 0xEF000001;

  e = NJSON_STR(argv, "SECRET_KEY_0", _argv);
  if ( e > 0 ) sprintf(&_iargv[_iap], "%s\n", _argv);
  else sprintf(&_iargv[_iap], "\n", _argv);
  _iap = strlen(_iargv);

  e = NJSON_STR(argv, "SECRET_KEY_1", _argv);
  if ( e > 0 ) sprintf(&_iargv[_iap], "%s\n", _argv);
  else sprintf(&_iargv[_iap], "\n", _argv);
  _iap = strlen(_iargv);

  e = imtif->setMessage(&imtif->hdl, (void*)MAKELONG(INIT,MTIF), _iargv, 0);

  e = imtif->setMessage(&imtif->h, (void*)MAKELONG(INIT           , XHTTPD), (void*)imtif->hdl, 0);
  e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_DELAY_0     , XHTTPD), (void*)"1000",(void*)strlen("XXXX"));
  e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_ECHOMODE    , XHTTPD), (void*)"0",(void*)strlen("X"));
  e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_KEY,XHTTPD), (void*)"shinbaad@gmail.com",(void*)strlen("XXXXXXXXXXXXXXXXXX"));


  ///////   {"IP":"192.168.0.2","PORT":"7887","CSTYPE":"CLIENT","PROTOCOL":"TCP","CASTTYPE":"UNICAST"}
  if ( NJSON_STR(argv, "BUF_SZ", _argv)<0 )
  {
    imtif->pb = (int8_t*)calloc(__BUF_SZ, sizeof(int8_t));
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_BUFFER_PTR,XHTTPD), (void*)imtif->pb,(void*)__BUF_SZ);
  }
  else
  {
    imtif->pb = (int8_t*)calloc(atoi(_argv), sizeof(int8_t));
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_BUFFER_PTR,XHTTPD), (void*)imtif->pb,(void*)atoi(_argv));
  }

  if ( NJSON_STR(argv, "IP", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_IP,XHTTPD), (void*)"127.0.0.1",(void*)strlen("XXXXXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_IP,XHTTPD), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "PORT", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_PORT, XHTTPD), (void*)"443",(void*)strlen("XXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_PORT, XHTTPD), (void*)_argv,(void*)strlen(_argv));




  if ( NJSON_STR(argv, "CSTYPE", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_CSTYPE, XHTTPD), (void*)"SERVER",(void*)strlen("XXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_CSTYPE, XHTTPD), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "MAX_FD", _argv)<0 )
  {
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_MAX_FD, XHTTPD), (void*)"16",(void*)strlen("XX"));
    imtif->icd.fdset.max = atoi("16");
  }
  else
  {
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_MAX_FD, XHTTPD), (void*)_argv,(void*)strlen(_argv));
    imtif->icd.fdset.max = atoi(_argv);
  }

  if ( NJSON_STR(argv, "SYNC", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_SYNC, XHTTPD), (void*)"DISABLE",(void*)strlen("XXXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_SYNC, XHTTPD), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "BLOCK", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_BLOCK, XHTTPD), (void*)"DISABLE",(void*)strlen("XXXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_BLOCK, XHTTPD), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "TIMEOUT", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_TIMEOUT, XHTTPD), (void*)"0",(void*)strlen("X"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_TIMEOUT, XHTTPD), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "PROTOCOL", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_PROTO, XHTTPD), (void*)"TCP",(void*)strlen("XXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_PROTO, XHTTPD), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "CASTTYPE", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_CASTTYPE, XHTTPD), (void*)"UNICAST",(void*)strlen("XXXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_CASTTYPE, XHTTPD), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "HOME", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_HOME, XHTTPD), (void*)"/",(void*)strlen("X"));
  else 
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_HOME, XHTTPD), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "INDEX", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_INDEX, XHTTPD), (void*)"index.html",(void*)strlen("XXXXXXXXXX"));
  else 
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_INDEX, XHTTPD), (void*)_argv,(void*)strlen(_argv));

  e = imtif->setMessage( imtif->h, (void*)MAKELONG(SYSTEM_CALLBACK,XHTTPD),__httpsd_callback, imtif);

  e = imtif->setMessage( imtif->h, (void*)MAKELONG(READER,XHTTPD),__httpsd_reader, imtif);
  e = imtif->setMessage( imtif->h, (void*)MAKELONG(WRITER,XHTTPD),__httpsd_writer, imtif);
  e = imtif->setMessage( imtif->h, (void*)MAKELONG(IO_CLOSER,XHTTPD),__httpsd_closer, imtif);
  //e = imtif->setMessage( imtif->h, (void*)MAKELONG(IO_ACCEPTER,XHTTPD),__httpsd_accepter, imtif);


  if ( !__httpsd_pre_open(imtif, argv) ) return 0xEFFFFFF0;

  #if DEBUG
  printf("__httpsd_pre_open success\r\n");
  #endif
  imtif->fd = e = imtif->setMessage( imtif->h, (void*)MAKELONG(ENABLE,XHTTPD), 0,0);

  return e;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __httpsd_close(void** h)
{
  int32_t i = 0;
  int32_t e = 0;
  IMTIF* imtif = 0;

  imtif = *h;

  __httpsd_pre_close(*h);
  if ( *h )
  {
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(DISABLE,XHTTPD),0,0);
    if ( imtif->pb )
    {
      free(imtif->pb);
      imtif->pb = 0;
    }
    e = imtif->setMessage(&imtif->h, (void*)MAKELONG(RELEASE,XHTTPD),0,0);
    e = imtif->setMessage(&imtif->hdl, (void*)MAKELONG(RELEASE,MTIF), 0, 0);
    unmodulus(imtif);
    free(*h);
    *h = 0;
  }

  return e;
}
#endif
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////                                                                   //////
//////                                                                   //////
//////                           WebSocket                               //////
//////                                                                   //////
//////                                                                   //////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*****************************************************************************/
/*****************************************************************************/
/*********************************        ************************************/
/*********************************        ************************************/
/*********************************        ************************************/
/*****************************                ********************************/
/*******************************            **********************************/
/*********************************        ************************************/
/************************************   **************************************/
/************************************* ***************************************/
/*****************************************************************************/
#if __CUSTOM_X_WS__
void* __ws_callback(void* h, void* msg, void* wparam, void* lparam)
{
  IMTIF* imtif = (IMTIF*)h;
  int32_t fd=0;
  int32_t e=0;

  switch(LOWORD((uint32_t)msg))
  {
  case READ:
    if(imtif->callback[IMTIF_CALLBACK_READ]) e = imtif->callback[IMTIF_CALLBACK_READ](imtif->h, imtif->cbi.fd, (int8_t*)wparam, (int32_t)lparam, 0, imtif->obj);
    break;
  case FDSCRPTR:
    imtif->cbi.fd = (int32_t)wparam;
    break;

  default:
    switch ( LOWORD((uint32_t)msg)|0xE0000000 )
    {
    case 0xE000FD0B:
      break;
    case 0xE000FD0F:
      break;
    case 0xE000FD0A:
      break;
    case 0xE000100B:
      break;
    case 0xE000100A:
      break;
    case 0xE000100F:
      break;
    case 0xE000FD1B:
      break;
    case 0xE000FD1A:
      if(imtif->callback[IMTIF_CALLBACK_CONNECTED]) e = imtif->callback[IMTIF_CALLBACK_CONNECTED](imtif->h,(int32_t)wparam, 0, 0, lparam, imtif->obj);
      break;
    case 0xE000101B:
      break;
    case 0xE000101A:
      break;
    case 0xE000101F:
      if(imtif->callback[IMTIF_CALLBACK_DISCONNECTED]) e = imtif->callback[IMTIF_CALLBACK_DISCONNECTED](imtif->h, (int32_t)wparam, 0, 0, lparam, imtif->obj);
      break;
    case 0xE000FDFB:
      break;
    case 0xE000FDFA:
      if(imtif->callback[IMTIF_CALLBACK_DISCONNECTED]) e = imtif->callback[IMTIF_CALLBACK_DISCONNECTED](imtif->h, (int32_t)wparam, 0, 0, lparam, imtif->obj);
      break;
    case 0xE000FDFF:
      break;
    case 0xE00010B0:
      break;
    case 0xE00010F0: //IO Function Output
      break;
    case 0xE00010F1: //IO Function Input
      break;
    }
    break;
  }
  return 0;
}


#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __ws_fdset(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  IMTIF* imtif = (IMTIF*)h;
	return imtif->getMessage(imtif->h, MAKELONG(FDSCRPTRS, XWEBSOCKET), (void*)moreinfo,0);
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __ws_fd(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  IMTIF* imtif = (IMTIF*)h;
	imtif->getMessage(imtif->h, MAKELONG(FDSCRPTR, XWEBSOCKET), (void*)moreinfo,0);
  return *(int32_t*)(moreinfo);
}



#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __ws_read(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  int32_t e = 0;
  IMTIF* imtif = (IMTIF*)h;

  e = imtif->setMessage(imtif->h, (void*)MAKELONG(FDSCRPTR, XWEBSOCKET), (void*)fd, (void*)0);
  e = imtif->setMessage(imtif->h, (void*)MAKELONG(WRITE, XWEBSOCKET), (void*)b, (void*)sz);
  return e;
}



#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __ws_write(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  int32_t e = 0;
  IMTIF* imtif = (IMTIF*)h;

  e = imtif->setMessage(imtif->h, (void*)MAKELONG(FDSCRPTR, XWEBSOCKET), (void*)fd, (void*)0);
  e = imtif->setMessage(imtif->h, (void*)MAKELONG(WRITE, XWEBSOCKET), (void*)b, (void*)sz);
  return e;
}


#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __ws_open(void** h, int8_t* argv, int32_t (*f[])(void*,int32_t,int8_t*,int32_t,void*,void*), void* o)
{
  int8_t  _argv[128] = {0};
  int32_t e = 0;
  IMTIF* imtif = 0;
  int8_t  _iargv[4096] = {0};
  int8_t  _iap = 0;

  if ( *h ) return 0xEFFFFFFF;
  *h = imtif = (IMTIF*)calloc(1, sizeof(IMTIF));

  for ( e=0 ; e<IMTIF_CALLBACK ; e++ )
  {
    if ( *f[e] ) imtif->callback[e] = f[e];
  }
  imtif->obj = o;

  e = NJSON_STR(argv, "SYSTEM_LIBRARY", _argv);
  if ( e > 0 ) sprintf(&_iargv[_iap], "%s\n", _argv);
  else sprintf(&_iargv[_iap], "\n", _argv);
  _iap = strlen(_iargv);

  if ( modulus(imtif, _argv) < 0 ) return 0xEF000001;

  e = NJSON_STR(argv, "SECRET_KEY_0", _argv);
  if ( e > 0 ) sprintf(&_iargv[_iap], "%s\n", _argv);
  else sprintf(&_iargv[_iap], "\n", _argv);
  _iap = strlen(_iargv);

  e = NJSON_STR(argv, "SECRET_KEY_1", _argv);
  if ( e > 0 ) sprintf(&_iargv[_iap], "%s\n", _argv);
  else sprintf(&_iargv[_iap], "\n", _argv);
  _iap = strlen(_iargv);

  e = imtif->setMessage(&imtif->hdl, (void*)MAKELONG(INIT,MTIF), _iargv, 0);

  e = imtif->setMessage(&imtif->h, (void*)MAKELONG(INIT           ,   XWEBSOCKET), (void*)imtif->hdl,0);
  e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_DELAY_0     , XWEBSOCKET), (void*)"3000",(void*)strlen("XXX"));
  e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_ECHOMODE    , XWEBSOCKET), (void*)"0",(void*)strlen("X"));
  e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_KEY,XWEBSOCKET), (void*)"shinbaad@gmail.com",(void*)strlen("XXXXXXXXXXXXXXXXXX"));

  if ( NJSON_STR(argv, "BUF_SZ", _argv)<0 )
  {
    imtif->pb = (int8_t*)calloc(__BUF_SZ, sizeof(int8_t));
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_BUFFER_PTR,XWEBSOCKET), (void*)imtif->pb,(void*)__BUF_SZ);
  }
  else
  {
    imtif->pb = (int8_t*)calloc(atoi(_argv), sizeof(int8_t));
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_BUFFER_PTR,XWEBSOCKET), (void*)imtif->pb,(void*)atoi(_argv));
  }


  if ( NJSON_STR(argv, "IP", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_IP,XWEBSOCKET), (void*)"127.0.0.1",(void*)strlen("XXXXXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_IP,XWEBSOCKET), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "PORT", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_PORT, XWEBSOCKET), (void*)"80",(void*)strlen("XX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_PORT, XWEBSOCKET), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "CSTYPE", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_CSTYPE, XWEBSOCKET), (void*)"SERVER",(void*)strlen("XXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_CSTYPE, XWEBSOCKET), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "MAX_FD", _argv)<0 )
  {
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_MAX_FD, XWEBSOCKET), (void*)"32",(void*)strlen("XX"));
    imtif->icd.fdset.max = atoi("32");
  }
  else
  {
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_MAX_FD, XWEBSOCKET), (void*)_argv,(void*)strlen(_argv));
    imtif->icd.fdset.max = atoi(_argv);
  }

  if ( NJSON_STR(argv, "SYNC", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_SYNC, XWEBSOCKET), (void*)"DISABLE",(void*)strlen("XXXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_SYNC, XWEBSOCKET), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "BLOCK", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_BLOCK, XWEBSOCKET), (void*)"DISABLE",(void*)strlen("XXXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_BLOCK, XWEBSOCKET), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "TIMEOUT", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_TIMEOUT, XWEBSOCKET), (void*)"0",(void*)strlen("X"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_TIMEOUT, XWEBSOCKET), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "PROTOCOL", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_PROTO, XWEBSOCKET), (void*)"TCP",(void*)strlen("XXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_PROTO, XWEBSOCKET), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "CASTTYPE", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_CASTTYPE, XWEBSOCKET), (void*)"UNICAST",(void*)strlen("XXXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_CASTTYPE, XWEBSOCKET), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "URL", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_URL         , XWEBSOCKET), (void*)"http://127.0.0.1",(void*)strlen("XXXXXXXXXXXXXXXX"));
  else 
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_URL         , XWEBSOCKET), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "URI", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_URI         ,   XWEBSOCKET), (void*)"/",(void*)strlen("/"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_URI         ,   XWEBSOCKET), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "CONTENTS", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_CONTENTS    , XWEBSOCKET), (void*)"",(void*)strlen(""));
  else 
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_CONTENTS    , XWEBSOCKET), (void*)_argv,(void*)strlen(_argv));

    e = imtif->setMessage( imtif->h, (void*)MAKELONG(SYSTEM_CALLBACK,   XWEBSOCKET),__ws_callback, imtif);

  imtif->fd = e = imtif->setMessage( imtif->h, (void*)MAKELONG(ENABLE,XWEBSOCKET),0,0);

  return e;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __ws_close(void** h)
{
  int32_t e = 0;
  IMTIF* imtif = 0;

  imtif = *h;

  if ( *h )
  {
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(DISABLE,XWEBSOCKET),0,0);
    if ( imtif->pb )
    {
      free(imtif->pb);
      imtif->pb = 0;
    }
    e = imtif->setMessage(&imtif->h, (void*)MAKELONG(RELEASE,XWEBSOCKET),0,0);
    e = imtif->setMessage(&imtif->hdl, (void*)MAKELONG(RELEASE,MTIF), 0, 0);
    unmodulus(imtif);
    free(*h);
    *h = 0;
  }

  return e;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////                                                                   //////
//////                                                                   //////
//////                             Serial                                //////
//////                                                                   //////
//////                                                                   //////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*****************************************************************************/
/*****************************************************************************/
/*********************************        ************************************/
/*********************************        ************************************/
/*********************************        ************************************/
/*****************************                ********************************/
/*******************************            **********************************/
/*********************************        ************************************/
/************************************   **************************************/
/************************************* ***************************************/
/*****************************************************************************/
#if __CUSTOM_X_SERIAL__
void* __serial_callback(void* h, void* msg, void* wparam, void* lparam)
{
  IMTIF* imtif = (IMTIF*)h;
  int32_t fd=0;
  int32_t e=0;

  switch(LOWORD((uint32_t)msg))
  {
  case READ:
    if(imtif->callback[IMTIF_CALLBACK_READ]) e = imtif->callback[IMTIF_CALLBACK_READ](imtif->h, imtif->cbi.fd, (int8_t*)wparam, (int32_t)lparam, 0, imtif->obj);
    break;
  case FDSCRPTR:
    imtif->cbi.fd = (int32_t)wparam;
    break;

  default:
    switch ( LOWORD((uint32_t)msg)|0xE0000000 )
    {
    case 0xE000FD0B:
      break;
    case 0xE000FD0F:
      if(imtif->callback[IMTIF_CALLBACK_DISCONNECTED]) e = imtif->callback[IMTIF_CALLBACK_DISCONNECTED](imtif->h, (int32_t)wparam, 0, 0, 0, imtif->obj);
      break;
    case 0xE000FD0A:
      if(imtif->callback[IMTIF_CALLBACK_CONNECTED]) e = imtif->callback[IMTIF_CALLBACK_CONNECTED](imtif->h, (int32_t)wparam, 0, 0, 0, imtif->obj);
      break;
    case 0xE000100B:
      break;
    case 0xE000100A:
      break;
    case 0xE000100F:
      break;
    case 0xE000FD1B:
      break;
    case 0xE000FD1A:
      break;
    case 0xE000101B:
      break;
    case 0xE000101A:
      break;
    case 0xE000101F:
      break;
    case 0xE000FDFB:
      break;
    case 0xE000FDFA:
      if(imtif->callback[IMTIF_CALLBACK_DISCONNECTED]) e = imtif->callback[IMTIF_CALLBACK_DISCONNECTED](imtif->h, (int32_t)wparam, 0, 0, 0, imtif->obj);
      break;
    case 0xE000FDFF:
      break;
    case 0xE00010B0:
      break;
    case 0xE00010F0: //IO Function Output
      break;
    case 0xE00010F1: //IO Function Input
      break;
    }
    break;
  }
  return 0;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __serial_fd(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  IMTIF* imtif = (IMTIF*)h;
	imtif->getMessage(imtif->h, MAKELONG(FDSCRPTR, XSERIAL), (void*)moreinfo,0);
  return *(int32_t*)(moreinfo);
}


#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __serial_read(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  int32_t e = 0;
  IMTIF* imtif = (IMTIF*)h;

  e = imtif->setMessage(imtif->h, (void*)MAKELONG(FDSCRPTR, XSERIAL), (void*)fd, (void*)0);
  e = imtif->setMessage(imtif->h, (void*)MAKELONG(READ, XSERIAL), (void*)b, (void*)sz);
  return e;
}


#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __serial_write(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  int32_t e = 0;
  IMTIF* imtif = (IMTIF*)h;

  e = imtif->setMessage(imtif->h, (void*)MAKELONG(FDSCRPTR, XSERIAL), (void*)fd, (void*)0);
  e = imtif->setMessage(imtif->h, (void*)MAKELONG(WRITE, XSERIAL), (void*)b, (void*)sz);
  return e;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __serial_open(void** h, int8_t* argv, int32_t (*f[])(void*,int32_t,int8_t*,int32_t,void*,void*), void* o)
{
  int8_t  _argv[128] = {0};
  int32_t e = 0;
  IMTIF* imtif = 0;
  int8_t  _iargv[4096] = {0};
  int8_t  _iap = 0;

  if ( *h ) return 0xEFFFFFFF;
  *h = imtif = (IMTIF*)calloc(1, sizeof(IMTIF));

  for ( e=0 ; e<IMTIF_CALLBACK ; e++ )
  {
    if ( *f[e] ) imtif->callback[e] = f[e];
  }
  imtif->obj = o;

  e = NJSON_STR(argv, "SYSTEM_LIBRARY", _argv);
  if ( e > 0 ) sprintf(&_iargv[_iap], "%s\n", _argv);
  else sprintf(&_iargv[_iap], "\n", _argv);
  _iap = strlen(_iargv);

  if ( modulus(imtif, _argv) < 0 ) return 0xEF000001;

  e = NJSON_STR(argv, "SECRET_KEY_0", _argv);
  if ( e > 0 ) sprintf(&_iargv[_iap], "%s\n", _argv);
  else sprintf(&_iargv[_iap], "\n", _argv);
  _iap = strlen(_iargv);

  e = NJSON_STR(argv, "SECRET_KEY_1", _argv);
  if ( e > 0 ) sprintf(&_iargv[_iap], "%s\n", _argv);
  else sprintf(&_iargv[_iap], "\n", _argv);
  _iap = strlen(_iargv);

  e = imtif->setMessage(&imtif->hdl, (void*)MAKELONG(INIT,MTIF), _iargv, 0);

  e = imtif->setMessage(&imtif->h, (void*)MAKELONG(INIT,XSERIAL),(void*)imtif->hdl,0);
  e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_DELAY_0,XSERIAL), (void*)"3000",(void*)strlen("3000"));
  e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_ECHOMODE    , XSERIAL), (void*)"0",(void*)strlen("X"));
  e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_KEY,XSERIAL), (void*)"shinbaad@gmail.com",(void*)strlen("shinbaad@gmail.com"));

  if ( NJSON_STR(argv, "BUF_SZ", _argv)<0 )
  {
    imtif->pb = (int8_t*)calloc(__BUF_SZ, sizeof(int8_t));
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_BUFFER_PTR,XSERIAL), (void*)imtif->pb,(void*)__BUF_SZ);
  }
  else
  {
    imtif->pb = (int8_t*)calloc(atoi(_argv), sizeof(int8_t));
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_BUFFER_PTR,XSERIAL), (void*)imtif->pb,(void*)atoi(_argv));
  }

  if ( NJSON_STR(argv, "PORT", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_PORT,XSERIAL), (void*)__SYSTEM_SERIAL_PORT__,(void*)strlen(__SYSTEM_SERIAL_PORT__));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_PORT,XSERIAL), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "BAUDRATE", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_BAUDRATE,XSERIAL), (void*)"115200",(void*)strlen("XXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_BAUDRATE,XSERIAL), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "DATABIT", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_DATABIT,XSERIAL), (void*)"8",(void*)strlen("X"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_DATABIT,XSERIAL), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "STOPBIT", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_STOPBIT,XSERIAL), (void*)"1",(void*)strlen("X"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_STOPBIT,XSERIAL), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "PARITYBIT", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_PARITYBIT,XSERIAL), (void*)"0",(void*)strlen("X"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_PARITYBIT,XSERIAL), (void*)_argv,(void*)strlen(_argv));

  if ( NJSON_STR(argv, "SYNC", _argv)<0 )
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_SYNC, XSERIAL), (void*)"DISABLE",(void*)strlen("XXXXXXX"));
  else
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(XM_SYNC, XSERIAL), (void*)_argv,(void*)strlen(_argv));

  e = imtif->setMessage( imtif->h, (void*)MAKELONG(SYSTEM_CALLBACK,XSERIAL),__serial_callback, imtif);
  imtif->fd = e = imtif->setMessage( imtif->h, (void*)MAKELONG(ENABLE,XSERIAL), 0,0);

  return e;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __serial_close(void** h)
{
  int32_t e = 0;
  IMTIF* imtif = 0;

  imtif = *h;

  if ( *h )
  {
    e = imtif->setMessage( imtif->h, (void*)MAKELONG(DISABLE,XSERIAL),0,0);
    if ( imtif->pb )
    {
      free(imtif->pb);
      imtif->pb = 0;
    }
    e = imtif->setMessage(&imtif->h, (void*)MAKELONG(RELEASE,XSERIAL),0,0);
    e = imtif->setMessage(&imtif->hdl, (void*)MAKELONG(RELEASE,MTIF), 0, 0);
    unmodulus(imtif);
    free(*h);
    *h = 0;
  }

  return e;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////                                                                   //////
//////                                                                   //////
//////                              SIFR                                 //////
//////                                                                   //////
//////                                                                   //////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*****************************************************************************/
/*****************************************************************************/
/*********************************        ************************************/
/*********************************        ************************************/
/*********************************        ************************************/
/*****************************                ********************************/
/*******************************            **********************************/
/*********************************        ************************************/
/************************************   **************************************/
/************************************* ***************************************/
/*****************************************************************************/
#if __CUSTOM_X_SIFR__
void* __hmac_callback(void* h, void* msg, void* wparam, void* lparam)
{
  IMTIF* imtif = (IMTIF*)h;
  int32_t fd=0;

	switch( LOWORD((uint32_t)msg) )
	{
	case HASH:
    imtif->callback[0](imtif->h, 0, (int8_t*)wparam, (int32_t)lparam, 0, imtif->obj);
		break;
	}
  return 0;
}

#if defined __cplusplus
extern "C"
#endif
#if defined XWIN32 || defined WINCE
__declspec(dllexport)
#endif
int32_t __hmac(void** h, int8_t* argv, int32_t (*f[])(void*,int32_t,int8_t*,int32_t,void*,void*), void* o)
{
  //int8_t  key[2][512] =
  //{
  //  #include ".secret.key.x86.dat",
  //  #include ".secret.key.x86.idx"
  //};

  int8_t  _hbuf[2][2048] = {0};
  int32_t sz[2] = {0};

  int8_t  _argv[2048] = {0};
  int32_t e = 0;
  IMTIF* imtif = 0;
  int8_t  _iargv[4096] = {0};
  int8_t  _iap = 0;

  if ( *h ) return 0xEFFFFFFF;
  *h = imtif = (IMTIF*)calloc(1, sizeof(IMTIF));

  for ( e=0 ; *f[e] ; e++ )
  {
    imtif->callback[e] = f[e];
  }
  imtif->obj = o;

  e = NJSON_STR(argv, "SYSTEM_LIBRARY", _argv);
  if ( e > 0 ) sprintf(&_iargv[_iap], "%s\n", _argv);
  else sprintf(&_iargv[_iap], "\n", _argv);
  _iap = strlen(_iargv);

  if ( modulus(imtif, _argv) < 0 ) return 0xEF000001;


  e = NJSON_STR(argv, "SECRET_KEY_0", _argv);
  if ( e > 0 ) sprintf(&_iargv[_iap], "%s\n", _argv);
  else sprintf(&_iargv[_iap], "\n", _argv);
  _iap = strlen(_iargv);

  e = NJSON_STR(argv, "SECRET_KEY_1", _argv);
  if ( e > 0 ) sprintf(&_iargv[_iap], "%s\n", _argv);
  else sprintf(&_iargv[_iap], "\n", _argv);
  _iap = strlen(_iargv);

  e = imtif->setMessage(&imtif->hdl, (void*)MAKELONG(INIT,MTIF), _iargv, 0);

  e = imtif->setMessage( imtif->h, (void*)MAKELONG(SYSTEM_CALLBACK,SIFR_HMAC_SHA256),__hmac_callback, imtif);

  if ( NJSON_STR(argv, "KEY", _argv)<0 )
  {
    free(imtif);
    *h = imtif = 0;
    #if __STATIC_LIB__==0
    if ( imtif->hmodule )
    {
      #if defined LINUX
      dlclose(imtif->hmodule);
      #endif

      #if defined XWIN32
      FreeLibrary(imtif->hmodule);
      #endif
    }
    #endif
    return 0xEFFFFFFF;
  }
  sz[0] = asc_to_hex(_argv, _hbuf[0]);
	e = imtif->setMessage( imtif->h, MAKELONG(SIFR_PARAM_KEY, SIFR_HMAC_SHA256), _hbuf[0], sz[0]);


  if ( NJSON_STR(argv, "MESSAGE", _argv)<0 )
  {
    free(imtif);
    *h = imtif = 0;
    #if __STATIC_LIB__==0
    if ( imtif->hmodule )
    {
      #if defined LINUX
      dlclose(imtif->hmodule);
      #endif

      #if defined XWIN32
      FreeLibrary(imtif->hmodule);
      #endif
    }
    #endif

    return 0xEFFFFFFF;
  }
  sz[1] = asc_to_hex(_argv, _hbuf[1]);
  e = imtif->setMessage( imtif->h, MAKELONG(HASH, SIFR_HMAC_SHA256), _hbuf[1], sz[1]);

	e = imtif->setMessage( imtif->h, MAKELONG(SIFR_PARAM_CLEAR, SIFR_HMAC_SHA256), MAKELONG(SIFR_PARAM_KEY, 0), 0);


  e = imtif->setMessage(&imtif->h, (void*)MAKELONG(RELEASE,MTIF), 0, 0);

  #if __STATIC_LIB__==0
  if ( imtif->hmodule )
  {
    #if defined LINUX
    dlclose(imtif->hmodule);
    #endif

    #if defined XWIN32
    FreeLibrary(imtif->hmodule);
    #endif
  }
  #endif

  free(imtif);
  *h = imtif = 0;


  return e;
}
#endif