/****************************************************************************/
/*                                                                          */
/* DRIVER.C  -  This file contains the code to compare C vs. ASM for the    */
/*              latanal test case.                                          */
/*                                                                          */
/****************************************************************************/
#include <stdio.h>
#include <time.h>

int     ret_stub, ret_c;

clock_t time_stub (int *i);
clock_t time_c    (int *i);

/****************************************************************************/
/* TOP LEVEL DRIVER FOR THE TEST.                                           */
/****************************************************************************/
int main()
{
    int i, novhd;
    clock_t t_overhead, t_start, t_stop, t_stub, t_c;
 
    /************************************************************************/
    /* COMPUTE THE OVERHEAD OF CALLING CLOCK TWICE TO GET TIMING INFO.      */
    /************************************************************************/
    t_start    = clock();
    t_stop     = clock();
    t_overhead = t_stop - t_start;

    /************************************************************************/
    /* CALL THE INDIVIDIUAL TIMING ROUTINES.                                */
    /************************************************************************/
    t_stub = time_stub(&ret_stub) - t_overhead;
    t_c    = time_c(&ret_c)       - t_overhead;

    novhd = t_c - t_stub;

    /************************************************************************/
    /* PRINT TIMING RESULTS                                                 */ 
    /************************************************************************/ 
    printf("Puzzle: Stb: %8d, C: %8d, NoOhd: %8d\n", t_stub, t_c, novhd);
}

/****************************************************************************/
/* COMPUTE THE ELAPSED TIME OF THE STUB ROUTINE                             */
/****************************************************************************/
clock_t time_stub(int *i)
{
    clock_t t_start, t_stop;

    t_start = clock();
    *i      = puzzle_stub();
    t_stop  = clock();
    return  t_stop - t_start;
}

/****************************************************************************/
/* COMPUTE THE ELAPSED TIME OF THE COMPILED C ROUTINE                       */
/****************************************************************************/
clock_t time_c(int *i)
{
    clock_t t_start, t_stop;

    t_start = clock();
    *i      = Puzzle();
    t_stop  = clock();
    return  t_stop - t_start;
}
