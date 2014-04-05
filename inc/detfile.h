#ifndef DETFILE_H
#define DETFILE_H

#include <stdio.h>
#include <stdint.h>

#define MAX_KEY (20)
#define MAX_VALUE (1024)
#define MAX_KV (20)

#define MAX_FNAME (1024)

/* File format codes
 * First character
   S - Scalar
   C - Complex
   
   Second character
   B - Byte
   I - 2 byte integer int16_t
   L - 4 byte integer int32_t
   F - Single precision floating point
   D - Double precision floating point
*/

typedef struct{
  char key[MAX_KEY];
  char value[MAX_VALUE];
} KV_T;

typedef struct {
  char fname[MAX_FNAME];
  char data_rep[4];      // Data representation
  
  double data_start;     // data start in bytes
  double data_size;      // data size in bytes
  
  int32_t type;          // file type code
  char format[3];        // data format code
  int16_t flagmask;      // 16 bit flag mask
  double timecode;       // time code field
  
  double in_byte;        // next input byte
  
  // Adjunct for type 1000/2000 support
  double xstart;         // abscissa start
  double xdelta;         // abscissa delta
  int32_t xunits;        // abscissa units
  int32_t subsize;       // element size (type 2000)
  double ystart;         // secondary start
  double ydelta;         // secondary delta
  int32_t yunits;        // secondary units
  int32_t bid;           // buffer id
  
  // Space below here should be used for internal information and book keeping.
  char buf_type;
  int bpe;
  
  KV_T kv[MAX_KV];
  int kvp;
  
} DET_HDR_T;

FILE *det_open(char *,DET_HDR_T *);
long det_read(FILE *,DET_HDR_T *,void *,long );
void det_close(FILE *,DET_HDR_T *);
void det_print(DET_HDR_T *);
int det_read_hdr(FILE *, DET_HDR_T *);
int det_seek(FILE *, DET_HDR_T *, long );

#endif //DETFILE_H