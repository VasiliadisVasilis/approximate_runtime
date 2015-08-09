
/* ---------------------------------------------------------------*/
#if !FISHEYE_H
#define FISHEYE_H

#define IN_WIDTH   800
#define IN_HEIGHT  600

int OUT_WIDTH=80;	
int OUT_HEIGHT=60;
#define BUF_LENGTH 16

typedef unsigned short WORD;
typedef unsigned int  DWORD;

typedef struct tagBmpHeader {
  //	char  type[2];       /* = "BM", it is matipulated separately to make the size of this structure the multiple of 4, */
  DWORD sizeFile;      /* = offbits + sizeImage */
  DWORD reserved;      /* == 0 */
  DWORD offbits;       /* offset from start of file = 54 + size of palette */
  DWORD sizeStruct;    /* 40 */
  DWORD width, height;
  WORD  planes;        /* 1 */
  WORD  bitCount;      /* bits of each pixel, 256color it is 8, 24 color it is 24 */
  DWORD compression;   /* 0 */
  DWORD sizeImage;     /* (width+?)(till multiple of 4) * height£¬in bytes */
  DWORD xPelsPerMeter; /* resolution in mspaint */
  DWORD yPelsPerMeter;
  DWORD colorUsed;     /* if use all, it is 0 */
  DWORD colorImportant;/*  */
} BmpHeader;


typedef struct _program_args_t {
  unsigned char *frame;    /* the input frame */
  double   phi;       /* angle phi for Field of View fov=phi/(180*pi) */
  int x,y;
  int num_pics;
}program_args_t;

typedef struct _program_params_t {
  unsigned char *current_frame;
  unsigned char *initial_frame;
  unsigned char *xtd_current_frame;
  unsigned char **xtd_output_frame;
  unsigned char *output_frame;
  unsigned int num_pics; 
  
  unsigned int current_frame_x_size;
  unsigned int current_frame_y_size;
  
  unsigned int xtd_output_frame_x_size;
  unsigned int xtd_output_frame_y_size;
  
  unsigned int xtd_current_frame_x_size;
  unsigned int xtd_current_frame_y_size;
  
  unsigned int output_frame_x_size;
  unsigned int output_frame_y_size;
  
  char     *input_winname;
  char     *xtd_output_winname;
  char     *output_winname;
  double   cp[3];     /* the center of the ROI */
  double   phi;       /* angle phi for Field of View fov=phi/(180*pi) */
  double   ep[3];     /* the distortion center of the lens */
  double   k[5];
  double   k_inv[5];
  double   rot_mat[3][3];
  
  
}program_params_t;

/* Function stubs are inserted here */
void parse_args(int, char **, program_args_t *);
void init_params(program_params_t *, program_args_t *);
int sign(double);
int clip(int, int, int);
void print_usage(void);
void drawPixel(program_params_t *, int, int, int, int, int);
void forward_mapping(program_params_t *);
void inverse_mapping(program_params_t *, int, int, double []);
unsigned char bicubic_interpolation(program_params_t *, double [], unsigned int  );
// void FishEye_Beh(program_params_t *);
void EdgeFilter(program_params_t *, int);
void UserInput(unsigned char, int, int, program_params_t *);

#endif

