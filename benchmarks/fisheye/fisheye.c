/*==================================================================*/
/* fisheye.c                                                        */
/*                                                                  */
/* This file contains code for the fisheye lens distortion          */
/* correction.                                                      */
/* OpenCV camera drivers are used.                                  */ 
/* Author: Nikos Bellas                                             */
/*==================================================================*/


// #include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "fisheye.h" 
#include <assert.h>
#include <time.h>
#include <pthread.h>
// #include "cordic.h" 
// #include "sbtm.h" 

int Exit;    // flag to exit program
char* pfn;   // filename pointer
int T;       // threshold
int num_threads;

int STEP_X = 1;
float RATIO = 0;
int WIDTH = 2592;
int HEIGHT = 1944;
int input_size= 2592*1944;

#include "runtime.h"

typedef struct thread_work{
  int startx,starty;
  int step;
  int work;
  int endx,endy;
  long start_time;
  program_params_t *params;
  int approx;
  int sign;
  int b_idx,b_idy;
  int pic;
}tw;



/*==================================================================*/
/* It returns the sign of a double                                  */
/*==================================================================*/

int sign(double x) {
  if (x < 0.0)
    return -1;
  else if (x > 0.0)
    return 1;
  else 
    return 0;
}

/*==================================================================*/
/* Clip a double value x to [a,b]                                   */
/*==================================================================*/
int clip(int x, int a, int b) {
  if (x < a)
    return a;
  else if (x > b) 
    return b;
  else 
    return x;
}


/*==================================================================*/
/* parse_args() parses the input arguments of the FishEye program   */
/*==================================================================*/
void parse_args(int argc, char **argv, program_args_t *program_args) {
  char *filename;
  char *ptr;
  int  coord_x, coord_y;
  int argNum;
  double phi;
  int i;
  /* set some default values to have the ball rolling */
  coord_x = coord_y = 0;   
  phi = 20;
  argNum = 1;
  while (argNum < argc) {
    if (argv[argNum][0] == '-') {
      switch (argv[argNum][1]) {
        case 'f': case 'F':             /* PHI for Field of View */
          argNum++;
          phi = atof(argv[argNum]);
          assert(phi >= 0.0 && phi <= 180.0);
          program_args->phi = phi;
          break;
        case 'x': case 'X':
          argNum++;
          program_args->x = atoi(argv[argNum]);
          break;
        case 'y': case 'Y':
          argNum++;
          program_args->y = atoi(argv[argNum]);
          break;
        case 'w': case 'W':
          argNum++;
          OUT_WIDTH = atoi(argv[argNum]);
          break;
        case 'h': case 'H':
          argNum++;
          OUT_HEIGHT = atoi(argv[argNum]);
          break;  
        case 't':case 'T':
          argNum++;
          num_threads = atoi ( argv[argNum] ) ;
          break;
        case 'P': case 'p':
          argNum++;
          STEP_X = atoi(argv[argNum]);
          break;
        case 'R': case 'r':
          argNum++;
          RATIO = atof(argv[argNum]);
          break;  
        case 'S' :case 's':
          argNum++;
          program_args->num_pics = atoi(argv[argNum]);
          break;
        case 'I' : case 'i':
            argNum++;
            HEIGHT = atoi(argv[argNum]);
            break;
        case 'J' : case 'j':
            argNum++;
            WIDTH = atoi(argv[argNum]);
          break;
      }
    }
    else {
      input_size=WIDTH*HEIGHT;
      filename = argv[argNum];
      FILE *f = fopen(filename,"rb");
      program_args->frame = (unsigned char *) malloc (sizeof(unsigned char)*input_size*3);
      if(!(program_args->frame)){
        printf("Exiting 1 \n");
        exit(0);
      }
      unsigned char byte;
      for(i=0;i<54;i++) byte = getc(f); // Throw header
      size_t read_bytes = fread(program_args->frame, sizeof(unsigned char), input_size*3, f);
      if(read_bytes != input_size*3){
        printf("Exiting 22 \n");
        exit(0);
      }
    }
    argNum++;
  }

}   

/*====================================================================*/
/* init_params() : initialize the parameters of the FishEye system    */
/*====================================================================*/
void init_params(program_params_t *params, program_args_t *args)  {

  int i;
  params->num_pics = args->num_pics;
  params->initial_frame = args->frame;
  params->current_frame_x_size = WIDTH;
  params->current_frame_y_size = HEIGHT;
  params->current_frame = (unsigned char *) malloc (sizeof(unsigned char)*input_size*3);
  if(!(params->current_frame)){
        printf("Exiting\n");
    exit(0);
  }
  memcpy(params->current_frame,params->initial_frame,input_size*sizeof(unsigned char)*3);
  params->input_winname = "FishEye";


  params->xtd_current_frame_x_size = WIDTH + 2;
  params->xtd_current_frame_y_size= HEIGHT + 2;
  int size = 3*(WIDTH + 2)*(HEIGHT + 2);
  params->xtd_current_frame = (unsigned char*) malloc (sizeof(unsigned char)*size);
  if ( ! params->xtd_current_frame ) {
    printf("Exiting \n");
    exit(0);
  }

  printf("Num pics is %d\n",params->num_pics);
  params->xtd_output_frame = (unsigned char**) malloc (sizeof(unsigned char*) * params->num_pics);

  if ( !params->xtd_output_frame ) {
      printf("Exiting \n");
      exit(0);
    }


  params->xtd_output_frame_x_size = 2*OUT_WIDTH;
  params->xtd_output_frame_y_size = 2*OUT_HEIGHT;
  size = 3*4*OUT_HEIGHT*OUT_WIDTH;

  for ( i = 0 ; i < params->num_pics; i++){
    params->xtd_output_frame[i] = (unsigned char*) malloc (sizeof(unsigned char)*size);
    if ( ! params->xtd_output_frame[i]){
      printf("Exiting \n");
      exit(0);
    }

  }

  params->xtd_output_winname = "FishEye_2D_4X";

  params->cp[0] = 0.0;      /* default */
  params->cp[1] = 0.0;
  params->phi = args->phi;

  /* The following assignments assume the small Micron sensor */

  /* The following assignments assume the Sunex sensor lenses */
  params->ep[0] = 1.303777956478422e+003;
  params->ep[1] = 1.024845541998177e+003;
  params->ep[2] = 1.0;

  /* fractional accuracy is ACC, total 24 bits */
  params->k[0] = 16.20155219286276;
  params->k[1] = -2.119412185669920e2;
  params->k[2] = 12.31074028197216;
  params->k[3] =  1.452758764538667e3;
  params->k[4] = 0;

  /* Only used in floating point arithmetic */

  params->k_inv[0] = 7.035011928002930e-014;
  params->k_inv[1] = -6.820148000829702e-011;
  params->k_inv[2] = 5.998752129909016e-008;
  params->k_inv[3] = 6.771961136013822e-004;
  params->k_inv[4] = 0.0;


}



/*====================================================================*/
/* extend_image() : extend the current frame of the params to two     */
/*                  extra columns and two extra rows to accomodate    */
/*                  boundary conditions for the bicubic interpolation */
/*                  Input:                                            */
/*                          params->current_frame;                    */
/*                  Output:                                           */
/*                          params->xtd_current_frame;                */
/*====================================================================*/
void xtd_current_frame(program_params_t *params)  {

  unsigned char *cframe;
  unsigned char*xframe;
  int      w1, h1, w2, h2;
  int      r, c;
  int      loc1, loc2;

  cframe = params->current_frame;
  xframe = params->xtd_current_frame;
  w1 = params->xtd_current_frame_x_size;
  h1 = params->xtd_current_frame_y_size;
  w2 = params->current_frame_x_size;
  h2 = params->current_frame_y_size;

  /* First row, but not the two edges */
  for (c = 1; c < w1-1; c++) {
    loc1 = c*3;
    loc2 = (c-1)*3;
    xframe[loc1+0] = (unsigned char) (3*cframe[loc2+0] -  3*cframe[loc2+3*w2+0] + cframe[loc2+3*2*w2+0]);
    xframe[loc1+1] = (unsigned char) (3*cframe[loc2+1] -  3*cframe[loc2+3*w2+1] + cframe[loc2+3*2*w2+1]);
    xframe[loc1+2] = (unsigned char) (3*cframe[loc2+2] - 3*cframe[loc2+3*w2+2] +  cframe[loc2+3*2*w2+2]);
  }

  /* Last row, but not the two edges */
  for (c = 1; c < w1-1; c++) {
    loc1 = ((h1-1)*w1 + c)*3;
    loc2 = ((h2-1)*w2 + c-1)*3;
    xframe[loc1+0] = (unsigned char) (3*cframe[loc2+0] - 3*cframe[loc2-3*w2+0] +  cframe[loc2-3*2*w2+0]);
    xframe[loc1+1] = (unsigned char) (3*cframe[loc2+1] - 3*cframe[loc2-3*w2+1] + cframe[loc2-3*2*w2+1]);
    xframe[loc1+2] = (unsigned char) (3*cframe[loc2+2] - 3*cframe[loc2-3*w2+2] + cframe[loc2-3*2*w2+2]);

  }

  /* Main array */
  for (r = 1; r < h1-1; r++) {
    loc1 = (r*w1 + 1)*3;
    loc2 = (r-1)*w2*3;
    for (c = 1; c < w1-1; c++) {
      xframe[loc1+0] = (unsigned char) cframe[loc2+0]; 
      xframe[loc1+1] = (unsigned char) cframe[loc2+1];
      xframe[loc1+2] = (unsigned char) cframe[loc2+2]; 
      loc1 += 3;
      loc2 += 3;
    }
  }

  /* First column */
  for (r = 0; r < h1; r++) {
    loc1 = (r*w1 + 0)*3;
    loc2 = (r*w1 + 1)*3;    /* @ xframe */
    xframe[loc1+0] = (unsigned char) (3*xframe[loc2+0] - \
        3*xframe[loc2+3+0] + \
        xframe[loc2+3*2+0]);
    xframe[loc1+1] = (unsigned char) (3*xframe[loc2+1] - \
        3*xframe[loc2+3+1] + \
        xframe[loc2+3*2+1]);
    xframe[loc1+2] = (unsigned char) (3*xframe[loc2+2] - \
        3*xframe[loc2+3+2] + \
        xframe[loc2+3*2+2]);
  }

  /* Last column */
  for (r = 0; r < h1; r++) {
    loc1 = (r*w1 + w1-1)*3;
    loc2 = (r*w1 + w1-2)*3;    /* @ xframe */
    xframe[loc1+0] = (unsigned char) (3*xframe[loc2+0] - \
        3*xframe[loc2-3+0] + \
        xframe[loc2-3*2+0]);
    xframe[loc1+1] = (unsigned char) (3*xframe[loc2+1] - \
        3*xframe[loc2-3+1] + \
        xframe[loc2-3*2+1]);
    xframe[loc1+2] = (unsigned char) (3*xframe[loc2+2] - \
        3*xframe[loc2-3+2] + \
        xframe[loc2-3*2+2]);
  }
}



/*======================================================================*/
/* Performs mapping of the FishEye center of ROI to the 2D space.       */
/* This is done every time the ROI moves.                               */
/*      Input/Output: the parameters params                             */ 
/*======================================================================*/
void forward_mapping(program_params_t *params) {
  double Dd, Ru, Du, rs0, rs1, rs2;
  double cp0, cp1, ep0, ep1;
  double t0, t1, t2, tt0, tt1, tt2, ttt0, ttt1, ttt2, norm;
  double cross0, cross1, cross2;
  double fov, step_size;
  int    i;

  cp0 = params->cp[0]; 
  cp1 = params->cp[1]; 
  ep0 = params->ep[0]; 
  ep1 = params->ep[1]; 
  Dd = sqrt((cp0-ep0)*(cp0-ep0) + (cp1-ep1)*(cp1-ep1));

  Ru = params->k_inv[4] + Dd*(params->k_inv[3] + Dd*(params->k_inv[2] + Dd*(params->k_inv[1] + Dd*(params->k_inv[0])))); 

  Du = tan(Ru);
  rs0 = ((cp0-ep0)/Dd)*Du;
  rs1 = ((cp1-ep1)/Dd)*Du;
  rs2 = 1.0;

  /* r3 */
  norm = sqrt(rs0*rs0 + rs1*rs1 + rs2*rs2);
  t0 = rs0/norm;
  t1 = rs1/norm;
  t2 = rs2/norm;

  /* r1 */
  cross0 = t2;
  cross1 = 0.0;
  cross2 = -t0;
  norm = sqrt(cross0*cross0 + cross1*cross1 + cross2*cross2);
  tt0 = cross0/norm;
  tt1 = cross1/norm;
  tt2 = cross2/norm;

  /* r2 */
  cross0 = t1*tt2-t2*tt1;
  cross1 = t2*tt0-t0*tt2;
  cross2 = t0*tt1-t1*tt0;
  norm = sqrt(cross0*cross0 + cross1*cross1 + cross2*cross2);
  ttt0 = cross0/norm;
  ttt1 = cross1/norm;
  ttt2 = cross2/norm;

  fov = (params->phi/180)*PI;
  step_size = (tan(fov/2))/OUT_HEIGHT;

  params->rot_mat[0][0] = step_size*tt0;
  params->rot_mat[1][0] = step_size*tt1;
  params->rot_mat[2][0] = step_size*tt2;
  params->rot_mat[0][1] = step_size*ttt0;
  params->rot_mat[1][1] = step_size*ttt1;
  params->rot_mat[2][1] = step_size*ttt2;
  params->rot_mat[0][2] = t0;
  params->rot_mat[1][2] = t1;
  params->rot_mat[2][2] = t2;

}
/*======================================================================*/
/* Performs the inverse mapping to the Fisheye space                    */
/* This is done every time the ROI moves.                               */
/*      Input: the parameters params                                    */
/*             the Xp[3] perspective coordinates                        */
/*      Output: the coordinates array UV[2]                             */ 
/*======================================================================*/
void inverse_mapping(program_params_t *params, int j, int i, double UV[2]) {
  double Xp[3];
  double R2, Du, Ru, pol;
  double tempVal, sqrtR2, normVal, normFactor;
  int idx0, idx1, c01, c02;
  unsigned int intDu;
  unsigned long mask1, mask2, mask3;
  int      inverted, negative;

  Xp[0] = params->rot_mat[0][0] * i + params->rot_mat[0][1] * j + params->rot_mat[0][2];
  Xp[1] = params->rot_mat[1][0] * i + params->rot_mat[1][1] * j + params->rot_mat[1][2];
  Xp[2] = params->rot_mat[2][0] * i + params->rot_mat[2][1] * j+ params->rot_mat[2][2];

  R2 = Xp[0]*Xp[0] + Xp[1]*Xp[1];

  Du = sqrt(R2)/Xp[2];

  Ru = atan(Du);

  pol = params->k[4] + Ru*(params->k[3] + Ru*(params->k[2] + Ru*(params->k[1] + Ru*params->k[0]))); 

  UV[0] = (Xp[0] / sqrt(R2)) * pol + params->ep[0];    
  UV[1] = (Xp[1] / sqrt(R2)) * pol + params->ep[1];   
  UV[0] -= 1.0;
  UV[1] -= 1.0;
}

/*======================================================================*/
/*  Performs 2D bicubic interpolation to compute the pixel values of the*/
/*  corrected image at locations (U,V). It is based on the paper        */
/*  by R. Keyes,                                                        */
/* "Cubic convolution interpolation for digital image processing,"      */
/* IEEE Transactions on Acoustics, Speech, and Signal Processing, Dec.81*/
/*      Input: the parameters params                                    */
/*             the color channel(Blue=0, Green=1, Red=2)                */
/*      Output: the interpolated value at location UV                   */
/*======================================================================*/
unsigned char bicubic_interpolation(program_params_t *params, double UV[2], unsigned int channel) {
  unsigned char *cframe;
  unsigned char *xframe;
  long     double u, v, s, t;
  int      u_tl, v_tl, loc_tl, loc;
  double   interp_row_val[4];
  double   interp_col_val[4];
  double   temp, temp1, temp2, temp3, temp4;
  int      row;
  int      hc, wc, hx, wx;
  unsigned char c[4][4];
  unsigned char val;

  xframe = params->xtd_current_frame;
  cframe = params->current_frame;
  hc = params->current_frame_y_size;
  wc = params->current_frame_x_size;
  hx = params->xtd_current_frame_y_size;
  wx = params->xtd_current_frame_x_size;
  /* First , clip the parameters UV so that they always fall in the frame */
  if ((UV[0] < 0) || (UV[0] > (wc - 1)) || \
      (UV[1] < 0) || (UV[1] > (hc - 1)))
    return 0;

  u = (UV[0] < 0.0) ? 0.0 : ((UV[0] > wc-1) ? (wc-1) : (UV[0]));
  v = (UV[1] < 0.0) ? 0.0 : ((UV[1] > hc-1) ? (hc-1) : (UV[1]));
  u_tl = floor(u);
  v_tl = floor(v);

  /* loc_tl refers to the extended frame */
  loc_tl =  ((v_tl+1)*wx + u_tl+1)*3; /* loc_tl is always within the frame boundaries */
  /* Find the interpolation coefficients */
  c[0][0] = xframe[loc_tl - 3*wx - 3 + channel];
  c[0][1] = xframe[loc_tl - 3*wx + channel];
  c[0][2] = xframe[loc_tl - 3*wx + 3 + channel];
  c[0][3] = xframe[loc_tl - 3*wx + 3*2+ channel];
  c[1][0] = xframe[loc_tl - 3 + channel];
  c[1][1] = xframe[loc_tl + channel];
  c[1][2] = xframe[loc_tl + 3 + channel];
  c[1][3] = xframe[loc_tl + 3*2+ channel];
  c[2][0] = xframe[loc_tl + 3*wx - 3 + channel];
  c[2][1] = xframe[loc_tl + 3*wx + channel];
  c[2][2] = xframe[loc_tl + 3*wx + 3 + channel];
  c[2][3] = xframe[loc_tl + 3*wx + 3*2+ channel];
  c[3][0] = xframe[loc_tl + 3*2*wx - 3 + channel];
  c[3][1] = xframe[loc_tl + 3*2*wx + channel];
  c[3][2] = xframe[loc_tl + 3*2*wx + 3 + channel];
  c[3][3] = xframe[loc_tl + 3*2*wx + 3*2+ channel];

  /* The interpolation */
  s = u - (double) u_tl; 
  t = v - (double) v_tl; 

  /* First, interpolate using the pixels closest to the input pixel */
  interp_row_val[1] = ((3*s-5)*s*s+2)/2;
  interp_row_val[2] = (((4-3*s)*s+1)*s)/2;

  interp_col_val[1] = ((3*t-5)*t*t+2)/2;
  interp_col_val[2] = (((4-3*t)*t+1)*t)/2;

  temp1 = (double) (c[0][1]*interp_row_val[1] + c[0][2]*interp_row_val[2]);
  temp2 = (double) (c[1][1]*interp_row_val[1] + c[1][2]*interp_row_val[2]);
  temp3 = (double) (c[2][1]*interp_row_val[1] + c[2][2]*interp_row_val[2]);
  temp4 = (double) (c[3][1]*interp_row_val[1] + c[3][2]*interp_row_val[2]);

  interp_row_val[0] = (((2-s)*s-1)*s)/2;
  interp_row_val[3] = ((s-1)*s*s)/2;
  interp_col_val[0] = (((2-t)*t-1)*t)/2;
  interp_col_val[3] = ((t-1)*t*t)/2;
  temp1 +=(double) (c[0][0]*interp_row_val[0] + c[0][3]*interp_row_val[3]);
  temp2 += (double) (c[1][0]*interp_row_val[0] + c[1][3]*interp_row_val[3]);
  temp3 += (double) (c[2][0]*interp_row_val[0] + c[2][3]*interp_row_val[3]);
  temp4 += (double) (c[3][0]*interp_row_val[0] + c[3][3]*interp_row_val[3]);
  temp = (double) (temp1*interp_col_val[0] + temp2*interp_col_val[1] +temp3*interp_col_val[2] + temp4*interp_col_val[3]); 


  val = (unsigned char) clip(temp, 0, 255);
  return (unsigned char) val;
}


unsigned char bicubic_interpolation_approx(program_params_t *params, double UV[2], unsigned int channel) {
  unsigned char *cframe;
  unsigned char *xframe;
  long     double u, v, s, t;
  int      u_tl, v_tl, loc_tl, loc;
  double   interp_row_val[4];
  double   interp_col_val[4];
  double   temp, temp1, temp2, temp3, temp4;
  int      row;
  int      hc, wc, hx, wx;
  unsigned char c[4][4];
  unsigned char val;

  xframe = params->xtd_current_frame;
  cframe = params->current_frame;
  hc = params->current_frame_y_size;
  wc = params->current_frame_x_size;
  hx = params->xtd_current_frame_y_size;
  wx = params->xtd_current_frame_x_size;
  /* First , clip the parameters UV so that they always fall in the frame */
  if ((UV[0] < 0) || (UV[0] > (wc - 1)) || \
      (UV[1] < 0) || (UV[1] > (hc - 1)))
    return 0;

  u = (UV[0] < 0.0) ? 0.0 : ((UV[0] > wc-1) ? (wc-1) : (UV[0]));
  v = (UV[1] < 0.0) ? 0.0 : ((UV[1] > hc-1) ? (hc-1) : (UV[1]));
  u_tl = floor(u);
  v_tl = floor(v);

  /* loc_tl refers to the extended frame */
  loc_tl =  ((v_tl+1)*wx + u_tl+1)*3 + channel; /* loc_tl is always within the frame boundaries */
  /* Find the interpolation coefficients */
  return xframe[loc_tl];
}

/*======================================================================*/
/* imresize() performs a 2x downsampling on the input image             */
/*                    Inputs/Outputs: the algorithm parameters params   */
/*======================================================================*/
void imresize(program_params_t *params) {
}


/*======================================================================*/
/* FishEye_Beh() performs fisheye correction using the same structure   */
/* as the Matlab code                                                   */
/*                    Inputs: the algorithm parameters params           */
/*======================================================================*/

void FishEye_Beh(void *args) {
  tw *my_work = (tw*) args;
  program_params_t *params = my_work->params;
  int pic =my_work->pic;
  int    k,i, j,line_loc, loc,left,right;
  double UV[2];      
  unsigned char valb, valg, valr;
  int step = my_work->step;
  struct timespec now;
  my_work->approx = 2;
  clock_gettime(CLOCK_MONOTONIC_RAW, &now);
  my_work->start_time = now.tv_nsec + now.tv_sec * 1000000000;
  forward_mapping(params);  

  for (j = my_work->starty; j< my_work->endy &&  j < OUT_HEIGHT ; j++){
    line_loc = (( j + OUT_HEIGHT ) * (2*OUT_WIDTH) )*3;
    for (i = my_work->startx; i < my_work->endx && i < OUT_WIDTH ; i++)  {
      loc = line_loc + (i + OUT_WIDTH)*3;
      inverse_mapping(params, j , i, UV);
      valb = (unsigned char) bicubic_interpolation(params, UV, 0); 
      valg = (unsigned char) bicubic_interpolation(params, UV, 1);     
      valr = (unsigned char) bicubic_interpolation(params, UV, 2);
      params->xtd_output_frame[pic][loc++] = valb;
      params->xtd_output_frame[pic][loc++] = valg;
      params->xtd_output_frame[pic][loc++] = valr;
    }
  }   
}

void FishEye_Beh_approx(void *args ) {
  tw *my_work = (tw*) args;
  program_params_t *params = my_work->params;
  double UV[2];      
  unsigned char valb, valg, valr;

  int pic = my_work->pic;
  int    k,j,i,line_loc , loc, loc_left,loc_right;
  int step= my_work->step;
  my_work->approx = 2;
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC_RAW, &now);
  my_work->start_time = now.tv_nsec + now.tv_sec * 1000000000;
//  xtd_current_frame(params);
  forward_mapping(params);  
  for (j = my_work->starty; j< my_work->endy &&  j < OUT_HEIGHT ; j++){
    line_loc = (( j + OUT_HEIGHT ) * (2*OUT_WIDTH) )*3;
    for (i = my_work->startx; i < my_work->endx && i < OUT_WIDTH ; i++)  {
      loc = line_loc + (i + OUT_WIDTH)*3;
      inverse_mapping(params, j , i, UV);
      valb = (unsigned char) bicubic_interpolation_approx(params, UV, 0); 
      valg = (unsigned char) bicubic_interpolation_approx(params, UV, 1);     
      valr = (unsigned char) bicubic_interpolation_approx(params, UV, 2);
      params->xtd_output_frame[pic][loc++] = valb;
      params->xtd_output_frame[pic][loc++] = valg;
      params->xtd_output_frame[pic][loc++] = valr;

      /*
         loc = line_loc + (i + OUT_WIDTH)*3;
         int loc_left = loc  - step/2 * 3;
         for (k = 0 ; k < 3 ; k++){
         unsigned char val = params->xtd_output_frame[pic][loc_left];    
         params->xtd_output_frame[pic][loc] = val;
         loc_left++;
         loc++;
         */
    }
    }
  }

  //imresize(params);
  //}


int comp( const void* a, const void * b){
  const tw *ptr1 = (const tw*) a;
  const tw *ptr2 = (const tw*) b;
  return ptr1->start_time - ptr2->start_time;
}

/*====================================================================*/
/* The program is called as: % fisheye.exe filename.bmp -c X,Y -f PHI */
/* X:   the column coordinate of the center of the ROI                */
/* Y:   the row coordinate of the center of the ROI                   */
/* PHI: the field of view of the ROI given in angles PHI:0..180       */
/*      The FoV determines the size of the ROI. The actual FoV is     */
/*      PHI/(180*PI)                                                  */
/*====================================================================*/
int main(int argc, char **argv) {
  int p, key;
  char Save;
  char savefilename[80] = "0_";
  char *filename;
  program_args_t *args;
  program_params_t *params;
  int i,j,k,l;
  struct timespec  tv1, tv2;
  tw *threads;
  args = (program_args_t *) malloc(sizeof(program_args_t));
  params = (program_params_t *) malloc(sizeof(program_params_t));
  if (!args || !params)
    exit (0);
  /* Parse the input arguments */
  parse_args(argc, argv, args);
  /* and initialize the parameters of the correction algorithm */


  init_params(params, args); 



  int x = args->x;
  int y = args->y;
  params->cp[0] = x;   /* is the col x */
  params->cp[1] = y;   /* is the row y */


  int workx = 128;
  int worky=64;
  int num_blocksx = 2*(OUT_WIDTH)/workx;
  int num_blocksy = 2*(OUT_HEIGHT)/worky;



  threads = (tw*) malloc (sizeof(tw)*num_blocksx*num_blocksy);

  for ( i = 0 ; i < num_blocksy  ;  i++)
    for ( j = 0 ; j <  num_blocksx ; j++){
      threads[i*num_blocksx+j].startx =  j*workx - OUT_WIDTH ;
      threads[i*num_blocksx+j].endx =  (j+1) *workx - OUT_WIDTH ;
      threads[i*num_blocksx+j].starty = i*worky - OUT_HEIGHT;
      threads[i*num_blocksx+j].endy = (i+1)*worky - OUT_HEIGHT;
      threads[i*num_blocksx+j].params = params;
      threads[i*num_blocksx+j].b_idy = i; 
      threads[i*num_blocksx+j].b_idx = j;
      threads[i*num_blocksx+j].step = 1;
    }

  clock_gettime(CLOCK_MONOTONIC_RAW, &tv1);

  if (params->num_pics == 0){
    printf("You should define at least on picture\n");
    exit(0);
  }

  xtd_current_frame(params);
  init_system(num_threads);
	char group[1024];
	task_t *task;

  for ( l = 0; l < params->num_pics; l++){
		sprintf(group, "pic%d", l);
    for ( i = 0 ; i < num_blocksy  ;  i++) {
      for ( j = 0 ; j <  num_blocksx ; j++) {
        threads[i*num_blocksx + j].pic = l;
				task = new_task((TASK)FishEye_Beh, threads+i*num_blocksx+j, sizeof(tw), 
					(TASK)FishEye_Beh_approx, NON_SIGNIFICANT+1);
				push_task(task, group);
      }
    }
    fflush(stdout);
    wait_group(group,  NULL, NULL, SYNC_RATIO, 0, 0, RATIO , 0);
  }
	shutdown_system();
	#if 0
  for ( i = 0 ; i < num_blocksy  ;  i++)
    for ( j = 0 ; j <  num_blocksx ; j++)
     if ( threads[i*num_blocksx+j].approx != 2 )
       printf("I have not been executed\n");
	#endif
  // qsort(threads,num_blocksx*num_blocksy*5,sizeof(tw),comp);
  // for( i = 0 ; i< num_blocksx*num_blocksy*5; i++)
  //   printf("%d %d approx %d \tsignificance %d\t \n",threads[i].b_idy,threads[i].b_idx,threads[i].approx, threads[i].sign);
  BmpHeader head = {0, 0, 54, 40, 0, 0, 1, 24, 0, 0}; /* BMP file header */  
  FILE *fp = fopen("out.bmp","wb");
  fputc('B',fp);   fputc('M',fp);
  head.width = params->xtd_output_frame_x_size;
  head.height = params->xtd_output_frame_y_size;
  head.sizeImage = params->xtd_output_frame_y_size *  params->xtd_output_frame_x_size *3;
  head.sizeFile = head.sizeImage + head.offbits;
  fwrite(&head, sizeof head, 1, fp); 
  if (fwrite(params->xtd_output_frame[params->num_pics-1], 1, head.sizeImage, fp) != head.sizeImage) {
    fclose(fp);
    printf("Error when writing file\n");
    return 1;  
  }
	return 0;
}

