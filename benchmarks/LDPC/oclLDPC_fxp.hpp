//######################################################################################
//#																					   #
//#								  Joao Andrade Aug 2011								   #
//#									jandrade@co.it.pt								   #
//#																					   #
//######################################################################################


#ifndef TEMPLATE_H_
#define TEMPLATE_H_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <timer.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>         /* gethostbyname  */
#include <netinet/in.h>    /* htons          */
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>


//Struct LDPC_Code
typedef struct LDPC_T {
	int N;				//Code block length
	int M;              //# Parity Equations
	int BNW;            //Maximum BitNode Weight [Constant if regular]
	int CNW;            //Maximum CheckNode Weight [Constant if regular]
	int edges;          //# of edges in the Tanner graph
	int *H_bnw;			//Weight per BN
	int *H_cnw;			//Weight per CN
	int *H_cn;          //Tanner graph connections in respect to the BN Dimension
	int *H_bn;          //Tanner graph connections in respect to the CN Dimension
	int *ligacoesx;     //Memory layout for BN updating Lr to CNs
	int *ligacoesf;     //Memory layout for CN updating Lq to BNs
	int Offset;
	int Depth;			//Vector depth
} LDPC;

struct arg_t
{
  unsigned int *Lq, *Lr, *Pi, *ligacoesf, *ligacoesx, set, set_max, start, end;
};


/*** FUNCTION DECLARATIONS ***/
/*
 * OpenCL related initialisations are done here.
 * Context, Device list, Command Queue are set up.
 * Calls are made to set up OpenCL memory buffers that this program uses
 * and to load the programs into memory and get kernel handles.
 */
int ReadAlistFile(LDPC *,const char *);
void CleanLDPC(LDPC*);
int LogMemoryLayout(LDPC *);
void PrintDebug(LDPC *,const int,const char*);

/* Releases program's resources */
void cleanupHost(void);

void _CN( unsigned int *Lq,  unsigned int *Lr, unsigned int *ligacoesf, unsigned int set_max, 
  unsigned int start, unsigned int end);
void _BN( unsigned int *Lq,  unsigned int *Lr, unsigned int *Pi, unsigned int *ligacoesx,
  unsigned int set_max, unsigned int start, unsigned int end);
void CN(void *args);
void BN(void *args);

#endif  /* #ifndef TEMPLATE_H_ */
