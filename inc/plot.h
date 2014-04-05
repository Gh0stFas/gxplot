#ifndef GXPLOT_H
#define GXPLOT_H

#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#include <X11/Xcms.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include <bluefile.h>
#include <fileio.h>
#include <eventnames.h>


#define gxplot_bmp_width 40
#define gxplot_bmp_height 40

// Black on white bmp
static unsigned char gxplot_bw_bmp_bits[] = {
   0xff, 0xff, 0xff, 0xff, 0xff, 0xdf, 0xff, 0xbf, 0xff, 0xff, 0xdf, 0xff,
   0xff, 0xff, 0xff, 0xd9, 0xff, 0xff, 0xff, 0xff, 0xdb, 0xff, 0xbf, 0xff,
   0xff, 0xdb, 0xff, 0xff, 0xff, 0xff, 0xd1, 0xff, 0xff, 0xff, 0xff, 0xdf,
   0xff, 0xbf, 0xff, 0xff, 0xdf, 0xff, 0xff, 0xff, 0xff, 0xdf, 0x1f, 0xfe,
   0xff, 0xff, 0xdf, 0xef, 0xbd, 0xff, 0xff, 0xdf, 0xef, 0xfd, 0xff, 0xff,
   0xdf, 0xf7, 0xfb, 0xff, 0xff, 0xdf, 0xf7, 0xbb, 0xff, 0xff, 0xdf, 0xfb,
   0xf7, 0xff, 0xff, 0xdf, 0xfb, 0xf7, 0xff, 0xff, 0xdf, 0xfd, 0xaf, 0xff,
   0xff, 0xdf, 0xfd, 0xef, 0xff, 0xff, 0xdf, 0xfe, 0xdf, 0xff, 0xef, 0xd1,
   0xfe, 0x9f, 0xff, 0xef, 0x15, 0x6d, 0x9b, 0xb6, 0xc5, 0x55, 0xff, 0xbf,
   0xff, 0xf7, 0x91, 0xff, 0x3f, 0xff, 0xfb, 0x9f, 0xff, 0x7f, 0xff, 0xfb,
   0xdf, 0xff, 0xff, 0xfe, 0xfd, 0xdf, 0xff, 0xbf, 0xfe, 0xfd, 0xdf, 0xff,
   0xff, 0xfd, 0xfe, 0xdf, 0xff, 0xff, 0xfd, 0xfe, 0xdf, 0xff, 0xbf, 0x7b,
   0xff, 0xdf, 0xff, 0xff, 0x7b, 0xff, 0xdf, 0xff, 0xff, 0x87, 0xff, 0xdf,
   0xff, 0xbf, 0xff, 0xff, 0xd9, 0xff, 0xff, 0xff, 0xff, 0xdb, 0xff, 0xbf,
   0xff, 0xff, 0x1a, 0x00, 0x00, 0x00, 0xc0, 0xf1, 0xff, 0xff, 0xff, 0xff,
   0x9f, 0xff, 0x1f, 0xff, 0xe7, 0xbf, 0xff, 0x5f, 0xff, 0xef, 0xaf, 0xff,
   0x5f, 0xff, 0xef, 0x1f, 0xff, 0x1f, 0xff, 0xc7};
// White on black bmp
static unsigned char gxplot_wb_bmp_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x40, 0x00, 0x00, 0x20, 0x00,
   0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x40, 0x00,
   0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x2e, 0x00, 0x00, 0x00, 0x00, 0x20,
   0x00, 0x40, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x20, 0xe0, 0x01,
   0x00, 0x00, 0x20, 0x10, 0x42, 0x00, 0x00, 0x20, 0x10, 0x02, 0x00, 0x00,
   0x20, 0x08, 0x04, 0x00, 0x00, 0x20, 0x08, 0x44, 0x00, 0x00, 0x20, 0x04,
   0x08, 0x00, 0x00, 0x20, 0x04, 0x08, 0x00, 0x00, 0x20, 0x02, 0x50, 0x00,
   0x00, 0x20, 0x02, 0x10, 0x00, 0x00, 0x20, 0x01, 0x20, 0x00, 0x10, 0x2e,
   0x01, 0x60, 0x00, 0x10, 0xea, 0x92, 0x64, 0x49, 0x3a, 0xaa, 0x00, 0x40,
   0x00, 0x08, 0x6e, 0x00, 0xc0, 0x00, 0x04, 0x60, 0x00, 0x80, 0x00, 0x04,
   0x20, 0x00, 0x00, 0x01, 0x02, 0x20, 0x00, 0x40, 0x01, 0x02, 0x20, 0x00,
   0x00, 0x02, 0x01, 0x20, 0x00, 0x00, 0x02, 0x01, 0x20, 0x00, 0x40, 0x84,
   0x00, 0x20, 0x00, 0x00, 0x84, 0x00, 0x20, 0x00, 0x00, 0x78, 0x00, 0x20,
   0x00, 0x40, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x40,
   0x00, 0x00, 0xe5, 0xff, 0xff, 0xff, 0x3f, 0x0e, 0x00, 0x00, 0x00, 0x00,
   0x60, 0x00, 0xe0, 0x00, 0x18, 0x40, 0x00, 0xa0, 0x00, 0x10, 0x50, 0x00,
   0xa0, 0x00, 0x10, 0xe0, 0x00, 0xe0, 0x00, 0x38};
   
#define crosshair_width 16
#define crosshair_height 16
#define CROSSHAIR_CURSOR (1)
#define POINTER_CURSOR (2)
   
static unsigned char crosshair_box_sp_bits[] = {
   0x9f, 0x7c, 0x81, 0x40, 0x81, 0x40, 0x81, 0x40, 0x81, 0x40, 0x00, 0x00,
   0x00, 0x00, 0x9f, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x81, 0x40, 0x81, 0x40,
   0x81, 0x40, 0x81, 0x40, 0x81, 0x40, 0x1f, 0x7c};
   
static unsigned char crosshair_sp_bits[] = {
   0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x9f, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x80, 0x00,
   0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00};

#define MAX_CX_MODE (7)
#define DEF_CX_MODE (2)
#define MAX_SPECS (4)
#define MAX_VISUAL (6)

#define MIN(a,b) ( a < b ? a:b )
#define MAX(a,b) ( a > b ? a:b )
#define CMS_COLORMAP(DISP) ((Colormap *)(DISP->cms.clientCmaps))
#define MAX_ZOOM_LEVEL (32)
#define MAX_FILES (32)
#define MAX_FRAMES (16)
#define MAX_READ (32768L)

// Layer symbol names
#define MAX_SYMBOL (3)

// Default layer colors and their number values
#define MAX_COLOR (32)
   
// Scale modes
#define MAX_SCALE_MODE (2)
   
#define USER_DEF_START (0x00000001)
#define USER_DEF_END (0x00000002)
#define USER_DEF_YMIN (0x00000004)
#define USER_DEF_YMAX (0x00000008)
   
#define MAX_DEBUG (2)

#define NBTNS (1)
enum BTN {LAYER_OPTS};
   
static const char scale_mode_name[MAX_SCALE_MODE][15] = {"Full-Auto","Partial-Auto"};
static const char vis_name[MAX_VISUAL][15] = {"StaticGray", "GrayScale", "StaticColor", "PseudoColor", "TrueColor", "DirectColor"};
static const char cx_mode_name[MAX_CX_MODE][3] = {"Ma","Ph","Re","Im","IR","LO","L2"};
static const char specs_mode_name[MAX_SPECS][5] = {"absc","indx","1/ab","dydx"};
static const char symbol_name[MAX_SYMBOL][10] = {"Circle","Plus","X"};

// Colors used on a black background
static const char def_color[MAX_COLOR][15] = {"Green",            // 1
                                              "DeepPink",         // 2
                                              "Yellow",           // 3
                                              "Blue",             // 4
                                              "Red",              // 5
                                              "Orange",           // 6
                                              "Purple",           // 7
                                              "White",            // 8
                                              "Cyan",             // 9
                                              "Magenta",          // 10
                                              "Gray",             // 11
                                              "Sienna",           // 12
                                              "Maroon",           // 13
                                              "CornflowerBlue",   // 14
                                              "OliveDrab",        // 15
                                              "goldenrod",        // 16
                                              "SeaGreen",         // 17
                                              "DarkGreen",        // 18
                                              "Blue",             // 19
                                              "Red",              // 20
                                              "Green",            // 21
                                              "Pink",             // 22
                                              "Yellow",           // 23
                                              "Blue",             // 24
                                              "Red",              // 25
                                              "Green",            // 26
                                              "Pink",             // 27
                                              "Yellow",           // 28
                                              "Blue",             // 29
                                              "Red",              // 30
                                              "Green",            // 31
                                              "Pink"              // 32
                                              };
// Colors used on a white background
static const char def_colori[MAX_COLOR][15] = {"Blue",            // 1
                                              "Red",         // 2
                                              "DarkOrange",           // 3
                                              "Purple",             // 4
                                              "Magenta",              // 5
                                              "DarkSlateGray",           // 6
                                              "Black",           // 7
                                              "DarkGreen",            // 8
                                              "OliveDrab",             // 9
                                              "Violet",          // 10
                                              
                                              "DeepPink",             // 11
                                              "Sienna",           // 12
                                              "Maroon",           // 13
                                              "CornflowerBlue",   // 14
                                              "OliveDrab",        // 15
                                              "goldenrod",        // 16
                                              "SeaGreen",         // 17
                                              "DarkGreen",        // 18
                                              "Blue",             // 19
                                              "Red",              // 20
                                              "Green",            // 21
                                              "Pink",             // 22
                                              "Yellow",           // 23
                                              "Blue",             // 24
                                              "Red",              // 25
                                              "Green",            // 26
                                              "Pink",             // 27
                                              "Yellow",           // 28
                                              "Blue",             // 29
                                              "Red",              // 30
                                              "Green",            // 31
                                              "Pink"              // 32
                                              };

typedef struct{
  double xmin;
  double xmax;
  double ymin;
  double ymax;
  
  // Pixel to real world coordinates
  double x_p2r;
  double y_p2r;
} ZOOM_INFO_T;

typedef struct{
  int x1,x2,y1,y2;
} LINE_INFO_T;

typedef struct{
  unsigned int layer_cval;
  unsigned long layer_pixel;
  
  int line;
  int symbol;
  unsigned int radius;
} LAYER_INFO_T;

typedef struct{
  // Holds the absolute location of the
  // box in coordinates relative to the 
  // drawable
  int x1,x2,y1,y2;
  
  // Flag to tell if this box is a botton. If it is
  // then when the user puts the mouse over it or
  // if the user presses it, it should react to it
  // somehow
  int is_button;
  
  // Flag to tell if this box is highlighted already
  int hlit;
  
  // Flag to tell if this button is pressed
  int pressed;
  
  // Text for this button
  char text[26];
} BOX_INFO_T;

typedef struct{    
  // Width and height of the arrow
  unsigned int ar_width,ar_height;
  
  // Rectangular area covered by the PAN bar. This
  // includes the arrows and the drag region
  int x1,y1,x2,y2;
  
  // Rectangular area covered by the area which the 
  // PAN handle can be dragged across
  int drag_x1,drag_y1,drag_x2,drag_y2;
  
  // Rectangular area covered by the PAN handle
  //int hndl_x1,hndl_y1,hndl_x2,hndl_y2;
  BOX_INFO_T hndl;
  
  // Group of points to hold the arrow
  XPoint arrow[4][4];
  
  // Pixel per abscissa ratio
  double ppa;
} PAN_INFO_T;

typedef struct{
  Pixmap pmap;
  
  int x,y;
  unsigned int width,height;
  
  int x1,y1,x2,y2;
} PMAP_INFO_T;

typedef struct{
  Display *display;
  XVisualInfo *vis_info;
  char name[20];
  Colormap cmap;
  Window win;
  XFontStruct *font_info;
  int font_height;
  int font_width;
  GC gc_xor;
  GC gc_text;
  GC gc_data;
  GC gc_widget;
  int invert_bg_fg;
  unsigned long black;
  unsigned long white;  
  long light_grey,medium_grey,dark_grey;
  long h_light_grey,h_medium_grey,h_dark_grey;
  int win_width;
  int win_height;
  int border_width;
  int widget_spacing;
  
  Atom wmDeleteMessage;
  
  char xlabel[15];
  char ylabel[15];
  
  int nfiles;
  FILEIO_T *flist[MAX_FILES];
  
  // Holds the converted y-axis data sets for each file 
  double *ydata[MAX_FILES];
  
  // Color palette to use 
  const char *color_palette[MAX_COLOR];
  
  PMAP_INFO_T legend_pmap;
  
  // PAN start
  PAN_INFO_T xpan_bar;
  PAN_INFO_T ypan_bar;
  int pan_start;
  
  // Color definitions for the various layers
  LAYER_INFO_T layer_info[MAX_FILES];
  
  // Plotting area values
  int grid_x,grid_x2,grid_y,grid_y2;
  int grid_width,grid_height;
  
  // Holds the current mouse coordinates
  unsigned int mouse_x;
  unsigned int mouse_y;
  double mouse_xv;
  double mouse_yv;
  
  // Holds the position where the mouse was when
  // when button1 was pressed
  unsigned int mouse_xmrk;
  unsigned int mouse_ymrk;
  double mouse_xmrk_v;
  double mouse_ymrk_v;
  
  // Holds the position where the mouse was on the
  // PAN bar when button1 was pressed
  int mouse_panx;
  int mouse_pany;
  
  int mouse_grabx;
  
  // Flag to let you know if the mouse is in the grid 
  int mouse_ingrid;
  
  unsigned char cx_mode;
  unsigned char specs_mode;
  
  // Max and min abscissa values determined when openeing the files
  double abscissa_max, abscissa_min;
  
  // Max and min Y values
  double y_max, y_min;
  double init_ymin,init_ymax;
  
  // Max and min X values
  double x_max, x_min;
  
  // Flag to reduce plot lines
  int reduce_lines;
  
  // Holds user defined start, end, ymin and ymax values
  double u_start, u_end, u_ymin, u_ymax;
  unsigned int udef_flags;
  char udef_xunits; // Flag to let the plotter know what kind of units the start and end was given in (INDEX=0 or ABSCISSA=1)
  
  // Flag to let the setup processes know that this is the first
  // run. The only reason this is really here is in case the user
  // defined an initial start, end, ymin or ymax value as these
  // values are only honored at startup and as long as the user
  // maintains the initial cx mode started in. As soon as the user
  // changes modes, or scrolls these values will be ignored
  int initial_startup;
  
  // Debug flag
  int debug;
  
  // MAX Number of elements to read at a time
  long max_n2read;
  
  // Zoom information
  unsigned char zoom_level;
  ZOOM_INFO_T zoom_info[MAX_ZOOM_LEVEL];
  
  // X and Y scale modes
  int xscale_mode;
  int yscale_mode;
  
  double y_scale_factor;
  
  // Xdelta for the plot.
  double plot_xdelta;
  
  unsigned int framed_data;
  
  // Flag whether the crosshairs are on or not
  int crosshair_on;
  LINE_INFO_T crosshair_lines[4];
  
  // Push button for the layer options
  // Array holding all of the buttons pertaining to this plot instance
  BOX_INFO_T buttons[NBTNS];
  
  // Flag to tell whether the legend is up or not
  int legend_up;
  
  int read_all;
} PLOT_T;

// Below are a collection of routines associated with the plotter
// As the library progresses some of these will/should be split out
// into other libraries as some are specific to plotting while others
// are general X11 setup procedures. For now I'll attempt to seperate
// these with comments for the future.

// X11 routines which determine the display, visual, colormap, font loading and open
// the window(s).
void get_visual(PLOT_T *);
void get_colormap(PLOT_T *);
void open_win(PLOT_T *);
void get_gc(PLOT_T *);
void load_font(PLOT_T *);

// Routines for allocating the and destroying the PLOT_T structure
PLOT_T *alloc_plot();
void FreeDisp(PLOT_T *);
int init_plot_buffers(PLOT_T *);
void init_layer_colors(PLOT_T *);
void init_pan_bars(PLOT_T *);
void init_plot(PLOT_T *);

// Core routines used to draw the grid and plot the data
void draw_plot(PLOT_T *);
void draw_xpan(PLOT_T *);
void draw_xpan_drag(PLOT_T *);
void draw_ypan_drag(PLOT_T *);
void draw_ypan(PLOT_T *);
void plot_data(PLOT_T *);
void draw_symbols(PLOT_T *,XPoint *,int , int );
void draw_lines(PLOT_T *,XPoint *,int );
void convert_ydata(PLOT_T *, int );
void scroll_data(PLOT_T *,int );
void move_data(PLOT_T *, double );
void pan_clicked(PLOT_T *, int , int );
void xpan_clicked(PLOT_T *, int , int );
void ypan_clicked(PLOT_T *, int , int );
void pan_dragged(PLOT_T *, int, int , int );
void draw_bevel_box(PLOT_T *, BOX_INFO_T *);

// Supplemental routines used inside the core routines
void scale_xaxis(PLOT_T *);
void scale_yaxis(PLOT_T *,int,double);
void set_min_max(PLOT_T *,double *, double *, double *, double *, int );
void xy2rw(PLOT_T *,unsigned int , double *, unsigned int , double *);
void form_div(double , double , double *, double *, int );
void trim_label(char *, int *, int *);
void mult(double ,double ,double *);
void mouse_xy2rw(PLOT_T *);
void set_grid_dimensions(PLOT_T *);
int clipt(double , double , double *, double *);
void btn_clicked(PLOT_T *,int , int );
void legend_clicked(PLOT_T *,int , int );
void hilite_btn(PLOT_T *,int , int );
void un_hilite_btns(PLOT_T *);
void set_cursor(PLOT_T *,int);
void refresh_plot(PLOT_T *);
void refresh_window(PLOT_T *);
void toggle_crosshair(PLOT_T *);
void show_legend(PLOT_T *);
void hide_legend(PLOT_T *);

// Fucntions to quickly tell pointer location relative to parts of the plot_data
int in_pan(PLOT_T *,int , int );
int on_btn(PLOT_T *,int , int );
int in_grid(PLOT_T *, int , int );
int in_legend(PLOT_T *, int , int );
int on_pan_handle(PLOT_T *,int , int );

void zoom_nrw(PLOT_T *,double , double , double , double );
void zoom_npix(PLOT_T *,int , int , int , int );
void zoom_out(PLOT_T *);

void chnge_cx_mode(PLOT_T *, int );
void incr_cx_mode(PLOT_T *);
void decr_cx_mode(PLOT_T *);

void update_specs(PLOT_T *);
void incr_specs_mode(PLOT_T *);

#endif //GXPLOT_H
