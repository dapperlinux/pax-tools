/* randexhaust1.c - Tests the randomization under memory exhaustion
 * when trying to get a fixed address at the upper end of the address space
 */

#ifndef RUNDIR
#error RUNDIR not defined
#endif

const char testname[] = "Randomization under memory exhaustion @~0";
const char testprog[] = RUNDIR"/getexhaust1";
