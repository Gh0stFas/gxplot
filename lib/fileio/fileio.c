#include <fileio.h>

FILEIO_T *fio_open(char *fname, int debug){
  FILEIO_T *fio=NULL;
  int i,fname_s,fname_e;
  
  if(fname == NULL){
    fprintf(stderr, "NULL file name passed to fio_open\n");
  } else {
    if((fio = (FILEIO_T *) calloc(1,sizeof(FILEIO_T))) == NULL){
      fprintf(stderr, "Could not allocate space for FILEIO_T\n");
    } else {
      // Allocate space for the name
      fio->name = (char *) calloc(strlen(fname)+1,sizeof(char));
      
      // Allocate space for the various header types
      fio->bhdr = (BLUE_HDR_T *) calloc(1,sizeof(BLUE_HDR_T));
      fio->dhdr = (DET_HDR_T *) calloc(1,sizeof(DET_HDR_T));
      
      // Let's see if the file name needs trimming
      fname_s = 0;
      for(i=0;i<strlen(fname);i++){
        if(isspace(fname[i])){
          fname_s++;
        } else break;
      }
      fname_e = strlen(fname);
      for(i=strlen(fname);i>0;i--){
        if(isspace(fname[i])){
          fname_s--;
        } else break;
      }
      snprintf(fio->name,(fname_e-fname_s)+1,"%s",fname+fname_s);
      
      // Attempt to open the file
      if((fio->fstream = fopen(fio->name,"r")) == NULL){
        perror("FOPEN: ");
        
        fio_close(fio);
      } else {        
        // Check the file type
        fio->ftype = check_ftype(fio->fstream);
        
        fio->base_name = basename(fio->name);
        
        // NULL out the data buffer
        fio->data_ptr.void_ptr=NULL;
        fio->nelms=0;
        
        if(fio->ftype == BLUE){        
          if(bf_read_hdr(fio->fstream,fio->bhdr) == 0){
            fprintf(stderr,"An error occurred while trying to read the BLUE file header\n");
          }
          else {
          
            fprintf(stdout, "BLUE file format\n");
          
            // Bubble up the format code
            fio->format[0] = fio->bhdr->format[0];
            fio->format[1] = fio->bhdr->format[1];
          
            // Bubble up the xstart and xdelta
            fio->xstart=fio->current_xstart=fio->bhdr->xstart;
            fio->xdelta=fio->bhdr->xdelta;
            fio->xend=fio->xstart+(((fio->bhdr->data_size/(double)fio->bhdr->bpe)*fio->xdelta)-fio->xdelta);
          
            // Default the buffer type to the file type
            fio->buf_type = fio->format[1];
          
            // Bubble up the number of bytes per element
            fio->bpe = fio->bhdr->bpe;
          
            // Determine the data type if possible
            if(fio->bhdr->xunits == 1) fio->data_type = TIME;
            else if(fio->bhdr->xunits == 3) fio->data_type = FREQ;
            else {
              fprintf(stderr,"Unhandled data type in the BLUE header, %d. Assuming unframed time series data.\n",fio->bhdr->xunits);
              fio->data_type = TIME;
            }
          
            // Determine if this is framed data or not
            if(fio->bhdr->type == 2000 || fio->data_type == FREQ){
              fio->framed=1;
              fio->total_elements = (fio->bhdr->type == 1000 ? 1L:(long)(fio->bhdr->data_size/(double)fio->bhdr->subsize));
              fio->frame_size=(fio->bhdr->type == 1000 ? (int)(fio->bhdr->data_size/(double)fio->bpe):fio->bhdr->subsize);
              if(fio->bhdr->type == 2000) fio->ydelta = fio->bhdr->ydelta;
              else fio->ydelta = 0.0;
            } else {
              fio->framed=0;
              fio->frame_size=1;
              fio->total_elements = (long)(fio->bhdr->data_size/(double)fio->bhdr->bpe);
            }
          
            // Print the header. This should be a debug setting
            if(debug) bf_print(fio->bhdr);
            
            return(fio);
          }
        }
        else {        
          fprintf(stdout, "Detached file format. Data file is %s\n",fio->dhdr->fname);
          
          if(det_read_hdr(fio->fstream,fio->dhdr) == 0){
            fprintf(stderr,"An error occurred while trying to read the detached header\n");
          }
          else {          
            // We need to close the header file out and open up the data
            fclose(fio->fstream);
          
            if((fio->fstream = fopen(fio->dhdr->fname,"r")) == NULL){
              perror("FOPEN: ");
        
              fio_close(fio);
            }
            else {
              fio->base_name = basename(fio->dhdr->fname);
              
              // Bubble up the format code
              fio->format[0] = fio->dhdr->format[0];
              fio->format[1] = fio->dhdr->format[1];
          
              // Bubble up the xstart and xdelta
              fio->xstart=fio->current_xstart=fio->dhdr->xstart;
              fio->xdelta=fio->dhdr->xdelta;
              fio->xend=fio->xstart+(((fio->dhdr->data_size/(double)fio->dhdr->bpe)*fio->xdelta)-fio->xdelta);
          
              // Default the buffer type to the file type
              fio->buf_type = fio->format[1];
          
              // Bubble up the number of bytes per element
              fio->bpe = fio->dhdr->bpe;
          
              // Determine the data type if possible
              if(fio->dhdr->xunits == 1) fio->data_type = TIME;
              else if(fio->dhdr->xunits == 3) fio->data_type = FREQ;
              else {
                fprintf(stderr,"Unhandled data type in the Detached header, %d. Assuming unframed time series data.\n",fio->dhdr->xunits);
                fio->data_type = TIME;
              }
          
              // Determine if this is framed data or not
              if(fio->dhdr->type == 2000 || fio->data_type == FREQ){
                fio->framed=1;
                fio->total_elements = (fio->dhdr->type == 1000 ? 1L:(long)(fio->dhdr->data_size/(double)fio->dhdr->subsize));
                fio->frame_size=(fio->dhdr->type == 1000 ? (int)(fio->dhdr->data_size/(double)fio->bpe):fio->dhdr->subsize);
                if(fio->dhdr->type == 2000) fio->ydelta = fio->dhdr->ydelta;
                else fio->ydelta = 0.0;
              } else {
                fio->framed=0;
                fio->frame_size=1;
                fio->total_elements = (long)(fio->dhdr->data_size/(double)fio->dhdr->bpe);
              }
          
              // Print the header. This should be a debug setting
              if(debug) det_print(fio->dhdr);
        
              return(fio);
            }
          }
        }
      }
    }
  }
  
  return NULL;
}

enum FILE_TYPE check_ftype(FILE *fstream){
  char hbuf[5];
  enum FILE_TYPE rtn;
  
  if(fread(hbuf,sizeof(char),4,fstream) != 4){
    perror("fread");
  }
  
  hbuf[4]='\0';
  
  if(strcmp(hbuf,"BLUE") == 0) rtn=BLUE;
  else rtn=DETACHED;
  
  // Seek back to the beginning
  fseek(fstream,0L,SEEK_SET);
  
  return(rtn);
}

int fio_seek_abs(FILEIO_T *fio,double abs_dist){
  // -1 indicates an error, 0 a success
  int rtn=-1;
  long nelms,bytes;
  
  if(fio != NULL && fio->fstream != NULL){
    if(abs_dist != 0){
      // BLUE file seek
      //if(fio->ftype == BLUE){
        if(!(fio->framed)){
          // Determine the number of elements
          nelms = (long)round(abs_dist/fio->xdelta);
        
          // fio->nelms is expected to be our block size as we're actually
          // seeking relative to our last read position. So if fio->nelms were
          // 32768 and we wanted to move forward in the file by 8 elements we
          // need to actually tell bf_seek to move back 32760 from it's current
          // position. bf_seek handles any checking of the request.
          if(fio->ftype == BLUE) bf_seek(fio->fstream,fio->bhdr,nelms-fio->nelms);
          else det_seek(fio->fstream,fio->dhdr,nelms-fio->nelms);
        }
        else {
          nelms = (long)(abs_dist/fio->ydelta);
          printf("seeking %ld %0.5f %0.5f\n",nelms,abs_dist,fio->ydelta);
          
          if(fio->ftype == BLUE) bf_seek(fio->fstream,fio->bhdr,nelms-fio->nelms);
          else det_seek(fio->fstream,fio->dhdr,nelms-fio->nelms);
        }
      //}
    } else rtn=0;
  }
  return rtn;
}

long fio_read(FILEIO_T *fio,long size){
  long nread=0,i,mult,skip;
  int type;
  
  if(fio != NULL && fio->fstream != NULL && fio->data_ptr.void_ptr != NULL){
    // BLUE file read
    if(fio->ftype == BLUE) {
      type = fio->bhdr->type;
      nread = bf_read(fio->fstream,fio->bhdr,fio->data_ptr.void_ptr,(fio->framed && type == 1000 ? size*(long)fio->frame_size:size));
      skip=512L;
    }
    else {
      type = fio->dhdr->type;
      nread = det_read(fio->fstream,fio->dhdr,fio->data_ptr.void_ptr,(fio->framed && type == 1000 ? size*(long)fio->frame_size:size));
      skip=0L;
    }
    
      fio->nelms=(fio->framed && type == 1000 ? nread/(long)fio->frame_size:nread);
      
      // Calculate the buffers xstart value
      if(fio->framed){
        fio->current_xstart = fio->xstart;
        fio->current_xend = fio->xstart + ((double)fio->frame_size-1.0*fio->xdelta);
      }
      else {
        fio->current_xstart = fio->xstart + ((double)((ftell(fio->fstream) - (fio->nelms*(long)fio->bpe) - skip)/(long)fio->bpe) * fio->xdelta);
        fio->current_xend = fio->current_xstart + ((double)nread*fio->xdelta) - fio->xdelta;
      }
      
      // Check to see if the user requested a reformating of the buffer
      if(fio->buf_type != fio->format[1]){
        if(fio->format[0] == 'C') mult=2;
        else mult=1;
        
        if(fio->buf_type == 'B'){
          char *tbuf = (char *) calloc(fio->nelms*mult*fio->frame_size,sizeof(char));
          
          for(i=0;i<fio->nelms*mult*fio->frame_size;i++){
            if(fio->format[1] == 'I') tbuf[i] = (char)(fio->data_ptr.sint_ptr[i]);
            else if(fio->format[1] == 'L') tbuf[i] = (char)(fio->data_ptr.int_ptr[i]);
            else if(fio->format[1] == 'F') tbuf[i] = (char)(fio->data_ptr.float_ptr[i]);
            else if(fio->format[1] == 'D') tbuf[i] = (char)(fio->data_ptr.double_ptr[i]);
          }
          
          memcpy(fio->data_ptr.void_ptr,tbuf,fio->nelms*mult*sizeof(char)*fio->frame_size);
          
          free(tbuf);
        }
        else if(fio->buf_type == 'I'){
          int16_t *tbuf = (int16_t *) calloc(fio->nelms*mult*fio->frame_size,sizeof(int16_t));
          
          for(i=0;i<fio->nelms*mult*fio->frame_size;i++){
            if(fio->format[1] == 'B') tbuf[i] = (int16_t)(fio->data_ptr.byte_ptr[i]);
            else if(fio->format[1] == 'L') tbuf[i] = (int16_t)(fio->data_ptr.int_ptr[i]);
            else if(fio->format[1] == 'F') tbuf[i] = (int16_t)(fio->data_ptr.float_ptr[i]);
            else if(fio->format[1] == 'D') tbuf[i] = (int16_t)(fio->data_ptr.double_ptr[i]);
          }
          
          memcpy(fio->data_ptr.void_ptr,tbuf,fio->nelms*mult*sizeof(int16_t)*fio->frame_size);
          
          free(tbuf);
        }
        else if(fio->buf_type == 'L'){
          int32_t *tbuf = (int32_t *) calloc(fio->nelms*mult*fio->frame_size,sizeof(int32_t));
          
          for(i=0;i<fio->nelms*mult*fio->frame_size;i++){
            if(fio->format[1] == 'B') tbuf[i] = (int32_t)(fio->data_ptr.byte_ptr[i]);
            else if(fio->format[1] == 'I') tbuf[i] = (int32_t)(fio->data_ptr.sint_ptr[i]);
            else if(fio->format[1] == 'F') tbuf[i] = (int32_t)(fio->data_ptr.float_ptr[i]);
            else if(fio->format[1] == 'D') tbuf[i] = (int32_t)(fio->data_ptr.double_ptr[i]);
          }
          
          memcpy(fio->data_ptr.void_ptr,tbuf,fio->nelms*mult*sizeof(int32_t)*fio->frame_size);
          
          free(tbuf);
        }
        else if(fio->buf_type == 'F'){
          float *tbuf = (float *) calloc(fio->nelms*mult*fio->frame_size,sizeof(float));
          
          for(i=0;i<fio->nelms*mult*fio->frame_size;i++){
            if(fio->format[1] == 'B') tbuf[i] = (float)(fio->data_ptr.byte_ptr[i]);
            else if(fio->format[1] == 'I') tbuf[i] = (float)(fio->data_ptr.sint_ptr[i]);
            else if(fio->format[1] == 'L') tbuf[i] = (float)(fio->data_ptr.int_ptr[i]);
            else if(fio->format[1] == 'D') tbuf[i] = (float)(fio->data_ptr.double_ptr[i]);
          }
          
          memcpy(fio->data_ptr.void_ptr,tbuf,fio->nelms*mult*sizeof(float)*fio->frame_size);
          
          free(tbuf);
        }
        else if(fio->buf_type == 'D'){
          double *tbuf = (double *) calloc(fio->nelms*mult*fio->frame_size,sizeof(double));
          
          for(i=0;i<fio->nelms*mult*fio->frame_size;i++){
            if(fio->format[1] == 'B') tbuf[i] = (double)(fio->data_ptr.byte_ptr[i]);
            else if(fio->format[1] == 'I') tbuf[i] = (double)(fio->data_ptr.sint_ptr[i]);
            else if(fio->format[1] == 'L') tbuf[i] = (double)(fio->data_ptr.int_ptr[i]);
            else if(fio->format[1] == 'F') tbuf[i] = (double)(fio->data_ptr.float_ptr[i]);
          }
          
          memcpy(fio->data_ptr.void_ptr,tbuf,fio->nelms*mult*sizeof(double)*fio->frame_size);
          
          free(tbuf);
        }
      }
    //}
  }
  
  return nread;
}
  

void fio_alloc_buf(FILEIO_T *fio, size_t size){
  int mult;
  size_t t_size = (fio->framed == 0 ? size:size*(fio->frame_size*fio->bpe));
  
  if(fio != NULL){
    if(fio->format[0] == 'C') mult=2;
    else mult=1;
  
    // buf_type member represents the promoted type. This is set to the file format
    // type on file open but can be changed by a user to force a type change. All
    // changes happen in place so we need to make sure we have enough space to handle
    // the promoted type.
    if(fio->buf_type == 'B') fio->data_ptr.void_ptr = calloc(t_size,sizeof(char)*mult);
    else if(fio->buf_type == 'I') fio->data_ptr.void_ptr = calloc(t_size,sizeof(int16_t)*mult);
    else if(fio->buf_type == 'L') fio->data_ptr.void_ptr = calloc(t_size,sizeof(int32_t)*mult);
    else if(fio->buf_type == 'F') fio->data_ptr.void_ptr = calloc(t_size,sizeof(float)*mult);
    else if(fio->buf_type == 'D') fio->data_ptr.void_ptr = calloc(t_size,sizeof(double)*mult);
  }
  return;
}

void fio_free_buf(FILEIO_T *fio){
  if(fio != NULL){
    if(fio->data_ptr.void_ptr != NULL) free(fio->data_ptr.void_ptr);
  }
  return;
}

void fio_close(FILEIO_T *fio){
  if(fio != NULL) {
    if(fio->name != NULL) free(fio->name);
    if(fio->bhdr != NULL) free(fio->bhdr);
    if(fio->dhdr != NULL) free(fio->dhdr);
    
    fio_free_buf(fio);
    
    if(fio->fstream != NULL) {
      if(fclose(fio->fstream) != 0) perror("FCLOSE: ");
    }
    free(fio);
  }
  return;
}