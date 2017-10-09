/* This program takes a text file formatted as 16x16 ints, which
 * represents an S-box, as its first argument and the name of the 
 * output pgm and txt file that you want to create as its second argument.
 *
 * A description of how the linear analysis is performed can be 
 * found by looking at the comments in the findprobs function.
 *
 * Once the linear analysis is performed, the probabilities are sorted
 * in their respective columns.
 *
 * The scaling of the greymap representation is not from deviation 0 to
 * deviation .5.  If this were the case, it would be very difficult to
 * see the differences between a random S-box and an AES S-box.  Instead,
 * the shade range of the greymap is from deviation 0 to deviation .1640625 (21/128) .
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void findprobs(double probs[][256], int* sbox);
int odd(int a);
void sortprobs(double probs[][256]);
int cmpfunc (const void * a, const void * b);
void topgm(double probs[][256], char* filename);
void totxt(double probs[][256], char* filename);

int main(int ac, char *av[])
{
    // Open file with S-box values.
    int Sbox[256];
    FILE *f = fopen(av[1], "r");
    if (f == NULL)
    {
        perror("Error opening file!\n");
        exit(1);
    }
    // Put values from file into Sbox.
    int i;
    for (i=0; i<256; i++)
        fscanf(f, "%d", &Sbox[i]);
    fclose(f);

    double probs[256][256];  // probs[0][] is not used.
    
    // Generate the 255x256 linear analysis table and put it in probs array.
    findprobs(probs, Sbox);
    
    // Sort the probabilities from top to bottom in each column.
    sortprobs(probs);

    // Write probabilities to text file and pgm file.
    totxt(probs, av[2]);
    topgm(probs, av[2]);
    
    // Print out the maximum deviation.
    double max=0;
    for (i=1; i<256; i++)
        if (probs[i][255] > max)
            max = probs[i][255];
    printf("maximum deviation: %g\n", max);
    
    return EXIT_SUCCESS;
}

void findprobs(double probs[][256], int *Sbox)
{
    int i, j, k;
    for (i=1; i<256; i++)  // i is the current Y being looked at and acts as a bitmask for Sbox outputs.
    {        
        for (j=0; j<256; j++)  // j is the current condition X being examined and acts as a bitmask for which Sbox inputs to test.
        {
            int equal=0, notequal=0;
            double sum=0;
            for (k=0; k<256; k++)  // k iterates through all potential Sbox values to be tested.
            {
                // If Sbox input k fits the condition to be tested.
                if (odd(j&k) || j==0)
                {
                    // Examine bits in corresponding Sbox value designated by i to see if the xor between them is 1.
                    if (odd(Sbox[k]&i))
                        equal++;
                    else
                        notequal++;
                    sum++;
                }
            }
            
            // Calculates deviation from 50%.
            if (equal>notequal)
               probs[i][j] = .5 - notequal/sum;
            else
               probs[i][j] = .5 - equal/sum;
        }
    }
}


// Performs the odd function on right-most 8 bits.
int odd(int a)
{
	int i, sum = 0;
	for (i=0; i<8; i++)
	{
		if (a%2==1)
			sum++;
		a = a >> 1;
	}
	return sum%2 == 1;
}

// Uses quick sort to sort all columns.
void sortprobs(double probs[][256])
{
    int i;
    for (i=1; i<256; i++)
        qsort(probs[i], 256, sizeof(double), cmpfunc);

}

// Compare function for quick sort.
int cmpfunc (const void * pa, const void * pb)
{
    double a = *((double*)pa);
    double b = *((double*)pb);
        if (a>=b)
            return 1;
        else
            return -1;
}

void topgm(double probs[][256], char *av2)
{
    char filename[128];
    strcpy(filename, av2);
    strcat(filename, ".pgm");
    
    FILE *f = fopen(filename, "w");
    if (f == NULL)
        perror("Error opening file!\n");
    
    fprintf(f, "P2\n255 256\n21\n");

    int i, j;
    for (i=255; i>0; i--)
    {
        for (j=1; j<256; j++)
            fprintf(f, "%d\t", (int)(probs[j][i] * 128));
        fprintf(f, "\n");
    }
    fclose(f);
}

void totxt(double probs[][256], char *av2)
{
    char filename[128];
    strcpy(filename, av2);
    strcat(filename, ".txt");
    
    FILE *f = fopen(filename, "w");
    if (f == NULL)
        perror("Error opening file!\n");
    
    int i, j;
    for (i=255; i>0; i--)
    {
        for (j=1; j<256; j++)
            fprintf(f, "%g\t", probs[j][i]);
        fprintf(f, "\n");
    }
    fclose(f);
}
