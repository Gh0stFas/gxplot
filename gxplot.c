#include <plot.h>

void print_extra_help(char *opt){
  int i,j,len;
  if(strcmp(opt,"color") == 0){
    fprintf(stderr, "\nThere is a different default color palette depending\n");
    fprintf(stderr, "on the inversion of the background and foreground.\n\n");
    fprintf(stderr, "Available colors are:\n");
    len = 0;
    for(i=0;i<MAX_COLOR;i++) if(strlen(def_color[i]) > len) len = strlen(def_color[i]);
    
    fprintf(stderr, " norm");
    for(i=0;i<len+5;i++) fprintf(stderr," ");
    fprintf(stderr, " invert\n ");
    
    for(i=0;i<len+5;i++) fprintf(stderr, "-");
    fprintf(stderr, "     ");
    for(i=0;i<len+5;i++) fprintf(stderr, "-");
    fprintf(stderr, "\n");
    
    for(i=0;i<MAX_COLOR;i++){
      if(i+1 < 10) fprintf(stderr, "  %d -  %s",(i+1),def_color[i]);
      else fprintf(stderr, "  %d - %s",(i+1),def_color[i]);
      for(j=0;j<(len+5)-strlen(def_color[i]);j++) fprintf(stderr, " ");
      fprintf(stderr, "%s\n",def_colori[i]);
    }
  }
  else if(strcmp(opt,"symbol") == 0){
    fprintf(stderr, "\nAvailable symbols are:\n");
    for(i=0;i<MAX_SYMBOL;i++){
      fprintf(stderr, "  %d - %s\n",(i+1),symbol_name[i]);
    }
  }
  else if(strcmp(opt,"mode") == 0){
    fprintf(stderr, "\nAvailable modes are:\n");
    for(i=0;i<MAX_CX_MODE;i++){
      fprintf(stderr, "  %d - %s\n",(i+1),cx_mode_name[i]);
    }
  } 
  else if(strcmp(opt,"yscale") == 0){
    fprintf(stderr, "\nAvailable modes are:\n");
    fprintf(stderr, "\t1 = Use the entire buffer to determine the y min/max values\n");
    fprintf(stderr, "\t2 = Use the first %ld samples of the buffer to determine the y min/max values\n",MAX_READ);
  }  
  else if(strcmp(opt,"xscale") == 0){
    fprintf(stderr, "\nAvailable modes are:\n");
    fprintf(stderr, "\t1 = Use the entire buffer to determine the x min/max values\n");
    fprintf(stderr, "\t2 = Use the first %ld samples of the buffer to determine the x min/max values\n",MAX_READ);
    fprintf(stderr, "NOTE - This option only means anything when starting up in I/R mode\n");
  }   
  else if(strcmp(opt,"file") == 0){
    fprintf(stderr, "\nThere are 2 file formats currently supported, BLUE and Detached. If you don't know what BLUE is\n");
    fprintf(stderr, "then you don't have to worry about that. The Detached header format is a simple text file which\n");
    fprintf(stderr, "defines the data you want to look at. An example is as follows:\n\n");    
    fprintf(stderr, "\t##########################################\n");
    fprintf(stderr, "\t#\n");
    fprintf(stderr, "\t#     Test Detached File Header\n");
    fprintf(stderr, "\t#\n");
    fprintf(stderr, "\t##########################################\n\n");
    fprintf(stderr, "\t# Data file you wish to plot\n");
    fprintf(stderr, "\tfile=/data/myfile.dat\n\n");
    fprintf(stderr, "\t# File type. There are only 2 types currently support\n");
    fprintf(stderr, "\t# type 1000 = a flat data file such as time-series data\n");
    fprintf(stderr, "\t# type 2000 = framed data such as frames out of an FFt routine\n");
    fprintf(stderr, "\ttype=1000\n\n");
    fprintf(stderr, "\t# Data mode/format. The following mode/formats are supported\n");
    fprintf(stderr, "\t# Modes:\n");
    fprintf(stderr, "\t#   S = Scalar or Real\n");
    fprintf(stderr, "\t#   C = Complex\n");
    fprintf(stderr, "\t# Formats:\n");
    fprintf(stderr, "\t#   B = Byte\n");
    fprintf(stderr, "\t#   I = 2 Byte Integer\n");
    fprintf(stderr, "\t#   L = 4 Byte Integer\n");
    fprintf(stderr, "\t#   F = 4 Byte Float\n");
    fprintf(stderr, "\t#   D = 8 Byte Float\n");
    fprintf(stderr, "\tformat=CF\n\n");  
    fprintf(stderr, "\t# Abscissa value of the first sample\n");
    fprintf(stderr, "\txstart = 0.0\n\n");
    fprintf(stderr, "\t# Abscissa delta between subsequent samples.\n");
    fprintf(stderr, "\t# The following value represents a sample rate of 1562500 sps\n");
    fprintf(stderr, "\txdelta = 6.4e-7\n\n");
    fprintf(stderr, "The ordering of these fields is not important. Comments are preceded with a # and ignored by\n");
    fprintf(stderr, "the parser along with white space. Certain values can be represented as scientific notation\n");
    fprintf(stderr, "such as xstart, xdelta, ystart and ydelta. Below is an example of framed data such as the output\n");
    fprintf(stderr, "of an FFT routine.\n\n");
    fprintf(stderr, "\t##########################################\n");
    fprintf(stderr, "\t#\n");
    fprintf(stderr, "\t#     Test Detached File Header\n");
    fprintf(stderr, "\t#\n");
    fprintf(stderr, "\t#     ***The input file is the output of an\n");
    fprintf(stderr, "\t#     FFT routine using a 32K FFT with 0.5\n");
    fprintf(stderr, "\t#     overlap on a time-series input who's\n");
    fprintf(stderr, "\t#     sample rate was 1562500 sps\n");
    fprintf(stderr, "\t#\n");
    fprintf(stderr, "\t##########################################\n");
    fprintf(stderr, "\t# Data file you wish to plot\n"); 
    fprintf(stderr, "\tfile=/data/myfft.dat\n\n");
    fprintf(stderr, "\t# File type. There are only 2 types currently support\n");
    fprintf(stderr, "\t# type 1000 = a flat data file such as time-series data\n");
    fprintf(stderr, "\t# type 2000 = framed data such as frames out of an FFt routine\n");
    fprintf(stderr, "\ttype=2000\n\n");
    fprintf(stderr, "\t# Data mode/format. The following mode/formats are supported\n");
    fprintf(stderr, "\t# Modes:\n");
    fprintf(stderr, "\t#   S = Scalar or Real\n");
    fprintf(stderr, "\t#   C = Complex\n");
    fprintf(stderr, "\t# Formats:\n");
    fprintf(stderr, "\t#   B = Byte\n");
    fprintf(stderr, "\t#   I = 2 Byte Integer\n");
    fprintf(stderr, "\t#   L = 4 Byte Integer\n");
    fprintf(stderr, "\t#   F = 4 Byte Float\n");
    fprintf(stderr, "\t#   D = 8 Byte Float\n");
    fprintf(stderr, "\tformat=CF\n\n");  
    fprintf(stderr, "\t# Abscissa value of the first sample \n");
    fprintf(stderr, "\txstart = -1.041603088378906E+05\n\n");
    fprintf(stderr, "\t# Abscissa delta between subsequent samples.\n");
    fprintf(stderr, "\txdelta = 6.357828776041667E+00\n\n");
    fprintf(stderr, "\t# The value of the first frame \n");
    fprintf(stderr, "\tystart = 7.864320000000000E-02\n\n");
    fprintf(stderr, "\t# The delta between subsequent frames\n");
    fprintf(stderr, "\tydelta = 7.864320000000000E-02\n\n");
    fprintf(stderr, "\t# Number of samples per frame.\n");
    fprintf(stderr, "\tframe_length = 32768\n\n");
    fprintf(stderr, "If multiple files are given they are plotted in different colors, one on\n");
    fprintf(stderr, "top of the other. You can plot up to %d files at a time. The location of\n",MAX_FILES);
    fprintf(stderr, "the file on the command line is important only in that it defines the layer.\n");
    fprintf(stderr, "So the first file is the first layer, second file the second layer,etc.\n");
    fprintf(stderr, "This comes into play when applying the layer modifiers which follow the -l or\n");
    fprintf(stderr, "--layer option.\n\n");
    fprintf(stderr, "Mixed mode plotting is supported, so you can plot real data with complex data\n");
    fprintf(stderr, "types. This will however limit some of the plotting modes available.\n");
  } else fprintf(stderr, "\nNo extra help available for %s\n",opt);
  fprintf(stderr, "\n");
  return;
}

void print_help(){
  fprintf(stderr, "\nUsage:\n \
  gxplot [OPTION]...\n \
  \nArguments are mandatory for those options shown.\n \
  -f, --file=FILE     input file to plot. See --help=file for accepted file types. \n \
                        The location of this parameter on the CL\n \
                        is only important in that it defines the layer structure.\n \
                        Meaning the first occurence of this option is for layer 1,\n \
                        the second layer 2, etc.\n \
  -s, --start=INDEX   start point into the file. Default values are\n \
                        given as an index into the file. This can be\n \
                        modified to be the abscissa value instead.\n \
  -e, --end=INDEX     end point into the file. Default valuse are\n \
                        given as an index into the file. This can be\n \
                        modified to be the abscissa value instead\n \
  -a, --abscissa      force the START and END values to be interpreted as\n \
                        ABSCISSA values rather than INDEX. This is only valid\n \
                        for non-framed data sets.\n \
  -m, --mode=M        mode to start up in. Default is Real. See --help=mode\n \
                        to view the modes available.\n \
      --yscale=MODE   set the y-axis scaling mode. See --help=yscale for a list\n \
                        of modes available.\n \
      --xscale=MODE   set the x-axis scaling mode. Only applicable when in I/R mode.\n \
                        See --help=xscale for a list of modes available.\n \
      --ymin=VAL      override the autoscale with a YMIN value of your\n \
                        own. This only lasts in the current mode. Once the\n \
                        mode has changed Full-Auto will take over\n \
      --ymax=VAL      override the autoscale with a YMAX value of your\n \
                        own This only lasts in the current mode. Once the\n \
                        mode has changed Full-Auto will take over\n \
  -h, --help=OPT      display this help and exit. If the long option is used\n \
                       and supplied with an OPT arg, then special help is displayed\n \
                       for that particular OPT if available. For instance, if you\n \
                       want to view all the codes for the various colors available\n \
                       simply run:\n\n \
                         gxplot --help=color\n\n \
  -i, --invert        invert the background and foreground colors\n \
      --no-reduce     turn off the line reduction algorithm. This option\n \
                        was intended for debug purposes. Turning it on can\n \
                        drastically reduce the performance of the plotter\n \
                        in dense applications.\n \
  -l, --layer=LAYER   layer for which any layer options which follow this\n \
                        will pertain to.\n \
  \nLayer OPTIONS:\n \
      --color=COLOR   color value corresponding to a predifined list of colors. See\n \
                        --help=color for a list of available colors\n \
      --radius=RAD    radius of the symbol requested. If a radius of 1 is given\n \
                        then a single pixel will be used to represent each value\n \
      --symbol=SYM    if this option is invoked then lines will be turned off\n \
                        and each value in the file will be represented by the symbol\n \
                        given rather than connecting lines. See --help=symbol for\n \
                        a list of available symbols\n \
  \nHot Keys:\n \
    The following keys can be used while the plotter is up to modify its behavior\n \
    a     - Cycles through the display modes of the pointer location values. Default is\n \
            abscissa. The follow on modes are index, 1 / abscissa and dydx.\n \
    n,p   - Cycles forwards and backwards respectively through the plotting modes.\n \
            Default is Real followed by Imaginary, Imag vs. Real, 10*log10, 20*log20,\n \
            Magnitude and Phase\n \
    c     - Toggles the grid length crosshairs which follow the pointer.\n \
    enter - Pressing the enter key on the plot will drop it.\n\n");
  return;
}

int parse_cl(PLOT_T *pt,int argc, char **argv){
  int c,option_index;
  char *fname;
  int fname_s,fname_e,i;
  double start,end,ymin,ymax,tdbl;
  int got_xr=0,got_yr=0;
  int layer_opt = -1,tc;
  int rtn=0;
  
  static struct option long_options[] =
             {
               {"help", optional_argument, 0, 'h'},
               {"invert", no_argument, 0, 'i'},
               {"abscissa", no_argument, 0, 'a'},
               
               {"start", required_argument, 0, 's'},
               {"end", required_argument, 0, 'e'},
               {"mode", required_argument, 0, 'm'},
               {"file", required_argument, 0, 'f'},
               
               {"debug", optional_argument, 0, 0},
               {"no-reduce", no_argument, 0, 0},
               {"all", no_argument, 0, 0},
               {"ymin", required_argument, 0, 0},
               {"ymax", required_argument, 0, 0},
               {"yscale", required_argument, 0, 0},
               {"xscale", required_argument, 0, 0},
               
               // Layer option (arguments should be 1 based)
               {"layer", required_argument, 0, 'l'},
               
               // Layer modifier options (arguments should be 1 based)
               {"color", required_argument, 0, 0},
               {"symbol", required_argument, 0, 0},
               {"radius", required_argument, 0, 0},
               
               {0,0,0,0}
             };
             
  // Don't let getopt report errors
  opterr = 0;
  
  // Default the start, end, ymin and ymax values
  start = pt->zoom_info[0].xmin;
  end = pt->zoom_info[0].xmax;
  ymin = pt->zoom_info[0].ymin;
  ymax = pt->zoom_info[0].ymax;
  while((c = getopt_long (argc, argv, "if:s:e:m:l:h::ad::",long_options, &option_index)) != -1){
    // This is the file input argument. The location of this parameter on the command line
    // is important only in that it defines the layer structure. So the first file on the CL
    // is layer 1, the second layer 2, etc.
    if(c == 'f'){
      // Attempt to parse the files
      fname = strtok(optarg,",");
      if(fname != NULL){
        pt->flist[pt->nfiles] = fio_open(fname,1);
      
        if(pt->flist[pt->nfiles] != NULL){
          fprintf(stdout,"file%d=%s\n",pt->nfiles,pt->flist[pt->nfiles]->name);          
          pt->nfiles++;
        }
        while((fname = strtok(NULL,",")) != NULL && pt->nfiles < MAX_FILES){
          pt->flist[pt->nfiles] = fio_open(fname,1);
        
          if(pt->flist[pt->nfiles] != NULL){
            fprintf(stdout,"file%d=%s\n",pt->nfiles,pt->flist[pt->nfiles]->name);
            pt->nfiles++;
          }
        }
      }
    }
    
    // Help
    else if(c == 'h'){
      // Print the help page
      if(optarg == 0) print_help();
      else print_extra_help(optarg);
      rtn=-1;
      break;
    }
    
    // Flag the X start and end values are in abscissa
    else if(c == 'a'){
      pt->udef_xunits=1;
      fprintf(stdout, "%s: expecting X start and end arguments to be ABSCISSA values.\n",pt->name);
    }
    
    // Sets the initial mode the data is to be displayed in.
    else if(c == 'm'){
      char *eptr=NULL;
      tc = strtol(optarg, &eptr, 10);
      
      if(tc == 0 && eptr != NULL){
        fprintf(stderr, "%s: Invalid value '%s' passed to --mode\n",pt->name,optarg);
      } else {        
        if(tc > MAX_CX_MODE || tc <= 0){
          fprintf(stderr, "%s: Invalid mode %d given\n",pt->name, tc);
        }
        else {
          pt->cx_mode = tc-1;
        
          fprintf(stdout, "%s: Mode set to %s\n",pt->name, cx_mode_name[pt->cx_mode]);
        }
      }
    }
    
    // This is more a flag used to tell the command line parser that any layer
    // options following this argument pertain to the specified layer.
    else if(c == 'l'){
      // The next layer options will pertain to the layer number
      // given here
      char *eptr=NULL;
      tc = strtol(optarg, &eptr, 10);
      
      if(tc == 0 && eptr != NULL){
        fprintf(stderr, "%s: Invalid value '%s' passed to --layer\n",pt->name,optarg);
      } else {
        if(tc <= 0 || tc > MAX_FILES){
          fprintf(stderr, "%s: Invalid layer %d given...bounds are 1 to %d\n",pt->name,tc,MAX_FILES);
          layer_opt = -1;
        } else layer_opt = tc-1;
      }
    }
    else if(c == 'i'){
      fprintf(stdout, "%s: BG/FG inversion set\n",pt->name);
        
      // set the flag in the plotter
      pt->invert_bg_fg = 1;
      
      // Set the new color palette
      for(i=0;i<MAX_COLOR;i++)  pt->color_palette[i] = def_colori[i];
    }
    
    // The user defined start and end is suggestive. These values will
    // be stored to be used in the initial plotting routine. If either
    // values exceeds what is available in the file, then the values
    // will be truncated or completely moved and a warning printed.
    else if(c == 's'){
      char *eptr=NULL;
      pt->u_start = strtod(optarg, &eptr);
      
      if(eptr != NULL && strcmp(optarg,eptr) == 0){
        fprintf(stderr, "%s: Invalid value '%s' passed to --start\n",pt->name,optarg);
      } else {
        pt->udef_flags = pt->udef_flags | USER_DEF_START;
      }
    }
    else if(c == 'e'){
      char *eptr=NULL;
      pt->u_end = strtod(optarg, &eptr);
      
      if(eptr != NULL && strcmp(optarg,eptr) == 0){
        fprintf(stderr, "%s: Invalid value '%s' passed to --end\n",pt->name,optarg);
      } else {
        pt->udef_flags = pt->udef_flags | USER_DEF_END;
      }
    }
    
    // This block handles those options which do not have a short
    // flag and must precede with --
    else if(c == 0){
      // This is a flag used to force the line plotting routine to plot
      // all lines available rather than attempting to use the lines
      // reduction routine. This should really only be used for test
      // purposes as not reducing the lines can significantly impact
      // the performance when a lot of values are being plotted.
      if(strncmp(long_options[option_index].name,"no-reduce",9) == 0) {
        pt->reduce_lines = 0;
        fprintf(stdout, "%s: Line reduction code turned off\n",pt->name);
      }
      
      // Turn debug on
      else if(strncmp(long_options[option_index].name,"debug",5) == 0){
        if(optarg == 0) pt->debug=1;
        else {
          char *eptr=NULL;
          tc = strtol(optarg, &eptr, 10);
          
          if(eptr != NULL && strcmp(optarg,eptr) == 0){
            fprintf(stderr, "%s: Invalid value '%s' passed to --debug\n",pt->name,optarg);
          } else {
            if(tc > 0 && tc <= MAX_DEBUG){
              pt->debug=tc;
              fprintf(stdout, "%s: DEBUG mode set to %d\n",pt->name,pt->debug);
            } else fprintf(stderr, "%s: Invalid DEBUG mode %d, defaulting to %d\n",pt->name,tc,pt->debug);
          }
        }
      }
      
      else if(strncmp(long_options[option_index].name,"all",3) == 0){
        pt->read_all=1;
      }
      
      // Set the y scale mode
      else if(strncmp(long_options[option_index].name,"yscale",5) == 0){
        char *eptr=NULL;
        tc = strtol(optarg, &eptr, 10);
          
        if(eptr != NULL && strcmp(optarg,eptr) == 0){
          fprintf(stderr, "%s: Invalid value '%s' passed to --yscale\n",pt->name,optarg);
        } else {
          if(tc > 0 && tc <= MAX_SCALE_MODE){
            pt->yscale_mode = tc-1; // Command line input is 1 based
            fprintf(stdout, "%s: Y scale mode set to %s\n",pt->name,scale_mode_name[pt->yscale_mode]);
          } else fprintf(stderr, "%s: Invalid Y scale mode %d, defaulting to %s\n",pt->name,tc,scale_mode_name[pt->yscale_mode]);
        }
      }
      
      // Set the x scale mode
      else if(strncmp(long_options[option_index].name,"xscale",5) == 0){
        char *eptr=NULL;
        tc = strtol(optarg, &eptr, 10);
          
        if(eptr != NULL && strcmp(optarg,eptr) == 0){
          fprintf(stderr, "%s: Invalid value '%s' passed to --xscale\n",pt->name,optarg);
        } else {          
          if(tc > 0 && tc <= MAX_SCALE_MODE){
            pt->xscale_mode = tc-1; // Command line input is 1 based
            fprintf(stdout, "%s: X scale mode set to %s\n",pt->name,scale_mode_name[pt->xscale_mode]);
          } else fprintf(stderr, "%s: Invalid X scale mode %d, defaulting to %s\n",pt->name,tc,scale_mode_name[pt->xscale_mode]);
        }
      }
      
      // The user defined ymin and ymax, unlike the start and end, are
      // not suggestive and will be taken in place of the autoscaling
      // done by default
      else if(strncmp(long_options[option_index].name,"ymin",4) == 0) {
        char *eptr=NULL;
        pt->u_ymin = strtod(optarg, &eptr);
      
        if(eptr != NULL && strcmp(optarg,eptr) == 0){
          fprintf(stderr, "%s: Invalid value '%s' passed to --ymin\n",pt->name,optarg);
        } else {
          pt->udef_flags = pt->udef_flags | USER_DEF_YMIN;
        }
      }
      else if(strncmp(long_options[option_index].name,"ymax",4) == 0) {
        char *eptr=NULL;
        pt->u_ymax = strtod(optarg, &eptr);
      
        if(eptr != NULL && strcmp(optarg,eptr) == 0){
          fprintf(stderr, "%s: Invalid value '%s' passed to --ymax\n",pt->name,optarg);
        } else {
          pt->udef_flags = pt->udef_flags | USER_DEF_YMAX;
        }
      }
      
      // The color is a value which should match one of those
      // stored in the def_color list found in plot.h. In the future
      // this argument will be expanded to allow color names.
      else if(strncmp(long_options[option_index].name,"color",5) == 0){
        if(layer_opt >= 0){
          char *eptr=NULL;
          tc = strtol(optarg, &eptr, 10);
          
          if(eptr != NULL && strcmp(optarg,eptr) == 0){
            fprintf(stderr, "%s: Invalid value '%s' passed to --color\n",pt->name,optarg);
          } else {          
            if(tc > 0 && tc <= MAX_COLOR){
              pt->layer_info[layer_opt].layer_cval = tc-1; // Command line input is 1 based
              fprintf(stdout, "%s: Layer %d color set to %s\n",pt->name,layer_opt,def_color[pt->layer_info[layer_opt].layer_cval]);
            } else fprintf(stderr, "%s: Invalid color %d for layer %d given...bounds are 1 to %d\n",pt->name,tc,layer_opt,MAX_FILES);
          }
        }
        else fprintf(stderr, "%s: Layer option must precede the layer modification option --color, ignoring\n",pt->name);
      }
      
      // Used in conjunction with the -l (layer) option set. This modifies the specified
      // layer to use a valid symbol for each point rather than connecting lines.
      else if(strncmp(long_options[option_index].name,"symbol",6) == 0){
        if(layer_opt >= 0){
          char *eptr=NULL;
          tc = strtol(optarg, &eptr, 10);
          
          if(eptr != NULL && strcmp(optarg,eptr) == 0){
            fprintf(stderr, "%s: Invalid value '%s' passed to --symbol\n",pt->name,optarg);
          } else {          
            if(tc > 0 || tc <= MAX_SYMBOL){
              pt->layer_info[layer_opt].symbol = tc-1; // Command line input is 1 based
              pt->layer_info[layer_opt].line=-1; // Disable lines
            
              fprintf(stdout, "%s: Layer %d symbol set to %s\n",pt->name,layer_opt,symbol_name[pt->layer_info[layer_opt].symbol]);
            } else fprintf(stderr, "%s: Invalid symbol %d for layer %d given...bounds are 1 to %d\n",pt->name,tc,layer_opt,MAX_SYMBOL);
          }
        }
        else fprintf(stderr, "%s: Layer option must precede the layer modification option --symbol, ignoring\n",pt->name);
      }
      
      // Used in conjunction with the --symbol option to define the radius of
      // the symbol requested. A radius of 1 with any symbol will result in
      // a single pixel being used to represent the value.
      else if(strncmp(long_options[option_index].name,"radius",5) == 0){
        if(layer_opt >= 0){
          char *eptr=NULL;
          tc = strtol(optarg, &eptr, 10);
          
          if(eptr != NULL && strcmp(optarg,eptr) == 0){
            fprintf(stderr, "%s: Invalid value '%s' passed to --radius\n",pt->name,optarg);
          } else {          
            if(tc <= 0) pt->layer_info[layer_opt].radius = 1;
            else pt->layer_info[layer_opt].radius = tc;
          
            fprintf(stdout, "%s: Layer %d radius set to %d\n",pt->name,layer_opt,pt->layer_info[layer_opt].radius);
          }
        }
        else fprintf(stderr, "%s: Layer option must precede the layer modification option --symbol, ignoring\n",pt->name);
      }
      else {
        fprintf(stdout, "%s: Unhandled option %s recieved with arg %s\n",pt->name,long_options[option_index].name,optarg);
      }
    }
    else if(c == '?'){
      // An unrecognized option was on the command line. Most applications
      // fall off here and drop execution while printing the usage statement.
      // We're going to print a warning and continue execution.
      fprintf(stderr, "%s: Unrecognized option found\n",pt->name);
      
      print_help();
      rtn=-1;
      break;
    }
    else {
      fprintf(stdout, "%s: Unhandled option %s recieved with arg %s\n",pt->name,long_options[option_index].name,optarg);
    }
  }
  
  // If no arguments were given then print the help. If no files were given then print an error. In
  // either case flag an exit
  if(argc <= 1){
    print_help();
    rtn=-1;
  }
  else if(pt->nfiles <= 0 && rtn != -1){
    fprintf(stderr, "%s: You must supply a file with either -f or --file\n",pt->name);
    rtn=-1;
  }
  
  return(rtn);
}

int main(int argc, char **argv){
  char *display_name=NULL;
  XEvent report;
  PLOT_T *plotter=NULL;
  
  int btn_start_x,btn_start_y,btn_pressed=-1,old_x=-1,old_y=-1,x_origin,y_origin,mouse_x,mouse_y,zbox_width,zbox_height;
  int x,y;
  x=y=0;
  Status status;
  
  int key_str_size=20;
  char key_str[key_str_size];
  KeySym keysym;
  int key_str_len;
  int entry_return=0;
  int ret_val=0;
  int panning=0;
  int zooming=0;
  int btn_x,btn_y;
  
  Window root_win,child_win;
  int root_x,root_y;
  unsigned int mask_return;
  
  // Allocate the plotter
  plotter = alloc_plot();
  
  // Set the name
  sprintf(plotter->name,"%s","GX-plot");
  
  // Parse the command line
  ret_val = parse_cl(plotter,argc,argv);
  
  if(ret_val != 0) {
    FreeDisp(plotter);
    return 1;
  }
  
  /* Connect to the X server */
  if((plotter->display=XOpenDisplay(display_name)) == NULL){
    fprintf(stderr, "%s: cannot connect to X server %s\n", plotter->name, XDisplayName(display_name));
    FreeDisp(plotter);
    return 1;
  }
  
  fprintf(stdout, "%s: Connected to X server version %d, revision %d\n",plotter->name,ProtocolVersion(plotter->display),ProtocolRevision(plotter->display));
  
  // Get/set the visual information
  get_visual(plotter);
  
  // Get/set the colormap information
  get_colormap(plotter);
  
  // Open the window
  open_win(plotter);
  
  // Create the GC's
  get_gc(plotter);
  
  // Load and set the font
  load_font(plotter);
  
  // Initial various elements of the plot structure (has to be after the call to the colormap generator)
  init_plot(plotter);
  
   /* Main event processing loop */
  while(1){
    XNextEvent(plotter->display,&report);
    
    if(report.type == Expose){
      if(plotter->debug==2) fprintf(stdout, "Expose event captured\n");
      
      if(report.xexpose.count != 0) continue;
      
      refresh_window(plotter);
    }
    else if(report.type == ConfigureNotify){
      if(plotter->debug==2) fprintf(stdout, "ConfigureNotify event captured\n");
      
      plotter->win_width = report.xconfigure.width;
      plotter->win_height = report.xconfigure.height;
    }
    else if(report.type == ButtonPress){
      if(plotter->debug==2) fprintf(stdout, "ButtonPress event captured\n");
      
      // Record the button and the x/y coords of the press 
      btn_pressed=report.xbutton.button;
      btn_x=report.xbutton.x;
      btn_y=report.xbutton.y;
      
      // This could be a ZOOM event or the user attempting to make a measurement
      if(btn_pressed == Button1){
        // For now we'll only update the mark and specs if we're not
        // in the pan region, the legend or a menu
        if((panning = on_pan_handle(plotter,btn_x,btn_y))){
          // Record where the click happened. This is only important
          // if the user has clicked on the PAN handle as this plus
          // the delta if the user moves the mouse will be used to
          // calculate the abscissa distance to move.
          plotter->mouse_panx=btn_x;
          plotter->mouse_pany=btn_y;
        }
        // Until we declare another variable which strictly holds the mouse click
        // then we're reliant on the mouse_xmrk and mouse_ymrk to tell us in the motionnotify
        // message, whether or not the click started in the legend area.
        else if(in_legend(plotter,report.xbutton.x,report.xbutton.y)){
          // If we're in the legend area, then do nothing
        }
        else {
          plotter->mouse_xmrk=btn_x;
          plotter->mouse_ymrk=btn_y;
        
          update_specs(plotter);
        }
      }
      else if(btn_pressed == Button2){
        // Middle mouse button
        plotter->mouse_grabx=btn_x;
      }
      else if(btn_pressed == Button4){
        if(plotter->debug==2) fprintf(stdout, "Mouse scroll up event captured\n");
      }
      else if(btn_pressed == Button5){
        if(plotter->debug==2) fprintf(stdout, "Mouse scroll down event captured\n");
      }
    }
    else if(report.type == ButtonRelease){
      if(plotter->debug==2) fprintf(stdout, "ButtonRelease event captured\n");
      
      // Shouldn't we just be able to use report.xbutton.button????
      if(btn_pressed == Button1){
        if(zooming){
          // Clear the last rectangle drawn for a zoom if one is available
          if(old_x > -1 && old_y > -1) {
            XDrawRectangle(plotter->display,plotter->win,plotter->gc_xor,x_origin,y_origin,zbox_width,zbox_height);
          
            // Let's go ahead and zoom
            zoom_npix(plotter,x_origin,y_origin,x_origin+zbox_width,y_origin+zbox_height);
          }
          old_x=old_y=-1;
          
          // Clear the zoom flag
          zooming=0;
        }
        else if(in_pan(plotter,report.xbutton.x,report.xbutton.y)){
          // Do something about the PAN
          // The reason we pass in the current x and y rather than
          // the pre-recorded panx and pany is this gives the user
          // a chance to negate the process and take the event back 
          // by simply moving the mouse off the pan before releasing
          // the mouse button
          pan_clicked(plotter,report.xbutton.x,report.xbutton.y);
        }
        else if(on_btn(plotter,report.xbutton.x,report.xbutton.y)){
          btn_clicked(plotter,report.xbutton.x,report.xbutton.y);
        }
        // If the button was released in the legend, and we started in
        // the legend then execute the callback
        else if(in_legend(plotter,report.xbutton.x,report.xbutton.y) &&
                in_legend(plotter,btn_x,btn_y)){
          legend_clicked(plotter,report.xbutton.x,report.xbutton.y);
        }
        
        if(panning) panning=0;
      }
      else if(btn_pressed == Button3){
        // If we right clicked within the grid then zoom out
        if(plotter->mouse_x >= plotter->grid_x-2 && plotter->mouse_x <= plotter->grid_x2+2 &&
           plotter->mouse_y >= plotter->grid_y-2 && plotter->mouse_y <= plotter->grid_y2+2){
          zoom_out(plotter);
        }
      }
      btn_pressed=-1;
    }
    else if(report.type == MotionNotify){
      if(plotter->debug==2) fprintf(stdout, "ButtonMotion event captured\n");
      
      // Bind this value to the window dimensions
      plotter->mouse_x=max(0,min(report.xbutton.x,plotter->win_width-1));
      plotter->mouse_y=max(0,min(report.xbutton.y,plotter->win_height-1));
      
      // If the user was holding down button 1 then this is a ZOOM event.
      if(btn_pressed == Button1){
        if(panning){
          while(XCheckTypedEvent(plotter->display,MotionNotify,&report));
          if(XQueryPointer(plotter->display,plotter->win,&root_win,&child_win,&root_x,&root_y,&(report.xbutton.x),&(report.xbutton.y),&mask_return)) pan_dragged(plotter,panning,report.xbutton.x,report.xbutton.y);
        }
        // As long as the zoom origin is not in the legend then initiate a zoom
        else if(!(on_btn(plotter,btn_x,btn_y)) && !(in_pan(plotter,btn_x,btn_y)) && !(in_legend(plotter,btn_x,btn_y))){
	  // Clear the old rectangle
	  if(old_x > -1 && old_y > -1) XDrawRectangle(plotter->display,plotter->win,plotter->gc_xor,x_origin,y_origin,zbox_width,zbox_height);
	
	  // Check direction and set boundaries
	  if(plotter->mouse_x>plotter->mouse_xmrk) { x_origin=plotter->mouse_xmrk; zbox_width=plotter->mouse_x-plotter->mouse_xmrk; }
	  else { x_origin = plotter->mouse_x; zbox_width = plotter->mouse_xmrk-plotter->mouse_x; }
	
	  if(plotter->mouse_y>plotter->mouse_ymrk) { y_origin=plotter->mouse_ymrk; zbox_height = plotter->mouse_y-plotter->mouse_ymrk; }
	  else { y_origin=plotter->mouse_y; zbox_height = plotter->mouse_ymrk-plotter->mouse_y; }
	
	  // Draw the new rectangle
          XDrawRectangle(plotter->display,plotter->win,plotter->gc_xor,x_origin,y_origin,zbox_width,zbox_height);
	
	  // Record the old x and y mouse position
	  old_x=x_origin;
	  old_y=y_origin;
          
          zooming=1;
        }
      }
      else if(btn_pressed == Button2){
        if(panning){
          // We're going to do nothing here for now
        }
        else {
          // The middle mouse button has been pressed and the user is moving
          // the mouse. Let's initiate a scroll of the data set.
          // Need to revisit this as the scrolling is causing the mouse to move off
          // position
          while(XCheckTypedEvent(plotter->display,MotionNotify,&report));
          if(XQueryPointer(plotter->display,plotter->win,&root_win,&child_win,&root_x,&root_y,&(report.xbutton.x),&(report.xbutton.y),&mask_return)) scroll_data(plotter,report.xbutton.x);
        }
      } else {
        // If no motion modifier button has been pressed then
        // let's check to see if we're in the bounds of the grid.
        // If we aren't then we'll change the cursor to the pointer
        if(in_grid(plotter,report.xbutton.x,report.xbutton.y)){
          // See if the users mouse is in the legend
          if(in_legend(plotter,report.xbutton.x,report.xbutton.y)){
            if(plotter->mouse_ingrid){
              set_cursor(plotter,POINTER_CURSOR);
              plotter->mouse_ingrid=0;
            }
          }
          else{
            if(!(plotter->mouse_ingrid)){
              set_cursor(plotter,CROSSHAIR_CURSOR);
              plotter->mouse_ingrid=1;
            }
          }
        }
        else {
          // We're not inside the grid anymore so change the cursor to
          // the pointer and set the ingrid flag to not.
          if(plotter->mouse_ingrid){
            set_cursor(plotter,POINTER_CURSOR);
            plotter->mouse_ingrid=0;
          }
        }
      }
            
      // Take a moment here to see if the mouse is over a button
      // If it is then let's go ahead and highlight it so the user
      // knows it can be pressed. Otherwise call un_hilite_btn which
      // will remove highlights from all buttons if they are lit up
      if(on_btn(plotter,report.xbutton.x,report.xbutton.y)) hilite_btn(plotter,report.xbutton.x,report.xbutton.y);
      else un_hilite_btns(plotter);
      
      if(btn_pressed != Button2) update_specs(plotter);
      if(plotter->crosshair_on) draw_crosshair(plotter);
    }
    else if(report.type == KeyRelease){
      if(report.xkey.window == plotter->win){
	// Get the keysym
        key_str_len = XLookupString(&(report.xkey),key_str,key_str_size,&keysym,NULL);
        key_str[key_str_len] = '\0';
        
        // If a carriage return was pressed then go ahead and exit
        if(keysym == XK_Return || keysym == XK_KP_Enter || keysym == XK_Linefeed) {
	  // For some reason, the application recieves the carriage return used to open
	  // the window from the command line. This captures that and ignores it. I'm
	  // not sure if this is expected as this behavior is not documented anywhere
	  if(entry_return) break;
	  else entry_return=1;
	}
	else if(keysym == XK_a){
          // Change the specs readout to the next in line
          incr_specs_mode(plotter);
        }
        else if(keysym == XK_c){
          // Toggle the crosshair 
          toggle_crosshair(plotter);
        }
        else if(keysym == XK_n){
          // Change the plotting mode to the next in line
          incr_cx_mode(plotter);
        }
        else if(keysym == XK_p){
          // Change the plotting mode to the next in line
          decr_cx_mode(plotter);
        }
      }
    }
    else if (report.type == ClientMessage && report.xclient.data.l[0] == plotter->wmDeleteMessage) break;
    else{
      if(report.type < nevents) if(plotter->debug==2) fprintf(stdout, "Unhandled event: %s\n",event_names[report.type]);
      else if(plotter->debug==2) fprintf(stdout, "Unhandled event: %d\n",report.type);
    }
  }
  
  FreeDisp(plotter);
  
  return 1;
}