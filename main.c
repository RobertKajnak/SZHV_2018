#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <crypt.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdarg.h>


#include "prep.h"


///SECTION dictionary -----------------------------------------
typedef struct {
    char ** words;
    int wordcount;
    int width;
    int * iterators;
    int end_word;
}dictionary;

dictionary* initDict_all(char ** words, int length, int width);//I forgot overloading was C++ only
dictionary* initDict(char ** words, int wordcount, int width, int start_section, int end_section);
void next_candidate(dictionary * dict, char * word);

char ** get_words(char * file_name, int* wordcount);


///SECTION hash and username data -------------------------------
struct user;
typedef struct{
    struct user * next_user; ///For the hash table
    char * uname;
    char * hash;
} user;
int cusers;
user ** users;
///loads the users and hashes from the shadow file
void users_read(char* shadow_filename, int* user_count);

void user_remove(char * hash,char * password);


///SECTION Misc --------------------------------------------------
///Expects a file pointer to the sadow file. File is assumed to be well-formatted
char* getSalt(FILE * file);

FILE * guesses_output_file;///TODO -change back

///SECTION Multithread --------------------------------------------
///Prototype for going through the dictionary and testing the hashes
const int THREADCOUNT = 8;

typedef struct{
    int ind;
    int dictcount;
    char *** words;
    int *wordcounts;
    int *widths;
    char * salt;
} limits; ///Yes, it is now technically all the thread parameters. I'm not renaming it
limits* limits_init(int ind, int dictcount,char***words, int* wordcounts, int* widths, char* salt);

void *guess(void * lims);

pthread_mutex_t mutex_print, mutex_thread_init;
pthread_mutexattr_t mattr_print, mattr_thread_init;
void printf_r(const char*format, ...);
void fprintf_r(FILE *f,const char*format, ...);

int main(int argc, char ** argv)
{
    //buildDict("dictionary/top250.txt","dictionary_top250.txt",TOP250);
    //return;

    char * filename_shadow = "training-shadow.txt";
    FILE *f = fopen(filename_shadow,"r");
    ///obtain salt
    char * salt = getSalt(f);
    //printf("%s\n\n",salt);
    fclose(f);

    ///create the dictionaries
    int dictcount = 1;
    char *** words = malloc(dictcount * sizeof (char**));
    int *widths = malloc(dictcount*sizeof(int));
    int *wordcounts = malloc (dictcount*sizeof(int));

    ///TODO: add each individual character to the dictionary
    ///Password types
    /*
        1. Top 250 stuff   --- 735
        2. single words
        3. single word with random capitalization
        4. single word with random letter replacement
        5. 2 words stuck together
        6. words from training and test
    */
    widths[0]=3;
    words[0]= get_words("dictionary_top250.txt",&(wordcounts[0]));

    ///add usernames and passwords
    int user_count=0;
    users_read("training-shadow.txt",&user_count);

    ///TODO: switch this back to stdout;
    guesses_output_file = fopen("guesses.txt","w");

    /// ----- Multi-threaded should start here -------
    ///Threadless time:103-114s; 777 Guesses
    ///Threaded time(4 threads): 29s; 777 Guesses

    //printf("Wordcount = %d\n",wordcount);
    //Single thread version:
    /*dictionary *dict = initDict_all(words,wordcount,2);

    char word [100];

    struct crypt_data data;
    data.initialized = 0;

    next_candidate(dict,word);
    int i=0;
    while (word[0]!='\0')
    {
        i++;
        char *enc = crypt_r(word, salt, &data);
        ///this is the point where I read, that we are guaranteed that the salt length is 2
        user_remove(enc+6,word);
        next_candidate(dict,word);
    }
    printf("Words Attempted(single Thread) :%d\n",i);*/


    //pthread_mutex_t mutex; ///TODO figure this out
    pthread_mutexattr_init(&mattr_print);
    pthread_mutex_init(&mutex_print,&mattr_print);

    pthread_mutexattr_init(&mattr_thread_init);
    pthread_mutex_init(&mutex_thread_init,&mattr_thread_init);

    pthread_t thread_id[THREADCOUNT];
    pthread_attr_t thread_attr[THREADCOUNT];

    int i;
    for (i=0;i<THREADCOUNT;i++)
    {
        pthread_attr_init(&thread_attr[i]);
        limits *lims = limits_init(i,dictcount,words,wordcounts,widths,salt);
        pthread_create(&thread_id[i],&thread_attr[i],guess,(void*)lims);
    }

    for (i=0;i<THREADCOUNT;i++){
        pthread_join(thread_id[i],NULL);//the main fuction waits for the threads
    }

    return 0;
}

void* guess(void* lims_vp)
{
    limits *lims = (limits*)lims_vp;
    int ind = lims->ind;

    int dict_ind;
    for (dict_ind=0;dict_ind<lims->dictcount;dict_ind++)
    {
        int start = ind*lims->wordcounts[dict_ind]/THREADCOUNT;
        int end = (ind+1)*lims->wordcounts[dict_ind]/THREADCOUNT;

        pthread_mutex_lock(&mutex_thread_init);
        dictionary *dict = initDict(lims->words[dict_ind],lims->wordcounts[dict_ind],lims->widths[dict_ind],start,end);
        pthread_mutex_unlock(&mutex_thread_init);

        char word [100];

        struct crypt_data data;
        data.initialized = 0;

        next_candidate(dict,word);
        long i=0;
        while (word[0]!='\0')
        {
            i++;
            char *enc = crypt_r(word, lims->salt, &data);
            ///this is the point where I read, that we are guaranteed that the salt length is 2
            user_remove(enc+6,word);
            next_candidate(dict,word);
        }
        printf_r("Words Attempted using dictionary %d:%ld\n",dict_ind,i);
    }

    return NULL;
}

dictionary* initDict(char ** words, int wordcount, int width, int start_section, int end_section)
{
    dictionary *dict = malloc(sizeof(dictionary));
    dict->words = words;
    dict->width = width;
    //if (width>1)
        dict->end_word = end_section>wordcount?wordcount:end_section;
    //else
     //   dict->wordcount = end_section-start_section;
    dict->wordcount = wordcount;
    dict->iterators = calloc(width, sizeof(int));
    dict->iterators[width-1] = start_section;
    return dict;
}

dictionary* initDict_all(char ** words, int wordcount, int width)
{
    return initDict(words,wordcount,width,0,wordcount);
}



void next_candidate(dictionary * dict, char * word)
{
    dict->iterators[0]++;
    int i=0;
    //go through iterators, add them to the word count;
    while (i<dict->width && dict->iterators[i]>=dict->wordcount)
    {
        dict->iterators[i]=0;
        if (i<dict->width-1)
        {
            dict->iterators[i+1]++;
            if (dict->iterators[dict->width-1]==dict->end_word)
            {
                word[0]='\0';
                return;
            }
        }
        i++;
    }

    for (i=dict->width-1;i>0;i--)
    {
        if (dict->iterators[i]!=0 && dict->iterators[i-1] == 0)
        {
            dict->iterators[i-1]++;
        }
    }

    int offset=0;
    for (i=0;i<dict->width;i++)
    {
        if (dict->iterators[i]==0)
        {
            if (i==0)
            {
                word[0]='\0';
            }
            break;
        }


        ///TODO: create a function that does both in a single pass
        strcpy((word+offset),(dict->words)[dict->iterators[i]]);
        offset = strlen(word);

    }
    ///TODO: continue here
    //word = strcpy(sour);

}

char* getSalt(FILE * file)
{
    char * salt = malloc(16*sizeof(char));
    //fscanf(file,"%*[^$]%*c%*[^$]%*c%[^$]",salt);
    salt[0] = '$';
    fscanf(file,"%*[^$]$%[^$]$",(salt+1));
    int offset = strlen(salt);
    salt[offset]='$';
    fscanf(file,"%[^$]",(salt+offset+1));
    offset = strlen(salt);
    salt[offset+1]='\0';
    salt[offset]='$';
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


void user_insert(user * usr)
{
    unsigned long idx= (usr->hash[0]<<24) + (usr->hash[1]<<16)+(usr->hash[2]<<8)+usr->hash[3]+13372;
    //memccpy(&idx,usr->hash,4,1); /// faster than shifting
    user **slot = &users[idx%cusers];
    if (*slot == NULL) ///calloc used, this should be fine
    {
        *slot = usr;
        return;
    }

    ///if first bucket empty, go through the list
    while ((*slot)->next_user != NULL)
    {
        slot = (user**)&((*slot)->next_user);
    }
    (*slot)->next_user = (struct user*)usr;
}

void user_remove(char * hash,char * password)
{
    unsigned long idx = (hash[0]<<24) + (hash[1]<<16)+(hash[2]<<8)+hash[3]+13372; ///TODO:remove this addition
    //memccpy(&idx,hash,4,1); /// faster than shifting
    user **slot = &users[idx%cusers];
    //user **prev;
    while (*slot!=NULL)
    {
        if (strcmp((*slot)->hash,hash) == 0)
        {

            fprintf_r(guesses_output_file ,"%s:%s\n",(*slot)->uname,password);
            ///TODO fix memory leak
            //prev = slot;
            *slot = (user*)((*slot)->next_user);
            //free(*prev);
        }
        else
        {
            slot=(user**)&((*slot)->next_user);
        }


    }
}

///Since we have the hashes already, the easiest way is probably a hash table
///with chaining. Open addressing may be faster for this load in general, but
///given that multiple users can have the same password and they are ought to be
///removed when found, IMO a (separate) chaining apporach is better here

///Also, optimal load factor should be around log(2)=~0.7, but since space is not an issue
///anyway, 8+32byte for the content and 12 for the pointers, usage will be lower than 1MB even with
///0.3-0.4 load factor, so I'll go with 3*expected entry size, even if I'm wasting 400kB of RAM.
///I don't have the space shuttle's limitations :P
void users_read(char* shfn, int * user_count)
{
    FILE* f = fopen(shfn,"r");

    ///since both testing and training were ~8100, I am going to assume a starting value of 10k
    cusers = 30013;

    users = calloc(cusers, sizeof(user*));
    user* user;
    int sz=1,i;
    for (i=0;sz>0;i++)
    {
        user = malloc(sizeof(user));
        user->uname = malloc(9*sizeof(char));
        user->hash = malloc(33 * sizeof(char));
        user->next_user = NULL;

        sz = fscanf(f,"%[^:]:$%*[^$]$%*[^$]$%[^:]%*[^\n]\n",user->uname,user->hash);
        user_insert(user);

    }
    *user_count = i;
    fclose(f);
    //return users;
}

limits *limits_init(int ind, int dictcount,char***words, int* wordcounts, int* widths, char* salt)
{
    limits *lims = malloc(sizeof(limits));
    lims->ind = ind;
    lims->words = words;
    lims->wordcounts = wordcounts;
    lims->widths = widths;
    lims->dictcount = dictcount;
    lims->salt = salt;
    return lims;
}

void fprintf_r(FILE *f,const char*format, ...)
{
    va_list args;
    va_start(args,format);
    pthread_mutex_lock(&mutex_print);
    vfprintf(guesses_output_file,format, args);
    fflush_unlocked(f);
    pthread_mutex_unlock(&mutex_print);
    va_end(args);
}

void printf_r(const char*format, ...)
{
    va_list args;
    va_start(args,format);
    pthread_mutex_lock(&mutex_print);
    vprintf(format, args);
    fflush_unlocked(stdout);
    pthread_mutex_unlock(&mutex_print);
    va_end(args);
}
