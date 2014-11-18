//#######################################################################
//#		Copyright 2011 University of Coimbra. All rights reserved.		#
//#																		#
//#		Instituto de Telecomunicacoes, November 2011					#
//#				Joao Andrade (jandrade@co.it.pt)						#
//#				Gabriel Falcao (gff@co.it.pt)							#
//#																		#
//#######################################################################

#define EDGES 24000
#define N 8000
#define BNW 3
#define M 4000
#define CNW 6
#define IOFFSET 128

#include <stdlib.h>
#include <ctype.h>
#include <macros.hpp>

void CN_Processing( unsigned int *Lq,  unsigned int *Lr, int *ligacoesf,unsigned int tid,unsigned int set, unsigned int set_max){

  //unsigned int tid = get_global_id(0);

  int Lq0[4],Lq1[4],Lq2[4],Lq3[4],Lq4[4],Lq5[4];
  int ld0,ld1,ld2,ld3,ld4,ld5;
  int addr0,addr1,addr2,addr3,addr4,addr5;
  int sig0,sig1,sig2,sig3,sig4,sig5,sigT;
  int min1,min2;
  int i;

  if(tid < M && set < set_max)
  {
    addr0=ligacoesf[tid*CNW];
    addr1=ligacoesf[tid*CNW+1];
    addr2=ligacoesf[tid*CNW+2];
    addr3=ligacoesf[tid*CNW+3];
    addr4=ligacoesf[tid*CNW+4];
    addr5=ligacoesf[tid*CNW+5];

    for ( i=0; i<4; ++i )
    {
      Lq0[i]=Lq[(set*EDGES+tid*CNW  )*4 + i];
      Lq1[i]=Lq[(set*EDGES+tid*CNW+1)*4 + i];
      Lq2[i]=Lq[(set*EDGES+tid*CNW+2)*4 + i];
      Lq3[i]=Lq[(set*EDGES+tid*CNW+3)*4 + i];
      Lq4[i]=Lq[(set*EDGES+tid*CNW+4)*4 + i];
      Lq5[i]=Lq[(set*EDGES+tid*CNW+5)*4 + i];
    }

    //############ Decode 4 byte segment 0 x ###############//
    HLOAD(ld0,ld1,ld2,ld3,ld4,ld5,0,IOFFSET)
    SIG(ld0,ld1,ld2,ld3,ld4,ld5,sig0,sig1,sig2,sig3,sig4,sig5,sigT)
    ABS(ld0,ld1,ld2,ld3,ld4,ld5)
    HINIT(min1,min2,ld0,ld1)
    HPROP(min1,min2,ld2)
    HPROP(min1,min2,ld3)
    HPROP(min1,min2,ld4)
    HPROP(min1,min2,ld5)
    HUPDATE(min1,min2,ld0,sig0,sigT)
    HUPDATE(min1,min2,ld1,sig1,sigT)
    HUPDATE(min1,min2,ld2,sig2,sigT)
    HUPDATE(min1,min2,ld3,sig3,sigT)
    HUPDATE(min1,min2,ld4,sig4,sigT)
    HUPDATE(min1,min2,ld5,sig5,sigT)
    HSTORE(ld0,ld1,ld2,ld3,ld4,ld5,0,IOFFSET)

    //############ Decode 4 byte segment 1 x ###############//
    HLOAD(ld0,ld1,ld2,ld3,ld4,ld5,0,IOFFSET)
    SIG(ld0,ld1,ld2,ld3,ld4,ld5,sig0,sig1,sig2,sig3,sig4,sig5,sigT)
    ABS(ld0,ld1,ld2,ld3,ld4,ld5)
    HINIT(min1,min2,ld0,ld1)
    HPROP(min1,min2,ld2)
    HPROP(min1,min2,ld3)
    HPROP(min1,min2,ld4)
    HPROP(min1,min2,ld5)
    HUPDATE(min1,min2,ld0,sig0,sigT)
    HUPDATE(min1,min2,ld1,sig1,sigT)
    HUPDATE(min1,min2,ld2,sig2,sigT)
    HUPDATE(min1,min2,ld3,sig3,sigT)
    HUPDATE(min1,min2,ld4,sig4,sigT)
    HUPDATE(min1,min2,ld5,sig5,sigT)
    HSTORE(ld0,ld1,ld2,ld3,ld4,ld5,0,IOFFSET)

    //############ Decode 4 byte segment 2 x ###############//
    HLOAD(ld0,ld1,ld2,ld3,ld4,ld5,0,IOFFSET)
    SIG(ld0,ld1,ld2,ld3,ld4,ld5,sig0,sig1,sig2,sig3,sig4,sig5,sigT)

    ABS(ld0,ld1,ld2,ld3,ld4,ld5)

    HINIT(min1,min2,ld0,ld1)
    HPROP(min1,min2,ld2)
    HPROP(min1,min2,ld3)
    HPROP(min1,min2,ld4)
    HPROP(min1,min2,ld5)

    HUPDATE(min1,min2,ld0,sig0,sigT)
    HUPDATE(min1,min2,ld1,sig1,sigT)
    HUPDATE(min1,min2,ld2,sig2,sigT)
    HUPDATE(min1,min2,ld3,sig3,sigT)
    HUPDATE(min1,min2,ld4,sig4,sigT)
    HUPDATE(min1,min2,ld5,sig5,sigT)

    HSTORE(ld0,ld1,ld2,ld3,ld4,ld5,0,IOFFSET)

    //############ Decode 4 byte segment 3 x ###############//
    HLOAD(ld0,ld1,ld2,ld3,ld4,ld5,0,IOFFSET)
    SIG(ld0,ld1,ld2,ld3,ld4,ld5,sig0,sig1,sig2,sig3,sig4,sig5,sigT)

    ABS(ld0,ld1,ld2,ld3,ld4,ld5)

    HINIT(min1,min2,ld0,ld1)
    HPROP(min1,min2,ld2)
    HPROP(min1,min2,ld3)
    HPROP(min1,min2,ld4)
    HPROP(min1,min2,ld5)

    HUPDATE(min1,min2,ld0,sig0,sigT)
    HUPDATE(min1,min2,ld1,sig1,sigT)
    HUPDATE(min1,min2,ld2,sig2,sigT)
    HUPDATE(min1,min2,ld3,sig3,sigT)
    HUPDATE(min1,min2,ld4,sig4,sigT)
    HUPDATE(min1,min2,ld5,sig5,sigT)

    HSTORE(ld0,ld1,ld2,ld3,ld4,ld5,0,IOFFSET)

    //############ Decode 4 byte segment 0 y ###############//
    HLOAD(ld0,ld1,ld2,ld3,ld4,ld5,1,IOFFSET)
    SIG(ld0,ld1,ld2,ld3,ld4,ld5,sig0,sig1,sig2,sig3,sig4,sig5,sigT)

    ABS(ld0,ld1,ld2,ld3,ld4,ld5)

    HINIT(min1,min2,ld0,ld1)
    HPROP(min1,min2,ld2)
    HPROP(min1,min2,ld3)
    HPROP(min1,min2,ld4)
    HPROP(min1,min2,ld5)

    HUPDATE(min1,min2,ld0,sig0,sigT)
    HUPDATE(min1,min2,ld1,sig1,sigT)
    HUPDATE(min1,min2,ld2,sig2,sigT)
    HUPDATE(min1,min2,ld3,sig3,sigT)
    HUPDATE(min1,min2,ld4,sig4,sigT)
    HUPDATE(min1,min2,ld5,sig5,sigT)

    HSTORE(ld0,ld1,ld2,ld3,ld4,ld5,1,IOFFSET)

    //############ Decode 4 byte segment 1 y ###############//
    HLOAD(ld0,ld1,ld2,ld3,ld4,ld5,1,IOFFSET)
    SIG(ld0,ld1,ld2,ld3,ld4,ld5,sig0,sig1,sig2,sig3,sig4,sig5,sigT)

    ABS(ld0,ld1,ld2,ld3,ld4,ld5)

    HINIT(min1,min2,ld0,ld1)
    HPROP(min1,min2,ld2)
    HPROP(min1,min2,ld3)
    HPROP(min1,min2,ld4)
    HPROP(min1,min2,ld5)

    HUPDATE(min1,min2,ld0,sig0,sigT)
    HUPDATE(min1,min2,ld1,sig1,sigT)
    HUPDATE(min1,min2,ld2,sig2,sigT)
    HUPDATE(min1,min2,ld3,sig3,sigT)
    HUPDATE(min1,min2,ld4,sig4,sigT)
    HUPDATE(min1,min2,ld5,sig5,sigT)

    HSTORE(ld0,ld1,ld2,ld3,ld4,ld5,1,IOFFSET)

    //############ Decode 4 byte segment 2 y ###############//
    HLOAD(ld0,ld1,ld2,ld3,ld4,ld5,1,IOFFSET)
    SIG(ld0,ld1,ld2,ld3,ld4,ld5,sig0,sig1,sig2,sig3,sig4,sig5,sigT)

    ABS(ld0,ld1,ld2,ld3,ld4,ld5)

    HINIT(min1,min2,ld0,ld1)
    HPROP(min1,min2,ld2)
    HPROP(min1,min2,ld3)
    HPROP(min1,min2,ld4)
    HPROP(min1,min2,ld5)

    HUPDATE(min1,min2,ld0,sig0,sigT)
    HUPDATE(min1,min2,ld1,sig1,sigT)
    HUPDATE(min1,min2,ld2,sig2,sigT)
    HUPDATE(min1,min2,ld3,sig3,sigT)
    HUPDATE(min1,min2,ld4,sig4,sigT)
    HUPDATE(min1,min2,ld5,sig5,sigT)

    HSTORE(ld0,ld1,ld2,ld3,ld4,ld5,1,IOFFSET)

    //############ Decode 4 byte segment 3 y ###############//
    HLOAD(ld0,ld1,ld2,ld3,ld4,ld5,1,IOFFSET)
    SIG(ld0,ld1,ld2,ld3,ld4,ld5,sig0,sig1,sig2,sig3,sig4,sig5,sigT)

    ABS(ld0,ld1,ld2,ld3,ld4,ld5)

    HINIT(min1,min2,ld0,ld1)
    HPROP(min1,min2,ld2)
    HPROP(min1,min2,ld3)
    HPROP(min1,min2,ld4)
    HPROP(min1,min2,ld5)

    HUPDATE(min1,min2,ld0,sig0,sigT)
    HUPDATE(min1,min2,ld1,sig1,sigT)
    HUPDATE(min1,min2,ld2,sig2,sigT)
    HUPDATE(min1,min2,ld3,sig3,sigT)
    HUPDATE(min1,min2,ld4,sig4,sigT)
    HUPDATE(min1,min2,ld5,sig5,sigT)

    HSTORE(ld0,ld1,ld2,ld3,ld4,ld5,1,IOFFSET)

    //############ Decode 4 byte segment 0 z ###############//
    HLOAD(ld0,ld1,ld2,ld3,ld4,ld5,2,IOFFSET)
    SIG(ld0,ld1,ld2,ld3,ld4,ld5,sig0,sig1,sig2,sig3,sig4,sig5,sigT)

    ABS(ld0,ld1,ld2,ld3,ld4,ld5)

    HINIT(min1,min2,ld0,ld1)
    HPROP(min1,min2,ld2)
    HPROP(min1,min2,ld3)
    HPROP(min1,min2,ld4)
    HPROP(min1,min2,ld5)

    HUPDATE(min1,min2,ld0,sig0,sigT)
    HUPDATE(min1,min2,ld1,sig1,sigT)
    HUPDATE(min1,min2,ld2,sig2,sigT)
    HUPDATE(min1,min2,ld3,sig3,sigT)
    HUPDATE(min1,min2,ld4,sig4,sigT)
    HUPDATE(min1,min2,ld5,sig5,sigT)

    HSTORE(ld0,ld1,ld2,ld3,ld4,ld5,2,IOFFSET)

    //############ Decode 4 byte segment 1 z ###############//
    HLOAD(ld0,ld1,ld2,ld3,ld4,ld5,2,IOFFSET)
    SIG(ld0,ld1,ld2,ld3,ld4,ld5,sig0,sig1,sig2,sig3,sig4,sig5,sigT)

    ABS(ld0,ld1,ld2,ld3,ld4,ld5)

    HINIT(min1,min2,ld0,ld1)
    HPROP(min1,min2,ld2)
    HPROP(min1,min2,ld3)
    HPROP(min1,min2,ld4)
    HPROP(min1,min2,ld5)

    HUPDATE(min1,min2,ld0,sig0,sigT)
    HUPDATE(min1,min2,ld1,sig1,sigT)
    HUPDATE(min1,min2,ld2,sig2,sigT)
    HUPDATE(min1,min2,ld3,sig3,sigT)
    HUPDATE(min1,min2,ld4,sig4,sigT)
    HUPDATE(min1,min2,ld5,sig5,sigT)

    HSTORE(ld0,ld1,ld2,ld3,ld4,ld5,2,IOFFSET)

    //############ Decode 4 byte segment 2 z ###############//
    HLOAD(ld0,ld1,ld2,ld3,ld4,ld5,2,IOFFSET)
    SIG(ld0,ld1,ld2,ld3,ld4,ld5,sig0,sig1,sig2,sig3,sig4,sig5,sigT)

    ABS(ld0,ld1,ld2,ld3,ld4,ld5)

    HINIT(min1,min2,ld0,ld1)
    HPROP(min1,min2,ld2)
    HPROP(min1,min2,ld3)
    HPROP(min1,min2,ld4)
    HPROP(min1,min2,ld5)

    HUPDATE(min1,min2,ld0,sig0,sigT)
    HUPDATE(min1,min2,ld1,sig1,sigT)
    HUPDATE(min1,min2,ld2,sig2,sigT)
    HUPDATE(min1,min2,ld3,sig3,sigT)
    HUPDATE(min1,min2,ld4,sig4,sigT)
    HUPDATE(min1,min2,ld5,sig5,sigT)

    HSTORE(ld0,ld1,ld2,ld3,ld4,ld5,2,IOFFSET)

    //############ Decode 4 byte segment 3 z ###############//
    HLOAD(ld0,ld1,ld2,ld3,ld4,ld5,2,IOFFSET)
    SIG(ld0,ld1,ld2,ld3,ld4,ld5,sig0,sig1,sig2,sig3,sig4,sig5,sigT)

    ABS(ld0,ld1,ld2,ld3,ld4,ld5)

    HINIT(min1,min2,ld0,ld1)
    HPROP(min1,min2,ld2)
    HPROP(min1,min2,ld3)
    HPROP(min1,min2,ld4)
    HPROP(min1,min2,ld5)

    HUPDATE(min1,min2,ld0,sig0,sigT)
    HUPDATE(min1,min2,ld1,sig1,sigT)
    HUPDATE(min1,min2,ld2,sig2,sigT)
    HUPDATE(min1,min2,ld3,sig3,sigT)
    HUPDATE(min1,min2,ld4,sig4,sigT)
    HUPDATE(min1,min2,ld5,sig5,sigT)

    HSTORE(ld0,ld1,ld2,ld3,ld4,ld5,2,IOFFSET)

    //############ Decode 4 byte segment 0 w ###############//
    HLOAD(ld0,ld1,ld2,ld3,ld4,ld5,3,IOFFSET)

    SIG(ld0,ld1,ld2,ld3,ld4,ld5,sig0,sig1,sig2,sig3,sig4,sig5,sigT)

    ABS(ld0,ld1,ld2,ld3,ld4,ld5)

    HINIT(min1,min2,ld0,ld1)
    HPROP(min1,min2,ld2)
    HPROP(min1,min2,ld3)
    HPROP(min1,min2,ld4)
    HPROP(min1,min2,ld5)

    HUPDATE(min1,min2,ld0,sig0,sigT)
    HUPDATE(min1,min2,ld1,sig1,sigT)
    HUPDATE(min1,min2,ld2,sig2,sigT)
    HUPDATE(min1,min2,ld3,sig3,sigT)
    HUPDATE(min1,min2,ld4,sig4,sigT)
    HUPDATE(min1,min2,ld5,sig5,sigT)

    HSTORE(ld0,ld1,ld2,ld3,ld4,ld5,3,IOFFSET)

    //############ Decode 4 byte segment 1 w ###############//
    HLOAD(ld0,ld1,ld2,ld3,ld4,ld5,3,IOFFSET)

    SIG(ld0,ld1,ld2,ld3,ld4,ld5,sig0,sig1,sig2,sig3,sig4,sig5,sigT)

    ABS(ld0,ld1,ld2,ld3,ld4,ld5)

    HINIT(min1,min2,ld0,ld1)
    HPROP(min1,min2,ld2)
    HPROP(min1,min2,ld3)
    HPROP(min1,min2,ld4)
    HPROP(min1,min2,ld5)

    HUPDATE(min1,min2,ld0,sig0,sigT)
    HUPDATE(min1,min2,ld1,sig1,sigT)
    HUPDATE(min1,min2,ld2,sig2,sigT)
    HUPDATE(min1,min2,ld3,sig3,sigT)
    HUPDATE(min1,min2,ld4,sig4,sigT)
    HUPDATE(min1,min2,ld5,sig5,sigT)

    HSTORE(ld0,ld1,ld2,ld3,ld4,ld5,3,IOFFSET)

    //############ Decode 4 byte segment 2 w ###############//
    HLOAD(ld0,ld1,ld2,ld3,ld4,ld5,3,IOFFSET)

    SIG(ld0,ld1,ld2,ld3,ld4,ld5,sig0,sig1,sig2,sig3,sig4,sig5,sigT)

    ABS(ld0,ld1,ld2,ld3,ld4,ld5)

    HINIT(min1,min2,ld0,ld1)
    HPROP(min1,min2,ld2)
    HPROP(min1,min2,ld3)
    HPROP(min1,min2,ld4)
    HPROP(min1,min2,ld5)

    HUPDATE(min1,min2,ld0,sig0,sigT)
    HUPDATE(min1,min2,ld1,sig1,sigT)
    HUPDATE(min1,min2,ld2,sig2,sigT)
    HUPDATE(min1,min2,ld3,sig3,sigT)
    HUPDATE(min1,min2,ld4,sig4,sigT)
    HUPDATE(min1,min2,ld5,sig5,sigT)

    HSTORE(ld0,ld1,ld2,ld3,ld4,ld5,3,IOFFSET)

    //############ Decode 4 byte segment 3 w ###############//
    HLOAD(ld0,ld1,ld2,ld3,ld4,ld5,3,IOFFSET)

    SIG(ld0,ld1,ld2,ld3,ld4,ld5,sig0,sig1,sig2,sig3,sig4,sig5,sigT)

    ABS(ld0,ld1,ld2,ld3,ld4,ld5)

    HINIT(min1,min2,ld0,ld1)
    HPROP(min1,min2,ld2)
    HPROP(min1,min2,ld3)
    HPROP(min1,min2,ld4)
    HPROP(min1,min2,ld5)

    HUPDATE(min1,min2,ld0,sig0,sigT)
    HUPDATE(min1,min2,ld1,sig1,sigT)
    HUPDATE(min1,min2,ld2,sig2,sigT)
    HUPDATE(min1,min2,ld3,sig3,sigT)
    HUPDATE(min1,min2,ld4,sig4,sigT)
    HUPDATE(min1,min2,ld5,sig5,sigT)

    HSTORE(ld0,ld1,ld2,ld3,ld4,ld5,3,IOFFSET)

    /*if(set==1&&tid==0){
    printf("Lqx:%x %x %x %x %x %x\n",Lq0.x,Lq1.x,Lq2.x,Lq3.x,Lq4.x,Lq5.x);
    printf("Lqy:%x %x %x %x %x %x\n",Lq0.y,Lq1.y,Lq2.y,Lq3.y,Lq4.y,Lq5.y);
    printf("Lqz:%x %x %x %x %x %x\n",Lq0.z,Lq1.z,Lq2.z,Lq3.z,Lq4.z,Lq5.z);
    printf("Lqw:%x %x %x %x %x %x\n",Lq0.w,Lq1.w,Lq2.w,Lq3.w,Lq4.w,Lq5.w);
    }*/
    
    for ( i=0; i<4; ++i )
    {
      Lr[(set*EDGES+addr0)*4 + i]=Lq0[i];
      Lr[(set*EDGES+addr1)*4 + i]=Lq1[i];
      Lr[(set*EDGES+addr2)*4 + i]=Lq2[i];
      Lr[(set*EDGES+addr3)*4 + i]=Lq3[i];
      Lr[(set*EDGES+addr4)*4 + i]=Lq4[i];
      Lr[(set*EDGES+addr5)*4 + i]=Lq5[i];
    }
  }
}


void BN_Processing( unsigned int *Lq,  unsigned int *Lr, unsigned int *Pi, int *ligacoesx,unsigned int tid,unsigned int set, unsigned int set_max)
{

  //unsigned int tid = get_global_id(0);

  unsigned int Lr0[4],Lr1[4],Lr2[4],lPi[4];
  int ld0,ld1,ld2,pi;
  int sum;
  int addr0,addr1,addr2;
  int i;

  if(tid<N&&set<set_max)
  {
    addr0=ligacoesx[tid*BNW];
    addr1=ligacoesx[tid*BNW+1];
    addr2=ligacoesx[tid*BNW+2];

    for ( i=0; i<4; ++i )
    {
      lPi[i]=Pi[(tid)*4 + i];

      Lr0[i]=Lr[(set*EDGES+tid*BNW)*4 + i];
      Lr1[i]=Lr[(set*EDGES+tid*BNW+1)*4 + i];
      Lr2[i]=Lr[(set*EDGES+tid*BNW+2)*4 + i];
    }



    //############ Decode 4 byte segment 0 x ###############//
    VLOAD(pi,ld0,ld1,ld2,0,IOFFSET,24)

    VUPDATE(sum,ld0,ld1,ld2,pi)

    HCLIP(ld0,IOFFSET)
    HCLIP(ld1,IOFFSET)
    HCLIP(ld2,IOFFSET)

    LCLIP(ld0,IOFFSET);
    LCLIP(ld1,IOFFSET);
    LCLIP(ld2,IOFFSET);

    VSTORE(Lr0,Lr1,Lr2,ld0,ld1,ld2,0,IOFFSET)

    //############ Decode 4 byte segment 1 x ###############//
    VLOAD(pi,ld0,ld1,ld2,0,IOFFSET,16)

    VUPDATE(sum,ld0,ld1,ld2,pi)

    HCLIP(ld0,IOFFSET);
    HCLIP(ld1,IOFFSET);
    HCLIP(ld2,IOFFSET);

    LCLIP(ld0,IOFFSET);
    LCLIP(ld1,IOFFSET);
    LCLIP(ld2,IOFFSET);

    VSTORE(Lr0,Lr1,Lr2,ld0,ld1,ld2,0,IOFFSET)

    //############ Decode 4 byte segment 2 x ###############//
    VLOAD(pi,ld0,ld1,ld2,0,IOFFSET,8)

    VUPDATE(sum,ld0,ld1,ld2,pi)

    HCLIP(ld0,IOFFSET);
    HCLIP(ld1,IOFFSET);
    HCLIP(ld2,IOFFSET);

    LCLIP(ld0,IOFFSET);
    LCLIP(ld1,IOFFSET);
    LCLIP(ld2,IOFFSET);

    VSTORE(Lr0,Lr1,Lr2,ld0,ld1,ld2,0,IOFFSET)

    //############ Decode 4 byte segment 3 x ###############//
    VLOAD(pi,ld0,ld1,ld2,0,IOFFSET,0)

    VUPDATE(sum,ld0,ld1,ld2,pi)

    HCLIP(ld0,IOFFSET);
    HCLIP(ld1,IOFFSET);
    HCLIP(ld2,IOFFSET);

    LCLIP(ld0,IOFFSET);
    LCLIP(ld1,IOFFSET);
    LCLIP(ld2,IOFFSET);

    VSTORE(Lr0,Lr1,Lr2,ld0,ld1,ld2,0,IOFFSET)

    //############ Decode 4 byte segment 0 y ###############//
    VLOAD(pi,ld0,ld1,ld2,1,IOFFSET,24)

    VUPDATE(sum,ld0,ld1,ld2,pi)

    HCLIP(ld0,IOFFSET);
    HCLIP(ld1,IOFFSET);
    HCLIP(ld2,IOFFSET);

    LCLIP(ld0,IOFFSET);
    LCLIP(ld1,IOFFSET);
    LCLIP(ld2,IOFFSET);

    VSTORE(Lr0,Lr1,Lr2,ld0,ld1,ld2,1,IOFFSET)

    //############ Decode 4 byte segment 1 y ###############//
    VLOAD(pi,ld0,ld1,ld2,1,IOFFSET,16)

    VUPDATE(sum,ld0,ld1,ld2,pi)

    HCLIP(ld0,IOFFSET);
    HCLIP(ld1,IOFFSET);
    HCLIP(ld2,IOFFSET);

    LCLIP(ld0,IOFFSET);
    LCLIP(ld1,IOFFSET);
    LCLIP(ld2,IOFFSET);

    VSTORE(Lr0,Lr1,Lr2,ld0,ld1,ld2,1,IOFFSET)

    //############ Decode 4 byte segment 2 y ###############//
    VLOAD(pi,ld0,ld1,ld2,1,IOFFSET,8)

    VUPDATE(sum,ld0,ld1,ld2,pi)

    HCLIP(ld0,IOFFSET);
    HCLIP(ld1,IOFFSET);
    HCLIP(ld2,IOFFSET);

    LCLIP(ld0,IOFFSET);
    LCLIP(ld1,IOFFSET);
    LCLIP(ld2,IOFFSET);

    VSTORE(Lr0,Lr1,Lr2,ld0,ld1,ld2,1,IOFFSET)

    //############ Decode 4 byte segment 3 y ###############//
    VLOAD(pi,ld0,ld1,ld2,1,IOFFSET,0)

    VUPDATE(sum,ld0,ld1,ld2,pi)

    HCLIP(ld0,IOFFSET);
    HCLIP(ld1,IOFFSET);
    HCLIP(ld2,IOFFSET);

    LCLIP(ld0,IOFFSET);
    LCLIP(ld1,IOFFSET);
    LCLIP(ld2,IOFFSET);

    VSTORE(Lr0,Lr1,Lr2,ld0,ld1,ld2,1,IOFFSET)

    //############ Decode 4 byte segment 0 z ###############//
    VLOAD(pi,ld0,ld1,ld2,2,IOFFSET,24)

    VUPDATE(sum,ld0,ld1,ld2,pi)

    HCLIP(ld0,IOFFSET);
    HCLIP(ld1,IOFFSET);
    HCLIP(ld2,IOFFSET);

    LCLIP(ld0,IOFFSET);
    LCLIP(ld1,IOFFSET);
    LCLIP(ld2,IOFFSET);

    VSTORE(Lr0,Lr1,Lr2,ld0,ld1,ld2,2,IOFFSET)

    //############ Decode 4 byte segment 1 z ###############//
    VLOAD(pi,ld0,ld1,ld2,2,IOFFSET,16)

    VUPDATE(sum,ld0,ld1,ld2,pi)

    HCLIP(ld0,IOFFSET);
    HCLIP(ld1,IOFFSET);
    HCLIP(ld2,IOFFSET);

    LCLIP(ld0,IOFFSET);
    LCLIP(ld1,IOFFSET);
    LCLIP(ld2,IOFFSET);

    VSTORE(Lr0,Lr1,Lr2,ld0,ld1,ld2,2,IOFFSET)

    //############ Decode 4 byte segment 2 z ###############//
    VLOAD(pi,ld0,ld1,ld2,2,IOFFSET,8)

    VUPDATE(sum,ld0,ld1,ld2,pi)

    HCLIP(ld0,IOFFSET);
    HCLIP(ld1,IOFFSET);
    HCLIP(ld2,IOFFSET);

    LCLIP(ld0,IOFFSET);
    LCLIP(ld1,IOFFSET);
    LCLIP(ld2,IOFFSET);

    VSTORE(Lr0,Lr1,Lr2,ld0,ld1,ld2,2,IOFFSET)

    //############ Decode 4 byte segment 3 z ###############//
    VLOAD(pi,ld0,ld1,ld2,2,IOFFSET,0)

    VUPDATE(sum,ld0,ld1,ld2,pi)

    HCLIP(ld0,IOFFSET);
    HCLIP(ld1,IOFFSET);
    HCLIP(ld2,IOFFSET);

    LCLIP(ld0,IOFFSET);
    LCLIP(ld1,IOFFSET);
    LCLIP(ld2,IOFFSET);

    VSTORE(Lr0,Lr1,Lr2,ld0,ld1,ld2,2,IOFFSET)

    //############ Decode 4 byte segment 0 w ###############//
    VLOAD(pi,ld0,ld1,ld2,3,IOFFSET,24)

    sum = ld0 + ld1 + ld2 + pi;


    ld0 = sum-ld0;
    ld1 = sum-ld1;
    ld2 = sum-ld2;

    HCLIP(ld0,IOFFSET);
    HCLIP(ld1,IOFFSET);
    HCLIP(ld2,IOFFSET);

    LCLIP(ld0,IOFFSET);
    LCLIP(ld1,IOFFSET);
    LCLIP(ld2,IOFFSET);

    VSTORE(Lr0,Lr1,Lr2,ld0,ld1,ld2,3,IOFFSET)

    //############ Decode 4 byte segment 1 w ###############//
    VLOAD(pi,ld0,ld1,ld2,3,IOFFSET,16)

    sum = ld0 + ld1 + ld2 + pi;


    ld0 = sum-ld0;
    ld1 = sum-ld1;
    ld2 = sum-ld2;

    HCLIP(ld0,IOFFSET);
    HCLIP(ld1,IOFFSET);
    HCLIP(ld2,IOFFSET);

    LCLIP(ld0,IOFFSET);
    LCLIP(ld1,IOFFSET);
    LCLIP(ld2,IOFFSET);

    VSTORE(Lr0,Lr1,Lr2,ld0,ld1,ld2,3,IOFFSET)

    //############ Decode 4 byte segment 2 w ###############//
    VLOAD(pi,ld0,ld1,ld2,3,IOFFSET,8)

    sum = ld0 + ld1 + ld2 + pi;


    ld0 = sum-ld0;
    ld1 = sum-ld1;
    ld2 = sum-ld2;

    HCLIP(ld0,IOFFSET);
    HCLIP(ld1,IOFFSET);
    HCLIP(ld2,IOFFSET);

    LCLIP(ld0,IOFFSET);
    LCLIP(ld1,IOFFSET);
    LCLIP(ld2,IOFFSET);

    VSTORE(Lr0,Lr1,Lr2,ld0,ld1,ld2,3,IOFFSET)

    //############ Decode 4 byte segment 3 w ###############//
    VLOAD(pi,ld0,ld1,ld2,3,IOFFSET,0)

    sum = ld0 + ld1 + ld2 + pi;


    ld0 = sum-ld0;
    ld1 = sum-ld1;
    ld2 = sum-ld2;

    HCLIP(ld0,IOFFSET);
    HCLIP(ld1,IOFFSET);
    HCLIP(ld2,IOFFSET);

    LCLIP(ld0,IOFFSET);
    LCLIP(ld1,IOFFSET);
    LCLIP(ld2,IOFFSET);

    VSTORE(Lr0,Lr1,Lr2,ld0,ld1,ld2,3,IOFFSET)
    
    for ( i=0; i<4; ++i )
    {
      Lq[(set*EDGES+addr0)*4 + i]=Lr0[i];
      Lq[(set*EDGES+addr1)*4 + i]=Lr1[i];
      Lq[(set*EDGES+addr2)*4 + i]=Lr2[i];
    }
  }
}

void CN( unsigned int *Lq,  unsigned int *Lr, int *ligacoesf, unsigned int set_max, 
  unsigned int start, unsigned int end)
{
  unsigned int tid, i, set;

  for ( i=start; i < end; ++i )
  {
    set = i / M;
    tid = i % M;
    CN_Processing(Lq,Lr,ligacoesf,tid,set,set_max);
  }
}

void BN( unsigned int *Lq,  unsigned int *Lr, unsigned int *Pi, int *ligacoesx,
  unsigned int set_max, unsigned int start, unsigned int end)
{
  unsigned int tid, i, set;

  for ( i=start; i < end; ++i )
  {
    set = i / N;
    tid = i % N;
    BN_Processing(Lq,Lr,Pi,ligacoesx,tid,set,set_max);
  }
}
