#include <plot.h>

void get_visual(PLOT_T *pt){
  XVisualInfo vTemplate;
  XVisualInfo *visualList=NULL;
  int visualsMatched;
  int vbest=-1;
  XVisualInfo *xv_info=NULL;
  int i;
  
  vTemplate.screen = DefaultScreen(pt->display);
  vTemplate.depth = 8;
  
  visualList = XGetVisualInfo(pt->display, VisualScreenMask, &vTemplate, &visualsMatched);
  
  if(visualsMatched == 0) fprintf(stderr, "Could not get a valid visual\n");
  
  for(i=0;i<visualsMatched;i++){
    if(visualList[i].class < StaticColor) continue;
    if(visualList[i].class > TrueColor) continue;
    if(visualList[i].depth < 8) continue;
    

    if (vbest < 0) {    /* Initial selection ... */
      vbest = i;
      continue;
    }

    /* Try to upgrade the "best" visual class. */
    if (visualList[i].class == PseudoColor) { 
      if ( visualList[vbest].class == PseudoColor  &&  visualList[i].depth < visualList[vbest].depth )
        vbest = i;
    } else if ( visualList[i].depth < visualList[vbest].depth ) {
      /* ReadOnly visual (Static- or True-color) */
      vbest = i;
    } else if ( visualList[i].depth == visualList[vbest].depth  &&
                visualList[i].class == StaticColor &&
                visualList[vbest].class == TrueColor ) {
      vbest = i;  /* prefer StaticColor over TrueColor at same depth */
    }
  }
  
  if(vbest > -1){
    if(pt->debug){
      fprintf(stdout,"(DEBUG) Best visual match is:\n");
      fprintf(stdout,"\t(DEBUG) Visual ID=0x%lx\n",visualList[vbest].visualid);
      fprintf(stdout,"\t(DEBUG) Depth=%d\n",visualList[vbest].depth);
      fprintf(stdout,"\t(DEBUG) Class=%s\n",vis_name[visualList[vbest].class]);
      fprintf(stdout,"\t(DEBUG) CMAP size=%d\n",visualList[vbest].colormap_size);
      fprintf(stdout,"\t(DEBUG) BPRGB=%d\n",visualList[vbest].bits_per_rgb);
      fprintf(stdout,"\t(DEBUG) Default?=%s\n",(visualList[vbest].visualid == DefaultVisualOfScreen(DefaultScreenOfDisplay(pt->display))->visualid ? "YES":"NO"));
    }
    
    pt->vis_info = (XVisualInfo *) calloc(1,sizeof(XVisualInfo));
    memcpy(pt->vis_info,&(visualList[vbest]),sizeof(XVisualInfo));
  }
  
  XFree(visualList);
}

void get_colormap(PLOT_T *pt){
  XStandardColormap *def_map_info=NULL;
  XStandardColormap *new_map_info=NULL;
  int count=0;
  int i;
  Colormap Ctmp;
  XcmsCCC ccc;
  
  if(pt->vis_info->visualid == DefaultVisualOfScreen(DefaultScreenOfDisplay(pt->display))->visualid){
    pt->cmap = DefaultColormapOfScreen(DefaultScreenOfDisplay(pt->display));
  } else {
    if(XGetRGBColormaps(pt->display,RootWindow(pt->display, DefaultScreen(pt->display)), &def_map_info, &count, XA_RGB_DEFAULT_MAP)){
      /* XA_RGB_DEFAULT_MAP exists for this display, let's see if any of them match our visual ID */
      fprintf(stdout,"%d matche(s) for XA_RGB_DEFAULT_MAP\n",count);
      for(i=0;i<count;i++){
        if(def_map_info[i].visualid == pt->vis_info->visualid){
          pt->cmap = def_map_info[i].colormap;
          fprintf(stdout,"Great, found a default colormap 0x%lx for my visual ID\n",pt->cmap);
          fprintf(stdout,"\tColormap: 0x%lx\n",def_map_info[i].colormap);
          fprintf(stdout,"\tred_max: %lu\n",def_map_info[i].red_max);
          fprintf(stdout,"\tred_mult: %lu\n",def_map_info[i].red_mult);
          fprintf(stdout,"\tgreen_max: %lu\n",def_map_info[i].green_max);
          fprintf(stdout,"\tgreen_mult: %lu\n",def_map_info[i].green_max);
          fprintf(stdout,"\tblue_max: %lu\n",def_map_info[i].blue_max);
          fprintf(stdout,"\tblue_mult: %lu\n",def_map_info[i].blue_max);
          fprintf(stdout,"\tvisualid=0x%lx\n",def_map_info[i].visualid);
          fprintf(stdout,"\tkillid=0x%lx\n",def_map_info[i].killid);
          break;
        }
      }
    } else fprintf(stdout,"RGB_DEFAULT_MAP colormap property not set\n");
  
    if(pt->cmap==0){
      fprintf(stdout,"Colormap not created, creating it\n");
      /* We need to create it and store it permanently so that follow on
       * processes can access it without having to create it */
      Display *t_display=NULL;
      Window window;
      XColor color;
    
      t_display = XOpenDisplay(NULL);
    
      pt->cmap = XCreateColormap(t_display,DefaultRootWindow(t_display),pt->vis_info->visual,AllocNone);
      new_map_info = (XStandardColormap *) calloc( count+1 , sizeof(XStandardColormap) );
    
      new_map_info[0].colormap = pt->cmap;
      new_map_info[0].visualid = pt->vis_info->visualid;
    
      if(count > 0) memcpy(new_map_info+1,def_map_info,sizeof(XStandardColormap)*count);
    
      XSetRGBColormaps (t_display , DefaultRootWindow(t_display) , new_map_info , count+1 , XA_RGB_DEFAULT_MAP);
    
      if (pt->vis_info->class & 1) {
        fprintf(stdout,"Storing black and white\n");
      
        /* Store black, white cells for R/W Cmaps */
        unsigned long  planes[2],pix[2];
        XAllocColorCells ( t_display ,pt->cmap , False , planes , 0 , pix , 2 );
        color.flags = DoRed | DoGreen | DoBlue;
        color.pixel=pix[0];
        color.red=0;
        color.green=0;
        color.blue=0;
        XStoreColor (t_display , pt->cmap , &color);
      
        color.pixel=pix[1];
        color.red=0xFFFF;
        color.green=0xFFFF;
        color.blue=0xFFFF;
        XStoreColor (t_display , pt->cmap , &color);
      }
    
      XSetCloseDownMode(t_display , RetainPermanent);
      XCloseDisplay(t_display);
      free (new_map_info);
    
      fprintf(stdout,"Created colormap 0x%lx\n",pt->cmap);
    }
  
    XFree(def_map_info);
  
    Ctmp = XCreateColormap (pt->display, DefaultRootWindow(pt->display), pt->vis_info->visual , AllocNone);
    ccc = XcmsCCCOfColormap (pt->display , Ctmp);
    if (CMS_COLORMAP(pt->display) == 0) {
      fprintf(stdout,"ColormapCMS sneak failed! (zero pointer)\n");
    } else if (*CMS_COLORMAP(pt->display) != Ctmp) {
      fprintf(stdout,"ColormapCMS sneak failed! (CmapID didn't match)\n");
    } else {
      *CMS_COLORMAP(pt->display) = pt->cmap; /* overload with old colormapID */
    }
    XFreeColormap(pt->display, Ctmp);
  }
}

void open_win(PLOT_T *pt){
  Window win;
  XSetWindowAttributes setwinattr;
  XSizeHints size_hints;
  XWMHints wm_hints;
  XClassHint class_hints;
  unsigned int valuemask;
  Window parent=0;
  Status status;
  XColor screen_def,exact_def;
  unsigned int width,height,display_width,display_height;
  unsigned int x,y;
  XTextProperty windowName,iconName;
  x=y=0;
  char *name_ptr;
  Pixmap icon_pixmap;
  XIconSize *icon_list=NULL;
  int nicons;
  //attr Atom GDeleteWinAtom;
  //attr Atom GProtocolsAtom;
  
  if(pt->invert_bg_fg == 0) status = XAllocNamedColor(pt->display,pt->cmap,"Black",&screen_def,&exact_def);
  else status = XAllocNamedColor(pt->display,pt->cmap,"White",&screen_def,&exact_def);
  pt->black=screen_def.pixel;
  
  if(pt->invert_bg_fg == 0) status = XAllocNamedColor(pt->display,pt->cmap,"White",&screen_def,&exact_def);
  else status = XAllocNamedColor(pt->display,pt->cmap,"Black",&screen_def,&exact_def);
  pt->white=screen_def.pixel;
  
  // Set various window attributes
  valuemask = CWEventMask | CWBackPixel | CWBorderPixel | CWColormap;
  setwinattr.background_pixel = pt->black;
  setwinattr.border_pixel = pt->white;
  setwinattr.colormap = pt->cmap;
  setwinattr.event_mask =   KeyPressMask      | KeyReleaseMask
    | ButtonPressMask     | ButtonReleaseMask | PointerMotionMask
    | StructureNotifyMask | ExposureMask      | FocusChangeMask
    | EnterWindowMask;
  
  // Get some basic screen information
  display_width = DisplayWidth(pt->display,DefaultScreen(pt->display));
  display_height = DisplayHeight(pt->display,DefaultScreen(pt->display));
  
  pt->win_width = MIN(1024,display_width);
  pt->win_height = MIN(768,display_height);
  pt->border_width = 1;
    
  // Create the window
  pt->win = XCreateWindow (pt->display, RootWindow(pt->display,DefaultScreen(pt->display)), 
                           x, y, pt->win_width, pt->win_height, pt->border_width, pt->vis_info->depth,
                           InputOutput, pt->vis_info->visual, valuemask, &setwinattr);

  //GDeleteWinAtom = XInternAtom(display, "WM_DELETE_WINDOW", False);
  //XSetWMProtocols(display, win, &GDeleteWinAtom, 1);
  //GProtocolsAtom = XInternAtom(display, "WM_PROTOCOLS", False);

  size_hints.flags = USPosition | USSize;/* | PMinSize | PMaxSize */
  size_hints.x = x;
  size_hints.y = y;
  size_hints.width = pt->win_width;
  size_hints.height = pt->win_height;
  XSetWMNormalHints (pt->display, pt->win, &size_hints);
  
  name_ptr = &(pt->name[0]);  
  if(XStringListToTextProperty(&(name_ptr),1,&windowName) == 0){
    fprintf(stderr,"%s: structure allocation for windowName failed.\n",pt->name);
  } else {  
    // Sets the title bar
    XSetWMName (pt->display, pt->win, &windowName);
  
    // Free the memory
    XFree(windowName.value);
  }
  if(XStringListToTextProperty(&(name_ptr),1,&iconName) == 0){
    fprintf(stderr,"%s: structure allocation for iconName failed.\n",pt->name);
  } else {  
    // Not sure if this is even needed anymore. Newer window managers
    // use the class_hints below to determine what name to display for
    // the icon. We'll leave this here for legacy support
    XSetWMIconName (pt->display, pt->win, &iconName);
    
    // Free the memory
    XFree(iconName.value);
  }
  
  // Get the list of icon sizes available
  if(XGetIconSizes(pt->display,pt->win,&icon_list,&nicons) == 0){
    if(pt->debug) fprintf(stdout,"(DEBUG) No desired icons specified by the WM\n");
  } else {
    // Haven't ran across a window manager that has done this yet
    fprintf(stdout,"%s: The WM has set %d desired icon sizes....what do I do?",pt->name,nicons);
  }
  // Free the icon_list in case it was allocated
  if(icon_list != NULL) XFree(icon_list);
  
  if(pt->invert_bg_fg == 0) icon_pixmap = XCreateBitmapFromData(pt->display,pt->win,gxplot_wb_bmp_bits,gxplot_bmp_width,gxplot_bmp_height);
  else icon_pixmap = XCreateBitmapFromData(pt->display,pt->win,gxplot_bw_bmp_bits,gxplot_bmp_width,gxplot_bmp_height);
  
  // This is the only way in later version of Xlib and various window managers
  // to get the name of the application to show up on the icon
  class_hints.res_name = pt->name;
  class_hints.res_class = pt->name;  
  XSetClassHint(pt->display,pt->win,&class_hints);
  
  wm_hints.flags = InputHint;
  wm_hints.input = 1;
  wm_hints.flags |= StateHint;
  wm_hints.initial_state = NormalState;
  wm_hints.flags |= IconPixmapHint;
  wm_hints.icon_pixmap = icon_pixmap;
  XSetWMHints (pt->display,pt->win, &wm_hints);
  
  // Register the WM_DELETE_WINDOW atom to make sure we receive the 
  // close event when the user clicks the "x" button
  pt->wmDeleteMessage = XInternAtom(pt->display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(pt->display,pt->win, &(pt->wmDeleteMessage), 1);

  XMapWindow (pt->display, pt->win);
}

void get_gc(PLOT_T *pt){
  GC gc;
  unsigned int valuemask = GCForeground | GCBackground | GCFunction | GCGraphicsExposures;
  XGCValues GCvalues;
  Status status;
  XColor screen_def,exact_def;
  char dash_list[2] = {1,4};
  int dash_offset=0;
  
  /* Creating the graphics context is a bit tricky as we need to make sure
   * that it's created with the same depth as the drawable. Since the depth
   * may not always be that of the root window we should come up with
   * a way to guarantee a match between the GC and the drawable or we risk
   * a BadMatch X11 error */
  GCvalues.graphics_exposures = 0;
  
  // This GC is meant to be used for the text but doubles
  // as the plotting area creator
  GCvalues.foreground = pt->white;
  GCvalues.background = pt->black;
  GCvalues.function = GXcopy;  
  pt->gc_text = XCreateGC(pt->display, pt->win, valuemask, &GCvalues);
  XSetDashes(pt->display,pt->gc_text,dash_offset,dash_list,2);
  
  // This GC is meant to be used for "rubberband" boxes/lines
  GCvalues.foreground = pt->white ^ pt->black;
  GCvalues.background = 0;
  GCvalues.function = GXxor;  
  pt->gc_xor = XCreateGC(pt->display, pt->win, valuemask, &GCvalues);
  
  // This GC should be used for plotting the data
  GCvalues.foreground = pt->white;
  GCvalues.background = pt->black;
  GCvalues.function = GXcopy;  
  pt->gc_data = XCreateGC(pt->display, pt->win, valuemask, &GCvalues);
  XSetLineAttributes(pt->display,pt->gc_data,1,LineSolid,CapRound,JoinRound);
  
  // This GC shall be used for drawing the buttons, menus, etc.
  GCvalues.foreground = pt->white;
  GCvalues.background = pt->black;
  GCvalues.function = GXcopy;
  pt->gc_widget = XCreateGC(pt->display, pt->win, valuemask, &GCvalues);
  XSetLineAttributes(pt->display,pt->gc_widget,1,LineSolid,CapRound,JoinRound);
}

void set_min_max(PLOT_T *pt,double *xmin, double *xmax, double *ymin, double *ymax, int level){
  int i;
  if(level == -1){
    for(i=0;i<MAX_ZOOM_LEVEL;i++){
      if(xmin != NULL) pt->zoom_info[i].xmin=*xmin;
      if(xmax != NULL) pt->zoom_info[i].xmax=*xmax;
      if(ymin != NULL) pt->zoom_info[i].ymin=*ymin;
      if(ymax != NULL) pt->zoom_info[i].ymax=*ymax;
    }
  } else {
    if(level < MAX_ZOOM_LEVEL){
      if(xmin != NULL) pt->zoom_info[level].xmin=*xmin;
      if(xmax != NULL) pt->zoom_info[level].xmax=*xmax;
      if(ymin != NULL) pt->zoom_info[level].ymin=*ymin;
      if(ymax != NULL) pt->zoom_info[level].ymax=*ymax;
    }
  }
  return;
}

void xy2rw(PLOT_T *pt,unsigned int x, double *xval, unsigned int y, double *yval){
  unsigned int x_pix,y_pix;
  
  // Determine the pixel
  x_pix = min(pt->grid_x2,max(pt->grid_x,x));
  y_pix = min(pt->grid_y2,max(pt->grid_y,y)); 
  
  *xval = pt->zoom_info[pt->zoom_level].xmin + (double)(x_pix - pt->grid_x) * ((pt->zoom_info[pt->zoom_level].xmax - pt->zoom_info[pt->zoom_level].xmin)/(double)pt->grid_width);
  *yval = pt->zoom_info[pt->zoom_level].ymin + (double)(pt->grid_y2 - y_pix) * ((pt->zoom_info[pt->zoom_level].ymax - pt->zoom_info[pt->zoom_level].ymin)/(double)pt->grid_height);
  
  return;
}

void mouse_xy2rw(PLOT_T *pt){
  // Convert the X and Y mouse positions from pixels to realworld values
  xy2rw(pt,pt->mouse_x,&(pt->mouse_xv),pt->mouse_y,&(pt->mouse_yv));
  
  if(pt->mouse_xmrk > 0 && pt->mouse_ymrk > 0){
    // Convert the X and Y mouse mark positions from pixels to realworld values
    xy2rw(pt,pt->mouse_xmrk,&(pt->mouse_xmrk_v),pt->mouse_ymrk,&(pt->mouse_ymrk_v));
  } else {
    pt->mouse_xmrk_v = 0.0;
    pt->mouse_ymrk_v = 0.0;
  }
  
  return;
}

void update_specs(PLOT_T *pt){
  char lbl_rowa[100];
  char lbl_rowb[100];
  char y_lbl[25];
  char dy_lbl[25];
  char x_lbl[25];
  char dx_lbl[25];
  int x_pix,y_pix;
  double dyval;
  double dxval;
  double xval,yval;
  int index_mode=0;
  
  mouse_xy2rw(pt);
  xval = pt->mouse_xv;
  yval = pt->mouse_yv;
  dyval = pt->mouse_yv - pt->mouse_ymrk_v;
  dxval = pt->mouse_xv - pt->mouse_xmrk_v;
  
  // Make adjustments for the current mode
  if(strncmp(specs_mode_name[pt->specs_mode],"indx",4) == 0){
    // Need to convert the xval and dxval to index
    if(pt->nfiles > 0){
      // When you have more than 1 file the index mode can only
      // be made relative to a single file. We'll choose the first file
      xval = round((xval - pt->flist[0]->xstart) / pt->plot_xdelta);
      dxval = round(dxval / pt->plot_xdelta);
    }
    index_mode=1;
  }
  else if(strncmp(specs_mode_name[pt->specs_mode],"1/ab",4) == 0){
    if(xval != 0.0) xval = 1.0/xval;
    if(yval != 0.0) yval = 1.0/yval;
    if(dxval != 0.0) dxval = 1.0/dxval;
    if(dyval != 0.0) dyval = 1.0/dyval;
  }
  
  if(fabs(yval) < 1e3 && fabs(yval) >= 0.1) sprintf(y_lbl,"y: %16.9f",yval);
  else sprintf(y_lbl,"y: %16.9e",yval);
  if(fabs(dyval) < 1e3 && fabs(dyval) >= 0.1) sprintf(dy_lbl,"dy: %16.9f",dyval);
  else sprintf(dy_lbl,"dy: %16.9e",dyval);
  
  sprintf(lbl_rowa,"%s %s L=%d %s",y_lbl,dy_lbl,pt->zoom_level,cx_mode_name[pt->cx_mode]);
  
  if((fabs(xval) < 1e3 && fabs(xval) >= 0.1) || index_mode) sprintf(x_lbl,"x: %16.9f",xval);
  else sprintf(x_lbl,"x: %16.9e",xval);
  if((fabs(dxval) < 1e3 && fabs(dxval) >= 0.1) || index_mode) sprintf(dx_lbl,"dx: %16.9f",dxval);
  else sprintf(dx_lbl,"dx: %16.9e",dxval);
    
  sprintf(lbl_rowb,"%s %s (%s)",x_lbl,dx_lbl,specs_mode_name[pt->specs_mode]);
  
  if(strncmp(specs_mode_name[pt->specs_mode],"dydx",4) == 0){
    if(dxval == 0.0) sprintf(lbl_rowa+20,"sl: Inf             ");
    else {
      if(fabs(dyval/dxval) < 1e3 && fabs(dyval/dxval) >= 0.1) sprintf(lbl_rowa+20,"sl: %16.9f",(dyval/dxval));
      else sprintf(lbl_rowa+20,"sl: %16.9e",(dyval/dxval));
    }
    
    // Remove the NULL character placed by sprintf
    lbl_rowa[40]=' ';
  }
  
  XDrawImageString(pt->display, pt->win, pt->gc_text, pt->font_width, pt->win_height - (int)(round(1.5*(double)pt->font_height)), lbl_rowa, strlen(lbl_rowa));
  XDrawImageString(pt->display, pt->win, pt->gc_text, pt->font_width, (pt->win_height - (int)(round(1.5*(double)pt->font_height)))+pt->font_height, lbl_rowb, strlen(lbl_rowb));
  
  pt->pan_start = (pt->font_width * strlen(lbl_rowa));
  return;
}

void form_div(double min, double max, double *tic1, double *tic, int ndiv){
  double dran,df,sig,nseg,ddf;
  int nsig;
  *tic=1.0;
  *tic1=min;
  if(max == min) return;
  
  dran = fabs(max-min);
  df = dran/(double)ndiv;
  sig = log10(max(df,1e-36));
  nsig=(int)sig;
  
  if(sig < 0.0) nsig = nsig-1;
  
  ddf = df * pow(10.0,(double)(-nsig));
  sig = pow(10.0,(double)nsig);
  
  if(ddf < 1.75) *tic=sig;
  else if(ddf < 2.25) *tic = sig*2.0;
  else if(ddf < 3.5) *tic = sig*2.5;
  else if(ddf < 7.0) *tic = sig*5.0;
  else *tic = sig*10.0;
  
  // redefine dmin and dmax to line up on 'nice' boundaries
  if(*tic == 0.0) *tic=1.0;
  if(max >= min){
    if(min >= 0.0) nseg = trunc(min / *tic+0.995);
    else nseg = trunc(min / *tic - 0.005);
    *tic1 = nseg* *tic;
  } else {
    if(min >= 0.0) nseg = trunc(min/ *tic+0.005);
    else nseg = trunc(min/ *tic-0.995);
    *tic1 = nseg* *tic;
    *tic = -(*tic);
  }
  
  if(*tic1+*tic == *tic1) *tic = max-min;
  
  return;
}

void mult(double end1,double end2,double *xmult){
  double absmax;
  int kengr;
  absmax = max(fabs(end1),fabs(end2));
  if(absmax == 0.0) *xmult = 1.0;
  else {
    kengr = (int)(0.1447648 * logf(absmax));
    if(absmax < 1.0) kengr=kengr-1;
    if(kengr < 0) *xmult = 1.0 / powf(10.0,(-3*kengr));
    else *xmult = powf(10.0,(3*kengr));
  }
}

void trim_label(char *str, int *begin, int *end){
  if(strncmp(str+5,".000000",7) == 0) *end=5;
  else {
    *end=11;
    while(str[*end] == '0') *end -= 1;
  }
  
  *begin = 0;
  while(str[*begin] == ' ') *begin += 1;
}

void set_grid_dimensions(PLOT_T *pt){
  XRectangle rect;
  
  pt->grid_x = 40;
  pt->grid_y = pt->font_height+pt->widget_spacing;
  //pt->grid_width = pt->win_width-pt->grid_x-pt->widget_spacing*2;
  pt->grid_width = pt->win_width-pt->grid_x-pt->ypan_bar.ar_width-pt->widget_spacing;
  pt->grid_height = (pt->win_height-pt->widget_spacing)-pt->font_height*4-pt->widget_spacing*2;
  
  // Derive the x2,y2 of the grid
  pt->grid_x2 = pt->grid_x + pt->grid_width;
  pt->grid_y2 = pt->grid_y + pt->grid_height;
  
  // Set the clip region
  rect.x=pt->grid_x;
  rect.y=pt->grid_y;
  rect.width=pt->grid_width;
  rect.height=pt->grid_height;
  XSetClipRectangles(pt->display,pt->gc_data,0,0,&rect,1,Unsorted);
}

void draw_grid(PLOT_T *pt){    
  double xtic1,xtic,ytic1,ytic,xmult,ymult,x,y;
  double fmul,fact;
  int i,tstart,tend,itexti,jtext;
  char disp_lbl[20];
  
  // Set our line to solid for the grid outline
  XSetLineAttributes(pt->display,pt->gc_text,0,LineSolid,CapRound,JoinRound);
  
  // Cheap way to update the grid dimensions in case the window has been resized
  // between calls
  set_grid_dimensions(pt);
  
  // Reset the real world x,y min/max to grid coordinate ratio
  pt->zoom_info[pt->zoom_level].x_p2r = (pt->zoom_info[pt->zoom_level].xmax-pt->zoom_info[pt->zoom_level].xmin) / (double)(pt->grid_x2 - pt->grid_x) ;
  pt->zoom_info[pt->zoom_level].y_p2r = (pt->zoom_info[pt->zoom_level].ymin-pt->zoom_info[pt->zoom_level].ymax) / (double)(pt->grid_y2 - pt->grid_y);
  
  // Draw the grid outline
  XDrawRectangle(pt->display,pt->win,pt->gc_text,pt->grid_x-2,pt->grid_y-2,pt->grid_width+3,pt->grid_height+3);
  
  // Set our line to dashed for the grid lines
  XSetLineAttributes(pt->display,pt->gc_text,0,LineOnOffDash,CapRound,JoinRound);
  
  // Setup the X-Tic marks and labels
  form_div(pt->zoom_info[pt->zoom_level].xmin, pt->zoom_info[pt->zoom_level].xmax, &xtic1, &xtic, 5);
  xmult = 1.0;
  mult(pt->zoom_info[pt->zoom_level].xmin, pt->zoom_info[pt->zoom_level].xmax, &xmult);
  
  if(pt->zoom_info[pt->zoom_level].xmin != pt->zoom_info[pt->zoom_level].xmax) fact = (double)(pt->grid_width) / (pt->zoom_info[pt->zoom_level].xmax - pt->zoom_info[pt->zoom_level].xmin);
  else fact = (double)(pt->grid_width) / 1.0;
  if(xmult != 0.0) fmul = 1.0/xmult;
  else fmul = 1.0;
  if(xtic == 0.0) xtic = pt->zoom_info[pt->zoom_level].xmax - xtic1 + 1.0;
  jtext = pt->grid_y2 + pt->font_height;
  
  for(x=xtic1;x<=pt->zoom_info[pt->zoom_level].xmax;x+=xtic){
    i = (int)pt->grid_x + (int)round(fact*(x- pt->zoom_info[pt->zoom_level].xmin));
    
    XDrawLine (pt->display, pt->win, pt->gc_text, i, pt->grid_y2, i, pt->grid_y);
    
    sprintf(disp_lbl,"%12.6f",(x*fmul));
    disp_lbl[12]='\0'; // NULL terminate
    trim_label(disp_lbl,&tstart,&tend);
    
    itexti = ((2+tend-tstart)/2) * pt->font_width;
    //XDrawImageString(pt->display, pt->win, pt->gc_text, i-itexti, jtext, disp_lbl+tstart, tend-tstart+1);
    XDrawString(pt->display, pt->win, pt->gc_text, i-itexti, jtext, disp_lbl+tstart, tend-tstart+1);
  }
  
  // Setup the Y-Tic marks and labels  
  form_div(pt->zoom_info[pt->zoom_level].ymin, pt->zoom_info[pt->zoom_level].ymax, &ytic1, &ytic, 5);
  ymult = 1.0;
  mult(pt->zoom_info[pt->zoom_level].ymin, pt->zoom_info[pt->zoom_level].ymax, &ymult);
  
  if(pt->zoom_info[pt->zoom_level].ymin != pt->zoom_info[pt->zoom_level].ymax) fact = (double)(-(pt->grid_height))/(pt->zoom_info[pt->zoom_level].ymax-pt->zoom_info[pt->zoom_level].ymin);
  else fact = (double)(-(pt->grid_height))/1.0;
  if(ymult != 0.0) fmul = 1.0/ymult;
  else fmul = 1.0;
  if(ytic == 0.0) ytic = pt->zoom_info[pt->zoom_level].ymax-ytic1 + 1.0;
  jtext = (int)(0.4*(double)pt->font_height);
  itexti = max (0, pt->grid_x - (int)(5.5*(double)pt->font_width));
  
  for(y=ytic1;y<=pt->zoom_info[pt->zoom_level].ymax;y+=ytic){
    i = pt->grid_y2 + (int)round(fact*(y-pt->zoom_info[pt->zoom_level].ymin));
    
    XDrawLine (pt->display, pt->win, pt->gc_text,pt->grid_x, i, pt->grid_x2, i);    
    
    sprintf(disp_lbl,"%12.6f",(y*fmul));
    disp_lbl[12]='\0'; // NULL terminate
    trim_label(disp_lbl,&tstart,&tend);
    
    itexti = pt->grid_x - ((tend-tstart+1)*pt->font_width);
    //XDrawImageString(pt->display, pt->win, pt->gc_text, itexti, min(pt->grid_y2,i+jtext), disp_lbl+tstart, tend-tstart+1);
    XDrawString(pt->display, pt->win, pt->gc_text, itexti, min(pt->grid_y2,i+jtext), disp_lbl+tstart, tend-tstart+1);
  }
  return;
}

void draw_plot(PLOT_T *pt){  
  // Place the units text
  // generate the y and x labels
  if(pt->cx_mode == 0){
    sprintf(pt->ylabel,"Magnitude (U)");
    sprintf(pt->xlabel,"Unknown (U)");
  }
  else if(pt->cx_mode == 1){
    sprintf(pt->ylabel,"Phase (deg)");
    sprintf(pt->xlabel,"Unknown (U)");
  }
  else if(pt->cx_mode == 2){
    sprintf(pt->ylabel,"Real (U)");
    sprintf(pt->xlabel,"Unknown (U)");
  }
  else if(pt->cx_mode == 3){
    sprintf(pt->ylabel,"Imaginary (U)");
    sprintf(pt->xlabel,"Unknown (U)");
  }
  else if(pt->cx_mode == 4){
    sprintf(pt->ylabel,"Imaginary (U)");
    sprintf(pt->xlabel,"Real (U)");
  }
  else if(pt->cx_mode == 5){
    sprintf(pt->ylabel,"10*Log (U)");
    sprintf(pt->xlabel,"Unknown (U)");
  }
  else if(pt->cx_mode == 6){
    sprintf(pt->ylabel,"20*Log (U)");
    sprintf(pt->xlabel,"Unknown (U)");
  }
  
  char units_label[strlen(pt->ylabel) + strlen(pt->xlabel) + 5];
  sprintf(units_label,"%s vs %s",pt->ylabel,pt->xlabel);
  XDrawString(pt->display,pt->win,pt->gc_text,pt->widget_spacing,pt->font_height,units_label,strlen(units_label));
      
  // Place the pointer value text
  update_specs(pt);
  
  // Draw the grid
  draw_grid(pt);
  
  // Draw the pan bars
  draw_xpan(pt);
  draw_ypan(pt);
  
  // Draw the layer options button 
  pt->buttons[LAYER_OPTS].x1 = pt->grid_x2+4;
  pt->buttons[LAYER_OPTS].y1 = 0;
  pt->buttons[LAYER_OPTS].x2 = pt->buttons[LAYER_OPTS].x1 + (pt->win_width - (pt->grid_x2+2)-4);
  pt->buttons[LAYER_OPTS].y2 = pt->grid_y-4;
  sprintf(pt->buttons[LAYER_OPTS].text,"L");  
  
  draw_bevel_box(pt,&(pt->buttons[LAYER_OPTS]));
  
  // Force an update
  XFlush(pt->display);
}

void draw_xpan(PLOT_T *pt){  
  // Draw the pan bars
  int p_xl,p_yl,p_xr,p_yr,i,npoints;
  XPoint arrow[4];
  
  // Left arrow starting point
  p_xl = pt->pan_start;
  p_yl = pt->win_height - (int)round((double)pt->font_height*1.5)-pt->font_height/4;
    
  // right arrow starting point
  p_xr = pt->grid_x2+2-pt->xpan_bar.ar_width;
  p_yr = pt->win_height - (int)round((double)pt->font_height*1.5)-pt->font_height/4;
  
  // Compute the rectangular area which this pan bar will cover
  pt->xpan_bar.x1 = p_xl;
  pt->xpan_bar.y1 = p_yl;
  pt->xpan_bar.x2 = p_xr+pt->xpan_bar.ar_width;
  pt->xpan_bar.y2 = p_yr+pt->xpan_bar.ar_height;
  
  // Display the pieces
  for(i=0;i<4;i++){
    npoints=4;
    
    // Set the color of the various pieces
    if(i==0) XSetForeground(pt->display,pt->gc_widget,pt->light_grey);
    else if(i==1 || i==2) XSetForeground(pt->display,pt->gc_widget,pt->dark_grey);
    else {
      XSetForeground(pt->display,pt->gc_widget,pt->medium_grey);
      npoints=3;
    }
    
    arrow[0].x = p_xl+pt->xpan_bar.arrow[i][0].x;
    arrow[0].y = p_yl+pt->xpan_bar.arrow[i][0].y;
    arrow[1].x = p_xl+pt->xpan_bar.arrow[i][1].x;
    arrow[1].y = p_yl+pt->xpan_bar.arrow[i][1].y;
    arrow[2].x = p_xl+pt->xpan_bar.arrow[i][2].x;
    arrow[2].y = p_yl+pt->xpan_bar.arrow[i][2].y;
    arrow[3].x = p_xl+pt->xpan_bar.arrow[i][3].x;
    arrow[3].y = p_yl+pt->xpan_bar.arrow[i][3].y;
    XFillPolygon(pt->display,pt->win, pt->gc_widget, arrow, npoints, Nonconvex, CoordModeOrigin);  
  
    // Reflect for the right arrow. When reflecting across the
    // x-axis, the y value remains the same
    arrow[0].x = p_xr+abs((int)pt->xpan_bar.ar_width-pt->xpan_bar.arrow[i][0].x);
    arrow[1].x = p_xr+abs((int)pt->xpan_bar.ar_width-pt->xpan_bar.arrow[i][1].x);
    arrow[2].x = p_xr+abs((int)pt->xpan_bar.ar_width-pt->xpan_bar.arrow[i][2].x);
    arrow[3].x = p_xr+abs((int)pt->xpan_bar.ar_width-pt->xpan_bar.arrow[i][3].x);
    XFillPolygon(pt->display,pt->win, pt->gc_widget, arrow, npoints, Nonconvex, CoordModeOrigin);     
  }
  
  // Connect the arrows
  pt->xpan_bar.drag_x1 = p_xl+pt->xpan_bar.ar_width+1;
  pt->xpan_bar.drag_y1 = p_yl;
  pt->xpan_bar.drag_x2 = p_xr-2;
  pt->xpan_bar.drag_y2 = p_yl+pt->xpan_bar.ar_height;
  
  draw_xpan_drag(pt);
  return;
}
  
void draw_xpan_drag(PLOT_T *pt){ 
  double b_start,b_width;
  
  // Draw the connecting line between the arrows
  XDrawLine (pt->display, pt->win, pt->gc_widget,
             pt->xpan_bar.drag_x1, 
             pt->xpan_bar.drag_y1+(pt->xpan_bar.ar_height/2), 
             pt->xpan_bar.drag_x2, 
             pt->xpan_bar.drag_y2-(pt->xpan_bar.ar_height/2));
  
  if(pt->cx_mode == 4 || pt->framed_data){ 
    // Draw the drag box (Need to revisit this area and clean up the mess)
    double z_xmin = pt->zoom_info[pt->zoom_level].xmin;
    double z_xmax = pt->zoom_info[pt->zoom_level].xmax;
    if(pt->debug) fprintf(stdout, "(DEBUG) xmin=%0.12e xmax=%0.12e\n",z_xmin,z_xmax);
  
    double pdx2 = (pt->x_max - pt->x_min) - (z_xmax - z_xmin);
    double pdx = (pt->x_max - pt->x_min) / (z_xmax - z_xmin);
    double sdx2 = (((double)pt->xpan_bar.drag_x2 - (double)pt->xpan_bar.drag_x1)+1.0) - (double)pt->xpan_bar.ar_height;
  
    double dx_r = (sdx2/pdx);
  
    b_width = ceil(dx_r);
    if(pt->debug) fprintf(stdout, "(DEBUG) dx_r=%0.12e sdx2=%0.12e height=%0.12e \n",dx_r,sdx2,b_width);
    if(b_width < pt->xpan_bar.ar_height) b_width=pt->xpan_bar.ar_height;
  
    double sdx = sdx2 - b_width;
    double dx_p2;
    if(sdx == 0.0) dx_p2=0.0;
    else dx_p2 = (pdx2/sdx);
  
    // This ratio is used at a later time to convert pixel to abscissa
    pt->xpan_bar.ppa = dx_p2;
  
    //double b_start = ceil((pt->zoom_info[pt->zoom_level].ymin - pt->y_min)/dx_p2);
    double s_off = (z_xmax + z_xmin)/2.0; // middle value of current zoom
    double y_mid = (pt->x_max + pt->x_min)/2.0; // overall middle value
    double l_mid = (((double)pt->xpan_bar.drag_x2 - (double)pt->xpan_bar.drag_x1)+1.0)/2.0;
    if(pt->debug) fprintf(stdout, "(DEBUG) x_mid=%0.12e l_mid=%0.12e\n",y_mid,l_mid);
    if(dx_p2 == 0.0) b_start = ceil(l_mid - (b_width/2.0));
    else b_start = ceil(l_mid - ((y_mid - s_off)/dx_p2)- (b_width/2.0));
    if(pt->debug) fprintf(stdout, "(DEBUG) %0.12e\n",((y_mid - s_off)*dx_p2));
    //double b_start = ceil((z_ymax-s_off)*dx_p2 - (b_width/2.0));
    if(pt->debug) fprintf(stdout, "(DEBUG) soff=%0.12e\n",s_off);
    if(pt->debug) fprintf(stdout, "(DEBUG) %0.12e %0.12e %g %g %g\n",z_xmin,
                                    z_xmax,
                                    dx_p2,
                                    b_start,
                                    sdx2);
    
    b_start -= (double)pt->xpan_bar.ar_height/2.0;
    b_width += (double)pt->xpan_bar.ar_height;
    
    // Need to revisit these fail safes as I don't believe their response is relevant anymore
    if(b_start < 0.0) b_start=0.0;
  
    if((double)pt->xpan_bar.drag_x1 + b_start + b_width-1 > (double)pt->xpan_bar.drag_x2) {
      if(pt->debug) fprintf(stdout, "(DEBUG) Hit end of x %d %d\n",pt->xpan_bar.drag_x1,pt->xpan_bar.drag_x2);
      if(b_start == 0.0){
        b_width = sdx2+(double)pt->xpan_bar.ar_height;
      } else {
        //b_start = ((double)pt->ypan_bar.drag_y2-b_width-(double)pt->ypan_bar.drag_y1)+1.0;
        b_width = sdx2+(double)pt->xpan_bar.ar_height-b_start;
      }
      //b_start = ((double)pt->xpan_bar.drag_x2-b_width-(double)pt->xpan_bar.drag_x1)+1.0;
    }
  } else {
    // Draw the drag box (Need to revisit this area and clean up the mess)
    double pdx2 = ((pt->abscissa_max - pt->abscissa_min) + pt->plot_xdelta) - ((pt->zoom_info[pt->zoom_level].xmax - pt->zoom_info[pt->zoom_level].xmin) + pt->plot_xdelta);
    double pdx = ((pt->abscissa_max - pt->abscissa_min) + pt->plot_xdelta) / ((pt->zoom_info[pt->zoom_level].xmax - pt->zoom_info[pt->zoom_level].xmin) + pt->plot_xdelta);
    double sdx2 = (((double)pt->xpan_bar.drag_x2 - (double)pt->xpan_bar.drag_x1)+1.0);
  
    double dx_r = (sdx2/pdx);
  
    b_width = ceil(dx_r);
    if(b_width < pt->xpan_bar.ar_height) b_width=pt->xpan_bar.ar_height;
  
    double sdx = sdx2 - b_width;
    double dx_p2;
    if(sdx == 0.0) dx_p2=0.0;
    else dx_p2 = (pdx2/sdx);
  
    // This ratio is used at a later time to convert pixel to abscissa
    pt->xpan_bar.ppa = dx_p2;
  
    if(dx_p2 == 0.0) b_start=0.0;
    else b_start = ceil((pt->zoom_info[pt->zoom_level].xmin - pt->abscissa_min)/dx_p2);
    if(pt->debug) fprintf(stdout, "(DEBUG) %0.12e %0.12e %g %g %d\n",pt->zoom_info[pt->zoom_level].xmin,
         pt->abscissa_max,
         dx_p2,
         b_start,
         ((pt->xpan_bar.drag_x2-pt->xpan_bar.drag_x1)+1));
  
    if((double)pt->xpan_bar.drag_x1 + b_start + b_width-1 > (double)pt->xpan_bar.drag_x2) {
      if(pt->debug) fprintf(stdout, "(DEBUG) Hit end\n");
      b_start = ((double)pt->xpan_bar.drag_x2-b_width-(double)pt->xpan_bar.drag_x1)+1.0;
    }
  }
  
  // Keep track of the dimensions of the handle so we can tell
  // if the user has clicked it later
  pt->xpan_bar.hndl.x1 = pt->xpan_bar.drag_x1 + b_start;
  pt->xpan_bar.hndl.y1 = pt->xpan_bar.drag_y1;
  pt->xpan_bar.hndl.x2 = pt->xpan_bar.hndl.x1+b_width;
  pt->xpan_bar.hndl.y2 = pt->xpan_bar.drag_y1+pt->xpan_bar.ar_height;
  
  //draw_bevel_box(pt,pt->xpan_bar.hndl_x1,pt->xpan_bar.hndl_y1,pt->xpan_bar.ar_height,(unsigned int)b_width,0);
  draw_bevel_box(pt,&(pt->xpan_bar.hndl));
  
  return;
}
  
void draw_bevel_box(PLOT_T *pt, BOX_INFO_T *tbox){
  XPoint box[4],pieces[5][4];
  int x1,y1;
  unsigned int height,width;
  long lg,dg,mg;
  
  x1=tbox->x1;
  y1=tbox->y1;
  height = tbox->y2-tbox->y1;
  width = tbox->x2-tbox->x1;
  
  if(tbox->hlit){
    mg = pt->h_medium_grey;
    if(tbox->pressed){
      lg = pt->h_dark_grey;
      dg = pt->h_light_grey;
    } else {
      lg = pt->h_light_grey;
      dg = pt->h_dark_grey;
    }
  }
  else{
    mg = pt->medium_grey;
    if(tbox->pressed){
      lg = pt->dark_grey;
      dg = pt->light_grey;
    } else {
      lg = pt->light_grey;
      dg = pt->dark_grey;
    }
  }
  
  // Build the pieces of the box. During the build the pieces are 0-based
  // meaning the start x and y values are not taken into account until
  // it's time to draw. Might want to revisit this in the future for speed
  // up purposes
  
  // Left
  pieces[0][0].x = 0;
  pieces[0][0].y = 0;
  pieces[0][1].x = 3;
  pieces[0][1].y = 3;
  pieces[0][2].x = 3;
  pieces[0][2].y = height-3;
  pieces[0][3].x = 0;
  pieces[0][3].y = height;
  
  // Top
  pieces[1][0].x = 0;
  pieces[1][0].y = 0;
  pieces[1][1].x = width;
  pieces[1][1].y = 0;
  pieces[1][2].x = width-3;
  pieces[1][2].y = 3;
  pieces[1][3].x = 3;
  pieces[1][3].y = 3;
  
  // Right
  pieces[2][0].x = width;
  pieces[2][0].y = 0;
  pieces[2][1].x = width-3;
  pieces[2][1].y = 3;
  pieces[2][2].x = width-3;
  pieces[2][2].y = height-3;
  pieces[2][3].x = width;
  pieces[2][3].y = height;
  
  // Bottom
  pieces[3][0].x = 0;
  pieces[3][0].y = height;
  pieces[3][1].x = 3;
  pieces[3][1].y = height-3;
  pieces[3][2].x = width-3;
  pieces[3][2].y = height-3;
  pieces[3][3].x = width;
  pieces[3][3].y = height;
  
  // Middle
  pieces[4][0].x = 3;
  pieces[4][0].y = 3;
  pieces[4][1].x = width-3;
  pieces[4][1].y = 3;
  pieces[4][2].x = width-3;
  pieces[4][2].y = height-3;
  pieces[4][3].x = 3;
  pieces[4][3].y = height-3;
  
  box[0].x = x1+pieces[0][0].x;
  box[0].y = y1+pieces[0][0].y;
  box[1].x = x1+pieces[0][1].x;
  box[1].y = y1+pieces[0][1].y;
  box[2].x = x1+pieces[0][2].x;
  box[2].y = y1+pieces[0][2].y;
  box[3].x = x1+pieces[0][3].x;
  box[3].y = y1+pieces[0][3].y;
  XSetForeground(pt->display,pt->gc_widget,lg);
  XFillPolygon(pt->display,pt->win, pt->gc_widget, box, 4, Nonconvex, CoordModeOrigin);
  
  box[0].x = x1+pieces[1][0].x;
  box[0].y = y1+pieces[1][0].y;
  box[1].x = x1+pieces[1][1].x;
  box[1].y = y1+pieces[1][1].y;
  box[2].x = x1+pieces[1][2].x;
  box[2].y = y1+pieces[1][2].y;
  box[3].x = x1+pieces[1][3].x;
  box[3].y = y1+pieces[1][3].y;
  XFillPolygon(pt->display,pt->win, pt->gc_widget, box, 4, Nonconvex, CoordModeOrigin);
  
  box[0].x = x1+pieces[2][0].x;
  box[0].y = y1+pieces[2][0].y;
  box[1].x = x1+pieces[2][1].x;
  box[1].y = y1+pieces[2][1].y;
  box[2].x = x1+pieces[2][2].x;
  box[2].y = y1+pieces[2][2].y;
  box[3].x = x1+pieces[2][3].x;
  box[3].y = y1+pieces[2][3].y;
  XSetForeground(pt->display,pt->gc_widget,dg);
  XFillPolygon(pt->display,pt->win, pt->gc_widget, box, 4, Nonconvex, CoordModeOrigin);
  
  box[0].x = x1+pieces[3][0].x;
  box[0].y = y1+pieces[3][0].y;
  box[1].x = x1+pieces[3][1].x;
  box[1].y = y1+pieces[3][1].y;
  box[2].x = x1+pieces[3][2].x;
  box[2].y = y1+pieces[3][2].y;
  box[3].x = x1+pieces[3][3].x;
  box[3].y = y1+pieces[3][3].y;
  XFillPolygon(pt->display,pt->win, pt->gc_widget, box, 4, Nonconvex, CoordModeOrigin);
  
  box[0].x = x1+pieces[4][0].x;
  box[0].y = y1+pieces[4][0].y;
  box[1].x = x1+pieces[4][1].x;
  box[1].y = y1+pieces[4][1].y;
  box[2].x = x1+pieces[4][2].x;
  box[2].y = y1+pieces[4][2].y;
  box[3].x = x1+pieces[4][3].x;
  box[3].y = y1+pieces[4][3].y;
  XSetForeground(pt->display,pt->gc_widget,mg);
  XFillPolygon(pt->display,pt->win, pt->gc_widget, box, 4, Nonconvex, CoordModeOrigin);
  
  // At some point this needs to be expanded to handle multi-character text. For now
  // it's really only good for single character (revisit)
  if(strlen(tbox->text) > 0) XDrawString(pt->display,pt->win,pt->gc_text,tbox->x1+width/2-pt->font_width/2+1,tbox->y1+height/2+pt->font_height/2-1,tbox->text,strlen(tbox->text));
  
  return;
}

void draw_ypan(PLOT_T *pt){  
  // Draw the pan bars
  int p_xt,p_yt,p_xb,p_yb,i,npoints;
  XPoint arrow[4];
  
  // Top arrow starting point
  p_xt = pt->grid_x2+2+pt->widget_spacing/2;
  p_yt = pt->grid_y-2;
    
  // botton arrow starting point
  p_xb = pt->grid_x2+2+pt->widget_spacing/2;
  p_yb = pt->grid_y2+2 - pt->ypan_bar.ar_height;
  
  // Compute the rectangular area which this pan bar will cover
  pt->ypan_bar.x1 = p_xt;
  pt->ypan_bar.y1 = p_yt;
  pt->ypan_bar.x2 = p_xb+pt->xpan_bar.ar_width;
  pt->ypan_bar.y2 = p_yb+pt->xpan_bar.ar_height;
  
  // Display the pieces
  for(i=0;i<4;i++){
    npoints=4;
    
    // Set the color of the various pieces
    if(i==0) XSetForeground(pt->display,pt->gc_widget,pt->light_grey);
    else if(i==1 || i==2) XSetForeground(pt->display,pt->gc_widget,pt->dark_grey);
    else {
      XSetForeground(pt->display,pt->gc_widget,pt->medium_grey);
      npoints=3;
    }
    
    arrow[0].x = p_xt+pt->ypan_bar.arrow[i][0].x;
    arrow[0].y = p_yt+pt->ypan_bar.arrow[i][0].y;
    arrow[1].x = p_xt+pt->ypan_bar.arrow[i][1].x;
    arrow[1].y = p_yt+pt->ypan_bar.arrow[i][1].y;
    arrow[2].x = p_xt+pt->ypan_bar.arrow[i][2].x;
    arrow[2].y = p_yt+pt->ypan_bar.arrow[i][2].y;
    arrow[3].x = p_xt+pt->ypan_bar.arrow[i][3].x;
    arrow[3].y = p_yt+pt->ypan_bar.arrow[i][3].y;
    XFillPolygon(pt->display,pt->win, pt->gc_widget, arrow, npoints, Nonconvex, CoordModeOrigin);  
  
    // Reflect for the down arrow. When reflecting across the
    // y-axis, the x value remains the same
    arrow[0].y = p_yb+abs((int)pt->ypan_bar.ar_height-pt->ypan_bar.arrow[i][0].y);
    arrow[1].y = p_yb+abs((int)pt->ypan_bar.ar_height-pt->ypan_bar.arrow[i][1].y);
    arrow[2].y = p_yb+abs((int)pt->ypan_bar.ar_height-pt->ypan_bar.arrow[i][2].y);
    arrow[3].y = p_yb+abs((int)pt->ypan_bar.ar_height-pt->ypan_bar.arrow[i][3].y);
    XFillPolygon(pt->display,pt->win, pt->gc_widget, arrow, npoints, Nonconvex, CoordModeOrigin);
  }
  
  pt->ypan_bar.drag_x1 = p_xt;
  pt->ypan_bar.drag_y1 = p_yt+pt->ypan_bar.ar_height+1;
  pt->ypan_bar.drag_x2 = p_xb+pt->ypan_bar.ar_width;
  pt->ypan_bar.drag_y2 = p_yb-2;
  
  draw_ypan_drag(pt);
  
  return;
}

void draw_ypan_drag(PLOT_T *pt){  
  // Connect the arrows
  XDrawLine (pt->display, pt->win, pt->gc_widget,
             pt->ypan_bar.drag_x1+(pt->ypan_bar.ar_width/2), 
             pt->ypan_bar.drag_y1, 
             pt->ypan_bar.drag_x2-(pt->ypan_bar.ar_width/2), 
             pt->ypan_bar.drag_y2); 

  // Draw the drag box (Need to revisit this area and clean up the mess)
  double z_ymin = pt->zoom_info[pt->zoom_level].ymin;
  double z_ymax = pt->zoom_info[pt->zoom_level].ymax;
  //double z_ymin = pt->init_ymin;
  //double z_ymax = pt->init_ymax;
  if(pt->debug) fprintf(stdout, "(DEBUG) ymin=%0.12e ymax=%0.12e\n",z_ymin,z_ymax);
  
  double pdx2 = (pt->y_max - pt->y_min) - (z_ymax - z_ymin);
  double pdx = (pt->y_max - pt->y_min) / (z_ymax - z_ymin);
  double sdx2 = (((double)pt->ypan_bar.drag_y2 - (double)pt->ypan_bar.drag_y1)+1.0)-(double)pt->ypan_bar.ar_width;
  
  double dx_r = (sdx2/pdx);
  
  double b_width = ceil(dx_r);
  if(pt->debug) fprintf(stdout, "(DEBUG) dx_r=%0.12e sdx2=%0.12e height=%0.12e \n",dx_r,sdx2,b_width);
  if(b_width < pt->ypan_bar.ar_width) b_width=pt->ypan_bar.ar_width;
  
  double sdx = sdx2 - b_width;
  double dx_p2;
  if(sdx == 0.0) dx_p2=0.0;
  else dx_p2 = (pdx2/sdx);
  
  // This ratio is used at a later time to convert pixel to abscissa
  pt->ypan_bar.ppa = dx_p2;
  
  double s_off = (z_ymax + z_ymin)/2.0; // middle value of current zoom
  double y_mid = (pt->y_max + pt->y_min)/2.0; // overall middle value
  double l_mid = (((double)pt->ypan_bar.drag_y2 - (double)pt->ypan_bar.drag_y1)+1.0)/2.0;
  if(pt->debug) fprintf(stdout, "(DEBUG) y_mid=%0.12e l_mid=%0.12e\n",y_mid,l_mid);
  double b_start;
  if(dx_p2 == 0.0) b_start = ceil(l_mid - (b_width/2.0));
  else b_start = ceil(l_mid + ((y_mid - s_off)/dx_p2)- (b_width/2.0));
  if(pt->debug) fprintf(stdout, "(DEBUG) %0.12e\n",((y_mid - s_off)*dx_p2));
  //double b_start = ceil((z_ymax-s_off)*dx_p2 - (b_width/2.0));
  if(pt->debug) fprintf(stdout, "(DEBUG) soff=%0.12e\n",s_off);
  if(pt->debug) fprintf(stdout, "(DEBUG) %0.12e %0.12e %g %g %g\n",z_ymin,
                                    z_ymax,
                                    dx_p2,
                                    b_start,
                                    sdx2);
    
  b_start -= (double)pt->ypan_bar.ar_width/2.0;
  b_width += (double)pt->ypan_bar.ar_width;
  
  // Need to revisit these fail safes as I don't believe their response is relevant anymore
  if(b_start < 0.0) b_start=0.0;
  
  if((double)pt->ypan_bar.drag_y1 + b_start + b_width-1 > (double)pt->ypan_bar.drag_y2) {
    if(pt->debug) fprintf(stdout, "(DEBUG) Hit end of y %d %d\n",pt->ypan_bar.drag_y1,pt->ypan_bar.drag_y2);
    if(b_start == 0.0){
      b_width = sdx2+(double)pt->ypan_bar.ar_width;
    } else {
      //b_start = ((double)pt->ypan_bar.drag_y2-b_width-(double)pt->ypan_bar.drag_y1)+1.0;
      b_width = sdx2+(double)pt->ypan_bar.ar_width-b_start;
    }
  }
  
  // Keep track of the dimensions of the handle so we can tell
  // if the user has clicked it later
  pt->ypan_bar.hndl.x1 = pt->ypan_bar.drag_x1;
  pt->ypan_bar.hndl.y1 = pt->ypan_bar.drag_y1 + b_start;
  pt->ypan_bar.hndl.x2 = pt->ypan_bar.hndl.x1+pt->ypan_bar.ar_width;
  pt->ypan_bar.hndl.y2 = pt->ypan_bar.hndl.y1 + b_width;
  
  //draw_bevel_box(pt,pt->ypan_bar.hndl_x1,pt->ypan_bar.hndl_y1,(unsigned int)b_width,pt->ypan_bar.ar_width,0);
  draw_bevel_box(pt,&(pt->ypan_bar.hndl));
  return;
}

void zoom_npix(PLOT_T *pt,int x1, int y1, int x2, int y2){
  // We need to do some bounds checking on the input values
  double x1_val,y1_val,x2_val,y2_val;
  
  // Let's make sure that the zoom actually covers our
  // plotting field
  if(((x2 > pt->grid_x && x2 < pt->grid_x2) && (y2 > pt->grid_y && y2 < pt->grid_y2)) ||
     ((x2 > pt->grid_x && x2 < pt->grid_x2) && (y1 > pt->grid_y && y1 < pt->grid_y2)) ||
     ((x1 > pt->grid_x && x1 < pt->grid_x2) && (y2 > pt->grid_y && y2 < pt->grid_y2)) ||
     ((x1 > pt->grid_x && x1 < pt->grid_x2) && (y1 > pt->grid_y && y1 < pt->grid_y2))){
  
    // First we need to determine our realworld coordinates
    xy2rw(pt,x1,&x1_val,y1,&y1_val);
    xy2rw(pt,x2,&x2_val,y2,&y2_val);
  
    // Increment our zoom level and set the xmin,xmax,ymin and ymax values
    pt->zoom_level+=1;
    set_min_max(pt,&x1_val,&x2_val,&y2_val,&y1_val,pt->zoom_level);
  
    // Redraw the plotter
    refresh_plot(pt);
  }
  
  return;
}

int in_pan(PLOT_T *pt,int x, int y){
  int rtn=0;
  
  // Check the X pan 
  if(x > pt->xpan_bar.x1 && x < pt->xpan_bar.x2 && y > pt->xpan_bar.y1 && y < pt->xpan_bar.y2) rtn=1;
  // Check the y pan 
  if(x > pt->ypan_bar.x1 && x < pt->ypan_bar.x2 && y > pt->ypan_bar.y1 && y < pt->ypan_bar.y2) rtn=2;
  
  return(rtn);
}

int on_pan_handle(PLOT_T *pt,int x, int y){
  int rtn=0;
  
  // Check the X pan 
  if(x > pt->xpan_bar.hndl.x1 && x < pt->xpan_bar.hndl.x2 && y > pt->xpan_bar.hndl.y1 && y < pt->xpan_bar.hndl.y2) rtn=1;
  // Check the y pan 
  if(x > pt->ypan_bar.hndl.x1 && x < pt->ypan_bar.hndl.x2 && y > pt->ypan_bar.hndl.y1 && y < pt->ypan_bar.hndl.y2) rtn=2;
  
  return(rtn);
}

int on_btn(PLOT_T *pt,int x, int y){
  int rtn=0;
  int btn;
  
  // Look over all the buttons to see if our x and y
  // coords fit within any of them  
  for(btn=0;btn<NBTNS;btn++){
    if(x > pt->buttons[btn].x1 && x < pt->buttons[btn].x2 && y > pt->buttons[btn].y1 && y < pt->buttons[btn].y2){
      rtn=1;
      break;
    }
  }
  
  return(rtn);
}

int in_grid(PLOT_T *pt, int x, int y){
  int rtn=0;
  if(x >= pt->grid_x-2 && x <= pt->grid_x2+2 && y >= pt->grid_y-2 && y <= pt->grid_y2+2) rtn=1;
  
  return(rtn);
}

int in_legend(PLOT_T *pt, int x, int y){
  int rtn=0;
  
  if(pt->legend_up){
    if(x > pt->legend_pmap.x1 && x < pt->legend_pmap.x1+pt->legend_pmap.width && y > pt->legend_pmap.y1 && y < pt->legend_pmap.y1+pt->legend_pmap.height) rtn=1;
  }
  
  return(rtn);
}

void hilite_btn(PLOT_T *pt, int x, int y){
  int btn;
  
  for(btn=0;btn<NBTNS;btn++){
    if(x > pt->buttons[btn].x1 && x < pt->buttons[btn].x2 && y > pt->buttons[btn].y1 && y < pt->buttons[btn].y2){
      // Draw the box 
      if(!(pt->buttons[btn].hlit)){
        pt->buttons[btn].hlit=1;
        draw_bevel_box(pt,&(pt->buttons[btn]));
      }
      break;
    }
  }
  
  return;
}

void un_hilite_btns(PLOT_T *pt){
  int btn;
  
  for(btn=0;btn<NBTNS;btn++){
    if(pt->buttons[btn].hlit){
      // Un-hightlight the box
      pt->buttons[btn].hlit=0;
      draw_bevel_box(pt,&(pt->buttons[btn]));
      break;
    }
  }
  return;
}

void btn_clicked(PLOT_T *pt, int x, int y){
  int btn;
  
  for(btn=0;btn<NBTNS;btn++){
    if(x > pt->buttons[btn].x1 && x < pt->buttons[btn].x2 && y > pt->buttons[btn].y1 && y < pt->buttons[btn].y2){
      // Draw the box 
      if(pt->buttons[btn].pressed) pt->buttons[btn].pressed=0;
      else pt->buttons[btn].pressed=1;
      
      draw_bevel_box(pt,&(pt->buttons[btn]));
      
      if(btn == LAYER_OPTS) {
        if(pt->buttons[btn].pressed) show_legend(pt);
        else hide_legend(pt);
        
        pt->legend_up = pt->buttons[btn].pressed;
      }
      break;
    }
  }
  return;
}

void legend_clicked(PLOT_T *pt, int x, int y){
  int ydelta;
  
  // First, let's determine which file was clicked on
  ydelta = y - (pt->buttons[LAYER_OPTS].y1+pt->widget_spacing);
  ydelta = ydelta/(pt->widget_spacing+pt->font_height);
  
  printf("Clicked file %d\n",ydelta);
  
  return;
}

// Function to zoom using realworld coordinates
void zoom_nrw(PLOT_T *pt,double x1, double y1, double x2, double y2){
  // We need to do some bounds checking on the input values
  
  // Increment our zoom level and set the xmin,xmax,ymin and ymax values
  pt->zoom_level+=1;
  set_min_max(pt,&x1,&x2,&y2,&y1,pt->zoom_level);
  
  // Redraw the plotter
  refresh_plot(pt);
  
  return;
}

void zoom_out(PLOT_T *pt){
  if(pt->zoom_level > 0){
    pt->zoom_level-=1;
    
    // Redraw the plotter
    refresh_plot(pt);
  }
  
  return;
}

void load_font(PLOT_T *pt){
  char fontname[] = "*COURIER-MEDIUM-R-NORMAL--*-120-75-75-M-*";
  
  if((pt->font_info = XLoadQueryFont(pt->display,fontname)) == NULL){
    fprintf(stderr,"Cannot open %s font\n",fontname);
    pt->font_height=-1;
  } else {
    // Set the font
    XSetFont(pt->display,pt->gc_text,pt->font_info->fid);
    
    // Record the height and width
    pt->font_height = pt->font_info->ascent + pt->font_info->descent;
    pt->font_width = pt->font_info->max_bounds.rbearing; // maybe width instead????
  }
}

PLOT_T *alloc_plot(){
  PLOT_T *pt=NULL;
  int i;
  double tx1,tx2,ty1,ty2;
  
  if((pt = (PLOT_T *)calloc(1,sizeof(PLOT_T))) == NULL){
    fprintf(stderr, "Could not allocate space for plot structure\n");
  } else {
    /** Initialize all the members **/
    pt->vis_info=NULL;
    pt->display=NULL;
    pt->font_info=NULL;
    pt->cmap=0;
    pt->font_height=-1;
    pt->win_width=-1;
    pt->win_height=-1;
    pt->widget_spacing=8;
    pt->invert_bg_fg=0;
    pt->read_all=0;
    
    pt->legend_pmap.pmap=0;
    pt->legend_up=0;
    
    // Set default color palette
    for(i=0;i<MAX_COLOR;i++) pt->color_palette[i] = def_color[i];
    
    // Default the number to read
    pt->max_n2read = MAX_READ;
    
    // Flag that this is the initial startup so those routines
    // which pay attention to this will listen
    pt->initial_startup=1;
    
    // Initially, turn debug off
    pt->debug=0;
    
    // Default the user defined x values to be in INDEX
    pt->udef_xunits=0;
    
    // Default the scale modes
    pt->xscale_mode=0;
    pt->yscale_mode=0;
    
    // zero out the user definable values
    pt->udef_flags = 0;
    
    // Default the plot xdelta
    pt->plot_xdelta = 1.0;
    
    // Default to unframed data 
    pt->framed_data = 0;
    
    // Initialize all array pertaining to the layers
    for(i=0;i<MAX_FILES;i++) {
      pt->layer_info[i].layer_cval=i;
      pt->layer_info[i].line = 1;    // Whether to use lines to contect the values. Also doubles as pixel width for the line
      pt->layer_info[i].symbol = -1; // Whether to use symbols (with no connecting line)
      pt->layer_info[i].radius = 3;  // Radius of the symbol if it pertains.
      
      pt->flist[i] = NULL;
    }
    pt->nfiles = 0;
    
    sprintf(pt->ylabel,"None (U)");
    sprintf(pt->xlabel,"Unknown (U)\0");
    
    pt->cx_mode=DEF_CX_MODE; // Default to Real
    pt->specs_mode=0; // Default to Abscissa
    pt->zoom_level=0;
    
    // Initialize the zoom levels
    tx1=-1.0;
    tx2=1.0;
    ty1=-1.04;
    ty2=1.04;
    set_min_max(pt,&tx1,&tx2,&ty1,&ty2,-1);
    
    // Initialize some mouse variables
    pt->mouse_xmrk = 0;
    pt->mouse_ymrk = 0;
    pt->mouse_ingrid = 0;
    
    pt->abscissa_min = 0.0;
    pt->abscissa_max = 0.0;
    
    // Default the y-axis scale factor
    pt->y_scale_factor = 0.04;
    
    // Default the line reduction
    pt->reduce_lines = 1;
    
    // Default crosshair 
    pt->crosshair_on = 0;
    for(i=0;i<4;i++){
      pt->crosshair_lines[i].x1=0;
      pt->crosshair_lines[i].y1=0;
      pt->crosshair_lines[i].x2=0;
      pt->crosshair_lines[i].y2=0;
    }
    
    // Set all buttons initial values
    for(i=0;i<NBTNS;i++){
      pt->buttons[i].hlit=0;
      pt->buttons[i].pressed=0;
      pt->buttons[i].is_button=1;
    }
  }
  
  return pt;
}

void incr_specs_mode(PLOT_T *pt){
  pt->specs_mode += 1;
  pt->specs_mode = pt->specs_mode%MAX_SPECS;
  
  // Update only the area displaying the specs
  update_specs(pt);
  
  return;
}

void scale_xaxis(PLOT_T *pt){
  int i;
  long j,skip;
  double xmin,xmax;
  
  // In I/R mode we'll use all files to scale the X-Axis
  if(pt->cx_mode == 4){
    for(i=0;i<pt->nfiles;i++){
      if(pt->flist[i]->format[0] == 'C') skip=2;
      else skip=1;
      
      if(i==0){
        xmin = pt->flist[i]->data_ptr.double_ptr[0];
        xmax = pt->flist[i]->data_ptr.double_ptr[0];
        for(j=1;j<pt->flist[i]->nelms*pt->flist[i]->frame_size;j++){
          if(pt->xscale_mode == 0 || j < MIN(MAX_READ,pt->flist[i]->nelms)){
            if(xmin > pt->flist[i]->data_ptr.double_ptr[j*skip]) pt->x_min = xmin = pt->flist[i]->data_ptr.double_ptr[j*skip];
            if(xmax < pt->flist[i]->data_ptr.double_ptr[j*skip]) pt->x_max = xmax = pt->flist[i]->data_ptr.double_ptr[j*skip];
          } else {
            if(pt->x_min > pt->flist[i]->data_ptr.double_ptr[j*skip]) pt->x_min = pt->flist[i]->data_ptr.double_ptr[j*skip];
            if(pt->x_max < pt->flist[i]->data_ptr.double_ptr[j*skip]) pt->x_max = pt->flist[i]->data_ptr.double_ptr[j*skip];
          }
        }
      } else {
        for(j=0;j<pt->flist[i]->nelms*pt->flist[i]->frame_size;j++){
          if(pt->xscale_mode == 0 || j < MIN(MAX_READ,pt->flist[i]->nelms)){
            if(xmin > pt->flist[i]->data_ptr.double_ptr[j*skip]) pt->x_min = xmin = pt->flist[i]->data_ptr.double_ptr[j*skip];
            if(xmax < pt->flist[i]->data_ptr.double_ptr[j*skip]) pt->x_max = xmax = pt->flist[i]->data_ptr.double_ptr[j*skip];
          } else {
            if(pt->x_min > pt->flist[i]->data_ptr.double_ptr[j*skip]) pt->x_min = pt->flist[i]->data_ptr.double_ptr[j*skip];
            if(pt->x_max < pt->flist[i]->data_ptr.double_ptr[j*skip]) pt->x_max = pt->flist[i]->data_ptr.double_ptr[j*skip];
          }
        }
      }
    }
  } else {
    // For time series data we'll use the first file to determine
    // the X-Axis scale
    if(pt->nfiles > 0){
      pt->x_min = xmin = pt->flist[0]->xstart;
      pt->x_max = xmax = pt->flist[0]->xstart + (pt->plot_xdelta*(double)(pt->framed_data == 0 ? (pt->flist[0]->nelms-1L) : (pt->flist[0]->frame_size-1)));
    }
  }
  
  // Regardless of what's been done above, if this is the first run
  // then attempt to honor any values supplied by the user
  if(pt->initial_startup == 1){
    if(pt->framed_data){
    }
    else {
      // User defined XMIN/XMAX
      if(pt->udef_flags&USER_DEF_START){
        // If the user defined an end, make sure the start is before the end
        if(pt->udef_flags&USER_DEF_END){
          if(pt->u_start < pt->u_end){
            // Great, take both
            xmin=pt->u_start;
            xmax=pt->u_end;
          } else fprintf(stderr, "%s: User supplied START is >= user supplied END, autoscaling\n",pt->name);
        } else {
          // Make sure the start is within the available range
          if(pt->u_start >= pt->abscissa_min && pt->u_start <= pt->abscissa_max) {
            xmin=pt->u_start;
            xmax=pt->u_start + pt->max_n2read*pt->plot_xdelta;
          } else fprintf(stderr, "%s: User supplied START is not in a valid range for the files given, autoscaling\n",pt->name);
        }
      }
      else if(pt->udef_flags&USER_DEF_END){
          // Make sure the end is within the available range
          if(pt->u_end >= pt->abscissa_min && pt->u_end <= pt->abscissa_max) xmax=pt->u_end;
          else fprintf(stderr, "%s: User supplied END is not in a valid range for the files given, autoscaling\n",pt->name);
      }
    }
  }
      
  // Set the x,y min/max for the first zoom level
  if(pt->debug) fprintf(stdout,"(DEBUG) xmin=%f xmax=%f\n",xmin,xmax);
  
  // We're only doing this for the first level as scaling
  // of the x-axis only takes place during the initial
  // startup phase and when the cx mode is changed from
  // or to I/R
  set_min_max(pt,&xmin,
                 &xmax,
                 NULL,
                 NULL,
                 0);
  
  return;
}

void scale_yaxis(PLOT_T *pt, int level, double scale_val){
  int i;
  long j;
  double ymin,ymax;
  
  // Removed the ability to simply scale the y-axis as this was becoming more trouble
  // than it was worth (it would be worth revisiting when looking at large data sets as this would no doubt help improve performance)
  //if(scale_val == 0.0){
    for(i=0;i<pt->nfiles;i++){      
      // Let's find the max and the min y value and autoscale
      // the y axis. Here, we're looking for the smallest x,y min
      // amongst all the layers and the largest x,y max amongst all
      // the layers.
      if(i == 0){
        ymin = pt->ydata[i][0];
        ymax = pt->ydata[i][0];
        for(j=1;j<(pt->yscale_mode == 0 ? pt->flist[i]->nelms*pt->flist[i]->frame_size:MIN(MAX_READ,pt->flist[i]->nelms));j++){
          if(pt->ydata[i][j] > ymax) ymax = pt->ydata[i][j];
          if(pt->ydata[i][j] < ymin) ymin = pt->ydata[i][j];
        }
      } else {
        for(j=0;j<(pt->yscale_mode == 0 ? pt->flist[i]->nelms*pt->flist[i]->frame_size:MIN(MAX_READ,pt->flist[i]->nelms));j++){
          if(pt->ydata[i][j] > ymax) ymax = pt->ydata[i][j];
          if(pt->ydata[i][j] < ymin) ymin = pt->ydata[i][j];
        }
      }    
    }
  
    // Regardless of what's been done above, if this is the first run
    // then attempt to honor any values supplied by the user
    if(pt->initial_startup == 1){  
      // User defined YMIN/YMAX
      if(pt->udef_flags&USER_DEF_YMIN){
        // If a YMAX was supplied as well then spot check
        // that the YMAX is actually larger than the YMIN
        if(pt->udef_flags&USER_DEF_YMAX){
          if(pt->u_ymin < pt->u_ymax){
            // Take them both
            ymin=pt->u_ymin;
            ymax=pt->u_ymax;
          } else  fprintf(stderr, "%s: User supplied YMIN is >= user supplied YMAX, autoscaling\n",pt->name);
        } else {
          // Make sure the autoscaled ymax is actually greater than the requested
          // YMIN
          if(pt->u_ymin < ymax) ymin = pt->u_ymin;
          else fprintf(stderr, "%s: User supplied YMIN is invalid for the given data set in the startup CX MODE %s, autoscaling\n",pt->name,cx_mode_name[pt->cx_mode]);
        }
      } 
      else if(pt->udef_flags&USER_DEF_YMAX){
        // make sure the user defined YMAX is greater than the autoscaled YMIN
        if(pt->u_ymax > ymin) ymax = pt->u_ymax;
        else fprintf(stderr, "%s: User supplied YMAX is invalid for the given data set in the startup CX MODE %s, autoscaling\n",pt->name,cx_mode_name[pt->cx_mode]);
      } else {        
        // Give us a little room on the edges
        ymin += ymin * pt->y_scale_factor;
        ymax += ymax * pt->y_scale_factor;
        // Record the initial y min/max
        pt->init_ymin = ymin;
        pt->init_ymax = ymax;
      }
    } else {
      // Give us a little room on the edges
      ymin += ymin * pt->y_scale_factor;
      ymax += ymax * pt->y_scale_factor;
    }  
      
    // Set the x,y min/max for the first zoom level
    if(pt->debug) fprintf(stdout,"(DEBUG) ymin=%f ymax=%f\n",ymin,ymax);
    if(pt->cx_mode == 4){
      // This happens when I/R mode is selected against a scalar data set
      // Here we're simply setting the scale for plotting purposes
      if(ymin == 0.0 && ymax == 0.0){
        ymin = -1.0;
        ymax = 1.0;
      }
    }
  
    // Scale the y-axis for all zoom levels. We do this because
    // the y-axis is scaled on startup and when changing cx modes.
    // Doing it this way makes sure that if the user is zoomed in
    // when he/she changes the cx mode, then the data will still
    // be displayed properly
    if(level == -1){
      for(i=pt->zoom_level;i>=0;i--){
        set_min_max(pt,NULL,
                 NULL,
                 &ymin,
                 &ymax,
                 i);
      }
    } else {
      set_min_max(pt,NULL,
                 NULL,
                 &ymin,
                 &ymax,
                 level);
    }
  /*} else {
    pt->y_min = (pt->zoom_info[0].ymin-pt->ymin_offset)*scale_val;
    pt->y_max = (pt->zoom_info[0].ymax-pt->ymax_offset)*scale_val;
    if(level == -1){
      for(i=pt->zoom_level;i>=0;i--){
        ymin = (pt->zoom_info[i].ymin*scale_val);
        ymax = (pt->zoom_info[i].ymax*scale_val);
        set_min_max(pt,NULL,
                 NULL,
                 &ymin,
                 &ymax,
                 i);
      }
    } else {
      ymin = (pt->zoom_info[level].ymin*scale_val);
      ymax = (pt->zoom_info[level].ymax*scale_val);
        
      set_min_max(pt,NULL,
                 NULL,
                 &ymin,
                 &ymax,
                 level);
    }
  }*/
  
  return;
}

void chnge_cx_mode(PLOT_T *pt, int mode){
  int i,omode;
  
  if(mode < MAX_CX_MODE){
    if(mode != pt->cx_mode){
      omode = pt->cx_mode;
      pt->cx_mode=mode;
      
      // Convert the y data points to the new mode
      for(i=0;i<pt->nfiles;i++){
        convert_ydata(pt,i);
      }
      
      // Scale the x axis only if we're moving from or to
      // I/R mode
      if(mode == 4 || omode == 4) {
        // You lose your zoom levels when changing from or
        // to I/R mode
        pt->zoom_level=0;
        
        // We must rescale the X axis when changing from or
        // to I/R mode
        scale_xaxis(pt);
      }
      
      // Scale the y axis
      // If we're going from Real to Imag or vice versa then only full scale the first zoom level
      if((mode == 2 && omode == 3) || (mode == 3 && omode == 2)) scale_yaxis(pt,0,0.0);
      
      // If we're going from 10*log10 to 20*log10 or vice versa simply apply a scale factor to all zoom levels
      else if((mode == 5 && omode == 6)) scale_yaxis(pt,-1,0.5);
      else if((mode == 6 && omode == 5)) scale_yaxis(pt,-1,2.0);
      
      // Otherwise full autoscale all zoom levels to fit the new data
      else scale_yaxis(pt,-1,0.0);
    
      // For a mode change we need to update the entire plotting area 
      //refresh_plot(pt); 
      refresh_window(pt);
    }
  } else {
    fprintf(stderr,"%s: Invalid mode %d passed to chnge_cx_mode\n",pt->name,mode);
  }
  
  return;
}

void incr_cx_mode(PLOT_T *pt){  
  chnge_cx_mode(pt,((pt->cx_mode+1)%MAX_CX_MODE));
  
  return;
}

void decr_cx_mode(PLOT_T *pt){  
  chnge_cx_mode(pt,(pt->cx_mode == 0 ? MAX_CX_MODE-1:(pt->cx_mode-1)));
  
  return;
}

void FreeDisp(PLOT_T *pt){
  int i;
  if(pt != NULL){
    if(pt->display != NULL) {
      /* Note: We must free the font before freeing the GC or you will have memory problems */
      if(pt->font_info != NULL) XFreeFont(pt->display,pt->font_info);
      XFreeGC(pt->display,pt->gc_xor);
      XFreeGC(pt->display,pt->gc_text);
      XFreeGC(pt->display,pt->gc_data);
      XFreeGC(pt->display,pt->gc_widget);
  
      if(pt->vis_info != NULL) free(pt->vis_info);
      
      if(pt->legend_pmap.pmap != 0) XFreePixmap(pt->display,pt->legend_pmap.pmap);
      
      XCloseDisplay(pt->display);
      
      // Free the file list
      for(i=0;i<pt->nfiles;i++) {
        fio_close(pt->flist[i]);
        if(pt->ydata[i] != NULL) free(pt->ydata[i]);
      }
    }
    free(pt);
  }
}

void draw_lines(PLOT_T *pt,XPoint *points,int npoints){
  int i,j;
  long max_request;
  int reduction=0;
  double ymin=0.0,ymax=0.0;
  short xpix;
  int np=0,num_same=1;
  
  int p_chk=5;
  
  // Figure out the max number of line we can request
  // Need to put a server request in to figure out what verison
  // of the X server we're running. The XExtendedMaxRequestSize was
  // introduced in X11R6 v2.0. WM's are not guaranteed to actual 
  // support an extended request, in which case XMaxRequestSize will
  // return the same value.
  max_request = XMaxRequestSize(pt->display);
  
  // Just because you can doesn't mean you should. In order to get a quicker response it's best to use the smaller request size
  //max_request = XExtendedMaxRequestSize(pt->display);
  max_request = (max_request-3)/2;
  
  if (npoints > p_chk) for (i=1,j=0; i<p_chk && (j+=abs(points[i].x-points[i-1].x))<2; i++);
  else i = 0;
  
  if(i==p_chk && pt->reduce_lines){
    XPoint *tp = (XPoint *) calloc(npoints,sizeof(XPoint));
    
    // A quick and dirty method of reducing the number of lines drawn. This
    // can significantly reduce the load on the X server. The basic idea is if
    // 3 or more points share the same x pixel coordinate, then simply record
    // the ymin and ymax values thus reducing many points down to just 2.
    for(i=1;i<npoints;i++){
      if((points[i].x - points[i-1].x) == 0){
        if(num_same == 1){
          ymin = points[i-1].y;
          ymax = points[i-1].y;
        } else {
          if(points[i-1].y > ymax) ymax = points[i-1].y;
          if(points[i-1].y < ymin) ymin = points[i-1].y;
        }
        xpix = points[i-1].x;
        num_same++;
      } 
      
      if((points[i].x - points[i-1].x) != 0 || i == npoints-1) {
        if(num_same > 2){
          if(points[i-1].y > ymax) ymax = points[i-1].y;
          if(points[i-1].y < ymin) ymin = points[i-1].y;
          
          // Make sure we take the y value of the last point
          // into account for our ymin and ymax if it was part
          // of the same x pixel coordinate as the previous
          if((points[i].x - points[i-1].x) == 0 && i == npoints-1){
            if(points[i].y > ymax) ymax = points[i].y;
            if(points[i].y < ymin) ymin = points[i].y;
          }
        
          // Reduce these points to 2
          tp[np].x = xpix;
          tp[np].y = ymin;
          np++;
          tp[np].x = xpix;
          tp[np].y = ymax;
          np++;
        } else {
          // Record the current point
          tp[np].x = points[i-1].x;
          tp[np].y = points[i-1].y;
          np++;
        }
        
        // This is to catch the last point if it did not share
        // an x pixel with the previous point
        if((points[i].x - points[i-1].x) != 0 && i == npoints-1){
          tp[np].x = points[i-1].x;
          tp[np].y = points[i-1].y;
          np++;
        }
        num_same=1;
      }
    }
    //if(pt->debug) fprintf(stdout, "(DEBUG) Reduced %d to %d\n",npoints,np);
    
    for(i=0;i<np;i+=max_request){
      XDrawLines(pt->display,pt->win,pt->gc_data,&(tp[i]),min(np-i,max_request),CoordModeOrigin);
    }
    
    free(tp);
  }
  else {  
    for(i=0;i<npoints;i+=max_request){
      XDrawLines(pt->display,pt->win,pt->gc_data,&(points[i]),min(npoints-i,max_request),CoordModeOrigin);
    }
  }
};

void draw_symbols(PLOT_T *pt,XPoint *points,int npoints, int layer){
  int i;
  long max_request;
  int radius,diam;
    
  // Figure out the max number of line we can request
  // Need to put a server request in to figure out what verison
  // of the X server we're running. The XExtendedMaxRequestSize was
  // introduced in X11R6 v2.0. WM's are not guaranteed to actual 
  // support an extended request, in which case XMaxRequestSize will
  // return the same value. This part is a little bit trickier in
  // that XDrawArc and XDrawPoints don't have to listen to the
  // max request as they may break it up themselves depending on
  // server implementation.
  max_request = XMaxRequestSize(pt->display);
  // Just because you can doesn't mean you should. In order to get a quicker response it's best to use the smaller request size
  max_request = XExtendedMaxRequestSize(pt->display);
  max_request = max_request-3;
  
  //if(pt->debug) fprintf(stdout, "(DEBUG) Max request=%ld\n",max_request);
  
  radius = pt->layer_info[layer].radius;
  
  if(radius > 1){
    diam = radius*2;
    
    // Circles
    if(pt->layer_info[layer].symbol == 0){
      for(i=0;i<npoints;i++) XDrawArc(pt->display, pt->win, pt->gc_data, points[i].x-radius, points[i].y-radius, diam, diam, 0, 23040);
    }
    // Plusses
    else if(pt->layer_info[layer].symbol == 1){
      for(i=0;i<npoints;i++){
        XDrawLine(pt->display,pt->win,pt->gc_data,points[i].x,points[i].y+radius,points[i].x,points[i].y-radius);
        XDrawLine(pt->display,pt->win,pt->gc_data,points[i].x+radius,points[i].y,points[i].x-radius,points[i].y);
      }
    }
    // X's
    else if(pt->layer_info[layer].symbol == 2){
      for(i=0;i<npoints;i++){
        XDrawLine(pt->display,pt->win,pt->gc_data,points[i].x-radius,points[i].y-radius,points[i].x+radius,points[i].y+radius);
        XDrawLine(pt->display,pt->win,pt->gc_data,points[i].x+radius,points[i].y-radius,points[i].x-radius,points[i].y+radius);
      }
    }
  } else {
    // Single pixel
    for(i=0;i<npoints;i+=max_request) XDrawPoints(pt->display,pt->win,pt->gc_data,&(points[i]),min(npoints-i,max_request),CoordModeOrigin);
  }
};

int clipt(double denom, double num, double *tE, double *tL){
  int rtn=1;
  double t;
  
  if(denom > 0.0){
    t = num/denom;
    
    if(t > *tL) rtn=0;
    else if(t > *tE) *tE = t;
  }
  else if(denom < 0.0) {
    t = num/denom;
    if(t < *tE) rtn=0;
    else if(t < *tL) *tL=t;
  }
  else {
    if(num > 0.0) rtn=0;
  }
  return rtn;
}

void plot_data(PLOT_T *pt){
  int i,k;
  long j,skip;
  int lvisible,ib,visible;
  int left,top;
  double x,y,lx,ly,xmin,xmax,ymin,ymax;
  double xxmin,yymin,xscl,yscl,dx,dy,tL,tE;
  XColor screen_def,exact_def;
  
  left = pt->grid_x;
  top = pt->grid_y;
  
  xxmin = pt->zoom_info[pt->zoom_level].xmin;
  xscl = 1.0/pt->zoom_info[pt->zoom_level].x_p2r;
  
  yymin = pt->zoom_info[pt->zoom_level].ymin;
  yscl = 1.0/pt->zoom_info[pt->zoom_level].y_p2r;
    
  dx = fabs(pt->zoom_info[pt->zoom_level].xmax - pt->zoom_info[pt->zoom_level].xmin);
  dy = fabs(pt->zoom_info[pt->zoom_level].ymax - pt->zoom_info[pt->zoom_level].ymin);
  
  xmin = pt->zoom_info[pt->zoom_level].xmin;
  ymin = pt->zoom_info[pt->zoom_level].ymin;
  xmax = xmin + dx;
  ymax = ymin + dy;
  
  for(i=0;i<pt->nfiles;i++){
    if(pt->flist[i]->format[0] == 'C') skip=2;
    else skip=1;
    
    if(pt->flist[i]->framed){
    
      // Allocate the points buffer
      XPoint *points = (XPoint *) calloc(pt->flist[i]->frame_size,sizeof(XPoint));
    
      // Initialize the number of elements in view
      pt->flist[i]->nelms_vis=0;
      
      for(k=0;k<pt->flist[i]->nelms;k++){
        // Set the color for this layer    
        XSetForeground(pt->display,pt->gc_data,pt->layer_info[k].layer_pixel);
    
        ib = 0;
    
        if(pt->cx_mode == 4) x = pt->flist[i]->data_ptr.double_ptr[0];
        else x = pt->flist[i]->current_xstart;
        y = pt->ydata[i][k*pt->flist[i]->frame_size];
        lvisible = (x >= xmin && x <= xmax && y >= ymin && y <= ymax);
    
        if(lvisible){
          points[ib].x = ((int)round((x - xxmin)*xscl)) + left;
          points[ib].y = ((int)round((y - ymax)*yscl)) + top;
          //if(pt->debug) fprintf(stdout, "(DEBUG) x=%d y=%d x=%f y=%f\n",points[ib].x,points[ib].y,x,y);
          ib++;
          if(pt->layer_info[k].symbol > -1) {
            draw_symbols(pt,points,ib,i);
            //pt->flist[i]->nelms_vis += ib;
          }
        }
    
        for(j=1;j<pt->flist[i]->frame_size;j++){
          lx = x;
          ly = y;
      
          if(pt->cx_mode == 4) x = pt->flist[i]->data_ptr.double_ptr[j*skip];
          else x = x+pt->flist[i]->xdelta;
          y = pt->ydata[i][(k*pt->flist[i]->frame_size) + j];
      
          visible = (x >= xmin && x <= xmax && y >= ymin && y <= ymax);
      
          // Check for clipping
          if(lvisible && visible){
            // No clipping
            points[ib].x = ((int)round((x - xxmin)*xscl)) + left;
            points[ib].y = ((int)round((y - ymax)*yscl)) + top;
            //if(pt->debug) fprintf(stdout, "(DEBUG) x=%d y=%d x=%f y=%f\n",points[ib].x,points[ib].y,x,y);
            ib++;
          } else {
            //if(pt->debug) fprintf(stdout, "(DEBUG) Clipping requred %d\n",j);
            // Clipping needed
            lvisible = visible;
            dx = lx - x;
            dy = ly - y;
        
            if(dx != 0.0 || dy != 0.0){
              tL = 0.0;
              tE = 1.0;
          
              if(clipt(dx,xmin-x,&tL,&tE)) { // inside xmin edge
                if(clipt(-dx,x-xmax,&tL,&tE)) { // inside xmax edge
                  if(clipt(dy,ymin-y,&tL,&tE)) { // inside ymin edge
                    if(clipt(-dy,y-ymax,&tL,&tE)) { // inside ymax edge
                      //if(pt->debug) fprintf(stdout, "(DEBUG) In\n");
                      if(tE < 1.0) { // new entry point
                        //if(pt->debug) fprintf(stdout, "(DEBUG) New entry 1\n");
                        points[ib].x = (int)(round((x - xxmin + tE*dx)*xscl)) + left;
                        points[ib].y = (int)(round((y - ymax + tE*dy)*yscl)) + top;
                        ib++;
                      }
                  
                      if(tL > 0.0) { // new exit point, draw lines
                        //if(pt->debug) fprintf(stdout, "(DEBUG) New exit\n");
                        //if(pt->debug) fprintf(stdout, "(DEBUG) %f\n",tL);
                        points[ib].x = (int)(round((x - xxmin + tL*dx)*xscl)) + left;
                        points[ib].y = (int)(round((y - ymax + tL*dy)*yscl)) + top;
                        ib++;
                        if(pt->layer_info[k].line > -1) {
                          draw_lines(pt,points,ib);
                          //pt->flist[i]->nelms_vis += ib;
                        }
                        if(pt->layer_info[k].symbol > -1 && ib > 2) {
                          draw_symbols(pt,&(points[1]),ib-2,i);
                          //pt->flist[i]->nelms_vis += (ib-2);
                        }
                        ib=0;
                      } else { // new entry point set, continue
                        //if(pt->debug) fprintf(stdout, "(DEBUG) New entry 2\n");
                        points[ib].x = (int)(round((x - xxmin)*xscl)) + left;
                        points[ib].y = (int)(round((y - ymax)*yscl)) + top;
                        ib++;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      
        if(ib > 0) {
          if(pt->layer_info[k].line > -1) {
            draw_lines(pt,points,ib);
            //pt->flist[i]->nelms_vis += ib;
          }
          else if(pt->layer_info[k].symbol > -1 && ib > 2) {
            draw_symbols(pt,&(points[1]),ib-2,i);
            //pt->flist[i]->nelms_vis += (ib-2);
          }
        }
      }
    
      free(points);
    } else {
    
    // Initialize the number of elements in view
    pt->flist[i]->nelms_vis=0;
    
    // Set the color for this layer    
    XSetForeground(pt->display,pt->gc_data,pt->layer_info[i].layer_pixel);
    
    ib = 0;
    
    // Allocate the points buffer
    XPoint *points = (XPoint *) calloc(pt->flist[i]->nelms*pt->flist[i]->frame_size,sizeof(XPoint));
    
    if(pt->cx_mode == 4) x = pt->flist[i]->data_ptr.double_ptr[0];
    else x = pt->flist[i]->current_xstart;
    y = pt->ydata[i][0];
    lvisible = (x >= xmin && x <= xmax && y >= ymin && y <= ymax);
    
    if(lvisible){
      points[ib].x = ((int)round((x - xxmin)*xscl)) + left;
      points[ib].y = ((int)round((y - ymax)*yscl)) + top;
      //if(pt->debug) fprintf(stdout, "(DEBUG) x=%d y=%d x=%f y=%f\n",points[ib].x,points[ib].y,x,y);
      ib++;
      if(pt->layer_info[i].symbol > -1) {
        draw_symbols(pt,points,ib,i);
        pt->flist[i]->nelms_vis += ib;
      }
    }
    
    for(j=1;j<pt->flist[i]->nelms*pt->flist[i]->frame_size;j++){
      lx = x;
      ly = y;
      
      if(pt->cx_mode == 4) x = pt->flist[i]->data_ptr.double_ptr[j*skip];
      else x = x+pt->flist[i]->xdelta;
      y = pt->ydata[i][j];
      
      visible = (x >= xmin && x <= xmax && y >= ymin && y <= ymax);
      
      // Check for clipping
      if(lvisible && visible){
        // No clipping
        points[ib].x = ((int)round((x - xxmin)*xscl)) + left;
        points[ib].y = ((int)round((y - ymax)*yscl)) + top;
        //if(pt->debug) fprintf(stdout, "(DEBUG) x=%d y=%d x=%f y=%f\n",points[ib].x,points[ib].y,x,y);
        ib++;
      } else {
        //if(pt->debug) fprintf(stdout, "(DEBUG) Clipping requred %d\n",j);
        // Clipping needed
        lvisible = visible;
        dx = lx - x;
        dy = ly - y;
        
        if(dx != 0.0 || dy != 0.0){
          tL = 0.0;
          tE = 1.0;
          
          if(clipt(dx,xmin-x,&tL,&tE)) { // inside xmin edge
            if(clipt(-dx,x-xmax,&tL,&tE)) { // inside xmax edge
              if(clipt(dy,ymin-y,&tL,&tE)) { // inside ymin edge
                if(clipt(-dy,y-ymax,&tL,&tE)) { // inside ymax edge
                  //if(pt->debug) fprintf(stdout, "(DEBUG) In\n");
                  if(tE < 1.0) { // new entry point
                    //if(pt->debug) fprintf(stdout, "(DEBUG) New entry 1\n");
                    points[ib].x = (int)(round((x - xxmin + tE*dx)*xscl)) + left;
                    points[ib].y = (int)(round((y - ymax + tE*dy)*yscl)) + top;
                    ib++;
                  }
                  
                  if(tL > 0.0) { // new exit point, draw lines
                    //if(pt->debug) fprintf(stdout, "(DEBUG) New exit\n");
                    //if(pt->debug) fprintf(stdout, "(DEBUG) %f\n",tL);
                    points[ib].x = (int)(round((x - xxmin + tL*dx)*xscl)) + left;
                    points[ib].y = (int)(round((y - ymax + tL*dy)*yscl)) + top;
                    ib++;
                    if(pt->layer_info[i].line > -1) {
                      draw_lines(pt,points,ib);
                      pt->flist[i]->nelms_vis += ib;
                    }
                    if(pt->layer_info[i].symbol > -1 && ib > 2) {
                      draw_symbols(pt,&(points[1]),ib-2,i);
                      pt->flist[i]->nelms_vis += (ib-2);
                    }
                    ib=0;
                  } else { // new entry point set, continue
                    //if(pt->debug) fprintf(stdout, "(DEBUG) New entry 2\n");
                    points[ib].x = (int)(round((x - xxmin)*xscl)) + left;
                    points[ib].y = (int)(round((y - ymax)*yscl)) + top;
                    ib++;
                  }
                }
              }
            }
          }
        }
      }
    }
      
    if(ib > 0) {
      if(pt->layer_info[i].line > -1) {
        draw_lines(pt,points,ib);
        pt->flist[i]->nelms_vis += ib;
      }
      else if(pt->layer_info[i].symbol > -1 && ib > 2) {
        draw_symbols(pt,&(points[1]),ib-2,i);
        pt->flist[i]->nelms_vis += (ib-2);
      }
    }
    
    free(points);
    }
  }
  
  return;
}

void convert_ydata(PLOT_T *pt, int layer){
  long i,j=0,start,skip;
  double mag2;
  
  if(pt != NULL){
    if(layer < pt->nfiles){
      if(pt->cx_mode == 0) { // Magnitude
        if(pt->flist[layer]->format[0] == 'C'){
          skip=2;
          for(i=0;i<pt->flist[layer]->nelms*pt->flist[layer]->frame_size;i++){
            mag2 = powf(pt->flist[layer]->data_ptr.double_ptr[i*skip],2.0) + powf(pt->flist[layer]->data_ptr.double_ptr[i*skip+1],2.0);
            pt->ydata[layer][i] = sqrt(mag2);
            
            // Use the first sample from the first layer to initialize the y min/max values
            if(i==0 && layer == 0){
              pt->y_min = pt->ydata[layer][i];
              pt->y_max = pt->ydata[layer][i];
            } else {
              if(pt->ydata[layer][i] < pt->y_min) pt->y_min = pt->ydata[layer][i];
              if(pt->ydata[layer][i] > pt->y_max) pt->y_max = pt->ydata[layer][i];
            }
          }
        }
        else {
          for(i=0;i<pt->flist[layer]->nelms*pt->flist[layer]->frame_size;i++){
            mag2 = powf(pt->flist[layer]->data_ptr.double_ptr[i],2.0);
            pt->ydata[layer][i] = sqrt(mag2);
            
            // Use the first sample from the first layer to initialize the y min/max values
            if(i==0 && layer == 0){
              pt->y_min = pt->ydata[layer][i];
              pt->y_max = pt->ydata[layer][i];
            } else {
              if(pt->ydata[layer][i] < pt->y_min) pt->y_min = pt->ydata[layer][i];
              if(pt->ydata[layer][i] > pt->y_max) pt->y_max = pt->ydata[layer][i];
            }
          }
        }
      }
      else if(pt->cx_mode == 1) { // Phase
        if(pt->flist[layer]->format[0] == 'C') {        
          for(i=0;i<pt->flist[layer]->nelms*pt->flist[layer]->frame_size;i++){
            pt->ydata[layer][i] = atan2(pt->flist[layer]->data_ptr.double_ptr[i*2+1],pt->flist[layer]->data_ptr.double_ptr[i*2]) * (180.0/M_PI);
            
            // Use the first sample from the first layer to initialize the y min/max values
            if(i==0 && layer == 0){
              pt->y_min = pt->ydata[layer][i];
              pt->y_max = pt->ydata[layer][i];
            } else {
              if(pt->ydata[layer][i] < pt->y_min) pt->y_min = pt->ydata[layer][i];
              if(pt->ydata[layer][i] > pt->y_max) pt->y_max = pt->ydata[layer][i];
            }
          }
        }
        else {
          //memcpy(pt->ydata[layer],pt->flist[layer]->data_ptr.double_ptr,pt->flist[layer]->nelms*sizeof(double));
          // The memcpy above would be much faster, however, we need to determine the y min/max values
          for(i=0;i<pt->flist[layer]->nelms*pt->flist[layer]->frame_size;i++){
            pt->ydata[layer][i] = pt->flist[layer]->data_ptr.double_ptr[i];
            
            // Use the first sample from the first layer to initialize the y min/max values
            if(i==0 && layer == 0){
              pt->y_min = pt->ydata[layer][i];
              pt->y_max = pt->ydata[layer][i];
            } else {
              if(pt->ydata[layer][i] < pt->y_min) pt->y_min = pt->ydata[layer][i];
              if(pt->ydata[layer][i] > pt->y_max) pt->y_max = pt->ydata[layer][i];
            }
          }
        }
      }
      else if(pt->cx_mode == 2) { // Real
        if(pt->flist[layer]->format[0] == 'C') {        
          for(i=0;i<pt->flist[layer]->nelms*pt->flist[layer]->frame_size;i++){
            pt->ydata[layer][i] = pt->flist[layer]->data_ptr.double_ptr[i*2];
            
            // Use the first sample from the first layer to initialize the y min/max values
            if(i==0 && layer == 0){
              pt->y_min = pt->ydata[layer][i];
              pt->y_max = pt->ydata[layer][i];
            } else {
              if(pt->ydata[layer][i] < pt->y_min) pt->y_min = pt->ydata[layer][i];
              if(pt->ydata[layer][i] > pt->y_max) pt->y_max = pt->ydata[layer][i];
            }
          }
        }
        else {
          //memcpy(pt->ydata[layer],pt->flist[layer]->data_ptr.double_ptr,pt->flist[layer]->nelms*sizeof(double));
          // The memcpy above would be much faster, however, we need to determine the y min/max values
          for(i=0;i<pt->flist[layer]->nelms*pt->flist[layer]->frame_size;i++){
            pt->ydata[layer][i] = pt->flist[layer]->data_ptr.double_ptr[i];
            
            // Use the first sample from the first layer to initialize the y min/max values
            if(i==0 && layer == 0){
              pt->y_min = pt->ydata[layer][i];
              pt->y_max = pt->ydata[layer][i];
            } else {
              if(pt->ydata[layer][i] < pt->y_min) pt->y_min = pt->ydata[layer][i];
              if(pt->ydata[layer][i] > pt->y_max) pt->y_max = pt->ydata[layer][i];
            }
          }
        }
      }
      else if(pt->cx_mode == 3) { // Imaginary
        if(pt->flist[layer]->format[0] == 'C') {
        
          for(i=0;i<pt->flist[layer]->nelms*pt->flist[layer]->frame_size;i++){
            pt->ydata[layer][i] = pt->flist[layer]->data_ptr.double_ptr[i*2+1];
            
            // Use the first sample from the first layer to initialize the y min/max values
            if(i==0 && layer == 0){
              pt->y_min = pt->ydata[layer][i];
              pt->y_max = pt->ydata[layer][i];
            } else {
              if(pt->ydata[layer][i] < pt->y_min) pt->y_min = pt->ydata[layer][i];
              if(pt->ydata[layer][i] > pt->y_max) pt->y_max = pt->ydata[layer][i];
            }
          }
        }
        else {
          //memcpy(pt->ydata[layer],pt->flist[layer]->data_ptr.double_ptr,pt->flist[layer]->nelms*sizeof(double));
          // The memcpy above would be much faster, however, we need to determine the y min/max values
          for(i=0;i<pt->flist[layer]->nelms*pt->flist[layer]->frame_size;i++){
            pt->ydata[layer][i] = pt->flist[layer]->data_ptr.double_ptr[i];
            
            // Use the first sample from the first layer to initialize the y min/max values
            if(i==0 && layer == 0){
              pt->y_min = pt->ydata[layer][i];
              pt->y_max = pt->ydata[layer][i];
            } else {
              if(pt->ydata[layer][i] < pt->y_min) pt->y_min = pt->ydata[layer][i];
              if(pt->ydata[layer][i] > pt->y_max) pt->y_max = pt->ydata[layer][i];
            }
          }
        }
      }
      else if(pt->cx_mode == 4) { // I/R
        if(pt->flist[layer]->format[0] == 'C') {
        
          for(i=0;i<pt->flist[layer]->nelms*pt->flist[layer]->frame_size;i++){
            pt->ydata[layer][i] = pt->flist[layer]->data_ptr.double_ptr[i*2+1];
            
            // Use the first sample from the first layer to initialize the y min/max values
            if(i==0 && layer == 0){
              pt->y_min = pt->ydata[layer][i];
              pt->y_max = pt->ydata[layer][i];
            } else {
              if(pt->ydata[layer][i] < pt->y_min) pt->y_min = pt->ydata[layer][i];
              if(pt->ydata[layer][i] > pt->y_max) pt->y_max = pt->ydata[layer][i];
            }
          }
        }
        else {
          pt->y_min = -1.0;
          pt->y_max = 1.0;
          memset(pt->ydata[layer],0x00,pt->flist[layer]->nelms*pt->flist[layer]->frame_size*sizeof(double));
        }
      }
      else if(pt->cx_mode == 5) { // 10*log10
        if(pt->flist[layer]->format[0] == 'C'){
          skip=2;
          for(i=0;i<pt->flist[layer]->nelms*pt->flist[layer]->frame_size;i++){
            mag2 = powf(pt->flist[layer]->data_ptr.double_ptr[i*skip],2.0) + powf(pt->flist[layer]->data_ptr.double_ptr[i*skip+1],2.0);
            if(mag2 > 0.0) pt->ydata[layer][i] = 5.0*log10(mag2);
            else pt->ydata[layer][i] = -100.0;
            
            // Use the first sample from the first layer to initialize the y min/max values
            if(i==0 && layer == 0){
              pt->y_min = pt->ydata[layer][i];
              pt->y_max = pt->ydata[layer][i];
            } else {
              if(pt->ydata[layer][i] < pt->y_min) pt->y_min = pt->ydata[layer][i];
              if(pt->ydata[layer][i] > pt->y_max) pt->y_max = pt->ydata[layer][i];
            }
          }
        }
        else {
          for(i=0;i<pt->flist[layer]->nelms*pt->flist[layer]->frame_size;i++){
            mag2 = fabs(pt->flist[layer]->data_ptr.double_ptr[i]);
            if(mag2 > 0.0) pt->ydata[layer][i] = 10.0*log10(mag2);
            else pt->ydata[layer][i] = -200.0;
            
            // Use the first sample from the first layer to initialize the y min/max values
            if(i==0 && layer == 0){
              pt->y_min = pt->ydata[layer][i];
              pt->y_max = pt->ydata[layer][i];
            } else {
              if(pt->ydata[layer][i] < pt->y_min) pt->y_min = pt->ydata[layer][i];
              if(pt->ydata[layer][i] > pt->y_max) pt->y_max = pt->ydata[layer][i];
            }
          }
        }
      }
      else if(pt->cx_mode == 6) { // 20*log10
        if(pt->flist[layer]->format[0] == 'C'){
          skip=2;
          for(i=0;i<pt->flist[layer]->nelms*pt->flist[layer]->frame_size;i++){
            mag2 = powf(pt->flist[layer]->data_ptr.double_ptr[i*skip],2.0) + powf(pt->flist[layer]->data_ptr.double_ptr[i*skip+1],2.0);
            if(mag2 > 0.0) pt->ydata[layer][i] = 10.0*log10(mag2);
            else pt->ydata[layer][i] = -200.0;
            
            // Use the first sample from the first layer to initialize the y min/max values
            if(i==0 && layer == 0){
              pt->y_min = pt->ydata[layer][i];
              pt->y_max = pt->ydata[layer][i];
            } else {
              if(pt->ydata[layer][i] < pt->y_min) pt->y_min = pt->ydata[layer][i];
              if(pt->ydata[layer][i] > pt->y_max) pt->y_max = pt->ydata[layer][i];
            }
          }
        }
        else {
          for(i=0;i<pt->flist[layer]->nelms*pt->flist[layer]->frame_size;i++){
            mag2 = fabs(pt->flist[layer]->data_ptr.double_ptr[i]);
            if(mag2 > 0.0) pt->ydata[layer][i] = 20.0*log10(mag2);
            else pt->ydata[layer][i] = -400.0;
            
            // Use the first sample from the first layer to initialize the y min/max values
            if(i==0 && layer == 0){
              pt->y_min = pt->ydata[layer][i];
              pt->y_max = pt->ydata[layer][i];
            } else {
              if(pt->ydata[layer][i] < pt->y_min) pt->y_min = pt->ydata[layer][i];
              if(pt->ydata[layer][i] > pt->y_max) pt->y_max = pt->ydata[layer][i];
            }
          }
        }
      }
      
      // Scale factor should only be applied after all layers have
      // been evaluated
      if(layer == pt->nfiles-1){
        pt->y_min += pt->y_min*pt->y_scale_factor;
        pt->y_max += pt->y_max*pt->y_scale_factor;
      }
    }
  }
}

void move_data(PLOT_T *pt, double abs_dist){ 
  double xmin,xmax,delta_dist,max_dist=0.0,z_xmin,z_xmax;
  int nread,i,j;
  int read_data=0,replot=1,any_read=0; 
  
  // Determine what the X min/max will be
  xmin = pt->zoom_info[pt->zoom_level].xmin+abs_dist;
  xmax = pt->zoom_info[pt->zoom_level].xmax+abs_dist;
  
  if(pt->debug) fprintf(stdout, "(DEBUG) abs_dist=%0.24e\n",abs_dist);
  
  // Bind these values to the min and max X values available
  if(xmin < pt->abscissa_min) {
    // Since the X min and max are supposed to move at the same rate
    // then we need to adjust the xmax to fit that of the movement
    // of the xmin
    xmax = xmax + (pt->abscissa_min - xmin);
    xmin = pt->abscissa_min;
  }
  if(xmax > pt->abscissa_max) {
    // Since the X min and max are supposed to move at the same rate
    // then we need to adjust the xmin to fit that of the movement
    // of the xmax
    if(pt->debug) fprintf(stdout, "(DEBUG) xmin=%0.24e xmax=%0.24e\n",xmin,xmax);
    xmin = xmin + (pt->abscissa_max-xmax);
    xmax = pt->abscissa_max;
    if(pt->debug) fprintf(stdout, "(DEBUG) xmin=%0.24e xmax=%0.24e\n",xmin,xmax);
  }
  
  // Go over all the files, reading/converting data from those that require it
  for(j=0;j<pt->nfiles;j++){   
    read_data=0;
  
    // We only need to read data if we've pushed past our current buffer's limits
    if(xmin < pt->flist[j]->current_xstart && pt->flist[j]->current_xstart != pt->flist[j]->xstart){
      if(pt->debug) fprintf(stdout, "(DEBUG) xmin=%0.24e cxs=%0.24e\n",xmin,pt->flist[j]->current_xstart);
      // Pushed past the beginning
      delta_dist = round((xmin-pt->flist[j]->current_xstart)/pt->plot_xdelta);
      if(delta_dist <= 0.0) delta_dist -= 1.0;
        
      /*
      // Make sure we aren't going to go past the start of this particular file
      if(pt->flist[j]->current_xstart + (delta_dist*pt->plot_xdelta) < pt->flist[j]->xstart){
        delta_dist = ceil((pt->flist[j]->xstart - pt->flist[j]->current_xstart)/pt->plot_xdelta);
        if(delta_dist < 0.0) delta_dist -= 1.0;
      }
      */
    }
    else if(xmax > pt->flist[j]->current_xend && pt->flist[j]->current_xend != pt->flist[j]->xend){
      if(pt->debug) fprintf(stdout, "(DEBUG) xmax=%0.24e cxs=%0.24e %d\n",xmax,pt->flist[j]->current_xend,(xmax == pt->flist[j]->current_xend));
      // Pushed past the end
      delta_dist = round((xmax - pt->flist[j]->current_xend)/pt->plot_xdelta);
        
      // Make sure we aren't going past the end of this particular file
      /*
      if(pt->flist[j]->current_xend + (delta_dist*pt->flist[j]->xdelta) > pt->flist[j]->xend){
        delta_dist = ceil((pt->flist[j]->xend - pt->flist[j]->current_xend)/pt->plot_xdelta);
        if(delta_dist > 0.0) delta_dist += 1.0;
      }
      */
      if(pt->debug) fprintf(stdout, "(DEBUG) dd=%g\n",delta_dist);
    } else delta_dist = 0.0;
      
    if(fabs(delta_dist) > 0.0) {
      if(fabs(delta_dist) > fabs(max_dist)) max_dist=delta_dist;
      read_data=1;
      any_read=1;
    }
    
    if(read_data){
      // Seek to the new position
      fio_seek_abs(pt->flist[j],delta_dist*pt->plot_xdelta);
  
      // Read the data
      nread = fio_read(pt->flist[j],pt->max_n2read);
      
      if(pt->debug) fprintf(stdout, "(DEBUG) %0.24e %0.24e %ld\n",pt->flist[j]->current_xstart,pt->flist[j]->current_xend,pt->flist[j]->nelms);
        
      // Convert the data to the current mode
      convert_ydata(pt,j);
    }
  }
  
  // If we've hit the beginning or end of the min/max values for the plot
  // and we didn't read any data to get there and the current zoom level is
  // at the edges of the plot min or max then don't bother replotting
  if(xmin == pt->abscissa_min && any_read == 0 && pt->zoom_info[pt->zoom_level].xmin == pt->abscissa_min) replot=0;
  else if(xmax == pt->abscissa_max && any_read == 0 && pt->zoom_info[pt->zoom_level].xmax == pt->abscissa_max) replot=0;
  
  if(replot){
    // Set the X min/max for the current zoom level
    set_min_max(pt,&xmin,&xmax,NULL,NULL,pt->zoom_level);
    
    // If we're zoomed in and we had to read data because we pushed passed
    // our buffer then go ahead and adjust all the previous zoom levels
    // to match the new buffer limits.
    if(pt->zoom_level != 0 && any_read){      
      for(i=pt->zoom_level-1;i>=0;i--){
        z_xmin = pt->zoom_info[i].xmin+(max_dist*pt->plot_xdelta);
        z_xmax = pt->zoom_info[i].xmax+(max_dist*pt->plot_xdelta);
        set_min_max(pt,&z_xmin,&z_xmax,NULL,NULL,i);
      }
    }
    
    // Should only do this if data has been read
    //if(any_read > 0) scale_yaxis(pt,-1,0.0);
  
    // Clear and Redraw the plotter
    //refresh_plot(pt);
    refresh_plot(pt);
  }
  return;
}

void scroll_data(PLOT_T *pt,int current_pix){
  double abs_dist,xmin,xmax;
  
  // Need to revisit the framed data scrolling as the sample is not
  // exactly following the mouse position
  if(pt->cx_mode == 4 || pt->framed_data){  
    if(pt->zoom_info[pt->zoom_level].xmax < pt->x_max || pt->zoom_info[pt->zoom_level].xmin > pt->x_min){
      //abs_dist = (double)(x - pt->mouse_panx) * pt->xpan_bar.ppa;
      abs_dist = (double)((pt->mouse_grabx - current_pix) + 1) * pt->zoom_info[pt->zoom_level].x_p2r;
      
      if(fabs(abs_dist) < (pt->zoom_info[pt->zoom_level].xmax-pt->zoom_info[pt->zoom_level].xmin)*0.008) return;
      
      // Determine the new xmin and xmax
      xmin = pt->zoom_info[pt->zoom_level].xmin+abs_dist;
      xmax = pt->zoom_info[pt->zoom_level].xmax+abs_dist;
    
      // Limit this to that of our file
      if(xmin < pt->x_min) {
        xmax = pt->zoom_info[pt->zoom_level].xmax + (pt->x_min-pt->zoom_info[pt->zoom_level].xmin);
        xmin = pt->x_min;
      }
      if(xmax > pt->x_max) {
        xmin = pt->zoom_info[pt->zoom_level].xmin + (pt->x_max-pt->zoom_info[pt->zoom_level].xmax);
        xmax = pt->x_max;
      }
    
      // Set the new y min/max for this zoom level
      set_min_max(pt,&xmin,&xmax,NULL,NULL,pt->zoom_level);
    
      // Refresh the plot
      refresh_plot(pt);
    }
  } else {    
    abs_dist = (double)((pt->mouse_grabx - current_pix) + 1) * pt->zoom_info[pt->zoom_level].x_p2r;
    if(fabs(abs_dist) < (pt->zoom_info[pt->zoom_level].xmax-pt->zoom_info[pt->zoom_level].xmin)*0.008) return;
  
    move_data(pt,abs_dist);
  }
  
  // Update the middle mouse x position to that of the current.
  // This gives the illusion of smoother scrolling
  pt->mouse_grabx = current_pix;
}

void xpan_dragged(PLOT_T *pt,int x, int y){
  double abs_dist,xmin,xmax;
  // We need to determine how much abscissa the user
  // moved by
  if(abs(x - pt->mouse_panx) < 4) return;
  if(pt->cx_mode == 4 || pt->framed_data){  
    if(pt->zoom_info[pt->zoom_level].xmax < pt->x_max || pt->zoom_info[pt->zoom_level].xmin > pt->x_min){
      abs_dist = (double)(x - pt->mouse_panx) * pt->xpan_bar.ppa;
      
      // Determine the new xmin and xmax
      xmin = pt->zoom_info[pt->zoom_level].xmin+abs_dist;
      xmax = pt->zoom_info[pt->zoom_level].xmax+abs_dist;
    
      // Limit this to that of our file
      if(xmin < pt->x_min) {
        xmax = pt->zoom_info[pt->zoom_level].xmax + (pt->x_min-pt->zoom_info[pt->zoom_level].xmin);
        xmin = pt->x_min;
      }
      if(xmax > pt->x_max) {
        xmin = pt->zoom_info[pt->zoom_level].xmin + (pt->x_max-pt->zoom_info[pt->zoom_level].xmax);
        xmax = pt->x_max;
      }
    
      // Set the new y min/max for this zoom level
      set_min_max(pt,&xmin,&xmax,NULL,NULL,pt->zoom_level);
    
      // Refresh the plot
      refresh_plot(pt);
    }
  } else {
    abs_dist = (double)(x - pt->mouse_panx) * pt->xpan_bar.ppa;
  
    move_data(pt,abs_dist);
  }
          
  pt->mouse_panx=x;
  pt->mouse_pany=y;
  
  return;
}

void ypan_dragged(PLOT_T *pt,int x, int y){
  double abs_dist,ymin,ymax;
  // We need to determine how much abscissa the user
  // moved by
  if(abs(y - pt->mouse_pany) < 4) return;
  
  if(pt->zoom_info[pt->zoom_level].ymax < pt->y_max || pt->zoom_info[pt->zoom_level].ymin > pt->y_min){
    abs_dist = (double)(y - pt->mouse_pany) * pt->ypan_bar.ppa;
      
    // Determine the new ymin and ymax
    ymin = pt->zoom_info[pt->zoom_level].ymin-abs_dist;
    ymax = pt->zoom_info[pt->zoom_level].ymax-abs_dist;
    
    // Limit this to that of our file
    if(ymax > pt->y_max) {
      ymin = pt->zoom_info[pt->zoom_level].ymin + (pt->y_max-pt->zoom_info[pt->zoom_level].ymax);
      ymax = pt->y_max;
    }
    
    if(ymin < pt->y_min) {
      ymax = pt->zoom_info[pt->zoom_level].ymax + (pt->y_min-pt->zoom_info[pt->zoom_level].ymin);
      ymin = pt->y_min;
    }
    
    // Set the new y min/max for this zoom level
    set_min_max(pt,NULL,NULL,&ymin,&ymax,pt->zoom_level);
    
    // Refresh the plot
    refresh_plot(pt);
  }
          
  pt->mouse_panx=x;
  pt->mouse_pany=y;
  
  return;
}

void pan_dragged(PLOT_T *pt, int pan, int x, int y){  
  // Check the X pan 
  if(pan == 1) xpan_dragged(pt,x,y);
  // Check the y pan
  if(pan == 2) ypan_dragged(pt,x,y);
  return;
}

void pan_clicked(PLOT_T *pt, int x, int y){  
  // Check the X pan 
  if(x > pt->xpan_bar.x1 && x < pt->xpan_bar.x2 && y > pt->xpan_bar.y1 && y < pt->xpan_bar.y2) xpan_clicked(pt,x,y);
  // Check the y pan 
  if(x > pt->ypan_bar.x1 && x < pt->ypan_bar.x2 && y > pt->ypan_bar.y1 && y < pt->ypan_bar.y2) ypan_clicked(pt,x,y);
  return;
}

void xpan_clicked(PLOT_T *pt, int x, int y){  
  double abs_dist;
  double xmin,xmax;
  // First let's figure out where the xpan bar was clicked
  
  // Left arrow
  // For now I'm not going to check the y coordinate as this should only
  // be called after a call to in_pan which tells you whether or not
  // you're in a PAN region
  if(x > pt->xpan_bar.x1 && x < pt->xpan_bar.x1+pt->xpan_bar.ar_width){
    if(pt->cx_mode == 4 || pt->framed_data){
      if(pt->zoom_info[pt->zoom_level].xmin > pt->x_min){
        abs_dist = (pt->zoom_info[pt->zoom_level].xmax-pt->zoom_info[pt->zoom_level].xmin)*0.05;
      
        // Determine the new xmin and xmax
        xmin = pt->zoom_info[pt->zoom_level].xmin-abs_dist;
        xmax = pt->zoom_info[pt->zoom_level].xmax-abs_dist;
    
        // Limit this to that of our file
        if(xmin < pt->x_min) {
          xmax = pt->zoom_info[pt->zoom_level].xmax + (pt->x_min-pt->zoom_info[pt->zoom_level].xmin);
          xmin = pt->x_min;
        }
    
        // Set the new y min/max for this zoom level
        set_min_max(pt,&xmin,&xmax,NULL,NULL,pt->zoom_level);
    
        // Refresh the plot
        refresh_plot(pt);
      }
    } else {
      abs_dist = -((pt->zoom_info[pt->zoom_level].xmax-pt->zoom_info[pt->zoom_level].xmin)*0.05);
      move_data(pt,abs_dist);
    }
  }
  // Right arrow
  else if(x < pt->xpan_bar.x2 && x > pt->xpan_bar.x2-pt->xpan_bar.ar_width){
    if(pt->cx_mode == 4 || pt->framed_data){
      // Make sure we haven't already hit the bottom
      if(pt->zoom_info[pt->zoom_level].xmax < pt->x_max){
        abs_dist = (pt->zoom_info[pt->zoom_level].xmax-pt->zoom_info[pt->zoom_level].xmin)*0.05;
      
        // Determine the new xmin and xmax
        xmin = pt->zoom_info[pt->zoom_level].xmin+abs_dist;
        xmax = pt->zoom_info[pt->zoom_level].xmax+abs_dist;
    
        // Limit this to that of our file
        if(xmax > pt->x_max) {
          xmin = pt->zoom_info[pt->zoom_level].xmin + (pt->x_max-pt->zoom_info[pt->zoom_level].xmax);
          xmax = pt->x_max;
        }
    
        // Set the new y min/max for this zoom level
        set_min_max(pt,&xmin,&xmax,NULL,NULL,pt->zoom_level);
    
        // Refresh the plot
        refresh_plot(pt);
      }
    } else {
      abs_dist = (pt->zoom_info[pt->zoom_level].xmax-pt->zoom_info[pt->zoom_level].xmin)*0.05;
      move_data(pt,abs_dist);
    }
  }
  // Gap to the right between the pan handle and the right arrow 
  // Gap to the left between the pan handle and the left arrow 
  // on the pan bar
  else if(x < pt->xpan_bar.hndl.x1 && x > pt->xpan_bar.drag_x1){
    if(pt->cx_mode == 4 || pt->framed_data){
    } else {
      abs_dist = -((pt->zoom_info[pt->zoom_level].xmax-pt->zoom_info[pt->zoom_level].xmin)*0.5);
      move_data(pt,abs_dist);
    }
  } 
  else if(x > pt->xpan_bar.hndl.x2 && x < pt->xpan_bar.drag_x2){
    if(pt->cx_mode == 4 || pt->framed_data){
    } else {
      abs_dist = (pt->zoom_info[pt->zoom_level].xmax-pt->zoom_info[pt->zoom_level].xmin)*0.5;
      move_data(pt,abs_dist);
    }
  }
  
  return;
}

void ypan_clicked(PLOT_T *pt, int x, int y){  
  double abs_dist,ymin,ymax;
  // First let's figure out where the ypan bar was clicked
  
  // The functionality here should be moved to a....function....so
  // it can be used elsewhere....revist
  
  // UP arrow
  // For now I'm not going to check the x coordinate as this should only
  // be called after a call to in_pan which tells you whether or not
  // you're in a PAN region
  if(y > pt->ypan_bar.y1 && y < pt->ypan_bar.y1+pt->ypan_bar.ar_height){
    //if(pt->debug) fprintf(stdout, "(DEBUG) Up arrow\n");
    // Make sure we haven't already hit the top
    if(pt->zoom_info[pt->zoom_level].ymax < pt->y_max){
      abs_dist = (pt->zoom_info[pt->zoom_level].ymax-pt->zoom_info[pt->zoom_level].ymin)*0.05;
      
      // Determine the new ymin and ymax
      ymin = pt->zoom_info[pt->zoom_level].ymin+abs_dist;
      ymax = pt->zoom_info[pt->zoom_level].ymax+abs_dist;
    
      // Limit this to that of our file
      if(ymax > pt->y_max) {
        ymin = pt->zoom_info[pt->zoom_level].ymin + (pt->y_max-pt->zoom_info[pt->zoom_level].ymax);
        ymax = pt->y_max;
      }
    
      // Set the new y min/max for this zoom level
      set_min_max(pt,NULL,NULL,&ymin,&ymax,pt->zoom_level);
    
      // Refresh the plot
      refresh_plot(pt);
    }
  }
  // Down arrow
  else if(y < pt->ypan_bar.y2 && y > pt->ypan_bar.y2-pt->ypan_bar.ar_height){
    //if(pt->debug) fprintf(stdout, "(DEBUG) Down arrow\n");
    // Make sure we haven't already hit the bottom
    if(pt->zoom_info[pt->zoom_level].ymin > pt->y_min){
      abs_dist = (pt->zoom_info[pt->zoom_level].ymax-pt->zoom_info[pt->zoom_level].ymin)*0.05;
      
      // Determine the new ymin and ymax
      ymin = pt->zoom_info[pt->zoom_level].ymin-abs_dist;
      ymax = pt->zoom_info[pt->zoom_level].ymax-abs_dist;
    
      // Limit this to that of our file
      if(ymin < pt->y_min) {
        ymax = pt->zoom_info[pt->zoom_level].ymax + (pt->y_min-pt->zoom_info[pt->zoom_level].ymin);
        ymin = pt->y_min;
      }
    
      // Set the new y min/max for this zoom level
      set_min_max(pt,NULL,NULL,&ymin,&ymax,pt->zoom_level);
    
      // Refresh the plot
      refresh_plot(pt);
    }
  }
  // Gap to the top between the pan handle and the up arrow 
  // Gap to the bottom between the pan handle and the bottom arrow 
  // on the pan bar
  else if(y < pt->ypan_bar.hndl.y1 && y > pt->ypan_bar.drag_y1){
    // Make sure we haven't already hit the top
    if(pt->zoom_info[pt->zoom_level].ymax < pt->y_max){
      abs_dist = (pt->zoom_info[pt->zoom_level].ymax-pt->zoom_info[pt->zoom_level].ymin)*0.5;
      
      // Determine the new ymin and ymax
      ymin = pt->zoom_info[pt->zoom_level].ymin+abs_dist;
      ymax = pt->zoom_info[pt->zoom_level].ymax+abs_dist;
    
      // Limit this to that of our file
      if(ymax > pt->y_max) {
        ymin = pt->zoom_info[pt->zoom_level].ymin + (pt->y_max-pt->zoom_info[pt->zoom_level].ymax);
        ymax = pt->y_max;
      }
    
      // Set the new y min/max for this zoom level
      set_min_max(pt,NULL,NULL,&ymin,&ymax,pt->zoom_level);
    
      // Refresh the plot
      refresh_plot(pt);
    }
  }
  else if(y > pt->ypan_bar.hndl.y2 && y < pt->ypan_bar.drag_y2){
    // Make sure we haven't already hit the bottom
    if(pt->zoom_info[pt->zoom_level].ymin > pt->y_min){
      abs_dist = (pt->zoom_info[pt->zoom_level].ymax-pt->zoom_info[pt->zoom_level].ymin)*0.5;
      
      // Determine the new ymin and ymax
      ymin = pt->zoom_info[pt->zoom_level].ymin-abs_dist;
      ymax = pt->zoom_info[pt->zoom_level].ymax-abs_dist;
    
      // Limit this to that of our file
      if(ymin < pt->y_min) {
        ymax = pt->zoom_info[pt->zoom_level].ymax + (pt->y_min-pt->zoom_info[pt->zoom_level].ymin);
        ymin = pt->y_min;
      }
    
      // Set the new y min/max for this zoom level
      set_min_max(pt,NULL,NULL,&ymin,&ymax,pt->zoom_level);
    
      // Refresh the plot
      refresh_plot(pt);
    }
  }
  
  return;
}

void init_plot(PLOT_T *pt){
  XColor pan_c;
  long pixel_val;
  int offset;
  
  if(pt->nfiles > 0){
    // Set the buffers up
    init_plot_buffers(pt);
  }
  
  // Define the layer colors (has to be after the call to the colormap generator)
  init_layer_colors(pt);
  
  // Define the PAN region colors (has to be after the call to the colormap generator)
  init_pan_bars(pt);
      
  offset=5000;
  // Light grey, nearly white
  pan_c.red=56352;
  pan_c.green=56352;
  pan_c.blue=56352;
  if(XAllocColor(pt->display,pt->cmap,&pan_c) == 0){
    fprintf(stderr, "%s: Could not allocate color for pan area, defaulting\n",pt->name);
    pixel_val = pt->black;
  } else pixel_val = pan_c.pixel;
  
  pt->light_grey=pixel_val;
  
  pan_c.red=pan_c.red+offset;
  pan_c.green=pan_c.green+offset;
  pan_c.blue=pan_c.blue+offset;
  if(XAllocColor(pt->display,pt->cmap,&pan_c) == 0){
    fprintf(stderr, "%s: Could not allocate color for pan area, defaulting\n",pt->name);
    pixel_val = pt->black;
  } else pixel_val = pan_c.pixel;
  
  pt->h_light_grey=pixel_val;
  
  // Medium grey
  pan_c.red=36352;
  pan_c.green=36352;
  pan_c.blue=36352;
  if(XAllocColor(pt->display,pt->cmap,&pan_c) == 0){
    fprintf(stderr, "%s: Could not allocate color for pan area, defaulting\n",pt->name);
    pixel_val = pt->black;
  } else pixel_val = pan_c.pixel;
  
  pt->medium_grey=pixel_val;  
  
  pan_c.red=pan_c.red+offset;
  pan_c.green=pan_c.green+offset;
  pan_c.blue=pan_c.blue+offset;
  if(XAllocColor(pt->display,pt->cmap,&pan_c) == 0){
    fprintf(stderr, "%s: Could not allocate color for pan area, defaulting\n",pt->name);
    pixel_val = pt->black;
  } else pixel_val = pan_c.pixel;
  
  pt->h_medium_grey=pixel_val;
  
  // Dark grey
  pan_c.red=26352;
  pan_c.green=26352;
  pan_c.blue=26352;
  if(XAllocColor(pt->display,pt->cmap,&pan_c) == 0){
    fprintf(stderr, "%s: Could not allocate color for pan area, defaulting\n",pt->name);
    pixel_val = pt->black;
  } else pixel_val = pan_c.pixel;
  
  pt->dark_grey=pixel_val;
  
  pan_c.red=pan_c.red+offset;
  pan_c.green=pan_c.green+offset;
  pan_c.blue=pan_c.blue+offset;
  if(XAllocColor(pt->display,pt->cmap,&pan_c) == 0){
    fprintf(stderr, "%s: Could not allocate color for pan area, defaulting\n",pt->name);
    pixel_val = pt->black;
  } else pixel_val = pan_c.pixel;
  
  pt->h_dark_grey=pixel_val;
    
  return;
}

int init_plot_buffers(PLOT_T *pt){
  int i,status=0,mult,j;
  char mode;
  char format;
  double abs_seek;
  enum DATA_TYPE dt;
  double xdelta;
  
  // Do some spot checking on the input files to make sure
  // they are supported combinations. Essentially all that matters
  // is that our data types are the same and the XDELTA is the 
  // same.
  dt = pt->flist[0]->data_type;
  xdelta = pt->flist[0]->xdelta;
  for(i=1;i<pt->nfiles;i++){
    if(pt->flist[i]->data_type != dt){
      fprintf(stderr, "(ERROR) Mixed data types are not supported\n");
      status=1;
    }
    else if(pt->flist[i]->xdelta != xdelta){
      fprintf(stderr, "(ERROR) Mixed XDELTA values are not supported\n");
      status=1;
    }
    
    if(status) break;
  }
  
  if(status==0){
    // Set the XDELTA
    pt->plot_xdelta  = pt->flist[0]->xdelta;
    
    // Set whether or not we're plotting framed data
    pt->framed_data = (pt->flist[0]->framed ? 1:0);
    
    pt->abscissa_min = pt->flist[0]->xstart;
    pt->abscissa_max = pt->flist[0]->xend;
    for(i=1;i<pt->nfiles;i++){      
      // Determine the smallest min and the largest max abscissa available between
      // all the files
      if(pt->flist[i]->xstart < pt->abscissa_min) pt->abscissa_min = pt->flist[i]->xstart;
      if(pt->flist[i]->xend > pt->abscissa_max) pt->abscissa_max = pt->flist[i]->xend;
    }
    
    if(pt->debug) fprintf(stdout, "(DEBUG) Plot abscissa min=%f, max=%f\n",pt->abscissa_min,pt->abscissa_max);        
    
    // Check to see if the user defined an ABSCISSA start and end. If so, let's
    // make sure it falls in an appropriate range
    abs_seek=0.0;
    
    if(pt->framed_data) {
      // For now user defined starts and stops do effect
      // framed data sets. Will revisit this in the future
      // to add support.
      pt->max_n2read = MIN(MAX_FRAMES,pt->flist[0]->total_elements);
      if(pt->udef_xunits == 1){
        fprintf(stderr, "%s: Abscissa start and stop values are not supported with framed data sets at the moment\n",pt->name);
        
        // Get rid of the flags so any follow on setup ignores the user defined values
        pt->udef_flags=0;
        /*
        if(pt->udef_flags&USER_DEF_START){
          // Make sure the start is within the available range
          if(pt->u_start < pt->abscissa_min || pt->u_start > pt->abscissa_max) {
            fprintf(stderr, "%s: User supplied START is not in a valid range for the files given, autoscaling\n",pt->name);
            pt->u_start = pt->abscissa_min;
          }
        }
      
         if(pt->udef_flags&USER_DEF_END){
          // Make sure the end is within the available range
          if(pt->u_end < pt->abscissa_min || pt->u_end > pt->abscissa_max) {
            fprintf(stderr, "%s: User supplied END is not in a valid range for the files given, autoscaling\n",pt->name);
          
            // If the user supplied value is less than the min then simply set the max to that
            // supported by the plotter or MAX_READ otherwise assume the user was trying to hit
            // the end of the file so give it to them
            if(pt->u_end < pt->abscissa_min) {
              if(pt->abscissa_min + ((double)MAX_READ*pt->plot_xdelta) > pt->abscissa_max) pt->u_end = pt->abscissa_max;
              else pt->u_end = pt->abscissa_min + ((double)MAX_READ*pt->plot_xdelta);
            }
            else pt->u_end = pt->abscissa_max;
          }
        }
        */
      } else {
        if(pt->udef_flags&USER_DEF_START){
          // Since this is in index mode make sure the start is greater than 0. The second
          // check makes sure that it's within the range of the first file. Due to the nature
          // of the start and stop parameters (for now at least), the start and end values
          // are based off of the first file. Eventually I'll have to revisit this and
          // make the start and end another layer option.
          if(pt->u_start > 0.0 && pt->u_start <= pt->flist[0]->total_elements){
            // For the start we're going to floor the value as we only accept whole numbers
            // This make sure we at least get the starting point correct
            pt->u_start = floor(pt->u_start);
          } else {
            fprintf(stderr, "%s: User supplied START index is invalid, defaulting to 1\n",pt->name);
            pt->u_start=1.0;
          }
        }
      
        // If the user defined an end, make sure it fits
        if(pt->udef_flags&USER_DEF_END){
          // We're going to ceil the end value in case the user supplied us with a floating point value
          if(pt->u_end > 0.0 && pt->u_end <= pt->flist[0]->total_elements){
            pt->u_end = ceil(pt->u_end);
          } else {
            fprintf(stderr, "%s: User supplied END index is > than the max available for the file(s) given, defaulting to the max\n",pt->name);
            pt->u_end = MIN(MAX_FRAMES,pt->flist[0]->total_elements);
          }
        }
      }
      
      if(pt->udef_flags&USER_DEF_START){
        if(pt->udef_flags&USER_DEF_END){
          // Make sure the calculated end is greater than the start
          if(pt->u_start < pt->u_end){
            abs_seek = (pt->u_start-1.0)*pt->flist[0]->ydelta;
            pt->max_n2read = (pt->u_end-pt->u_start)+1.0;
          }
        } else {
          abs_seek = (pt->u_start-1.0)*pt->flist[0]->ydelta;
          if(pt->u_start+MAX_FRAMES > pt->flist[0]->total_elements) pt->max_n2read = pt->flist[0]->total_elements-pt->u_start;
        }
      }
      else if(pt->udef_flags&USER_DEF_END){          
        pt->max_n2read = MIN(MAX_FRAMES,pt->u_end);
      }
      else {
        pt->max_n2read = MIN(MAX_FRAMES,pt->flist[0]->total_elements);
      }
    }
    else {
      if(pt->udef_xunits == 1){
        if(pt->udef_flags&USER_DEF_START){
          // Make sure the start is within the available range
          if(pt->u_start < pt->abscissa_min || pt->u_start > pt->abscissa_max) {
            fprintf(stderr, "%s: User supplied START is not in a valid range for the files given, autoscaling\n",pt->name);
            pt->u_start = pt->abscissa_min;
          }
        }
      
         if(pt->udef_flags&USER_DEF_END){
          // Make sure the end is within the available range
          if(pt->u_end < pt->abscissa_min || pt->u_end > pt->abscissa_max) {
            fprintf(stderr, "%s: User supplied END is not in a valid range for the files given, autoscaling\n",pt->name);
          
            // If the user supplied value is less than the min then simply set the max to that
            // supported by the plotter or MAX_READ otherwise assume the user was trying to hit
            // the end of the file so give it to them
            if(pt->u_end < pt->abscissa_min) {
              if(pt->abscissa_min + ((double)MAX_READ*pt->plot_xdelta) > pt->abscissa_max) pt->u_end = pt->abscissa_max;
              else pt->u_end = pt->abscissa_min + ((double)MAX_READ*pt->plot_xdelta);
            }
            else pt->u_end = pt->abscissa_max;
          }
        }  
      } else {
        if(pt->udef_flags&USER_DEF_START){
          // Since this is in index mode make sure the start is greater than 0
          if(pt->u_start >= 1.0 && pt->u_start < ((pt->abscissa_max - pt->abscissa_min)/pt->plot_xdelta) + 1.0){
            // For the start we're going to floor the value as we only accept whole numbers
            // This make sure we at least get the starting point correct
            pt->u_start = floor(pt->u_start);
          
            // Let's convert this value to abscissa for ease of use throughout the rest of the setup
            pt->u_start = pt->abscissa_min + ((pt->u_start-1.0) * pt->plot_xdelta);
          } else {
            fprintf(stderr, "%s: User supplied START index is invalid, defaulting to 1\n",pt->name);
            pt->u_start=pt->abscissa_min;
          }
        }
      
        // If the user defined an end, make sure it fits
        if(pt->udef_flags&USER_DEF_END){
          // We're going to ceil the end value in case the user supplied us with a floating point value
          if(pt->u_end > 0 && pt->u_end <= ((pt->abscissa_max - pt->abscissa_min)/pt->plot_xdelta) + 1.0){
            pt->u_end = ceil(pt->u_end);
          
            // Let's convert this value to abscissa for ease of use throughout the rest of the setup
            pt->u_end = pt->abscissa_min + ((pt->u_end-1.0) * pt->plot_xdelta);
          } else {
            fprintf(stderr, "%s: User supplied END index is > than the max available for the file(s) given, defaulting to the max\n",pt->name);
            pt->u_end = pt->abscissa_max;
          }
        }
      }
      
      if(pt->udef_flags&USER_DEF_START){
        if(pt->udef_flags&USER_DEF_END){
          // Make sure the calculated end is greater than the start
          if(pt->u_start < pt->u_end){
            abs_seek = (ceil(pt->u_start/pt->plot_xdelta)*pt->plot_xdelta) - pt->plot_xdelta;
            pt->max_n2read = ceil(((ceil(pt->u_end/pt->plot_xdelta)*pt->plot_xdelta) - abs_seek)/pt->plot_xdelta)+1.0;
          }
        } else {
          abs_seek = (ceil(pt->u_start/pt->plot_xdelta)*pt->plot_xdelta) - pt->plot_xdelta;
          if(ceil((pt->abscissa_max - abs_seek)/pt->plot_xdelta) < MAX_READ) pt->max_n2read = round((pt->abscissa_max - abs_seek)/pt->plot_xdelta)+1.0;
        }
      }
      else if(pt->udef_flags&USER_DEF_END){          
        pt->max_n2read = ceil(((ceil(pt->u_end/pt->plot_xdelta)*pt->plot_xdelta) - pt->abscissa_min)/pt->plot_xdelta)+1.0;
      }
      else {
        pt->max_n2read = MAX_READ;
      }
    }
    
    if(pt->read_all){
      for(i=0;i<pt->nfiles;i++) pt->max_n2read = MAX(pt->max_n2read,pt->flist[i]->total_elements);
    }
    
    // Allocate the space for the buffers
    for(i=0;i<pt->nfiles;i++){
      // We're going to promote all files to double precision for ease of use
      pt->flist[i]->buf_type='D';
      
      fio_alloc_buf(pt->flist[i],
                    (pt->flist[i]->framed == 0 ? MIN(pt->max_n2read,pt->flist[i]->total_elements) : 
                                                 MIN(MAX_FRAMES,pt->flist[i]->total_elements)));
      
      pt->ydata[i] = (double *) calloc((pt->flist[i]->framed == 0 ? MIN(pt->max_n2read,pt->flist[i]->total_elements) :
                                                                    MIN(pt->flist[i]->total_elements,MAX_FRAMES)*(pt->flist[i]->frame_size*pt->flist[i]->bpe)),
                                                                    sizeof(double));
      
      // If this value is greater than 0 then it means that the user defined
      // a start and/or end value so we should attempt to honor it
      if(abs_seek > 0.0) {
        fprintf(stdout, "Seeking %f\n",abs_seek);
        if(pt->flist[i]->framed == 0) abs_seek = (ceil((pt->u_start - pt->flist[i]->xstart)/pt->plot_xdelta)*pt->plot_xdelta) - pt->plot_xdelta;
        if(abs_seek > 0.0) fio_seek_abs(pt->flist[i],abs_seek);
      }
      
      // Perform an initial read of the files
      fio_read(pt->flist[i],(pt->flist[i]->framed == 0 ? MIN(pt->max_n2read,pt->flist[i]->total_elements) : 
                                                 MIN(MAX_FRAMES,pt->flist[i]->total_elements)));
      if(pt->debug) fprintf(stdout, "(DEBUG) Read %ld elements from file %d\n",pt->flist[i]->nelms,i);
    
      convert_ydata(pt,i);
    }
    
    scale_xaxis(pt);
    scale_yaxis(pt,-1,0.0);
  }
  
  pt->initial_startup=0;
  
  return(status);
}

void init_layer_colors(PLOT_T *pt){
  int i;
  XColor screen_def,exact_def;
  for(i=0;i<MAX_FILES;i++){
    if(XAllocNamedColor(pt->display,pt->cmap,pt->color_palette[pt->layer_info[i].layer_cval],&screen_def,&exact_def)){
      pt->layer_info[i].layer_pixel = screen_def.pixel;
    } else {
      fprintf(stderr, "%s: Could not find color %s in the colormap\n",pt->name,pt->color_palette[i]);
    }
    
  }
}

void init_pan_bars(PLOT_T *pt){
  int i;
  
  pt->xpan_bar.ar_width=16;
  pt->xpan_bar.ar_height=14;
  pt->ypan_bar.ar_width=14;
  pt->ypan_bar.ar_height=16;
  
  pt->xpan_bar.hndl.is_button=0;
  pt->xpan_bar.hndl.pressed=0;
  pt->xpan_bar.hndl.hlit=0;
  pt->xpan_bar.hndl.text[0]='\0';
  pt->ypan_bar.hndl.is_button=0;
  pt->ypan_bar.hndl.pressed=0;
  pt->ypan_bar.hndl.hlit=0;
  pt->ypan_bar.hndl.text[0]='\0';
  
  // Build the pieces for the left arrow. To build the right
  // arrow we'll simply reflect the points calculated for the
  // left
  pt->xpan_bar.arrow[0][0].x = 0;
  pt->xpan_bar.arrow[0][0].y = pt->xpan_bar.ar_height/2;
  pt->xpan_bar.arrow[0][1].x = pt->xpan_bar.ar_width;
  pt->xpan_bar.arrow[0][1].y = 0;
  pt->xpan_bar.arrow[0][2].x = pt->xpan_bar.ar_width-3;
  pt->xpan_bar.arrow[0][2].y = 4;
  pt->xpan_bar.arrow[0][3].x = 4;
  pt->xpan_bar.arrow[0][3].y = pt->xpan_bar.ar_height/2;
  
  pt->xpan_bar.arrow[1][0].x = pt->xpan_bar.ar_width;
  pt->xpan_bar.arrow[1][0].y = 0;
  pt->xpan_bar.arrow[1][1].x = pt->xpan_bar.ar_width-3;
  pt->xpan_bar.arrow[1][1].y = 4;
  pt->xpan_bar.arrow[1][2].x = pt->xpan_bar.ar_width-3;
  pt->xpan_bar.arrow[1][2].y = pt->xpan_bar.ar_height-4;
  pt->xpan_bar.arrow[1][3].x = pt->xpan_bar.ar_width;
  pt->xpan_bar.arrow[1][3].y = pt->xpan_bar.ar_height;
  
  pt->xpan_bar.arrow[2][0].x = pt->xpan_bar.ar_width;
  pt->xpan_bar.arrow[2][0].y = pt->xpan_bar.ar_height;
  pt->xpan_bar.arrow[2][1].x = pt->xpan_bar.ar_width-3;
  pt->xpan_bar.arrow[2][1].y = pt->xpan_bar.ar_height-4;
  pt->xpan_bar.arrow[2][2].x = 4;
  pt->xpan_bar.arrow[2][2].y = pt->xpan_bar.ar_height/2;
  pt->xpan_bar.arrow[2][3].x = 0;
  pt->xpan_bar.arrow[2][3].y = pt->xpan_bar.ar_height/2;
  
  pt->xpan_bar.arrow[3][0].x = 4;
  pt->xpan_bar.arrow[3][0].y = pt->xpan_bar.ar_height/2;
  pt->xpan_bar.arrow[3][1].x = pt->xpan_bar.ar_width-3;
  pt->xpan_bar.arrow[3][1].y = 4;
  pt->xpan_bar.arrow[3][2].x = pt->xpan_bar.ar_width-3;
  pt->xpan_bar.arrow[3][2].y = pt->xpan_bar.ar_height-4;  
  
  // Build the pieces for the up arrow. To build the down
  // arrow we'll simply reflect the points calculated across
  // the center X point
  pt->ypan_bar.arrow[0][0].x = 0;
  pt->ypan_bar.arrow[0][0].y = pt->ypan_bar.ar_height;
  pt->ypan_bar.arrow[0][1].x = 4;
  pt->ypan_bar.arrow[0][1].y = pt->ypan_bar.ar_height-3;
  pt->ypan_bar.arrow[0][2].x = pt->ypan_bar.ar_width/2;
  pt->ypan_bar.arrow[0][2].y = 4;
  pt->ypan_bar.arrow[0][3].x = pt->ypan_bar.ar_width/2;
  pt->ypan_bar.arrow[0][3].y = 0;
    
  pt->ypan_bar.arrow[1][0].x = pt->ypan_bar.ar_width;
  pt->ypan_bar.arrow[1][0].y = pt->ypan_bar.ar_height;
  pt->ypan_bar.arrow[1][1].x = pt->ypan_bar.ar_width-4;
  pt->ypan_bar.arrow[1][1].y = pt->ypan_bar.ar_height-3;
  pt->ypan_bar.arrow[1][2].x = 4;
  pt->ypan_bar.arrow[1][2].y = pt->ypan_bar.ar_height-3;
  pt->ypan_bar.arrow[1][3].x = 0;
  pt->ypan_bar.arrow[1][3].y = pt->ypan_bar.ar_height;
  
  pt->ypan_bar.arrow[2][0].x = pt->ypan_bar.ar_width/2;
  pt->ypan_bar.arrow[2][0].y = 0;
  pt->ypan_bar.arrow[2][1].x = pt->ypan_bar.ar_width;
  pt->ypan_bar.arrow[2][1].y = pt->ypan_bar.ar_height;
  pt->ypan_bar.arrow[2][2].x = pt->ypan_bar.ar_width-4;
  pt->ypan_bar.arrow[2][2].y = pt->ypan_bar.ar_height-3;
  pt->ypan_bar.arrow[2][3].x = pt->ypan_bar.ar_width/2;
  pt->ypan_bar.arrow[2][3].y = 4;
  
  pt->ypan_bar.arrow[3][0].x = pt->ypan_bar.ar_width/2;
  pt->ypan_bar.arrow[3][0].y = 4;
  pt->ypan_bar.arrow[3][1].x = pt->ypan_bar.ar_width-4;
  pt->ypan_bar.arrow[3][1].y = pt->ypan_bar.ar_height-3;
  pt->ypan_bar.arrow[3][2].x = 4;
  pt->ypan_bar.arrow[3][2].y = pt->ypan_bar.ar_height-3;  
}

void set_cursor(PLOT_T *pt, int curs){
  Pixmap shape, mask;
  Cursor cursor;
  XColor bg,fg;  
  
  if(curs == CROSSHAIR_CURSOR){
    shape = XCreatePixmapFromBitmapData(pt->display,pt->win,crosshair_sp_bits,crosshair_width,crosshair_height,1,0,1);
    mask = XCreatePixmapFromBitmapData(pt->display,pt->win,crosshair_sp_bits,crosshair_width,crosshair_height,1,0,1);
  
    fg.pixel = pt->black;
    bg.pixel = pt->white;
    XQueryColor(pt->display,pt->cmap,&bg);
    XQueryColor(pt->display,pt->cmap,&fg);
  
    cursor = XCreatePixmapCursor(pt->display,shape,mask,&bg,&fg,7,7);
  }
  else if(curs == POINTER_CURSOR) {
    cursor = XCreateFontCursor(pt->display,XC_arrow);
  }
  
  XDefineCursor(pt->display,pt->win,cursor);
  
  XFreeCursor(pt->display,cursor);
  return;
}

void draw_crosshair(PLOT_T *pt){
  // Undraw the previous lines
  if(pt->crosshair_lines[0].x1 != 0) {
    XDrawLine(pt->display,pt->win,pt->gc_xor,pt->crosshair_lines[0].x1,pt->crosshair_lines[0].y1,pt->crosshair_lines[0].x2,pt->crosshair_lines[0].y2);
    pt->crosshair_lines[0].x1 = 0;
  }
  if(pt->crosshair_lines[1].x1 != 0) {
    XDrawLine(pt->display,pt->win,pt->gc_xor,pt->crosshair_lines[1].x1,pt->crosshair_lines[1].y1,pt->crosshair_lines[1].x2,pt->crosshair_lines[1].y2);
    pt->crosshair_lines[1].x1 = 0;
  }
  if(pt->crosshair_lines[2].x1 != 0) {
    XDrawLine(pt->display,pt->win,pt->gc_xor,pt->crosshair_lines[2].x1,pt->crosshair_lines[2].y1,pt->crosshair_lines[2].x2,pt->crosshair_lines[2].y2);
    pt->crosshair_lines[2].x1 = 0;
  }
  if(pt->crosshair_lines[3].x1 != 0) {
    XDrawLine(pt->display,pt->win,pt->gc_xor,pt->crosshair_lines[3].x1,pt->crosshair_lines[3].y1,pt->crosshair_lines[3].x2,pt->crosshair_lines[3].y2);
    pt->crosshair_lines[3].x1 = 0;
  }
  
  if(pt->mouse_x > pt->grid_x-2 && pt->mouse_x < pt->grid_x2+1 && pt->mouse_y > pt->grid_y-2 && pt->mouse_y < pt->grid_y2+1){
  
    // Compute the new lines
    // Top
    if(pt->mouse_y - 10 >= pt->grid_y-1){
      pt->crosshair_lines[0].x1 = pt->mouse_x;
      pt->crosshair_lines[0].y1 = pt->grid_y-1;
      pt->crosshair_lines[0].x2 = pt->mouse_x;
      pt->crosshair_lines[0].y2 = pt->mouse_y-10;
      XDrawLine(pt->display,pt->win,pt->gc_xor,pt->crosshair_lines[0].x1,pt->crosshair_lines[0].y1,pt->crosshair_lines[0].x2,pt->crosshair_lines[0].y2);
    } else pt->crosshair_lines[0].x1 = 0;
    
    // Bottom
    if(pt->mouse_y + 10 <= pt->grid_y2+1){
      pt->crosshair_lines[1].x1 = pt->mouse_x;
      pt->crosshair_lines[1].y1 = pt->mouse_y+10;
      pt->crosshair_lines[1].x2 = pt->mouse_x;
      pt->crosshair_lines[1].y2 = pt->grid_y2+1;
      XDrawLine(pt->display,pt->win,pt->gc_xor,pt->crosshair_lines[1].x1,pt->crosshair_lines[1].y1,pt->crosshair_lines[1].x2,pt->crosshair_lines[1].y2);
    } else pt->crosshair_lines[1].x1 = 0;
    
    // Left
    if(pt->mouse_x - 10 >= pt->grid_x-1){
      pt->crosshair_lines[2].x1 = pt->grid_x-1;
      pt->crosshair_lines[2].y1 = pt->mouse_y;
      pt->crosshair_lines[2].x2 = pt->mouse_x-10;
      pt->crosshair_lines[2].y2 = pt->mouse_y;
      XDrawLine(pt->display,pt->win,pt->gc_xor,pt->crosshair_lines[2].x1,pt->crosshair_lines[2].y1,pt->crosshair_lines[2].x2,pt->crosshair_lines[2].y2);
    } else pt->crosshair_lines[2].x1 = 0;
    
    // Right
    if(pt->mouse_x + 10 <= pt->grid_x2-1){
      pt->crosshair_lines[3].x1 = pt->mouse_x+10;
      pt->crosshair_lines[3].y1 = pt->mouse_y;
      pt->crosshair_lines[3].x2 = pt->grid_x2+1;
      pt->crosshair_lines[3].y2 = pt->mouse_y;
      XDrawLine(pt->display,pt->win,pt->gc_xor,pt->crosshair_lines[3].x1,pt->crosshair_lines[3].y1,pt->crosshair_lines[3].x2,pt->crosshair_lines[3].y2);
    } else pt->crosshair_lines[3].x1 = 0;
  }
  return;
}

void toggle_crosshair(PLOT_T *pt){
  if(pt->crosshair_on){
    // Undraw the previous lines
    if(pt->crosshair_lines[0].x1 != 0){
      XDrawLine(pt->display,pt->win,pt->gc_xor,pt->crosshair_lines[0].x1,pt->crosshair_lines[0].y1,pt->crosshair_lines[0].x2,pt->crosshair_lines[0].y2);
      pt->crosshair_lines[0].x1=0;
    }
    if(pt->crosshair_lines[1].x1 != 0){
      XDrawLine(pt->display,pt->win,pt->gc_xor,pt->crosshair_lines[1].x1,pt->crosshair_lines[1].y1,pt->crosshair_lines[1].x2,pt->crosshair_lines[1].y2);
      pt->crosshair_lines[1].x1=0;
    }
    if(pt->crosshair_lines[2].x1 != 0){
      XDrawLine(pt->display,pt->win,pt->gc_xor,pt->crosshair_lines[2].x1,pt->crosshair_lines[2].y1,pt->crosshair_lines[2].x2,pt->crosshair_lines[2].y2);
      pt->crosshair_lines[2].x1=0;
    }
    if(pt->crosshair_lines[3].x1 != 0){
      XDrawLine(pt->display,pt->win,pt->gc_xor,pt->crosshair_lines[3].x1,pt->crosshair_lines[3].y1,pt->crosshair_lines[3].x2,pt->crosshair_lines[3].y2);
      pt->crosshair_lines[3].x1=0;
    }
    pt->crosshair_on=0;
  }
  else{
    draw_crosshair(pt);
    pt->crosshair_on=1;
  }
  return;
}

void show_legend(PLOT_T *pt){
  int l_x1,l_x2,l_y1,l_y2,i,j;
  int x1,y1,x2,nchars;
  
  unsigned int nlayers=0;
  
  for(i=0;i<pt->nfiles;i++){
    if(pt->flist[i]->framed){
      nlayers += pt->flist[i]->nelms;
    }
    else {
      nlayers++;
    }
  }
  
  // Determine the area which the legend is going to cover
  l_x1 = pt->grid_x2 - 5 - 2 - pt->font_width*20 - 8 - 2 - 4;
  l_y1 = pt->grid_y + 5;
  
  l_x2 = pt->grid_x2 - 5;
  l_y2 = l_y1 + pt->font_height*nlayers + pt->widget_spacing*(nlayers+1);
  
  // Set our line to solid for the grid outline
  XSetLineAttributes(pt->display,pt->gc_text,2,LineSolid,CapRound,JoinRound);
  
  if(pt->legend_pmap.pmap) XFreePixmap(pt->display,pt->legend_pmap.pmap);
  
  pt->legend_pmap.x1 = l_x1-1;
  pt->legend_pmap.y1 = l_y1-1;
  pt->legend_pmap.width = l_x2-l_x1+2;
  pt->legend_pmap.height = l_y2-l_y1+2;
  
  pt->legend_pmap.pmap = XCreatePixmap(pt->display, pt->win, pt->legend_pmap.width, pt->legend_pmap.height, pt->vis_info->depth);
  
  XCopyArea(pt->display,pt->win,pt->legend_pmap.pmap,pt->gc_text,pt->legend_pmap.x1,pt->legend_pmap.y1,pt->legend_pmap.width,pt->legend_pmap.height,0,0);
  
  XClearArea(pt->display,pt->win,pt->legend_pmap.x1,pt->legend_pmap.y1,pt->legend_pmap.width,pt->legend_pmap.height,False);
  
  // Draw the rectangle
  XDrawRectangle(pt->display,pt->win,pt->gc_text,l_x1,l_y1,l_x2-l_x1,l_y2-l_y1);
  
  // Show each layer
  x1 = l_x1+pt->widget_spacing;
  x2 = x1 + 20;
  y1 = l_y1+pt->widget_spacing+pt->font_height/2;
  nchars = (pt->legend_pmap.width - (pt->widget_spacing*2 + 20))/pt->font_width;
  
  for(i=0;i<pt->nfiles;i++){    
    for(j=0;j<(pt->flist[i]->framed ? pt->flist[i]->nelms:1);j++){
      // Set the color for this layer    
      XSetForeground(pt->display,pt->gc_data,pt->layer_info[j].layer_pixel);
    
      XDrawLine(pt->display,pt->win,pt->gc_data,x1,y1,x2,y1);
    
      XDrawString(pt->display,pt->win,pt->gc_text,x2+pt->widget_spacing,y1+pt->font_height/2,pt->flist[i]->base_name,MIN(strlen(pt->flist[i]->base_name),nchars));    
    
      y1 += pt->widget_spacing+pt->font_height;
    }
  }
  
  return;
}

void hide_legend(PLOT_T *pt){
  XCopyArea(pt->display,pt->legend_pmap.pmap,pt->win,pt->gc_text,0,0,pt->legend_pmap.width,pt->legend_pmap.height,pt->legend_pmap.x1,pt->legend_pmap.y1);
  
  XFlush(pt->display);
  
  return;
}

void refresh_plot(PLOT_T *pt){
  // Clear the plotting field
  XClearArea(pt->display, pt->win, pt->grid_x, pt->grid_y, pt->grid_width+1, pt->grid_height+1, False);
  
  // Clear the grid values
  // y-axis
  XClearArea(pt->display, pt->win, 0, pt->grid_y-(pt->font_height/2), pt->grid_x-3,pt->grid_height+pt->font_height,False);
  // x-axis
  XClearArea(pt->display, pt->win, 0, pt->grid_y2+3, pt->win_width,pt->font_height,False);
  
  // Clear the x-pan drag area
  XClearArea(pt->display, pt->win, pt->xpan_bar.drag_x1, 
                                   pt->xpan_bar.drag_y1, 
                                  (pt->xpan_bar.drag_x2-pt->xpan_bar.drag_x1)+1,
                                   pt->xpan_bar.ar_height,False);
  
  // Clear the y-pan drag area
  XClearArea(pt->display, pt->win, pt->ypan_bar.drag_x1, 
                                   pt->ypan_bar.drag_y1, 
                                  (pt->ypan_bar.drag_x2-pt->ypan_bar.drag_x1)+1,
                                  (pt->ypan_bar.drag_y2-pt->ypan_bar.drag_y1)+1,False);
  
  // Draw the pan drag areas
  draw_xpan_drag(pt);
  draw_ypan_drag(pt);
  
  // Replot the grid and axis values
  draw_grid(pt);
  
  // Replot the data
  plot_data(pt);
  
  if(pt->crosshair_on){
    // Zero these out because there's no need for the draw_crosshair
    // function to clear these as they've already been cleared by the 
    // call to XClearWindow above
    pt->crosshair_lines[0].x1 = 0;
    pt->crosshair_lines[1].x1 = 0;
    pt->crosshair_lines[2].x1 = 0;
    pt->crosshair_lines[3].x1 = 0;
    
    draw_crosshair(pt);
  }
  
  return;
}

void refresh_window(PLOT_T *pt){
  // Clear the entire window
  XClearWindow(pt->display,pt->win);
  
  // Draw plot redraws everything about the plot 
  draw_plot(pt);
  
  // Plot the data sets
  plot_data(pt);
  
  if(pt->crosshair_on){
    // Zero these out because there's no need for the draw_crosshair
    // function to clear these as they've already been cleared by the 
    // call to XClearWindow above
    pt->crosshair_lines[0].x1 = 0;
    pt->crosshair_lines[1].x1 = 0;
    pt->crosshair_lines[2].x1 = 0;
    pt->crosshair_lines[3].x1 = 0;
    
    draw_crosshair(pt);
  }
  
  return;
}