#ifndef FILEIO_H
#define FILEIO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <bluefile.h>
#include <detfile.h>

// File type definitions
enum FILE_TYPE {BLUE,DETACHED};

// Default data types
enum DATA_TYPE {NONE=0,TIME,FREQ};

typedef struct {
  FILE *fstream;
  char *name;
  char *base_name;
  enum FILE_TYPE ftype;
  
  // Accepted formats are
  // Mode:
  //  S - Scalar
  //  C - Complex
  // Format:
  //  B - Byte
  //  I - 2 byte integer
  //  L - 4 byte integer
  //  F - Single precision floating point
  //  D - Double precision floating point
  char format[2];
  
  double xstart; // Min XSTART value
  double xdelta; // delta between samples
  double ydelta; // delta between samples
  double xend;   // Max XEND value
  int bpe;       // bytes per element
  long total_elements; // Total number of elements available
  
  int framed;  // Flag whether or not this is framed data
  int frame_size;
  
  enum DATA_TYPE data_type;
  
  BLUE_HDR_T *bhdr;
  DET_HDR_T *dhdr;
  
  union data_rep {
    void *void_ptr;
    char *byte_ptr;
    unsigned char *ubyte_ptr;
    int16_t *sint_ptr;
    int32_t *int_ptr;
    float *float_ptr;
    double *double_ptr;
  } data_ptr;
  
  // Current XSTART and XEND of the data that's in the data_ptr union
  double current_xstart;
  double current_xend;
  
  // Number of elements currently in the data_ptr union
  long nelms;
  
  // Holds the number of elements currently
  // in view
  long nelms_vis;
  
  // Used to auto-reformat data when read if different
  // from that of format[1]
  char buf_type;
} FILEIO_T;

FILEIO_T *fio_open(char *,int);
void fio_close(FILEIO_T *);
long fio_read(FILEIO_T *,long );
void fio_alloc_buf(FILEIO_T *, size_t );
void fio_free_buf(FILEIO_T *);
int fio_seek_abs(FILEIO_T *,double );
enum FILE_TYPE check_ftype(FILE *fstream);

#endif //FILEIO_H
