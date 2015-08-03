#include "rc4.h"

void rc4_init(unsigned char *s,unsigned char *key, unsigned long key_len)
{
    int i = 0, j = 0;
    char k[256] = {0};
    unsigned char tmp = 0;
	
    for(i = 0; i < 256; i++)
    {
        s[i] = i;
        k[i] = key[i % key_len];
    }
    for (i = 0; i < 256; i++)
    {
        j = (j + s[i] + k[i]) % 256;
        tmp = s[i];
        s[i] = s[j]; //����s[i]��s[j]
        s[j] = tmp;
    }
}

/* �ӽ��� */
void rc4_crypt(unsigned char *_s,unsigned char *Data, unsigned long data_len)
{
    int i = 0, j = 0, t = 0;
    unsigned long k = 0;
    unsigned char tmp;
	unsigned char s[256];
	
	memcpy(s,_s,sizeof(s)*sizeof(unsigned char));
	
    for(k = 0; k < data_len; k++)
    {
        i = (i + 1) % 256;
        j = (j + s[i]) % 256;
        tmp = s[i];
        s[i] = s[j]; //����s[x]��s[y]
        s[j] = tmp;
        t = (s[i] + s[j]) % 256;
        Data[k] ^= s[t];
    }
}