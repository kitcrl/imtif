/*****************************************************************************/
/*                                                                           */
/*            NCLab (Networked Control Lab) CORE version 1.0                 */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*  File Name         : njson.h                                              */
/*                                                                           */
/*  Description       : This file contains all the necessary constants ,     */
/*                      type definitions according to CAST specifications,	 */
/*                      common system headers.                               */
/*                                                                           */
/*                                                                           */
/*  Issues / Problems : None                                                 */
/*                                                                           */
/*  Revision History  :                                                      */
/*                                                                           */
/*        DD MM YYYY   Author(s)        Changes (Describe the changes made)  */
/*        18 03 2021   Shin Seunghyeok  Draft                                */
/*                                                                           */
/*****************************************************************************/
#if 1 //__JSON__
#ifndef __NJSON_H_F2DBDC40_6196_4E67_A689_D31A9310BEC0__
#define __NJSON_H_F2DBDC40_6196_4E67_A689_D31A9310BEC0__

#if defined __cplusplus
extern "C"
#endif
#if __WIN32__
__declspec(dllexport)
#endif
int32_t njson(int8_t* s, int32_t sz, int8_t* kv);

#if defined __cplusplus
extern "C"
#endif
#if __WIN32__
__declspec(dllexport)
#endif
int32_t njson_kv(int8_t* s, int32_t cnt, int32_t idx, int8_t* key, int8_t* val);

#if defined __cplusplus
extern "C"
#endif
#if __WIN32__
__declspec(dllexport)
#endif
uint8_t njson_check(int8_t* s);

#if defined __cplusplus
extern "C"
#endif
#if __WIN32__
__declspec(dllexport)
#endif
int32_t njson_obj(int8_t* s, int8_t* val);

#if defined __cplusplus
extern "C"
#endif
#if __WIN32__
__declspec(dllexport)
#endif
int32_t njson_arr(int8_t* s, int8_t* val);

#if defined __cplusplus
extern "C"
#endif
#if __WIN32__
__declspec(dllexport)
#endif
int32_t njson_str(int8_t* s, int8_t* val);

#if defined __cplusplus
extern "C"
#endif
#if __WIN32__
__declspec(dllexport)
#endif
int32_t njson_n(int8_t* s, int32_t* val);


#if defined __cplusplus
extern "C"
#endif
#if __WIN32__
__declspec(dllexport)
#endif
int32_t NJSON_STR(int8_t* json, int8_t* key, int8_t* v);


#endif

#endif

