#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <crypt.h>
#include <string.h>

void getOffset(int *offset,int x);

struct dictionary{
    char ** words;
    int width;
    int * iterators;
}

dictionary initDict(char ** words, int width);

int main()
{

    FILE *f;
    f = fopen("guesses.txt","w");
    ///Generate all possible salts:
    char salts [4096][2] ;
    int i,j;
    int offseti, offsetj;

    offseti = '0';
    for (i=0;i<64;i++)
    {
        offsetj = '0';
        for (j=0;j<64;j++)
        {
            //fprintf(f,"%c%c\n",(offseti+i),(offsetj+j));
            salts[i*64+j][0]=offseti+i;
            salts[i*64+j][1]=offsetj+j;
            getOffset(&offsetj, j);
        }
        getOffset(&offseti, i);
    }
    //fclose(f);
    struct crypt_data data;
    data.initialized = 0;

//    char *enc = crypt_r(key, salt, &data);
   // printf("EncryptedL %s\n", enc);

    char pwd[] = "permissionproofreading";
    char shadow [] =".g5JI3K8smZB6UyE2Yh.0.";
    //printf(gnu_get_libc_version ());
    printf("%s\n%s\n%s",pwd,shadow,crypt("iloveyou","$1$M9$"));
    for (i=0;i<4096;i++)
    {
        if (strcmp(crypt(pwd,salts[i]),shadow)==0){
            printf("%d",i);
        }
    }




    return 0;
}


dictionary initDict(char ** words, int width);

void getOffset(int* offset, int x){
    if (*offset+x=='9')
        *offset += 'A' - '9' -1;
    else if (*offset+x=='Z')
        *offset += 'a' - 'Z' -1;
    else if (*offset + x =='z')
        *offset = '.'-x-1;
    else if (*offset +x =='.')
        *offset = '/' -x-1;
}
