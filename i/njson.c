#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <njson.h>


#if 0
{
  "intent": {
    "id": "084xwo3muq615c2a3bidlzg1",
    "name": "��� �̸�"
  },
  "userRequest": {
    "timezone": "Asia/Seoul",
    "params": {
      "ignoreMe": "true"
    },
    "block": {
      "id": "084xwo3muq615c2a3bidlzg1",
      "name": "��� �̸�"
    },
    "utterance": "��ȭ ����",
    "lang": null,
    "user": {
      "id": "748720",
      "type": "accountId",
      "properties": {}
    }
  },
  "bot": {
    "id": "604ac879048a962ecd895648",
    "name": "�� �̸�"
  },
  "action": {
    "name": "jbcvhaaavo",
    "clientExtra": null,
    "params": {
      "start": 0,
      "length": 10
    },
    "id": "dxk8z4ek3d1fm4q0n7zugmie",
    "detailParams": {
      "start": {
        "origin": 0,
        "value": 0,
        "groupName": ""
      },
      "length": {
        "origin": 10,
        "value": 10,
        "groupName": ""
      }
    }
  }
}
#endif


uint8_t njson_check(int8_t* s)
{
  for (  ; *s ; s++ )
  {
    if (*s == '\"') return 'S';
    else if (*s == '\'') return 'S';
    else if (*s == '{') return 'O';
    else if (*s == '[') return 'A';
    else if (*s>='0' && *s<='9') return 'N';
  }
  return 0;
}

int32_t njson_obj(int8_t* s, int8_t* val)
{
  int32_t i = 0;
  for (  ; *s!='{' ; s++ ); //FF

  for (  ;  ; s++, i++ )
  {
    *(val+i) = *s;
    if ( *s==0 )
    {
      for ( ; *(val+i)!='}' ; i-- ) *(val+i) = 0;
      break;
    }
  }
  return i;
}

int32_t njson_arr(int8_t* s, int8_t* val)
{
  int32_t i = 0;
  for (  ; *s!='[' ; s++ ); //FF
  for (  ;  ; s++, i++ )
  {
    *(val+i) = *s;
    if ( *s==0 )
    {
      for ( ; *(val+i)!=']' ; i-- ) *(val+i)=0;
      break;
    }
  }
  return i;
}


int32_t njson_str(int8_t* s, int8_t* val)
{
  int32_t i = 0;
  uint8_t remf = 0;
  for (  ; *s!='\"' ; s++ ); //FF
  s++;

  for (  ; *s ; s++, i++ )
  {
    *(val+i) = *s;
    if ( *s=='\"' )
    {
      remf = 1;
      continue;
    }
    if ( remf && *s!=' ' ) remf = 0;

    if ( remf && *s==':' )
    {
      for( i-- ; *(val+i) !='\"' ; i-- ) ; //REW
      *(val+i) = 0;
      remf = 0;
      break;
    }
  }
  if ( remf )
  {
    for( i-- ; *(val+i) !='\"' ; i-- ) ; //REW
    *(val+i) = 0;
    remf = 0;
  }

  return i;
}

int32_t njson_n(int8_t* s, int32_t* val)
{
  int32_t i = 0;

  for (  ; *s!=' ' ; s++ ); //FF
  s++;

  return *val = atoi(s);
}



int32_t njson_kv(int8_t* s, int32_t cnt, int32_t idx, int8_t* key, int8_t* val)
{
  int32_t i = 0;
  uint8_t mode = 0;
  int8_t* p = 0;

  if ( idx>cnt ) return -1;

  for ( i=0 ;  i<=cnt ; i++ )
  {
    mode = 0;

    for ( ; (*s!='\"') ; s++ ) ; //FF

    p = key;
    for ( ; *s!=0 ; s++, p++ )
    {
      if (mode==0 && *s == ':' )
      {
        mode = 1;
        *p = 0;
        p = val;
        p--;
        continue;
      }
      *p = *s;
    }
    *p = 0;


    if ( i==idx ) break;
  }
  return i;
}


int32_t njson(int8_t* s, int32_t sz, int8_t* kv)
{
  uint32_t i=0, j=0, k=0;
  int8_t nullp = 0;
  uint8_t jSR = 0;
  int32_t cnt = 0;
  int8_t* stack = (int8_t*)calloc(sz, sizeof(int8_t));

  for ( i=0 ; i<sz ; i++, j++ )
  {
    *(kv+j) = *(s+i);

    if ( *(s+i) == '{' )
    {
      *(stack+k) = *(s+i);
      if ( k==0 )
      {
        *(kv+j) = 0;
        j--;
      }
      k++;
    }
    else if ( *(s+i) == '}' )
    {
      k--;
      *(stack+k) = 0;
      if ( k ==0 ) *(kv+j) = 0;
    }
    else if ( *(s+i) == '"' )
    {
      (jSR&0x80)?(jSR&=0x7F):(jSR|=0x80);
    }
    else if ( *(s+i) == ',' )
    {
      if( (jSR&0x80) == 0 )
      {
        *(kv+j)=0;
        cnt++;
      }
    }
    else if ( *(s+i) == '[' )
    {

    }
    else if ( *(s+i) == ']' )
    {

    }

  }  


  free(stack);
  stack = 0;

  return cnt;
}


int32_t NJSON_STR(int8_t* json, int8_t* key, int8_t* v)
{
  int32_t e = -1;
  int32_t cnt = 0;
  int32_t i = 0;
  uint8_t type = 0;
  int32_t sz = strlen(json);
  volatile int8_t* tmp = 0;
  volatile int8_t* _key = 0;
  volatile int8_t* _val = 0;
  volatile int8_t* _out = 0;

  *v = 0;

  tmp = (int8_t*)calloc(sz, sizeof(int8_t));
  _key = (int8_t*)calloc(sz, sizeof(int8_t));
  _val = (int8_t*)calloc(sz, sizeof(int8_t));
  _out = (int8_t*)calloc(sz, sizeof(int8_t));

  cnt = njson(json, sz, tmp);

  for ( i=0 ; i<=cnt ; i++ )
  {
    njson_kv(tmp, cnt, i, _key, _val);
    memset(_out, 0, sz);
    njson_str(_key, _out);

    if ( strncmp(_out, key, strlen(key)) == 0 )
    {
      type = njson_check(_val);
      memset(_out, 0, sz);
      if ( type=='S' )
      {
        e = njson_str(_val, v);
        break;
      }
    }
  }

  free(tmp);
  free(_key);
  free(_val);
  free(_out);
  _key = 0;
  _val = 0;
  _out = 0;
  tmp = 0;

  return e;
}


int32_t NJSON_N(int8_t* json, int8_t* key, int32_t* v)
{
  int32_t e = 0;
  int32_t cnt = 0;
  int32_t i = 0;
  uint8_t type = 0;
  int32_t sz = strlen(json);
  volatile int8_t* tmp = 0;
  volatile int8_t* _key = 0;
  volatile int8_t* _val = 0;
  volatile int8_t* _out = 0;

  tmp = (int8_t*)calloc(sz, sizeof(int8_t));
  _key = (int8_t*)calloc(sz, sizeof(int8_t));
  _val = (int8_t*)calloc(sz, sizeof(int8_t));
  _out = (int8_t*)calloc(sz, sizeof(int8_t));

  cnt = njson(json, sz, tmp);
  for ( i=0 ; i<=cnt ; i++ )
  {
    njson_kv(tmp, cnt, i, _key, _val);
    memset(_out, 0, sz);
    njson_str(_key, _out);

    if ( strncmp(_out, key, strlen(key)) == 0 )
    {
      type = njson_check(_val);
      memset(_out, 0, sz);
      if ( type=='N' )
      {
        e = njson_n(_val, v);
        break;
      }
    }
  }

  free(tmp);
  free(_key);
  free(_val);
  free(_out);
  _key = 0;
  _val = 0;
  _out = 0;
  tmp = 0;

  return e;
}