#include "prep.h"

void toupper_a(char* char_arr);
void tolower_a(char* char_arr);

int _build_bruteforce(FILE* g);
int _build_top250(FILE* f, FILE* g);

void buildDict(char* source, char*dest, DICT_TYPE word_set)
{
    FILE *f = NULL;
    if (strcmp(source,"")!=0)
    {
         f= fopen(source ,"r");
    }
    FILE *g = fopen(dest,"w");

    fprintf(g,"%s","     \n"); /// should in no case be greater than 100k
    int wc=0;
    switch (word_set)
    {
        case (BRUTEFORCE):
            wc = _build_bruteforce(g);
            break;
        case (TOP250):
            wc = _build_top250(f,g);
            break;
        default:
            printf("Unexpected Error Occured.");
            break;
    }

    rewind(g);
    fprintf(g,"%d",wc);

    if (f!=NULL)
    {
        fclose(f);
    }
    fclose(g);

}

int _build_bruteforce(FILE* g)
{
    int wc=0;

    int i;

    for (i='!';i<'~';i++)
    {
        if (i=='`')
            continue;
        fprintf(g,"2 %c\n",i);
        wc++;

    }
    /*for (i='a';i<='z';i++)
        fprintf("1 %c\n",i);
    wc += 'z'-'a';

    for (i='A';i<='Z';i++)
        fprintf("1 %c\n",i);
    wc += 'Z' - 'A';

    for (i='9';i<='0';i++)
        fprintf("1 %c\n",i);
    wc += '9' - '0';*/


    return wc;
}


int _build_top250(FILE* f, FILE* g)
{
    int wc=0;
    char * word  = malloc(100*sizeof(char));

    int size=fscanf(f,"%*d\t%*f\t%*d\t%s",word);
    while (size>0)
    {
        ///write the read word
        fprintf(g,"%d %s\n",(int)strlen(word)+1,word);
        wc++;
        size=fscanf(f,"%*d\t%*f\t%*d\t%s",word);
    }
    return wc;

}

void buildDict_fromTop250_withCapitalization(char* source, char* dest)
{
    ///Reading and writing separately would be faster on a HDD,
    ///but since I'm using an SSD and it's only preparatory anyway, it's shorter to code
    FILE *f = fopen(source ,"r");
    FILE *g = fopen(dest,"w");

    char * word  = malloc(100*sizeof(char));

    fprintf(g,"%s","     \n"); /// should in no case be greater than 100k
    int size=fscanf(f,"%*d\t%*f\t%*d\t%s",word);
    int wc = 0;
    while (size>0)
    {
        ///write the read word
        fprintf(g,"%d %s\n",(int)strlen(word)+1,word);
        wc++;
        ///check if the first character is a letter
        if (isalpha(word[0]))
        {
            ///if lower case, make it upper, otherwise make it lower
            if (word[0]<='z' && word[0]>='a')
            {
                word[0] = toupper(word[0]);
            }
            else ///if character, and not lowercase, it's upper
            {
                word[0] = tolower(word[0]);
            }
            fprintf(g,"%d %s\n",(int)strlen(word)+1,word);
            wc++;
        }

        if (word[1]!='\0' && isalpha(word[1]))
        {
            if (word[1]<='z' && word[1]>='a')
            {
                toupper_a(word);
            }
            else
            {
                tolower_a(word);
            }
            fprintf(g,"%d %s\n",(int)strlen(word)+1,word);
            wc++;
        }


        size=fscanf(f,"%*d\t%*f\t%*d\t%s",word);
    }
    rewind(g);
    fprintf(g,"%d",wc);


    fclose(f);
    fclose(g);
}


void toupper_a(char* char_arr)
{
    int i;
    for (i=0;i<strlen(char_arr);i++)
    {
        char_arr[i] = toupper(char_arr[i]);
    }
}



void tolower_a(char* char_arr)
{
    int i;
    for (i=0;i<strlen(char_arr);i++)
    {
        char_arr[i] = tolower(char_arr[i]);
    }
}
