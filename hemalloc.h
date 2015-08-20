#ifndef _HEMALLOC_H_
#define _HEMALLOC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>

#include <time.h>
#include <signal.h>
#include <errno.h>

#include <pthread.h>

typedef unsigned char  bool; 

typedef void Sigfunc(int);  /* for signal handlers */    

#define  UL  unsigned long


#define  OK         1  /* success */
#define  NG        -1  /* faliure */

#define  TRUE       1  /* true  */
#define  FALSE      0  /* false */

#endif //end _hemalloc_h_
