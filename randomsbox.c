/* This programs generates an Sbox by means of the pseudo-random number generator
 * provided by the rand function from the C library.
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

void writesbox(int*);

int main()
{
    int usednum[256], Sbox[256];
        
    int i;
    for (i=0; i<256; i++)
        usednum[i] = 0;

    srand(time(NULL));
    
    for (i=0; i<256; i++)
    {
        int randnum;
        while (usednum[randnum = rand()%256])
            ;
        Sbox[i] = randnum;
        usednum[randnum] = 1;
    }
    writesbox(Sbox);

    return EXIT_SUCCESS;
}

void writesbox(int *sbox)
{
	FILE *f = fopen("randomsbox.txt", "w");
    if (f == NULL)
        perror("Error opening file!\n");
    
    int i;
    for (i=0; i<256; i++)
    {
        fprintf(f, "%d\t", sbox[i]);
        if (i%16 == 15)
            fprintf(f,"\n");
    }
    fclose(f);
}
