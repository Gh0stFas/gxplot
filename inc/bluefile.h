#ifndef BLUEFILE_H
#define BLUEFILE_H

#include <stdio.h>
#include <stdint.h>

#define BLUE_HDR_SIZE (512)

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

// This is the definition for the main header of the
// blue file format.
typedef struct {
  char version[4];       // Header version
  char head_rep[4];      // Header representation
  char data_rep[4];      // Data representation
  
  int32_t detached;      // detached header
  int32_t prot;          // protected from overwrite
  int32_t pipe;          // pipe mode
  
  int32_t ext_start;     // extended header start block
  int32_t ext_size;      // extended header size bytes
  
  double data_start;     // data start in bytes
  double data_size;      // data size in bytes
  
  int32_t type;          // file type code
  char format[2];        // data format code
  int16_t flagmask;      // 16 bit flag mask
  double timecode;       // time code field
  
  int16_t inlet;         // inlet owner
  int16_t outlets;       // number of outlets
  int32_t outmask;       // outlet async mask
  int32_t pipeloc;       // pipe location
  int32_t pipesize;      // pipe size in bytes
  double in_byte;        // next input byte
  double out_byte;       // next out byte (cumulative)
  double out_bytes[8];   // next out byte (each outlet) L_outlets
  
  int32_t keylength;     // length of keyword string
  char keywords[92];     // user defined keyword string L_keywords
  
  // Adjunct for type 1000/2000 support
  double xstart;         // abscissa start
  double xdelta;         // abscissa delta
  int32_t xunits;        // abscissa units
  int32_t subsize;       // element size (type 2000)
  double ystart;         // secondary start
  double ydelta;         // secondary delta
  int32_t yunits;        // secondary units
  int32_t bid;           // buffer id
  
  char fill[208];        // Fill for the remaining bytes out of 512
  
  // Space below here should be used for internal information and book keeping.
  char buf_type;
  int bpe;
  
} BLUE_HDR_T;

FILE *bf_open(char *,BLUE_HDR_T *);
long bf_read(FILE *,BLUE_HDR_T *,void *,long );
void bf_close(FILE *,BLUE_HDR_T *);
void bf_print(BLUE_HDR_T *);
int bf_read_hdr(FILE *, BLUE_HDR_T *);
int bf_seek(FILE *, BLUE_HDR_T *, long );

#endif //BLUEFILE_H