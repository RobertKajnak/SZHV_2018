#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <crypt.h>
#include <string.h>
#include "prep.h"

///SECTION dictionary
typedef struct {
    char ** words;
    int wordcount;
    int width;
    int * iterators;
}dictionary;

dictionary* initDict(char ** words, int length, int width);
void next_candidate(dictionary * dict, char * word);

char ** get_words(char * file_name, int* wordcount);


///SECTION hash and username data
typedef struct {
    char * pwd;
    char * uname;
    char * hash;
} user;

user * get_users(char* shadow_filename, int* user_count);


///Expects a file pointer to the sadow file. File is assumed to be well-formatted
char* getSalt(FILE * file);

int main(int argc, char ** argv)
{
    //buildDict_fromTop250("dictionary/top250.txt","dictionary.txt");

    char * filename_shadow = "training-shadow.txt";
    FILE *f = fopen(filename_shadow,"r");
    ///obtain salt
    char * salt = getSalt(f);

    ///create the dictionary
    int wordcount;
    char ** words = get_words("dictionary.txt",&wordcount);
    ///TODO: add each individual character to the dictionary

    int user_count=0;
    user* users = get_users("training-shadow.txt",&user_count);

    /// ----- Multi-threaded should start here -------
    //Two threads calculate the hashes, while the rest of the threads check if
    //there is a match in the users. If the first two finish, they start going backwards
    dictionary *dict = initDict(words,wordcount,4);

    int i;
    for (i=0;i<user_count;i++)
    {
        if (users[i].pwd==NULL){
            while (dict.words[0][0]!='\0')
            {

            }
        }

    }
    /*f = fopen("guesses.txt","w");


    //fclose(f);
    struct crypt_data data;
    data.initialized = 0;

//    char *enc = crypt_r(key, salt, &data);
   // printf("EncryptedL %s\n", enc);dddddddddddd

    char pwd[] = "permissionproofreading";
    char shadow [] =".g5JI3K8smZB6UyE2Yh.0.";
    //printf(gnu_get_libc_version ());
    printf("%s\n%s\n%s",pwd,shadow,crypt("iloveyou","$1$M9$"));
    for (i=0;i<4096;i++)
    {
        if (strcmp(crypt(pwd,salts[i]),shadow)==0){
            printf("%d",i);
        }
    }*/

    fclose(f);
    return 0;
}


dictionary* initDict(char ** words, int wordcount, int width)
{
    dictionary *dict = malloc(sizeof(dictionary));
    dict->words = words;
    dict->width = width;
    dict->wordcount = wordcount;
    dict->iterators = calloc(width, sizeof(int));
    return dict;
}

void next_candidate(dictionary * dict, char * word)
{
    iterators[0]++;
    int i=0;
    //go through iterators, add them to the word count;
    while (iterators[i]>=dict->wordcount)
    {
        iterators[i]=0;
        if (i<width-1){
            iterator[i+1]++;
        }
        i++;
    }
    word = strcpy(sour);

}

char* getSalt(FILE * file)
{
    char * salt = malloc(16*sizeof(char));
    fscanf(file,"%*[^$]%*c%*[^$]%*c%[^$]",salt);
    rewind(file);
    return salt;
}

char ** get_words(char * file_name,int * wordcount)
{
    FILE *f = fopen(file_name,"r");
    int wc;
    fscanf(f,"%d", &wc);
    char ** word_list = malloc((++wc) * sizeof(char*));
    word_list[0] = '\0';

    int i;
    int cc,sc=sizeof(char);
    for (i=1;i<wc;i++)
    {
        fscanf(f,"%d", &cc);
        word_list[i] = malloc (cc*sc);
        fscanf(f,"%s",word_list[i]);
    }

    fclose(f);

    *wordcount = wc;
    return word_list;
}

user * get_users(char* shfn, int * user_count)
{
    FILE* f = fopen(shfn,"r");

    ///since both testing and training were ~8100, I am going to assume a starting value of 10k
    ///but dynamic expansion is incuded, just in case
    int cusers = 10000;
    user* users = malloc(cusers * sizeof(user));

    // lcd418:$1$1G$.g5JI3K8smZB6UyE2Yh.0.:17427::::::

    int sz=1,i;
    for (i=0;sz>0;i++)
    {
        users[i].uname = malloc(9*sizeof(char));
        users[i].hash = malloc(33 * sizeof(char));
        users[i].pwd = NULL;
        sz = fscanf(f,"%[^:]:$%*[^$]$%*[^$]$%[^:]%*[^\n]\n",users[i].uname,users[i].hash);

        if (i==cusers-1)
        {
            user * new_arr = malloc((cusers*2)*sizeof(user));
            memcpy(new_arr,users,cusers * sizeof(user));
            user* temp = users;
            users = new_arr;
            free(temp);
            cusers *=2;
        }
    }
    *user_count = i;
    fclose(f);
    return users;
}
