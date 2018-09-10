#include "prep.h"

void toupper_a(char* char_arr);
void tolower_a(char* char_arr);

int _build_bruteforce(FILE* g);
int _build_top250(FILE* f, FILE* g);
int _build_compounds(FILE* f, FILE* g);
int _build_mods(FILE *f,FILE*g);

void buildDict(char* source, char*dest, DICT_TYPE word_set)
{
    FILE *f = NULL;
    if (strcmp(source,"")!=0)
    {
         f= fopen(source ,"r");
    }
    FILE *g = fopen(dest,"w");

    fprintf(g,"%s","      \n"); /// should in no case be greater than 100k
    int wc=0;
    switch (word_set)
    {
        case (BRUTEFORCE):
            wc = _build_bruteforce(g);
            break;
        case (TOP250):
            wc = _build_top250(f,g);
            break;
        case (MOST_USED):

            break;
        case (COMPOUNDS):
            wc = _build_compounds(f,g);
            break;
        case (MODS):
            wc = _build_mods(f,g);
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

///Returns the number of characters left in the string.
char* remove_non_alhpa(char * a)
{
    char * new_word = malloc(strlen(a)+1);
    new_word[0]='\0';
    int i,j;
    for (i=0,j=0;i<strlen(a);i++)
    {
        if (isalpha(a[i]))
        {
            new_word[j] = a[i];
            j++;
        }

    }
    if (new_word[0]=='\0')
    {
        free(new_word);
        return NULL;
    }
    else
        return new_word;
}

void insert_word(char*word,char ** wlist, int * frequency,int * iter)
{
    if (word==NULL)
        return;
    int i;
    for (i=0;i<=*iter;i++)
    {
        if (strcmp(word,wlist[i])==0)
        {
            frequency[i]++;
            return;
        }

    }
    (*iter)++;
    wlist[*iter]=word;
    frequency[*iter]++;
    return;
}

void sort_words(char ** wlist, int* frequency, int nr_words)
{
    int i,j;
    //char swapped=0;
    char* temp_c;
    int temp_i;
    for (i=0;i<nr_words-1;i++)
    {

        for (j=0; j<nr_words-1;j++)
        {
            if (frequency[j]<frequency[j+1])
            {
                temp_i = frequency[j];
                frequency[j] = frequency[j+1];
                frequency[j+1] = temp_i;

                temp_c = wlist[j];
                wlist[j] = wlist[j+1];
                wlist[j+1]=temp_c;
            }
        }
    }
}

void buildDictFromAll(char * output_file)
{
    FILE * g = fopen(output_file,"w");
    fprintf(g,"%s","     \n");

    char * file_names[] ={ "dictionary/gutenberg/Adventures of Huckleberry Finn.txt",
                    "dictionary/gutenberg/Alices Adventures in Wonderland.txt",
                    "dictionary/gutenberg/Frankenstein.txt",
                    "dictionary/gutenberg/Grimms Fairy Tales.txt",
                    "dictionary/gutenberg/Gullivers Travels.txt",
                    "dictionary/gutenberg/Pride and Prejudice.txt",
                    "dictionary/gutenberg/The Adventures of Sherlock Holmes.txt",
                    //"dictionary/gutenberg/The Divine Comedy.txt",
                    "dictionary/gutenberg/The Kama Sutra of Vatsyayana.txt",
                    "dictionary/gutenberg/The Prince.txt"
                    };
    int i, l=1;
    FILE * f;
    char word[1000];
    char **word_list = malloc(500000*sizeof(char*));
    int *frequency = malloc(500000*sizeof(int));
    int iter = -1;
    int wc =0;
    for (i=0;i<9;i++)
    {
        f = fopen(file_names[i],"r");
        l=1;
        while (l>0)
        {
            l=fscanf(f,"%s",word);
            tolower_a(word);
            insert_word(remove_non_alhpa(word),word_list,frequency,&iter);
        }

        fclose(f);
        printf("Closed file %s\n",file_names[i]);
    }

    sort_words(word_list,frequency,iter);

    int k;
    for (k=0;k<=iter;k++)
    {
        fprintf(g,"%d %s\n",(int)strlen(word_list[k])+1,word_list[k]);
        wc++;
    }
    printf("%d\n",wc);
    free(word_list);

    rewind(g);
    fprintf(g,"%d",wc);
    fclose(g);
}

///wordlengths initialized inside; don't  do it  beforehand
char ** get_words_2(FILE *f,int * wordcount, int ** wordlengths)
{
    int wc;
    fscanf(f,"%d", &wc);
    char ** word_list = malloc((++wc) * sizeof(char*));
    *wordlengths = malloc(wc * sizeof(int));
    word_list[0] = '\0';

    int i;
    int cc,sc=sizeof(char);
    for (i=0;i<wc;i++)
    {
        fscanf(f,"%d", &cc);
        word_list[i] = malloc (cc*sc);
        (*wordlengths)[i] = cc;
        fscanf(f,"%s",word_list[i]);
    }


    *wordcount = wc;
    return word_list;
}

int _build_compounds(FILE* f, FILE* g)
{
    int wc = 0,newwc=0;
    int *wcs;
    char ** words = get_words_2(f, &wc, &wcs);

    int i,j;
    char newWord [100];
    printf("Total: %d\n",wc);
    for (i=0;i<wc;i++)
    {
        if (i%1000==0)
        {
            printf("Current: %d\n",i);
        }
        for (j=0;j<wc;j++)
        {
            if (wcs[j]+wcs[i]==22)
            {
                strcat(newWord,words[i]);
                strcat(newWord,words[j]);
                tolower_a(newWord);
                fprintf(g,"%d %s\n",22,newWord);
                newWord[0]='\0';
                newwc++;
            }
        }
    }
    return newwc;
}


char* try_replace(char * source,char old_char, char * new_char)
{
    if (strchr(source,old_char)==NULL)
        return NULL;

    int i,j=0,k;
    char * newWord = malloc(100);

    for (i=0;i<strlen(source);i++)
    {
        if (source[i]==old_char)
        {
            for (k=0;k<strlen(new_char);k++)
            {
                newWord[j++] = new_char[k];
            }
        }
        else
        {
            newWord[j]=source[i];
            j++;
        }
    }
    newWord[j]='\0';
    return newWord;
}

int _build_mods(FILE *f,FILE*g)
{
    int wc = 0,newwc=0;
    int *wcs;
    char ** words = get_words_2(f, &wc, &wcs);

    int i,j,jmax=9;

    char olds[]={'p','a','n','m','e','l','c','d','o'};
    char *news[]={"|>","/\\","|\\|","/V\\","[-","|_","<","|)","()"};

    char* newWord;
    char *word;
    for (i=0;i<wc;i++)
    {
        word= words[i];

        for (j=0;j<jmax;j++)
        {
            newWord = try_replace(word,olds[j],news[j]);

            if (newWord!=NULL)
            {
                fprintf(g,"%d %s\n",(int)strlen(newWord),newWord);
                newwc++;
            }
        }

        newWord = malloc(100);
        strcpy(newWord,word);
        toupper_a(newWord);
        fprintf(g,"%d %s\n",(int)strlen(newWord),newWord);
        newwc++;

        for (j=0;j<strlen(word);j++)
        {
            word[j]=toupper(word[j]);
            fprintf(g,"%d %s\n",(int)strlen(word),word);
            newwc++;
            word[j]=tolower(word[j]);
        }
        fprintf(g,"%d %s\n",(int)strlen(word),word);
        newwc++;

    }
    return newwc;

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
