#include<stdio.h>
#include<ctype.h>
#include <string.h>

//hash function
unsigned long hash(unsigned char *str){
    unsigned long hash = 5381;
    int c;
    while (c = *str++)
        hash = ((hash << 5) + hash) + c;
    return hash;
}

int main(){
    int word_count = 0;
    FILE *fptr;
    // Open a file in read mode
    fptr = fopen("gutenberg.org_cache_epub_100_pg100.txt", "r");
    // Store the content of the file
    char line[100];
    char word[50];
    // Read the content and print it
    while(fgets(line, 100, fptr)) {
        int w_counter = 0;
        //get words from this line
        for(int i = 0; i < 100; i++){
            if(line[i] == ' '){
                word[w_counter] = '\0';
                printf("%lu\n", hash(word));
                memset(word, 0, sizeof(word));
                word_count += 1;
                w_counter = 0;
                continue;
            }
            if(isalnum(line[i])){
                //printf("%c", line[i]);
                word[w_counter] = line[i];
                w_counter += 1;
            }
        }
    }
    printf("%d\n", word_count);
    // Close the file
    fclose(fptr);
}
