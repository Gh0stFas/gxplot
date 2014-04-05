//gcc -c -fpic -I/home/jsegars bluefile.c
//gcc -shared -o libbluefile.so bluefile.o
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <detfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

FILE *det_open(char *fname,DET_HDR_T *hdr){
  FILE *tfile=NULL;
  
  // Attempt to open the file
  if((tfile = fopen(fname,"r")) == NULL){
    perror("FOPEN: ");
  } else {
    // Allocate the DET header
    if(hdr != NULL) fprintf(stdout, "WARN: Header appears to already be allocated. You may leak memory\n");
    
    if((hdr = (DET_HDR_T *) calloc(1,sizeof(DET_HDR_T))) == NULL){
      fprintf(stderr, "ERROR: Could not allocate space for the DET file header\n");
      
      det_close(tfile,hdr);
    } else {
      // Let's read in the header
      /*
      if(fread(hdr,sizeof(char),DET_HDR_SIZE,tfile) != DET_HDR_SIZE){
        if(feof(tfile) != 0){
          fprintf(stderr, "ERROR: End of file reached while attempting to read the header\n");
        }
        else if(ferror(tfile) != 0){
          fprintf(stderr, "ERROR: An error occurred while attempting to read the DET header\n");
        }
        
        // Regardless of what happened we need to close the stream
        det_close(tfile,hdr);
      } else {
        // Let's spot check the header values
        return(tfile);
      }
      */
    }
  }
  return NULL;
}

int is_comment(char *text){
  int i,got_text=0;
  
  
  for(i=0;i<strlen(text);i++){
    if(text[i] == '#') break;
    if(!(isspace(text[i]))) got_text=1;
  }
  
  if(got_text) return 0;
  else return 1;  
}

int det_read_hdr(FILE *fstream, DET_HDR_T *hdr){
  struct stat fstatus;
  
  char *tbuf;
  char *hbuf;
  char *thdr;
  char *rest;
  char *rest2;
  char *token;
  char *subtoken;
  int j,i,start,stop;
  int have;
  
  if(fstream != NULL && hdr != NULL){
    if(fstat(fileno(fstream),&fstatus) != 0){
      perror("fstat");
    }
    else {
      if(fstatus.st_size == 0){
        fprintf(stderr, "ERROR: Detached header is empty\n");
      }
      else {
        if((thdr = (char *) calloc(fstatus.st_size,sizeof(char))) == NULL){
          fprintf(stderr, "ERROR: Could not allocate memory for the temporary header buffer\n");
        }
        else {
          hdr->kvp=0;
          
          // Read in the entire header
          if(fread(thdr,sizeof(char),fstatus.st_size,fstream) != fstatus.st_size){
            perror("fread");
          }
          else {
            // Now we need to parse the header for all the important piece  
            hbuf = thdr;
            for (j = 1; ; j++, hbuf = NULL) {
              token = strtok_r(hbuf, "\n", &rest);
              if (token == NULL) break;
              //printf("%d: %s\n", j, token);
              
              // Check to see if this line was commented out
              if(is_comment(token)) continue;

              have=0;
              for (tbuf = token; ; tbuf = NULL) {
                subtoken = strtok_r(tbuf, "=", &rest2);
                //printf("%s %s '%s'\n",subtoken,token,rest2);
                if ((subtoken == NULL || strlen(rest2) <= 0) && have != 1) break;
                
                if(have == 1) {
                  // value
                  // Don't copy the spaces before or after
                  for(i=0;i<strlen(subtoken);i++){
                    if(!(isspace(subtoken[i]))) break;
                  }
                  start=i;
                  for(i=strlen(subtoken)-1;i>=0;i--){
                    if(!(isspace(subtoken[i]))) break;
                  }
                  stop=i;
                  
                  //printf("%d %d\n",start,stop);
                  snprintf(hdr->kv[hdr->kvp].value,(stop-start)+2,"%s",subtoken+start);
                  hdr->kvp++;
                  have=0;
                }
                else {
                  // key
                  // Don't copy the spaces before or after
                  for(i=0;i<strlen(subtoken);i++){
                    if(!(isspace(subtoken[i]))) break;
                  }
                  start=i;
                  for(i=strlen(subtoken)-1;i>=0;i--){
                    if(!(isspace(subtoken[i]))) break;
                  }
                  stop=i;
                  //printf("%d %d\n",start,stop);
                  snprintf(hdr->kv[hdr->kvp].key,(stop-start)+2,"%s",subtoken+start);
                  have=1;
                }
                //printf(" --> %s\n", subtoken);
              }
            }
            
            // Now we need to check for the important values and convert them  
            for(j=0;j<hdr->kvp;j++){
              //printf("'%s' = '%s'\n",hdr->kv[j].key,hdr->kv[j].value);
              
              if(strcmp(hdr->kv[j].key,"xdelta") == 0){
                char *eptr=NULL;
                hdr->xdelta = strtod(hdr->kv[j].value, &eptr);
                
                if(eptr != NULL && strcmp(hdr->kv[j].value,eptr) == 0){
                  fprintf(stderr, "Invalid XDELTA provided in the detached header, %s\n",hdr->kv[j].value);
                  return 0;
                }
              }
              else if(strcmp(hdr->kv[j].key,"xstart") == 0){
                char *eptr=NULL;
                hdr->xstart = strtod(hdr->kv[j].value, &eptr);
                
                if(eptr != NULL && strcmp(hdr->kv[j].value,eptr) == 0){
                  fprintf(stderr, "Invalid XSTART provided in the detached header, %s\n",hdr->kv[j].value);
                  return 0;
                }
              }
              else if(strcmp(hdr->kv[j].key,"ydelta") == 0){
                char *eptr=NULL;
                hdr->ydelta = strtod(hdr->kv[j].value, &eptr);
                
                if(eptr != NULL && strcmp(hdr->kv[j].value,eptr) == 0){
                  fprintf(stderr, "Invalid YDELTA provided in the detached header, %s\n",hdr->kv[j].value);
                  return 0;
                }
              }
              else if(strcmp(hdr->kv[j].key,"ystart") == 0){
                char *eptr=NULL;
                hdr->ystart = strtod(hdr->kv[j].value, &eptr);
                
                if(eptr != NULL && strcmp(hdr->kv[j].value,eptr) == 0){
                  fprintf(stderr, "Invalid YSTART provided in the detached header, %s\n",hdr->kv[j].value);
                  return 0;
                }
              }
              else if(strcmp(hdr->kv[j].key,"frame_length") == 0){
                char *eptr=NULL;
                hdr->subsize = strtol(hdr->kv[j].value, &eptr, 10);
                
                if(eptr != NULL && strcmp(hdr->kv[j].value,eptr) == 0){
                  fprintf(stderr, "Invalid FRAME_SIZE provided in the detached header, %s\n",hdr->kv[j].value);
                  return 0;
                }
              }
              else if(strcmp(hdr->kv[j].key,"file") == 0){
                if(strlen(hdr->kv[j].value) > MAX_FNAME){
                  fprintf(stderr, "Data file name length exceeds the max of %d\n",MAX_FNAME);
                  return 0;
                }
                else {
                  sprintf(hdr->fname,"%s",hdr->kv[j].value);
                  
                  struct stat tstat;
                  
                  if(stat(hdr->fname,&tstat) != 0){
                    perror("stat");
                    
                    fprintf(stderr, "Data file %s does not exist\n",hdr->kv[j].value);
                    return 0;
                  }
                  else hdr->data_size = tstat.st_size;
                }
              }
              else if(strcmp(hdr->kv[j].key,"type") == 0){
                char *eptr=NULL;
                hdr->type = strtol(hdr->kv[j].value, &eptr, 10);
                
                if((eptr != NULL && strcmp(hdr->kv[j].value,eptr) == 0) || (hdr->type != 1000 && hdr->type != 2000)){
                  fprintf(stderr, "Invalid TYPE provided in the detached header, %s\n",hdr->kv[j].value);
                  return 0;
                }
              }
              else if(strcmp(hdr->kv[j].key,"format") == 0){
                if(strlen(hdr->kv[j].value) > 2){
                  fprintf(stderr, "Invalid FORMAT provided in the detached header, %s\n",hdr->kv[j].value);
                  return 0;
                }
                else {
                  sprintf(hdr->format,"%s",hdr->kv[j].value);
                  
                  // Default the buff type to that of the input file header
                  hdr->buf_type = hdr->format[1];
      
                  // Calculate the bytes per element
                  hdr->bpe=1;
                  if(hdr->format[0] == 'C') hdr->bpe=2;
                  if(hdr->format[1] == 'I') hdr->bpe*=2;
                  else if(hdr->format[1] == 'L') hdr->bpe*=4;
                  else if(hdr->format[1] == 'F') hdr->bpe*=4;
                  else if(hdr->format[1] == 'D') hdr->bpe*=8;
                  else {
                    fprintf(stderr, "Invalid FORMAT provided in the detached header, %s\n",hdr->kv[j].value);
                    return 0;
                  }
                }
              }
            }
          }
          
          free(thdr);
          return 1;
        }
      }
    }
  } else fprintf(stderr, "ERROR: The input file stream or header is NULL\n");
  return 0;
}

void det_print(DET_HDR_T *hdr){
  // The char fields are not NULL terminated
  fprintf(stdout,"DET Header info:\n");
  //fprintf(stdout,"\tVersion    = %c%c%c%c\n",hdr->version[0],hdr->version[1],hdr->version[2],hdr->version[3]);
  //fprintf(stdout,"\tHdr rep    = %c%c%c%c\n",hdr->head_rep[0],hdr->head_rep[1],hdr->head_rep[2],hdr->head_rep[3]);
  fprintf(stdout,"\tData rep   = %c%c%c%c\n",hdr->data_rep[0],hdr->data_rep[1],hdr->data_rep[2],hdr->data_rep[3]);
  //fprintf(stdout,"\tDetached   = %d\n",hdr->detached);
  //fprintf(stdout,"\tProtected  = %d\n",hdr->prot);
  //fprintf(stdout,"\tPipe       = %d\n",hdr->pipe);
  //fprintf(stdout,"\tExt start  = %d\n",hdr->ext_start);
  //fprintf(stdout,"\tExt size   = %d\n",hdr->ext_size);
  fprintf(stdout,"\tData start = %g\n",hdr->data_start);
  fprintf(stdout,"\tData size  = %0.1f\n",hdr->data_size);
  fprintf(stdout,"\tType       = %d\n",hdr->type);
  fprintf(stdout,"\tFormat     = %c%c\n",hdr->format[0],hdr->format[1]);
  //fprintf(stdout,"\tFlag mask  = %d\n",hdr->flagmask);
  //fprintf(stdout,"\tTimecode   = %f\n",hdr->timecode);
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

long det_read(FILE *fstream,DET_HDR_T *hdr,void *buffer,long nelms){
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
      hdr->in_byte = (double)(ftell(fstream));
    }
  }
  
  return(nread);
}

int det_seek(FILE *file, DET_HDR_T *hdr, long nelms){
  int rtn=-1;
  //printf("seeking %ld elements\n",nelms);
  long bpe = (hdr->type == 2000 ? (long)hdr->bpe*(long)hdr->subsize:(long)hdr->bpe);
  
  if(file != NULL && hdr != NULL){
    if(hdr->in_byte + (double)(nelms*bpe) < 0.0){
      hdr->in_byte=0.0;
        
      fseek(file,0L,SEEK_SET);
    }
    else {
      if(hdr->in_byte + (double)(nelms*bpe) > hdr->data_size){
        nelms = (long)((hdr->data_size - hdr->in_byte) / (double)bpe);
        fprintf(stdout, "Seek request would place the buffer past the data, truncating to %ld\n",nelms);
      }
      if(fseek(file,(nelms * bpe),SEEK_CUR) != 0){
        perror("File seek failed:");
      } else {
        hdr->in_byte = (double)(ftell(file));
      }
    }
    rtn=0;
  }
  
  return(rtn);
}

void det_close(FILE *file,DET_HDR_T *hdr){
  if(file != NULL){
    if(fclose(file) != 0) perror("FCLOSE: ");
  }
  
  // Clear the header
  if(hdr != NULL) free(hdr);
}