//#######################################################################
//#		Copyright 2011 University of Coimbra. All rights reserved.		#
//#																		#
//#		Instituto de Telecomunicacoes, November 2011					#
//#				Joao Andrade (jandrade@co.it.pt)						#
//#				Gabriel Falcao (gff@co.it.pt)							#
//#																		#
//#######################################################################

#include "oclLDPC_fxp.hpp"
#include "SendMail.hpp"

//////////////////////////////////////////
// Initialization of input data with Lpn=1
#define Lpn 1
//////////////////////////////////////////

/*
 * \brief Host Initialization 
 *        Allocate and initialize memory 
 *        on the host. Print input array. 
 */
int initializeHost(LDPC *Code)
{
  int i,j;
  /////////////////////////////////////////////////////////////////
  // Allocate and initialize memory used by host 
  /////////////////////////////////////////////////////////////////
  Lq = (unsigned int *)malloc(sizeof(int)*4*Code->edges*Code->Depth);
  Lr = (unsigned int *)malloc(sizeof(int)*4*Code->edges*Code->Depth);
  Pi = (unsigned int *)malloc(sizeof(int)*4*Code->N*Code->Depth);


  for(i=0;i<Code->Depth*Code->N;i++){
    for(j=0;j<4;j++){
      Pi[i*4] = ((Pi[i*4] << 8) & 0xFFFFFF00) | ((Lpn + Code->Offset) & 0x000000FF);
      Pi[i*4+1] = ((Pi[i*4+1] << 8) & 0xFFFFFF00) | ((Lpn + Code->Offset) & 0x000000FF);
      Pi[i*4+2] = ((Pi[i*4+2] << 8) & 0xFFFFFF00) | ((Lpn + Code->Offset) & 0x000000FF);
      Pi[i*4+3] = ((Pi[i*4+3] << 8) & 0xFFFFFF00) | ((Lpn + Code->Offset) & 0x000000FF);
    }
  }

  for(i=0;i<Code->Depth*Code->N;i++)
    for(j=0;j<Code->BNW;j++){
      Lq[(i*Code->BNW+j)*4]=Pi[i*4];
      Lq[(i*Code->BNW+j)*4+1]=Pi[i*4+1];
      Lq[(i*Code->BNW+j)*4+2]=Pi[i*4+2];
      Lq[(i*Code->BNW+j)*4+3]=Pi[i*4+3];
    }

  return 0;
}

/*
 * \brief Run OpenCL program 
 *		  
 *        Bind host variables to kernel arguments 
 *		  Run the CL kernel
 */
int execute_kernels(int iterations, int tasks)
{
  int i, start, end, j;
  timer_reset(h_kernel_timer);

  timer_start(h_kernel_timer);
  for(i=0;i<iterations;i++){
    for ( j =0; j<tasks; ++j )
    {
      /* CN */
      start = j*oclCode->M/(double)tasks;
      end   = (j+1)*oclCode->M/(double)tasks;
      CN(Lq, Lr, oclCode->ligacoesx, oclCode->Depth, start, end);
    }
    for ( j=0 ; j<tasks; ++j)
    {
      /* BN */
      start = j*oclCode->N/(double)tasks;
      end   = (j+1)*oclCode->N/(double)tasks;
      BN(Lq, Lr, Pi, oclCode->ligacoesx, oclCode->Depth, start, end);
    }
  }
  timer_stop(h_kernel_timer);
  printf("-- Kernel time\t\t\t%lld\t[us]\n",(h_kernel_timer->total));
  return 0;
}


/* 
 * \brief Releases program's resources 
 */
void cleanupHost(void)
{
  free(Pi);
  free(Lr);
  free(Lq);
  free(oclCode);
}

int ReadAlistFile(LDPC *Code,const char *AlistFile){

  FILE *fd;
  int i,j,k;
  int test;

  Code->Offset=128;

  fd = fopen(AlistFile,"r");
  if(fd == NULL)
  {
    fprintf(stdout,"Error while opening alist file\n"); 
    return -1; 
  }

  fscanf(fd,"%d",&Code->N);
  fscanf(fd,"%d",&Code->M);
  fscanf(fd,"%d",&Code->BNW);
  fscanf(fd,"%d",&Code->CNW);


  printf("-- N=%d M=%d\n",Code->N,Code->M);


  Code->edges=Code->BNW*Code->N;
  if(Code->CNW*Code->M != Code->edges)
  {
    fprintf(stdout,"Number of edges is inconsistent, alist file may be corrupted\n");
    return -1; 
  }

  Code->H_bnw = (int *) malloc(sizeof(int)*Code->N);
  Code->H_cnw = (int *) malloc(sizeof(int)*Code->M);
  Code->H_bn = (int *) malloc(sizeof(int)*Code->edges);
  Code->H_cn = (int *) malloc(sizeof(int)*Code->edges);
  Code->ligacoesx = (int *) malloc(sizeof(int)*Code->edges);
  Code->ligacoesf = (int *) malloc(sizeof(int)*Code->edges);

  //Read BNW
  test = 0;
  for (i = 0; i < Code->N; i++){
    fscanf(fd,"%d",&Code->H_bnw[i]);
    test = Code->H_bnw[i] != Code->BNW;
  }
  if(test) fprintf(stdout,"Code is irregular\n");

  //Read CNW
  test = 0;
  for (i = 0; i < Code->M; i++){
    fscanf(fd,"%d",&Code->H_cnw[i]);
    test = Code->H_cnw[i] != Code->CNW;
  }
  if(test) fprintf(stdout,"Code is irregular\n");

  for (i = 0; i < Code->edges; i++)
    fscanf(fd,"%d",&Code->H_bn[i]);

  for (i = 0; i < Code->edges; i++)
    fscanf(fd,"%d",&Code->H_cn[i]);

  for(i = 0; i < Code->edges; i++){
    assert(Code->H_bn[i]!=0);
    assert(Code->H_cn[i]!=0);
  }

  k=0;
  for (i = 1; i <= Code->M; i++)
    for(j = 0; j < Code->edges; j++)
      if(Code->H_bn[j] == i){
        //Code->H_bn[j] = 0;
        Code->ligacoesx[j] = k++;
      }
  if(k != Code->edges) {
    fprintf(stdout,"Ligacoesx not successfully generated [%d]\n",k); 
    return -1;
  }

  k=0;
  for (i = 1; i <= Code->N; i++)
    for(j = 0; j < Code->edges; j++)
      if(Code->H_cn[j] == i){
        //Code->H_bn[j] = 0;
        Code->ligacoesf[j] = k++;
      }
  if(k != Code->edges) 
  {
    fprintf(stdout,"Ligacoesf not successfully generated [%d]\n",k); 
    return -1;
  }

  printf("-- Rate = %.3f\n",((float)Code->N-(float)Code->M)/(float)Code->N);
  printf("-- #Edges = %d\n",Code->edges);
  fclose(fd);

  return 0;
}

int LogMemoryLayout(LDPC *Code){

  FILE *fd;
  int i,j;

  fd = fopen("log/Memory Layout.log","w");
  if(fd == NULL)
  { 
    fprintf(stdout,"Could not log memory layout\n"); 
    return -1; 
  }


  fprintf(fd,"Ligacoesx: Update Indexes for BNs\n");
  for(i = 0; i < Code->N; i++){
    for(j = 0; j < Code->BNW; j++)
      fprintf(fd,"%d ",Code->ligacoesx[i*Code->BNW+j]);
    fprintf(fd,"\n");
  }


  fprintf(fd,"\n");
  fprintf(fd,"Ligacoesf: Update Indexes for CNs\n");
  for(i = 0; i < Code->M; i++){
    for(j = 0; j < Code->CNW; j++)
      fprintf(fd,"%d ",Code->ligacoesf[i*Code->CNW+j]);
    fprintf(fd,"\n");
  }

  fclose(fd);


  return 0;
}

void CleanLDPC(LDPC *Code){
  free(Code->ligacoesf); //printf("ligacoesf\n");
  free(Code->ligacoesx); //printf("ligacoesx\n");
  free(Code->H_cn);      //printf("H_cn\n");
  free(Code->H_bn);      //printf("H_bn\n");
  free(Code->H_cnw);     //printf("H_cnw\n");
  free(Code->H_bnw);     //printf("H_cnw\n");
  free(Code);            //printf("Code object\n");
}


void PrintDebug(LDPC *code,const int print_flag,const char *filename){

  FILE *fd_data_log;
  int i,j;
  char name[20];
  char var[5];
  char var1[5];
  unsigned int *data;
  int Dim1;

  for(j=0;j<code->Depth;j++)
  {

    if(print_flag==0){			//print out Lq
      data = (Lq+(j*code->edges)*4);
      strcpy(var,"Lq");
      Dim1=code->edges;
    }
    else if(print_flag==1){		//print out Lr
      data = (Lr+(j*code->edges)*4);
      strcpy(var,"Lr");
      Dim1=code->edges;
    }
    else{						//printf out Pi
      data = Pi;
      strcpy(var,"Pi");
      Dim1=code->N;
    }

    strcpy(name,"results/");
    strcat(name,filename);
    sprintf(var1,"%d",j);
    strcat(name,var1);
    strcat(name,".m");

    if((fd_data_log = fopen(name,"w"))==NULL){
      printf("-- Couldn't open debug information file\n");
      return;
    }

    fprintf(fd_data_log,"%s=[\n",var);
    for(i = 0;i < Dim1; i++){
      fprintf(fd_data_log,"%d %d %d %d ",(data[i*4] >> 24 & 0x000000FF) - code->Offset,
          (data[i*4] >> 16 & 0x000000FF) - code->Offset,
          (data[i*4] >> 8 & 0x000000FF) - code->Offset,
          (data[i*4] >> 0 & 0x000000FF) - code->Offset);
      fprintf(fd_data_log,"%d %d %d %d ",(data[i*4+1] >> 24 & 0x000000FF) - code->Offset,
          (data[i*4+1] >> 16 & 0x000000FF) - code->Offset,
          (data[i*4+1] >> 8 & 0x000000FF) - code->Offset,
          (data[i*4+1] >> 0 & 0x000000FF) - code->Offset);
      fprintf(fd_data_log,"%d %d %d %d ",(data[i*4+2] >> 24 & 0x000000FF) - code->Offset,
          (data[i*4+2] >> 16 & 0x000000FF) - code->Offset,
          (data[i*4+2] >> 8 & 0x000000FF) - code->Offset,
          (data[i*4+2] >> 0 & 0x000000FF) - code->Offset);
      fprintf(fd_data_log,"%d %d %d %d\n",(data[i*4+3] >> 24 & 0x000000FF) - code->Offset,
          (data[i*4+3] >> 16 & 0x000000FF) - code->Offset,
          (data[i*4+3] >> 8 & 0x000000FF) - code->Offset,
          (data[i*4+3] >> 0 & 0x000000FF) - code->Offset);
    }
    fprintf(fd_data_log,"];\n");
  }
  fclose(fd_data_log);
}

int main(int argc,char *argv[]){
  int iterations, samples, i, tasks;
  double depth;
  if(argc!=6){
    fprintf(stdout,"Usage: <executable> [Alist File] [Iterations] [Depth] [Samples] [Tasks]\n");
    return -1;
  }

  iterations = atoi(argv[2]);
  depth = atof(argv[3]);
  samples = atoi(argv[4]);
  tasks   = atoi(argv[5]);
  
  printf("-- Loading Code defined in %s\n",argv[1]);
  oclCode = (LDPC*) malloc(sizeof(LDPC));

  oclCode->Depth = depth;

  ReadAlistFile(oclCode,argv[1]);
  //LogMemoryLayout(oclCode);


  // Initialize Host application 
  printf("-- Initializing Host\n");
  if(initializeHost(oclCode) == 1)
    return 1;
  //PrintDebug(oclCode,0,"Lq_Init");
  //PrintDebug(oclCode,2,"Pi");

  // Initialize OpenCL resources
  // Run the CL program
  printf("-- Ready to launch\n");

  for (i = 0; i < samples; i++)
    if(execute_kernels(iterations, tasks) == 1)
      return 1;

  //Logging debug. Comment next 5 lines if no debug is to be produced in the "results" folder
  printf("-- Logging debug:\n");
  printf("\tLq ");
  PrintDebug(oclCode,0,"Lq");
  printf("Lr\n");
  PrintDebug(oclCode,1,"Lr");

  // Release host resources
  printf("-- Cleaning up Host\n");
  cleanupHost();

  return 0;
}

