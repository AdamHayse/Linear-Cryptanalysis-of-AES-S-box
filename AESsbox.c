/* This program is used to derive an S-box using an irreducible polynomial of degree 8
 * and an affine transformation.  It can take polynomials of degree 8 that are reducible,
 * but the results will not be unique.
 *
 * The program can be called with -p or -w to print or write the S-box to a file.  You 
 * can also include -i to print or write the inverses before the affine transformation.
 * It can also be called with -l to simply print out a list of irreducible polynomials.
 *
 * The inverses of the polynomials is found by brute force.  All combinations of polynomial
 * multiplications are computed and then reduced using the irreducible polynomial to fit
 * within the Galois Field 256.  If the result of the multiplication and modular division is 1, 
 * then the inverse is found.  All values excluding 0 will have an inverse unless the provided
 * polynomial is not irreducible.
 * 
 * Once the inverse is obtained, the following affine transformation is done to compute 
 * the S-box value:
 * 
 *  | 1 0 0 0 1 1 1 1 |     | x0 |     | 1 |     | b0 |
 *  | 1 1 0 0 0 1 1 1 |     | x1 |     | 1 |     | b1 |
 *  | 1 1 1 0 0 0 1 1 |     | x2 |     | 0 |     | b2 |
 *  | 1 1 1 1 0 0 0 1 |  x  | x3 |  +  | 0 |  =  | b3 |
 *  | 1 1 1 1 1 0 0 0 |     | x4 |     | 0 |     | b4 |
 *  | 0 1 1 1 1 1 0 0 |     | x5 |     | 1 |     | b5 |
 *  | 0 0 1 1 1 1 1 0 |     | x6 |     | 1 |     | b6 |
 *  | 0 0 0 1 1 1 1 1 |     | x7 |     | 0 |     | b7 |
 */

#include <stdlib.h>
#include <stdio.h>

int multpoly(unsigned char a, unsigned char b);
int modpoly(int mult, int irrpoly);
void afftrans(int* invs, int* sbox);
int odd(int a);
void printsbox(int* sbox);
void writesbox(int* sbox);
void processargs(int ac, char** av);
void irrpolylist(void);

static int pflag = 0,  // print Sbox
           wflag = 0,  // write Sbox to file
           iflag = 0;

int main(int ac, char* av[])
{
    // Set flags based on arguments provided.
    processargs(ac, av);

    int irrpoly;
    int found[256];
    int invs[256];
	int sbox[256];

    // Get irreducible polynomial.
    printf("Enter hexadecimal representation of degree 8 polynomial: ");
    scanf("%x", &irrpoly);

    // Initialize found to 0.
    int i;
    for (i=0; i<256; i++)
        found[i] = 0;

	// Find inverse polynomials.
	invs[0] = 0;  // Zero has no inverse, so map it to itself.
    invs[1] = 1;  // The inverse of 1 is one because 1 time 1 equals 1.
    int j;
	for (i=2; i<256; i++)
    {
        j = i;
        while (!found[i] && j<256)  // If the inverse of the current i was not already found.
        {
            // Brute force test to see if i*j mod P(x) equals 1.
            if (modpoly(multpoly(i, j), irrpoly))
            {
                invs[i] = j;
                invs[j] = i;
                found[i] = found[j] = 1;  // Polynomials i and j are inverses of each other.
            }
            j++;
        }
        if (j==256 && !found[i])  // Only happens if irrpoly is not irreducible.
            invs[i] = 0;
    }

	// Perform affine transformation on inverse polynomials and put result in sbox.
    if (!iflag)
        afftrans(invs, sbox);

	if (pflag)
    {
        if (iflag)
            printsbox(invs);
	    else
            printsbox(sbox);
    }
    if (wflag)
    {
        if (iflag)
            writesbox(invs);
	    else
            writesbox(sbox);
    }
	return EXIT_SUCCESS;
}

// Performs modular division in GF(256) on poly using irrpoly.
int modpoly(int poly, int irrpoly)
{
	int i=256;       // Used to line up irrpoly with poly to perform division.
	while (i < poly)  // Shift irrpoly until it is in line with poly.
	{
		irrpoly = irrpoly << 1;
		i = i << 1;
	}
	while (poly >= 256)  // Perform division while poly is degree 8 or higher.
	{
		if (poly>=i && poly<i*2)  // If poly and irrpolly are aligned.
			poly = poly ^ irrpoly;  // Divide.
		irrpoly = irrpoly >> 1;  // Shift right until aligned
		i = i >> 1;
	}
	return (poly == 1);  // Return true if result of modular division is 1.
}

// Does all 64 multiplications between the 8 coefficients of each polynomial.
// It performs multiplication between coefficients even if the coefficients are 
// both 0.  Finally, it performs XOR between all 64 of these values to obtained
// the product.
int multpoly(unsigned char a,unsigned char b)
{
    int gf2s[64];  // Holds the 64 products.
    int result=0;
    int i, j;

    // This loop rotates the 8-bit values to the left and uses the iterator variables
    // i and j to remember the value of each bit
    for(i=0; i<8; i++)
    {
        a = a << 1 | a >> 7;  // Rotate left
        for (j=0; j<8; j++)
        {
            b = b << 1 | b >> 7;
            gf2s[i*8+j] = ((a%2==1)?(1<<(7-i)):0) * ((b%2==1)?(1<<(7-j)):0);
        }
    }

    for (i=0; i<64; i++)
        result = result ^ gf2s[i];
	return result;
}

// Performs affine transformation on inverses.
void afftrans(int *invs, int *sbox)
{
	int i, j;
	unsigned char aff = 0xf1, bprime = 0;
	for (i=0; i<256; i++)
	{
		for (j=0; j<8; j++)
		{
			if (odd(aff & invs[i]))
				bprime++;
			bprime = bprime << 7 | bprime >> 1;
			aff = aff << 1 | aff >> 7;
		}
		sbox[i] = bprime ^ 0x63;
		bprime = 0;
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

void printsbox(int *sbox)
{
	printf("  \t00\t01\t02\t03\t04\t05\t06\t07\t08\t09\t0a\t0b\t0c\t0d\t0e\t0f\n");
	printf("    -------------------------------------------------------------------------------------------------------------------------------\n");
    int i;
    for (i=0; i<256; i++)
    {
        if (i%16 == 0)
            printf("%.2x  |\t", i);
        printf("%.2x\t", sbox[i]);
        if (i%16 == 15)
            putchar('\n');
	}
}

void writesbox(int *sbox)
{
    FILE *f = fopen("AESsbox.txt", "w");
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

void processargs(int ac, char* av[])
{
    if (ac!=2 || *av[1] != '-' || av[1][1] == '\0')
    {
        perror("Specify -p (print), -w (write), or -i (inverse), or -l for irreducible polynomial list.");
        exit(1);
    }
    char *argptr = av[1] + 1;  // Point to character after '-'
    if (*argptr == '\0')
        exit(1);
    while (*argptr != '\0')
    {
        if (*argptr == 'p')
            pflag = 1;
        if (*argptr == 'w')
            wflag = 1;
        if (*argptr == 'i')
            iflag = 1;
        if (*argptr == 'l')
            irrpolylist();
        argptr++;
    }
    if (iflag && (!pflag && !wflag))
    {
        perror("You must also specify p (print) or w (write)");
        exit(1);
    }
}

void irrpolylist()
{
    int i, j, irrpoly = 0x101, found[256], count = 0, list[128];

    while (irrpoly < 0x200)
    {
        for (i=0; i<256; i++)
            found[i] = 0;

        for (i=2; i<256; i++)
        {
            j = i;
            while (!found[i] && j<256)  // If the inverse of the current i was not already found.
            {
                // Brute force test to see if i*j mod P(x) equals 1.
                if (modpoly(multpoly(i, j), irrpoly))
                    found[i] = found[j] = 1;  // Polynomials i and j are inverses of each other.
                j++;
            }
            if (j==256 && !found[i])  // Only happens if irrpoly is not irreducible.
                break;
            if (i==255)
            {
                list[count] = irrpoly;
                count++;
            }
        }
        irrpoly += 2;
    }

    for (i=0; i<count; i++)
    {
        printf("0x%x, ", list[i]);
        if (i%4 == 3)
            putchar('\n');
    }
    putchar('\n');
    exit(1);
}
