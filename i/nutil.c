#include <nutil.h>

int32_t asc_to_hex(uint8_t* src, uint8_t* dst)
{
  int32_t i=0;
  for (i=0; *src ; src++, i++ )
  {
    if ( (*(src)>='A') && (*(src)<='Z') )
    {
      ( i&1 ) ? ((*dst |= (*(src) - 0x37)&0x0F ), dst++ ): (*dst=0, ( *dst |= ( (*(src) - 0x37)&0x0F) << 4 ));
    }
    else if ( (*(src)>='a') && (*(src)<='z') )
    {
      ( i&1 ) ? ((*dst |= (*(src) - 0x57)&0x0F ), dst++ ): (*dst=0, ( *dst |= ( (*(src) - 0x57)&0x0F) << 4 ));
    }
    else if ( (*(src)>='0') && (*(src)<='9') )
    {
      ( i&1 ) ? ((*dst |= (*(src) - 0x30)&0x0F ), dst++ ): (*dst=0, ( *dst |= ( (*(src) - 0x30)&0x0F) << 4 ));
    }
    else
    {
      i--;
    }
  }

  if ( i&1 )
  {
    *dst>>=4;
    dst++;
    *dst = 0;
  }
  return (i/2) + (i%2);
}

int32_t hex_to_asc(uint8_t* src, uint8_t* dst)
{
  int32_t e=0;
  for ( ; *src ; src++,e+=2 )
  {
    sprintf(dst, "%02X", *src);
    dst++;
    dst++;
  }
  *dst = 0;
  return e-=2;
}

int32_t to_raw(int8_t* in, int32_t isz, int8_t* out, int32_t* osz)
{
  for ( (*osz)=0 ; (*osz)<isz ; (*osz)++ )
  {
    sprintf((out+(*osz)*3), "%02X ", (uint8_t)*(in+(*osz)));
  }
  *(out+((*osz)*3)-1) = 0;
  return *osz;  
}


int32_t get_keyvalue(int8_t* in, int8_t* key, int8_t* val, int32_t* vsz)
{
  int32_t i=0, j=0;
  int8_t _in[64] = {0};
  *vsz = 0;

  for ( i=0 ; *(in+i) ; i++ )
  {
    _in[j] = *(in+i);
    j++;
    if ( *(in+i) == '=' )
    {
      j--;
      _in[j] = 0;
      if ( strncmp(_in, key, strlen(key)) == 0 )
      {
        for ( i++;  ; i++ )
        {
          if ( *(in+i)=='&' || *(in+i)==0 ) break;
          *(val+*vsz) = *(in+i);
          (*vsz)++;
        }
        break;
      }
      else
      {
        for( ;*(in+i)!='&'; i++ );
      }
      j=0;
    }
  }

  return *vsz;
}



int32_t get_dlmtr(int8_t* in, int8_t dlmtr, int32_t idx, int8_t* out)
{
  int32_t e=-1;
  int32_t i=0;
  int32_t ii=0;
  int32_t _idx = -1;

  if ( dlmtr == 0 ) return e;

  for ( i=0,ii=0 ;i<(int32_t)strlen(in) ; i++,ii++ )
  {
    *(out+ii) = *(in+i);
    if ( *(in+i)==dlmtr )
    {
      _idx++;
      e = idx-_idx;
    }

    if ( e >= 1 )
    {
      ii=-1;
      e=-1;
    }

    if ( e == 0 )
    {
      *(out+ii) = 0;
      e = ii;
      break;
    }
  }

  if ( i==(int32_t)strlen(in) )
  {
    if ( (idx-_idx)<=1 )
    {
      *(out+ii) = 0;
      e = ii?ii:-1;
    }
    else
    {
      *out = 0;
    }
  }

  return e;
}



int32_t get_trim(int8_t* in, int8_t pfx, int8_t sfx, int8_t* out)
{
  int32_t e=-1;
  int32_t i;
  int32_t ii;
  for ( i=0,ii=0 ;i<(int32_t)strlen(in) ; i++ )
  {
    if ( (*(in+i)==pfx) && (pfx!=0) && (e==-1))
    {
      i++;
      e++;
    }
    else if ( pfx==0 && e==-1)
    {
      e++;
    }

    if ( e>=0 )
    {
      *(out+ii) = *(in+i);
      if ( (*(in+i) == sfx) || (*(in+i)=='\n') ||  (*(in+i)==0))
      {
        *(out+ii) = 0;
        break;
      }
      ii++;
      e++;
    }
  }
  if (e>0) *(out+e) = 0;
  return e;
}


int32_t strcompare(int8_t* a, int8_t* b)
{
  int32_t e = -1;
  if ( strlen(a)==strlen(b) )
  {
    if ( strncmp(a,b,strlen(a)) == 0 )
    {
      e = 0;
    }
  }
  else if ( strlen(a)>strlen(b) ) e = -1;
  else if ( strlen(a)<strlen(b) ) e = 1;
  return e;
}



#define __MAX_CHAR__         16
void print_buffer(uint8_t* title, uint8_t* b, int32_t sz)
{
  int j=0;
	int i=0;
	int k=0;

  printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"
         " %s \r\n"
         "--------------------------------------------------------------------------------\r\n", title);
  if ( sz > 0 )
  {
    for ( j=0 ; j<sz ; j++ )
    {
      if ( j && (j%__MAX_CHAR__)==0 )
		  {
			  printf(" | %c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c\r\n",
						  ((b[j-16]<0x20) || (b[j-16]>=0x7F))?'.':b[j-16],
						  ((b[j-15]<0x20) || (b[j-15]>=0x7F))?'.':b[j-15],
						  ((b[j-14]<0x20) || (b[j-14]>=0x7F))?'.':b[j-14],
						  ((b[j-13]<0x20) || (b[j-13]>=0x7F))?'.':b[j-13],
						  ((b[j-12]<0x20) || (b[j-12]>=0x7F))?'.':b[j-12],
						  ((b[j-11]<0x20) || (b[j-11]>=0x7F))?'.':b[j-11],
						  ((b[j-10]<0x20) || (b[j-10]>=0x7F))?'.':b[j-10],
						  ((b[j- 9]<0x20) || (b[j- 9]>=0x7F))?'.':b[j- 9],
						  ((b[j- 8]<0x20) || (b[j- 8]>=0x7F))?'.':b[j- 8],
						  ((b[j- 7]<0x20) || (b[j- 7]>=0x7F))?'.':b[j- 7],
						  ((b[j- 6]<0x20) || (b[j- 6]>=0x7F))?'.':b[j- 6],
						  ((b[j- 5]<0x20) || (b[j- 5]>=0x7F))?'.':b[j- 5],
						  ((b[j- 4]<0x20) || (b[j- 4]>=0x7F))?'.':b[j- 4],
						  ((b[j- 3]<0x20) || (b[j- 3]>=0x7F))?'.':b[j- 3],
						  ((b[j- 2]<0x20) || (b[j- 2]>=0x7F))?'.':b[j- 2],
						  ((b[j- 1]<0x20) || (b[j- 1]>=0x7F))?'.':b[j- 1]);
				  // ((b[j-16]>='0')&&(b[j-16]<='9') || (b[j-16]>='A')&&(b[j-16]<='Z') || (b[j-16]>='a')&&(b[j-16]<='z'))?b[j-16]:'.',
				  // ((b[j-15]>='0')&&(b[j-15]<='9') || (b[j-15]>='A')&&(b[j-15]<='Z') || (b[j-15]>='a')&&(b[j-15]<='z'))?b[j-15]:'.',
				  // ((b[j-14]>='0')&&(b[j-14]<='9') || (b[j-14]>='A')&&(b[j-14]<='Z') || (b[j-14]>='a')&&(b[j-14]<='z'))?b[j-14]:'.',
				  // ((b[j-13]>='0')&&(b[j-13]<='9') || (b[j-13]>='A')&&(b[j-13]<='Z') || (b[j-13]>='a')&&(b[j-13]<='z'))?b[j-13]:'.',
				  // ((b[j-12]>='0')&&(b[j-12]<='9') || (b[j-12]>='A')&&(b[j-12]<='Z') || (b[j-12]>='a')&&(b[j-12]<='z'))?b[j-12]:'.',
				  // ((b[j-11]>='0')&&(b[j-11]<='9') || (b[j-11]>='A')&&(b[j-11]<='Z') || (b[j-11]>='a')&&(b[j-11]<='z'))?b[j-11]:'.',
				  // ((b[j-10]>='0')&&(b[j-10]<='9') || (b[j-10]>='A')&&(b[j-10]<='Z') || (b[j-10]>='a')&&(b[j-10]<='z'))?b[j-10]:'.',
				  // ((b[j- 9]>='0')&&(b[j- 9]<='9') || (b[j- 9]>='A')&&(b[j- 9]<='Z') || (b[j- 9]>='a')&&(b[j- 9]<='z'))?b[j- 9]:'.',
				  // ((b[j- 8]>='0')&&(b[j- 8]<='9') || (b[j- 8]>='A')&&(b[j- 8]<='Z') || (b[j- 8]>='a')&&(b[j- 8]<='z'))?b[j- 8]:'.',
				  // ((b[j- 7]>='0')&&(b[j- 7]<='9') || (b[j- 7]>='A')&&(b[j- 7]<='Z') || (b[j- 7]>='a')&&(b[j- 7]<='z'))?b[j- 7]:'.',
				  // ((b[j- 6]>='0')&&(b[j- 6]<='9') || (b[j- 6]>='A')&&(b[j- 6]<='Z') || (b[j- 6]>='a')&&(b[j- 6]<='z'))?b[j- 6]:'.',
				  // ((b[j- 5]>='0')&&(b[j- 5]<='9') || (b[j- 5]>='A')&&(b[j- 5]<='Z') || (b[j- 5]>='a')&&(b[j- 5]<='z'))?b[j- 5]:'.',
				  // ((b[j- 4]>='0')&&(b[j- 4]<='9') || (b[j- 4]>='A')&&(b[j- 4]<='Z') || (b[j- 4]>='a')&&(b[j- 4]<='z'))?b[j- 4]:'.',
				  // ((b[j- 3]>='0')&&(b[j- 3]<='9') || (b[j- 3]>='A')&&(b[j- 3]<='Z') || (b[j- 3]>='a')&&(b[j- 3]<='z'))?b[j- 3]:'.',
				  // ((b[j- 2]>='0')&&(b[j- 2]<='9') || (b[j- 2]>='A')&&(b[j- 2]<='Z') || (b[j- 2]>='a')&&(b[j- 2]<='z'))?b[j- 2]:'.',
				  // ((b[j- 1]>='0')&&(b[j- 1]<='9') || (b[j- 1]>='A')&&(b[j- 1]<='Z') || (b[j- 1]>='a')&&(b[j- 1]<='z'))?b[j- 1]:'.');
		  }
      else if ( j && (j%8)==0 ) printf("  ");
      printf("%02X ", b[j]);
    }

	  if ( j%__MAX_CHAR__ )
	  {
		  k = __MAX_CHAR__ - j%__MAX_CHAR__;
		  for (  i=k ; i>0 ; i-- )
		  {
			  if ( (i%8)==0 ) printf("  ");
			  printf("   ");
		  }
	  }

	  printf(" | ");

	  k = __MAX_CHAR__ - sz%__MAX_CHAR__;
	  j = sz%__MAX_CHAR__;
	  for( i=0 ; i<__MAX_CHAR__ ; i++ )
	  {
		  if ( i && (i%8)==0 ) printf(" ");
		  printf("%c", ((b[sz-j+i]<0x20)||(b[sz-j+i]>=0x7F))?'.':b[sz-j+i]);
	  }
  }
	printf("\r\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n");
}
