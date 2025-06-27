#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <peekpoke.h>

/*
 * pi6502
 *
 * This code isn't really intended for the 6502 (although I suspect with 
 * some work I could make it work with cc65) but instead is trying to 
 * test an algorithm for computing the digits of pi which could be reasonably
 * implemented on the Atari 400 and possibly even the Atari 2600.
 *
 * Hey, it's a hobby.
 *
 * Mark VandeWettering mvandewettering@gmail.com
 */


#define NDIGITS     (1000)
#define N           ((NDIGITS/2)+8)

/* The idea is for the sum to be contained in a character array in BCD
 * format, which the 6502 can process reasonably.  Each entry will contain
 * two digits.  In this simulation, it means that each entry in S[] and A[]
 * will store numbers between 0 and 100, noninclusive.
 */

void
printjiffies()
{
    long total = 256L* (256L * (*(unsigned char *) 18) +
                             (*(unsigned char *) 19)) +
                             (*(unsigned char *) 20) ;
            
    long s = (total + 30UL) / 60UL ;
    long m = s / 60UL ;
    long h ;
    s = s % 60UL ;
    h = m / 60UL ;
    m = m % 60UL ;
    
    printf("\n[%02ld:%02ld:%02ld]\n", h, m, s) ;
}



unsigned char S[N+1] ;
unsigned char A[N+1] ;
unsigned char T[N] ;

void
InitS()
{
    int i ;
    for (i=0; i<N; i++) S[i] = 0 ;
}

void
AddSA()
{
    int i ;
    int t, c = 0 ;

    for (i=N-1; i>=0; i--) {
        t = S[i] + A[i] + c  ;
        S[i] = t % 100 ;        /* will be a cheap bitwise op in BCD */
        c = t / 100 ;           /* will be a cheap bitwise op in BCD */
    }
}

void
SubSA()
{
    int i  ;
    int t, c = 0 ;

    for (i=N-1; i>=0; i--) {
        t = S[i] - A[i] - c  ;
        if (t < 0) {
            t += 100 ;
            S[i] = t ;
            c = 1 ;
        } else {
            S[i] = t % 100 ;
            c = 0 ;
        }
    }
}

void 
Mul(unsigned char dst[N], unsigned char src[N], unsigned int k)
{
    int i ;
    int tmp = 0 ;
    for (i=N-1; i>=0; i--) {
        tmp += src[i] * k ;
        dst[i] = tmp % 100 ;
        tmp /= 100 ;
    }
}


void
Divide(unsigned char dst[N], unsigned char src[N], unsigned long n)
{
    int i ;
    unsigned long t = 0, q, r ;

    for (i=0; i<N; i++) {
        t *= 100UL ;
        t += src[i] ;
        q = t / n ;             /* solve by repeated subtraction on 6502 */
        r = t % n ;
        dst[i] = q ;
        t = r ;
    }
}

void
DivideA(unsigned long n)
{
    Divide(A, A, n) ;
}

void
Init(char A[N], int k)
{
    int i ;
    A[0] = k ;
    for (i=1; i<N; i++) {
        A[i] = 0 ;
    }

}

int
IsZeroA()
{
    int i ;
    for (i=N; i>=0; i--)
        if (A[i]) return 0 ;

    return 1 ;
}

#define NPERLINE    (10)

void
Print(char *s, char a[N])
{
    int i, j, r, b ;
    int nrows ;

    nrows = N / NPERLINE ;
    if (N % NPERLINE) nrows ++ ;

    for (i=0, r=0; i<50; i += NPERLINE, r++) {
        printf("%s[%04d-%04d] : ", s, i*2+1, 2*(i+NPERLINE)) ;
        for (j=0; j<NPERLINE; j++) {
            if (i+j < N)
                printf("%02d", a[i+j]) ;
        }
        printf("\n") ;
    }
#if 1

    b = (NDIGITS / 2) ;
    b -= (b % NPERLINE) ;
    b -= 4 * NPERLINE ;

    printf("... [%d digits skipped]\n", 2*(b-i)) ;

    for (i=b, r=0; i<NDIGITS/2; i += NPERLINE, r++) {
        printf("%s[%04d-%04d] : ", s, i*2+1, 2*(i+NPERLINE)) ;
        for (j=0; j<NPERLINE; j++) {
            if (i+j < N) printf("%02d", a[i+j]) ;
        }
        printf("\n") ;
    }
#endif
}

void
ComputeA(int k, unsigned long base, unsigned long n)
{
    int i ;

    Init(A, k) ;
    DivideA(base) ;

    for (i=1; i<n; i++)
        DivideA(base*base) ;

    DivideA(2UL * n - 1UL) ;
}



int
main()
{
    int sgn ;
    unsigned long n ;
    unsigned char antic_value = PEEK(559) ;

    printf("\npi6502 running on the atari800\n") ;
    printf("by mvandewettering@gmail.com\n") ;
    printf("computing pi to %d digits\n", NDIGITS) ;
    printf("%d bytes of memory used.\n", 
            sizeof(S) + sizeof(A) + sizeof(T)
          ) ;

    // POKE(559, 0) ;
    InitS() ;

    printf("summing 16 * atan(1/5)")  ;
    n = 1UL ;
    sgn = 1 ;
    do {
        printf(".") ;
        if (n == 1) {
            Init(T, 16) ;
            Divide(T, T, 5UL) ;
            Divide(A, T, 1) ;
        } else {
            Divide(T, T, 25UL) ;
            Divide(A, T, 2UL * n - 1UL) ;
        }
        if (sgn > 0) {
            AddSA() ;
        } else {
            SubSA() ;
        }
        n = n + 1 ;
        sgn = 0 - sgn ;
    } while (!IsZeroA()) ;

    printjiffies() ;

    printf("summing 4 * atan(1/239)") ;

    Init(T, 4) ;
    Divide(T, T, 239) ;

    n = 1 ;
    sgn = -1 ;
    do {
        printf(".") ;

        if (n == 1) {
            Init(T, 4) ;
            Divide(T, T, 239UL) ;
            Divide(A, T, 1) ;
        } else {
            Divide(T, T, 239UL*239UL) ;
            Divide(A, T, 2 * n - 1) ;
        }
        if (sgn > 0) {
            AddSA() ;
        } else {
            SubSA() ;
        }
        n = n + 1 ;
        sgn = 0 - sgn ;
    } while (!IsZeroA()) ;

    Mul(S, S, 10) ;

    printjiffies() ;

    printf("\n") ;
    Print("PI", S) ;

    // POKE(559, antic_value) ;
    for (;;) ;

    return 0 ;
}
