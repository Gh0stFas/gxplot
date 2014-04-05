//gcc -c -fpic -I/home/jsegars bluefile.c
//gcc -shared -o libbluefile.so bluefile.o
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <bluefile.h>

FILE *bf_open(char *fname,BLUE_HDR_T *hdr){
  FILE *tfile=NULL;
  
  // Attempt to open the file
  if((tfile = fopen(fname,"r")) == NULL){
    perror("FOPEN: ");
  } else {
    // Allocate the BLUE header
    if(hdr != NULL) fprintf(stdout, "WARN: Header appears to already be allocated. You may leak memory\n");
    
    if((hdr = (BLUE_HDR_T *) calloc(1,sizeof(BLUE_HDR_T))) == NULL){
      fprintf(stderr, "ERROR: Could not allocate space for the BLUE file header\n");
      
      bf_close(tfile,hdr);
    } else {
      // Let's read in the header
      if(fread(hdr,sizeof(char),BLUE_HDR_SIZE,tfile) != BLUE_HDR_SIZE){
        if(feof(tfile) != 0){
          fprintf(stderr, "ERROR: End of file reached while attempting to read the header\n");
        }
        else if(ferror(tfile) != 0){
          fprintf(stderr, "ERROR: An error occurred while attempting to read the BLUE header\n");
        }
        
        // Regardless of what happened we need to close the stream
        bf_close(tfile,hdr);
      } else {
        // Let's spot check the header values
        return(tfile);
      }
    }
  }
  return NULL;
}

int bf_read_hdr(FILE *fstream, BLUE_HDR_T *hdr){
  if(fstream != NULL && hdr != NULL){
    if(fread(hdr,sizeof(char),BLUE_HDR_SIZE,fstream) != BLUE_HDR_SIZE){
      if(feof(fstream) != 0){
        fprintf(stderr, "ERROR: End of file reached while attempting to read the header\n");
      }
      else if(ferror(fstream) != 0){
        fprintf(stderr, "ERROR: An error occurred while attempting to read the BLUE header\n");
      }
    } else {
      // Let's go ahead and set any default internal structure members
      
      // Default the buff type to that of the input file header
      hdr->buf_type = hdr->format[1];
      
      // Calculate the bytes per element
      hdr->bpe=1;
      if(hdr->format[0] == 'C') hdr->bpe=2;
      if(hdr->format[1] == 'I') hdr->bpe*=2;
      else if(hdr->format[1] == 'L') hdr->bpe*=4;
      else if(hdr->format[1] == 'F') hdr->bpe*=4;
      else if(hdr->format[1] == 'D') hdr->bpe*=8;
      
      return 1;
    }
  }
  return 0;
}

void bf_print(BLUE_HDR_T *hdr){
  // The char fields are not NULL terminated
  fprintf(stdout,"BLUE Header info:\n");
  fprintf(stdout,"\tVersion    = %c%c%c%c\n",hdr->version[0],hdr->version[1],hdr->version[2],hdr->version[3]);
  fprintf(stdout,"\tHdr rep    = %c%c%c%c\n",hdr->head_rep[0],hdr->head_rep[1],hdr->head_rep[2],hdr->head_rep[3]);
  fprintf(stdout,"\tData rep   = %c%c%c%c\n",hdr->data_rep[0],hdr->data_rep[1],hdr->data_rep[2],hdr->data_rep[3]);
  fprintf(stdout,"\tDetached   = %d\n",hdr->detached);
  fprintf(stdout,"\tProtected  = %d\n",hdr->prot);
  fprintf(stdout,"\tPipe       = %d\n",hdr->pipe);
  fprintf(stdout,"\tExt start  = %d\n",hdr->ext_start);
  fprintf(stdout,"\tExt size   = %d\n",hdr->ext_size);
  fprintf(stdout,"\tData start = %g\n",hdr->data_start);
  fprintf(stdout,"\tData size  = %g\n",hdr->data_size);
  fprintf(stdout,"\tType       = %d\n",hdr->type);
  fprintf(stdout,"\tFormat     = %c%c\n",hdr->format[0],hdr->format[1]);
  fprintf(stdout,"\tFlag mask  = %d\n",hdr->flagmask);
  fprintf(stdout,"\tTimecode   = %f\n",hdr->timecode);
  fprintf(stdout,"\tXSTART     = %g\n",hdr->xstart);
  fprintf(stdout,"\tXDELTA     = %g\n",hdr->xdelta);
  fprintf(stdout,"\tXunits     = %d\n",hdr->xunits);
  fprintf(stdout,"\tSUBSIZE    = %d\n",hdr->subsize);
  fprintf(stdout,"\tYSTART     = %g\n",hdr->ystart);
  fprintf(stdout,"\tYDELTA     = %g\n",hdr->ydelta);
  fprintf(stdout,"\tYunits     = %d\n\n",hdr->yunits);
  
  fprintf(stdout,"\tBPE     = %d\n",hdr->bpe);
  return;
}

long bf_read(FILE *fstream,BLUE_HDR_T *hdr,void *buffer,long nelms){
  long nread=0;
  int epf=1;
  
  if(fstream != NULL && hdr != NULL && buffer != NULL && nelms > 0){    
    // We need to look at the number of elements requested versus
    // the data size reported by the header
    if(hdr->type == 1000){
      if((long)hdr->bpe*nelms > (long)(hdr->data_size-hdr->in_byte)){
        nelms = (long)(hdr->data_size-hdr->in_byte)/hdr->bpe;
      }
    }
    else if(hdr->type == 2000){
      epf = hdr->subsize;
    }
    
    if(nelms <= 0){
      fprintf(stdout, "WARN: End of blue file reached.\n");
    } else {
      // Attempt the read
      //fprintf(stdout,"Reading %ld elements\n",(hdr->bpe*epf)*nelms);
      if((nread = fread(buffer,hdr->bpe*epf,nelms,fstream)) != nelms){
        if(feof(fstream) != 0){
          fprintf(stdout, "WARN: End of blue file reached, truncating.\n");
        }
        else if(ferror(fstream) != 0){
          fprintf(stderr, "ERROR: An error occurred while attempting to read the blue file\n");
          nread=0;
        }
      }
      
      // Up the in_byte versus the number of bytes just read
      hdr->in_byte = (double)(ftell(fstream) - 512L);
    }
  }
  
  return(nread);
}

int bf_seek(FILE *file, BLUE_HDR_T *hdr, long nelms){
  int rtn=-1;
  //printf("seeking %ld elements\n",nelms);
  long bpe = (hdr->type == 2000 ? (long)hdr->bpe*(long)hdr->subsize:(long)hdr->bpe);
  
  if(file != NULL && hdr != NULL){
    if(hdr->in_byte + (double)(nelms*bpe) < 0.0){
      hdr->in_byte=0.0;
        
      fseek(file,512L,SEEK_SET);
    }
    else {
      if(hdr->in_byte + (double)(nelms*bpe) > hdr->data_size){
        nelms = (long)((hdr->data_size - hdr->in_byte) / (double)bpe);
        fprintf(stdout, "Seek request would place the buffer past the data, truncating to %ld\n",nelms);
      }
      if(fseek(file,(nelms * bpe),SEEK_CUR) != 0){
        perror("File seek failed:");
      } else {
        hdr->in_byte = (double)(ftell(file) - 512L);
      }
    }
    rtn=0;
  }
  
  return(rtn);
}

void bf_close(FILE *file,BLUE_HDR_T *hdr){
  if(file != NULL){
    if(fclose(file) != 0) perror("FCLOSE: ");
  }
  
  // Clear the header
  if(hdr != NULL) free(hdr);
}