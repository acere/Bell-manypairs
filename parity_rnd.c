#include <stdio.h>
#include <stdlib.h>

/*
Calculates the parity for each chunk of length N
and returns the average result over all the possible chunks.
for N > 1, the sequence of results is shuffled randomly before evaluating the
correlation to allow a monte-carlo like estimation of the error.

2016.11.22 Alessandro Cere
*/

/*Fisher-Yates algorithm*/
static int rand_int(int n) {
    int limit = RAND_MAX - RAND_MAX % n;
    uint16_t rnd;

    do {
        rnd = rand();
    }
    while (rnd >= limit);
    // printf("%d\n", rnd);
    return rnd % n;
}

void shuffle(char *array, int n) {
    int i, j;
    char tmp;

    for (i = n - 1; i > 0; i--) {
        j = rand_int(i + 1);
        tmp = array[j];
        array[j] = array[i];
        array[i] = tmp;
   }
}

int parityOf(unsigned int *vec, int N){
    int parity = 0;
    for (int k = 0; k < N; k++){
        if((vec[k] & 0x1) != ((vec[k] >> 1) & 0x1)){
            parity = !parity;
        }
        // parity = (parity + ((vec[k] & 0x1) ^ ((vec[k] >> 1) & 0x1))) & 0x1;
    }
    return 2 * parity - 1;
}

int main(int argc, char *argv[]) {
    int N;
    if (sscanf (argv[1], "%i", &N)!=1) { printf ("error - not an integer"); }
    unsigned int vec_n[N];
    int counter = 0;
    int p = 0;

    FILE *inhandle;
    long lSize;
    char *buffer;

    inhandle=fopen(argv[2], "r");
    if( !inhandle ) perror("Error opening file"), exit(1);
    fseek(inhandle, 0L , SEEK_END);
    lSize = ftell(inhandle);
    rewind(inhandle);

    /* allocate memory for entire content */
    buffer = calloc( 1, lSize+1 );
    if(!buffer) fclose(inhandle), fputs("memory alloc fails", stderr), exit(1);

    /* copy the file into the buffer, then free the file*/
    if(1!=fread(buffer , lSize, 1 , inhandle))
      fclose(inhandle), free(buffer), fputs("entire read fails", stderr), exit(1);
    fclose(inhandle);

    /* In case of N>1 it adds the randomization procedure */
    if(N>1){
        /* seed the pseudo-random number generator */
        FILE *urandom;
        unsigned int seed;

        urandom = fopen ("/dev/urandom", "r");
        if (urandom == NULL) {
            fprintf (stderr, "Cannot open /dev/urandom!\n");
            exit (1);
        }
        fread (&seed, sizeof (seed), 1, urandom);
        srand (seed);

        /* Shuffle the array. Be patient...*/
        shuffle(buffer, lSize);
    }

    for(int k=0; k < (int) lSize/N; k++){
        for (int i=0; i < N; i++){
            vec_n[i] = buffer[k*N+i] - '0';
        }

        // for (int i=0; i < N; i++){
        //     printf("%u", vec_n[i]);
        // }
        // printf("\n");

        counter++;
        p += parityOf(vec_n, N);
    }

   printf("%2.12f\n", (float)p/counter);

   /* final cleanup */
   free(buffer);
   return 0;
}