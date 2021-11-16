#ifndef __NMDNS_H_F2DBDC40_6196_4E67_A689_D31A9310BEC0__
#define __NMDNS_H_F2DBDC40_6196_4E67_A689_D31A9310BEC0__

#include <icode.x.h>
#include <inode.h>


#if defined XWIN32
#pragma pack(1)
#endif
typedef struct
#if defined LINUX
__attribute__((packed))
#endif
{
  iCode icd;
  iNode* ind;

  uint8_t pkt[128];
} cnmdns;
#if defined XWIN32
#pragma pack()
#endif

int32_t nmdns_Open(cnmdns* p);
int32_t nmdns_Close(cnmdns* p);
void nmdns(iNode* p);
void nmdns_Loop(iNode* p);


#endif
