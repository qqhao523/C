
#ifndef BASE64_H_INCLUDED
#define BASE64_H_INCLUDED

#include <stdio.h>
#include <string.h>

char base64Alphabet[]=
{'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/','='};

unsigned char* base64Encode(const char* source, const int sourceLength)
{
    unsigned int padding = sourceLength%3;
    unsigned int resultLength = sourceLength%3 ? ((sourceLength)/3 + 1)*4 : (sourceLength)/3*4;
    unsigned int i=0, j=0;

    unsigned char* result = (unsigned char*)malloc(resultLength + 1);
    memset(result, 0, resultLength+1);

    unsigned char temp = 0;
    for (i=0,j=0; i<sourceLength; i+=3, j+=4)
    {
        if (i+2 >= sourceLength)
        {
            result[j] = (source[i]>>2) & 0x3F;
            if (padding==1)
            {
                //����paddingʵ��Ϊ2
                result[j+1] = ((source[i] & 0x03)<<4 ) & 0x3F;
                result[j+2] = 0x40;
                result[j+3] = 0x40;
                break;
            }
            else if (padding==2)
            {
                //����paddingʵ��Ϊ1
                result[j+1] = (((source[i] & 0x03)<<4) | ((source[i+1]>>4) & 0x0F));
                result[j+2] = ((source[i+1] & 0x0f)<<2) & 0x3F;
                result[j+3] = 0x40;
                break;
            }
        }

        result[j] = (source[i]>>2) & 0x3F;//�����λҪ��Ϊ0
        result[j+1] = (((source[i] & 0x03)<<4) | ((source[i+1]>>4) & 0x0F));//0x03��ֻȡ�����λ,����λΪ0�� 0x0F(ֻȡ����λ������λΪ0)
        result[j+2] = (((source[i+1] & 0x0f)<<2) | ((source[i+2]>>6) & 0x03));
        result[j+3] = (source[i+2] & 0x3F);
    }

    for ( j=0; j<resultLength; ++j)
    {
        result[j] = base64Alphabet[result[j]];
    }

    return result;
}



#endif // BASE64_H_INCLUDED
