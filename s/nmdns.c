#include <nmdns.h>
#include <nutil.h>

static cnmdns gmdns;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////                                                                   //////
//////                                                                   //////
//////                         USER FUNCTIONS                            //////
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
void display_hex(uint8_t* h, int32_t s)
{
  int32_t i=0;

  printf("----------------------------------------"
         "----------------------------------------\r\n");
  for (i=0 ; i<s ; i++ )
  {
    if ( i%16 == 0 ) printf("\r\n");
    if ( i%8 == 0 ) printf("  ");
    printf(" %02X", *(h+i));
  }
  printf("\r\n");
  printf("----------------------------------------"
         "----------------------------------------\r\n\r\n");
}

typedef struct stFQDN
{
  uint8_t _tid[2];
  uint8_t _flg[2];
  uint16_t _qcnt;
  uint16_t _ansRR;
  uint16_t _authRR;
  uint16_t _addRR;
} stFQDN;


typedef struct cnFQDN
{
  stFQDN fqdn;
  uint8_t* org;
  uint16_t limit;
  uint8_t qtype[2];
  uint8_t qclass[2];
} cnFQDN;


int32_t hdrFQDN(uint8_t** b, int32_t sz, void* o)
{
  cnFQDN* p = (cnFQDN*)o;
  int32_t e = 0;

  memcpy(&p->fqdn, *b, sizeof(stFQDN));
  p->org = *b;

  #if 1 // header
  printf("%24s | %02X %02X \r\n", "tid", *(*b+0), *(*b+1));
  printf("%24s | %02X %02X \r\n", "flags", *(*b+2), *(*b+3));
  printf("%24s | %02X %02X \r\n", "question", *(*b+4), *(*b+5));
  printf("%24s | %02X %02X \r\n", "answer RRs", *(*b+6), *(*b+7));
  printf("%24s | %02X %02X \r\n", "authority RRs", *(*b+8), *(*b+9));
  printf("%24s | %02X %02X \r\n", "additional RRs", *(*b+10), *(*b+11));
  #endif

  if ( (p->fqdn._flg[0]&0x80) == 0 )
  {
    e = 0;
  }
  p->fqdn._qcnt   = ((p->fqdn._qcnt&0xFF00)>>8)   | (p->fqdn._qcnt&0x00FF);
  p->fqdn._ansRR  = ((p->fqdn._ansRR&0xFF00)>>8)  | (p->fqdn._ansRR&0x00FF);
  p->fqdn._authRR = ((p->fqdn._authRR&0xFF00)>>8) | (p->fqdn._authRR&0x00FF);
  p->fqdn._addRR  = ((p->fqdn._addRR&0xFF00)>>8)  | (p->fqdn._addRR&0x00FF);

  *b = *b+12;
  return e;
}

int32_t bdyFQDN(uint8_t** b, int32_t sz, void* o)
{
  cnFQDN* p = (cnFQDN*)o;
  int32_t e = 0;
  int32_t i = 0;
  uint8_t s = 0;
  uint8_t* _b = *b;
  uint8_t* _org[0xFF] = {0};
  uint8_t  _odx = 0;
  uint8_t _lbl_t = 0, _lbl_l = 0;
  int32_t count = 0;

  #if 1 
  printf("-------------------------------\r\n");
  for ( ; ; )
  {
    if ( count<p->limit )
    {
      for ( ; ; )
      {
        _lbl_t = (*_b)&0xC0;
        _lbl_l = (*_b)&0x3F;

        printf("[%02X]", *_b);
        _b ++;
        if ( _lbl_t == 0xC0 )
        {
      	  printf(">%02X<", *_b);
          _b++;
          _org[_odx] = _b; // keep the next pointer
          _b--;
          _b = p->org+*_b; // set the reference pointer
          _odx++;
          continue;
        }
        else
        {
          if ( _lbl_l == 0 )
          {
            for ( ; _odx>0 ; _odx-- ) _org[_odx] = 0;
            if ( _org[_odx] != 0 )
            {
              _b=_org[_odx]; // set the next pointer
              _org[_odx] = 0;
            }
            *b = _b;

            //printf("-------> %02X %02X %02X %02X %02X\r\n", *(_b-2), *(_b-1), *(_b+0), *(_b+1), *(_b+2));
            break;
          }
          for ( i=0 ; i<_lbl_l ; i++ )
          {
            printf("%c", *_b);
            _b ++;
          }
        }
      }
      printf("\r\n");

      //QType
      p->qtype[0] = *_b; _b++;
      p->qtype[1] = *_b; _b++;
      printf("%02X %02X \r\n", p->qtype[0], p->qtype[1]);

      //QClass
      p->qclass[0] = *_b; _b++;
      p->qclass[1] = *_b; _b++;
      printf("%02X %02X \r\n", p->qclass[0], p->qclass[1]);

      if ( p->qtype[1] == 0x01 )
      {
        // TTL
        for ( i=0 ; i<4 ; i++ ) printf("%02X ", *(_b+i));
        _b+=4;
        printf("\r\n");
        // data length
        s = *(_b)<<8 | *(_b+1);
        _b+=2;
        printf("\r\n");
        // data
        for( i=0 ; i<s ; i++ ) printf("%02X ", *(_b+i));
        _b+=s;
        printf("\r\n");
      }
      else if ( p->qtype[1] == 0x02 )
      {

      }
      else if ( p->qtype[1] == 0x03 )
      {

      }
      else if ( p->qtype[1] == 0x04 )
      {

      }
      else if ( p->qtype[1] == 0x05 )
      {

      }
      else if ( p->qtype[1] == 0x06 )
      {

      }
      else if ( p->qtype[1] == 0x07 )
      {

      }
      else if ( p->qtype[1] == 0x08 )
      {

      }
      else if ( p->qtype[1] == 0x09 )
      {

      }
      else if ( p->qtype[1] == 0x0A )
      {

      }
      else if ( p->qtype[1] == 0x0B )
      {

      }
      else if ( p->qtype[1] == 0x0C )
      {
        // TTL
        for ( i=0 ; i<4 ; i++ ) printf("%02X ", *(_b+i));
        _b+=4;
        printf("\r\n");
        // data length
        s = *(_b)<<8 | *(_b+1);
        _b+=2;
        printf("\r\n");

        //if (*_b&0xC0 == 0xC0 ) continue;

        // data
        for( i=0 ; i<s ; i++ ) printf("%02X ", *(_b+i));
        _b+=s;
        printf("\r\n");
     
      }
      else if ( p->qtype[1] == 0x0D )
      {
        // TTL
        for ( i=0 ; i<4 ; i++ ) printf("%02X ", *(_b+i));
        _b+=4;
        // data length
        s = *(_b)<<8 | *(_b+1);
        _b+=2;
        printf("\r\n");

        // cpu length
        s = *_b;
        _b++;
        // cpu
        for( i=0 ; i<s ; i++ ) printf("%c", *(_b+i));
        _b+=s;
        printf("\r\n");

        // os length
        s = *_b;
        _b++;
        // os
        for( i=0 ; i<s ; i++ ) printf("%c", *(_b+i));
        _b+=s;
        printf("\r\n");

      }
      else if ( p->qtype[1] == 0x0E )
      {

      }
      else if ( p->qtype[1] == 0x0F )
      {

      }
      else if ( p->qtype[1] == 0x10 )
      {
        // TTL
        for ( i=0 ; i<4 ; i++ ) printf("%02X ", *(_b+i));
        _b+=4;
        // data length
        s = *(_b)<<8 | *(_b+1);
        _b+=2;
        printf("\r\n");
        for( i=0 ; i<s ; i++ ) printf("%02X ", *(_b+i));
        _b+=s;
        printf("\r\n");

      }
      else if ( p->qtype[1] == 0x1C )
      {
        // TTL
        for ( i=0 ; i<4 ; i++ ) printf("%02X ", *(_b+i));
        _b+=4;
        // data length
        s = *(_b)<<8 | *(_b+1);
        _b+=2;
        printf("\r\n");
        for( i=0 ; i<s ; i++ ) printf("%02X ", *(_b+i));
        _b+=s;
        printf("\r\n");
      }
      else if ( p->qtype[1] == 0x21 )
      {
        // TTL
        for ( i=0 ; i<4 ; i++ ) printf("%02X ", *(_b+i));
        _b+=4;
        // data length
        s = *(_b)<<8 | *(_b+1);
        _b+=2;
        printf("\r\n");

        // priority
        printf("%02X %02X \r\n", *_b, *(_b+1));
        _b+=2;

        // weight
        printf("%02X %02X \r\n", *_b, *(_b+1));
        _b+=2;

        // port
        printf("%02X %02X \r\n", *_b, *(_b+1));
        _b+=2;

        //if (*_b&0xC0 == 0xC0 ) continue;

        for( i=0 ; i<s ; i++ ) printf("%02X ", *(_b+i));
        _b+=s;
        printf("\r\n");
      }
      else if ( p->qtype[1] == 0xFF )
      {

      }

      printf("count ----> %d\r\n", count);
      count ++;

      *b = _b;
    }

    if ( p->limit == count ) break;

  }
  printf("-------------------------------\r\n");
  #endif

  return e;
}

int32_t FQDN(uint8_t* b, int32_t sz,int32_t (*f[])(uint8_t**,int32_t,void*), void* o)
{
  cnFQDN* p = (cnFQDN*)o;

  if( f[0] ) f[0](&b,sz,p);
  p->limit = p->fqdn._qcnt;
  if( f[1] ) f[1](&b,sz,p);
  p->limit = p->fqdn._ansRR;
  if( f[2] ) f[2](&b,sz,p);
  p->limit = p->fqdn._authRR;
  if( f[3] ) f[3](&b,sz,p);
  p->limit = p->fqdn._addRR;
  if( f[4] ) f[4](&b,sz,p);

  return 0;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////                                                                   //////
//////                                                                   //////
//////                       SYSTEM FUNCTIONS                            //////
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
int32_t nmdns_Connected(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  cnmdns* p = (cnmdns*)o;
  int32_t idx = (moreinfo==0)?0:(*(int32_t*)moreinfo);

  //printf(" nmdns_Connected   ->   %08X %d\r\n", sz, idx);

  return 0;
}

int32_t nmdns_Disconnected(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  cnmdns* p = (cnmdns*)o;
  //printf(" nmdns_Disconnected   ->   %08X \r\n", sz);

  return 0;
}


int32_t nmdns_parse(void* h, int32_t fd, uint8_t* b, int32_t sz, void* moreinfo, void* o)
{
  cnmdns* p = (cnmdns*)o;
  int32_t i = 0, j = 0, k = 0;
  int32_t qcount = 0;
  int32_t ansRR = 0;
  int32_t authRR = 0;
  int32_t addRR = 0;
  uint8_t _lbl_t = 0, _lbl_l = 0;
  #if 1 // header
  printf("%24s | %02X %02X \r\n", "tid",   (uint8_t)*(b+0), (uint8_t)*(b+1));
  printf("%24s | %02X %02X \r\n", "flags", (uint8_t)*(b+2), (uint8_t)*(b+3));
  printf("%24s | %02X %02X \r\n", "question", (uint8_t)*(b+4), (uint8_t)*(b+5));
  printf("%24s | %02X %02X \r\n", "answer RRs", (uint8_t)*(b+6), (uint8_t)*(b+7));
  printf("%24s | %02X %02X \r\n", "authority RRs", (uint8_t)*(b+8), (uint8_t)*(b+9));
  printf("%24s | %02X %02X \r\n", "additional RRs", (uint8_t)*(b+10), (uint8_t)*(b+11));
  #endif

  if ( (*(b+2)&0x80) == 0 )
  {
    // Standard Query
  }
  qcount = *(b+4)<<8 | *(b+5);
  ansRR  = *(b+6)<<8 | *(b+7);
  authRR = *(b+8)<<8 | *(b+9);
  addRR  = *(b+10)<<8 | *(b+11);
  

  #if 1 // FQDN
  for ( i=12 ;i<sz ; i++ )
  {
    printf(" %d   ------> %d %02X <-----------------\r\n", qcount,i, *(b+i));
    if ( qcount > 0 )
    {
      for ( k=i ; ; )
      {
        _lbl_t = *(b+k)&0xC0;
        _lbl_l = *(b+k)&0x3F;

        printf("%02X ", (uint8_t)(*(b+k)));
        k++;
        if ( _lbl_t == 0xC0 )
        {
          k = *(b+k);
          continue;
        }
        else
        {
          if ( _lbl_l == 0 )
          {
            i = k;
            break;
          }
          for ( j=0 ; j<_lbl_l ; j++, k++ )
          {
            printf("[%c]", *(b+k));
          }
        }
      }
      printf("\r\n");

      //QType
      k++;
      printf("%02X %02X \r\n", (uint8_t)*(b+k), (uint8_t)*(b+k+1));

      //QClass
      k++;
      printf("%02X %02X \r\n", (uint8_t)*(b+k), (uint8_t)*(b+k+1));

      k++;
      qcount --;

      i = k;
    }

    if ( qcount == 0 ) break;

  }
  #endif

  return 0;
}

int32_t nmdns_Read(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  cnmdns* p = (cnmdns*)o;


  return 0;
}

int32_t nmdns_Readfrom(void* h, int32_t fd, int8_t* b, int32_t sz, void* moreinfo, void* o)
{
  cnFQDN fqdn = {0};
  cnmdns* p = (cnmdns*)o;
  int32_t ip_port[2];

  int32_t (*f[])(uint8_t**,int32_t,void*) = {
    hdrFQDN,
    bdyFQDN,
    0,
    bdyFQDN,
    0,
  };

  memcpy(ip_port, moreinfo, sizeof(int32_t)*2);
  printf("----->   %d.%d.%d.%d %d\r\n",
          (ip_port[0]&0x000000FF),
          (ip_port[0]&0x0000FF00)>>8,
          (ip_port[0]&0x00FF0000)>>16,
          (ip_port[0]&0xFF000000)>>24,
          ip_port[1]);

  display_hex(b, sz);

  //nmdns_parse(h,fd,b,sz,moreinfo,o);
  FQDN(b, sz, f, &fqdn);

  return 0;
}

int32_t nmdns_Open(cnmdns* p)
{
  int32_t e = 0;
  int8_t _json[1024] = {0};

  p->icd.callback[ICODE_CALLBACK_CONNECTED] = nmdns_Connected;
  p->icd.callback[ICODE_CALLBACK_DISCONNECTED] = nmdns_Disconnected;
  p->icd.callback[ICODE_CALLBACK_READ] = nmdns_Read;
  p->icd.callback[ICODE_CALLBACK_READFROM] = nmdns_Readfrom;

  #if defined XWIN32
  sprintf(_json,
          "{"
            "\"SYSTEM_LIBRARY\":\"code.X.x86.dll\","
            "\"IP\":\"192.168.0.2\","
            "\"PORT\":\"5353\","
            "\"CSTYPE\":\"CLIENT\","
            "\"PROTOCOL\":\"UDP\","
            "\"CASTTYPE\":\"MULTICAST\","
            "\"SYNC\":\"DISABLE\","
            "\"TIMEOUT\":\"2000\""
          "}"
          );
  #endif

  #if defined x32
  sprintf(_json,
          "{"
            "\"SYSTEM_LIBRARY\":\"/usr/xlocal/projectX/tools/ngine/pkg/code.X.x32.so\","
            "\"IP\":\"192.168.0.2\","
            "\"PORT\":\"5353\","
            "\"CSTYPE\":\"CLIENT\","
            "\"PROTOCOL\":\"UDP\","
            "\"CASTTYPE\":\"MULTICAST\","
            "\"SYNC\":\"DISABLE\","
            "\"TIMEOUT\":\"2000\""
          "}"
          );
  #endif


  #if defined a32
  sprintf(_json,
          "{"
            "\"SYSTEM_LIBRARY\":\"/usr/local/node/a32/code.X.hf.a32.so\","
            "\"IP\":\"224.0.0.251\","
            "\"PORT\":\"5353\","
            "\"CSTYPE\":\"CLIENT\","
            "\"PROTOCOL\":\"UDP\","
            "\"CASTTYPE\":\"MULTICAST\","
            "\"SYNC\":\"DISABLE\","
            "\"TIMEOUT\":\"2000\""
          "}"
          );
  #endif

  e = __socket_open(&p->icd.h, _json, p->icd.callback, p);

  return e;
}


int32_t nmdns_Close(cnmdns* p)
{
  return __socket_close(&p->icd.h);
}


void nmdns(iNode* p)
{
  int32_t e = 0;
  e = nmdns_Open(&gmdns);

}


void nmdns_Loop(iNode* p)
{

}
