/*****************************************************************************/
/*                                                                           */
/*            NCLab (Networked Control Lab) CORE version 1.0                 */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*  File Name         : common.h       	                                     */
/*                                                                           */
/*  Description       : This file contains all the necessary constants ,     */
/*                      type definitions according to CAST specifications,   */
/*                      common system headers.                               */
/*                                                                           */
/*                                                                           */
/*  Issues / Problems : None                                                 */
/*                                                                           */
/*  Revision History  :                                                      */
/*                                                                           */
/*        DD MM YYYY   Author(s)        Changes (Describe the changes made)  */
/*        01 01 2007   Shin Seunghyeok  Draft                                */
/*        01 12 2009   Shin Seunghyeok  Draft                                */
/*                                                                           */
/*****************************************************************************/
#ifndef __COMMON_H_F2DBDC40_6196_4E67_A689_D31A9310BEC0__
#define __COMMON_H_F2DBDC40_6196_4E67_A689_D31A9310BEC0__

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <signal.h>
#include <sys/types.h>
#if __LINUX__
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/if_ether.h>
#include <netdb.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>
#include <netdb.h>
#include <linux/if_packet.h>
#include <fcntl.h>
#include <pthread.h>
#ifndef __USE_GNU
#define __USE_GNU
#endif
#endif

#if __WIN32__
#include <winsock2.h>
#include <mstcpip.h>
#include <winreg.h>
#include <iphlpapi.h>
#include <sys/timeb.h>
#include <process.h>
#include <ws2tcpip.h>
#include <wincrypt.h>
#endif

#include <time.h>
#include <math.h>


#define BOX(a)        printf("+------------------------------------------------------------------------------+\r\n" \
                            "|                                                                              |\r\n"\
                            "| %-76s |\r\n" \
                            "|                                                                              |\r\n"\
                            "+------------------------------------------------------------------------------+\r\n", a)


#define MIN(x,y)  ((x)>(y)?(y):(x))
#define MAX(x,y)  ((x)<(y)?(y):(x))


#if __LINUX__
typedef int32_t             SOCKET;
typedef void *          HINSTANCE;
typedef void *          HANDLE;
typedef unsigned long   DWORD;
typedef void *          HKEY;
#define MAX_PATH        256
#endif

#if __WIN32__
typedef TCHAR            tchar;
#endif

#define BUFFER_LEN          81920
#define KB                  0x400
#define MB                  0x100000
#define GB                  0x40000000

#define ms                  0x400
#define SEMAPHORE_COUNTER   0x00002000

#if __WIN32__
#pragma comment(lib, "ws2_32.lib")
#endif

#define DEFAULT_KEY        7810

#if __LINUX__
#define MAKEWORD(a,b)      ((a&0x00FF)|((b&0x00FF)<<8))
#define MAKELONG(a,b)      ((a&0x0000FFFF)|((b&0x0000ffff)<<16))
#define LOWORD(l)           (l&0x0000FFFF)
#define HIWORD(l)          ((l&0xFFFF0000)>>16)
#define LOBYTE(w)           (w&0x00FF)
#define HIBYTE(w)          ((w&0xFF00)>>8)
#endif


#define MCSLEEP_BASE_VALUE         1


#define CRLF                  "\r\n"
#define UNDER_LINE          printf("\r\n________________________________________________________________________________\r\n")
#define DOUBLE_LINE         printf("\r\n================================================================================\r\n")
#define DASH_LINE           printf("\r\n--------------------------------------------------------------------------------\r\n")

#define BYTE0(x)  ( (uint8_t)(((x)    )&0x000000FF) )
#define BYTE1(x)  ( (uint8_t)(((x)>> 8)&0x000000FF) )
#define BYTE2(x)  ( (uint8_t)(((x)>>16)&0x000000FF) )
#define BYTE3(x)  ( (uint8_t)(((x)>>24)&0x000000FF) )
#define CHANGE_SEQ(x) ( ((x&0x000000FF)<<24 ) | ((x&0x0000FF00)<<8  ) | ((x&0x00FF0000)>>8  ) | ((x&0xFF000000)>>24 ) )

#define TO_BIT16(x)        ((x[0]&0xFF)|(x[1]&0xFF)<<8)
#define TO_BIT32(x)        ((x[0]&0xFF)|(x[1]&0xFF)<<8|(x[2]&0xFF)<<16|(x[3]&0xFF)<<24)
#define TO_INTEGER(x,y)     (y>2?TO_BIT32(x):TO_BIT16(x))

#define SET_BIT(a,v)   ( *(a+(v/8)) |=  (0x80>>(v%8)) )
#define GET_BIT(a,v)   ( *(a+(v/8)) &   (0x80>>(v%8)) )
#define CLR_BIT(a,v)   ( *(a+(v/8)) &= ~(0x80>>(v%8)) )



#define SPLIT(arg1,c,x,y)                                                 \
{                                                                         \
int8_t* p=arg1;                                                             \
int8_t* t=x;                                                                \
int8_t  _c = ((c>='A')&&(c<='Z'))?(c+0x20):((c>='a')&&(c<='z'))?(c-0x20):c; \
do {                                                                      \
  *t = *p;                                                                \
  if ( (*p==c) || (*p==_c) ) {                                            \
    *t = 0;                                                               \
    p++;                                                                  \
    t=y;                                                                  \
    continue;                                                             \
  }                                                                       \
  t++;                                                                    \
  p++;                                                                    \
  *t = 0;                                                                 \
} while(*p);                                                              \
}

#define TO_COORD(arg1,x,y)   SPLIT(arg1,'X',x,y)
#define TO_RANGE(arg1,x,y)   SPLIT(arg1,'~',x,y)

#define GET_X(arg1)      \
{                        \
  int8_t x[8], y[8];       \
  TO_COORD(arg1,x,y);    \
  return x;              \
}

#define GET_Y(arg1)      \
{                        \
  int8_t x[8], y[8];       \
  TO_COORD(arg1,x,y);    \
  return y;              \
}
  


#define ERROR_CODE(y,x)   {    \
int32_t i=0;                       \
while ( *(x+i) ) i++; i--;     \
while ( i>=0 ) {               \
  y|=((*(x+i)&0xFF)<<(8*i));   \
  i--;                         \
}                              \
}

#define   ERR_CODE(e)    (*(e+0)<<24 | *(e+1)<<16 | *(e+2)<<8  | *(e+3))
#define   ERR_STR(e)     (e>>24)&0x5F,(e>>16)&0xFF,(e>>8)&0xFF,e&0xFF


#define   __HSTR_TO_HEX(a)   (((a>='0')&&(a<='9'))?(a-'0'):(((a>='A')&&(a<='Z'))?(a-'A'+0x0A):(((a>='a')&&(a<='z'))?(a-'a'+0x0A):0)))
#define   HSTR_TO_HEX(a)    ((__HSTR_TO_HEX(*(a))<<4)|__HSTR_TO_HEX(*(a+1)))


#if __WIN32__
#define xLOCK_INIT(crit)       InitializeCriticalSection((CRITICAL_SECTION*)(crit))
#endif
#if __LINUX__
#define xLOCK_INIT(crit)       pthread_mutex_init((pthread_mutex_t*)(crit), 0)
#endif

#if __WIN32__
#define xLOCK_FINAL(crit)      DeleteCriticalSection((CRITICAL_SECTION*)(crit))
#endif
#if __LINUX__
#define xLOCK_FINAL(crit)      pthread_mutex_destroy((pthread_mutex_t*)(crit))
#endif

#if __WIN32__
#define xLOCK(crit)            EnterCriticalSection((CRITICAL_SECTION*)(crit))
#endif
#if __LINUX__
#define xLOCK(crit)            pthread_mutex_lock((pthread_mutex_t*)(crit))
#endif

#if __WIN32__
#define xUNLOCK(crit)          LeaveCriticalSection((CRITICAL_SECTION*)(crit))
#endif
#if __LINUX__
#define xUNLOCK(crit)          pthread_mutex_unlock((pthread_mutex_t*)(crit))
#endif


#if __WIN32__
#define xTHREAD_CREATE(f,arg,id,r)  (r=_beginthreadex(0,0,(uint32_t(__stdcall*)(void*))f,(void*)arg,0,(uint32_t*)id))
#endif
#if __LINUX__
#define xTHREAD_CREATE(f,arg,id,r) {                                    \
pthread_attr_t tattr = {0};                                             \
pthread_attr_init(&tattr);                                              \
pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);           \
r=pthread_create((pthread_t*)id, &tattr, f, arg);                       \
pthread_attr_destroy(&tattr);                                           \
}
#endif


#if __WIN32__
#define xTHREAD_EXIT(a,r)                  {if(r) CloseHandle(r);_endthreadex(a);}
#endif

#if __LINUX__
#define xTHREAD_EXIT(a,r)                  /*pthread_exit(a)*/
#endif

//
//#if __WIN32__
//#define xSLEEP(a)          Sleep((uint32_t)a);
//#endif
//#if __LINUX__
//#define xSLEEP(a)                 \
//{                                 \
//struct timeval v = {0};           \
//v.tv_sec = a/1000000;          \
//v.tv_usec = a%1000000;         \
//select(1,0,0,0,&v);               \
//}
//#endif


#define xSLEEP(a)          \
{                               \
struct timeval v = {0};         \
v.tv_sec = a/1000000;           \
v.tv_usec = a%1000000;          \
select(1,0,0,0,&v);             \
}

#if 0
#if __WIN32__
#define xSLEEP(a)         Sleep(a);
#endif
#if __LINUX__
#define xSLEEP(a)               \
{                               \
struct timeval v = {0};         \
v.tv_sec = a/1000000;           \
v.tv_usec = a%1000000;          \
select(1,0,0,0,&v);             \
}
#endif
#endif


#define xSET_SEMAPHORE(SR,v,chk)    ((v&chk)?(SR|=v):(SR&=~v))
#define xGET_SEMAPHORE(SR,v,chk,count,r)   \
for(r=count;r>=0;r--)                      \
{                                          \
if(((SR&v))==chk) { r=0;break;}            \
xSLEEP(1);                                 \
}
#define xCHECK_SEMAPHORE(SR,v,chk)    ((SR&chk)==v)






#if __WIN32__
#pragma pack(1)
#endif
#if __PACKED__
__attribute__((packed))
#endif
typedef struct xREG
{
  uint32_t SR;
  uint32_t COUNTER[32];
  uint32_t INT;
}
xREG;
#if __WIN32__
#pragma pack()
#endif

#endif

