/*
 * cave/window.c version 20060614
 * D. J. Bernstein
 * Public domain.
 */

#define TIMEOUT 50 /* milliseconds between calls to background() */
#define WINDOWWIDTH 1024
#define WINDOWHEIGHT 768
#define FONT "-adobe-helvetica-bold-r-normal--18-180-75-75-p-103-iso10646-1"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>
#include <X11/keysym.h>
#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include "window.h"

static void (*redraw)(void);
static int (*keypress)(long long);
static int (*background)(void);

static XtAppContext a;
static Widget top;
static Widget draw;
static Display *display;
static Screen *screen;
static Window root;
static XGCValues gcv;
static GC gc;
static Pixmap pix;
static Colormap colormap;
#define COLORS 7
static XColor color[COLORS];

void window_fillrectangle(int x,int y,int width,int height,int c)
{
  c %= COLORS;
  XSetForeground(display,gc,color[c].pixel);
  XFillRectangle(display,pix,gc,x,y,width,height);
}

void window_drawrectangle(int x,int y,int width,int height,int c)
{
  c %= COLORS;
  XSetForeground(display,gc,color[c].pixel);
  XDrawRectangle(display,pix,gc,x,y,width,height);
}

void window_drawline(int x,int y,int x2,int y2,int c)
{
  c %= COLORS;
  XSetForeground(display,gc,color[c].pixel);
  XDrawRectangle(display,pix,gc,x,y,x2,y2);
}

void window_drawstring(int x,int y,const char *s,int c)
{
  c %= COLORS;
  XSetForeground(display,gc,color[c].pixel);
  XDrawString(display,pix,gc,x,y,s,strlen(s));
}

static void redrawcopy(void)
{
  window_fillrectangle(0,0,WINDOWWIDTH,WINDOWHEIGHT,1);
  if (redraw) redraw();
  XCopyArea(display,pix,XtWindow(draw),gc,0,0,WINDOWWIDTH,WINDOWHEIGHT,0,0);
}

static void expose(Widget draw,XtPointer client_data,XmDrawingAreaCallbackStruct *cbk)
{
  XCopyArea(cbk->event->xexpose.display,pix,cbk->window,gc,0,0,WINDOWWIDTH,WINDOWHEIGHT,0,0);
}

static void timeout(XtPointer client_data,XtIntervalId *id)
{
  XtAppAddTimeOut(a,TIMEOUT,timeout,0);
  if (background)
    if (background())
      redrawcopy();
}

static void input(Widget draw,XtPointer client_data,XmDrawingAreaCallbackStruct *cbk)
{
  XKeyEvent *e;
  long long result;

  e = (XKeyEvent *) cbk->event;
  if (e->type != KeyPress && e->type != KeyRelease) return;
  result = XLookupKeysym(e,0);
  /* states: 1 shift, 2 caps lock, 4 ctrl, 8 alt, 64 windows */
  if (e->state & 3) result += 1000000;
  if (e->state & 4) result += 2000000;
  if (e->state & 8) result += 4000000;
  if (e->type == KeyRelease) result += 1000000000;
  switch(keypress(result)) {
    case -1:
      XFlush(display);
      exit(0);
    case 1:
      redrawcopy();
      XSync(display,False);
  }
}

void window_exec(int argc,char **argv,void (*r)(void),int (*k)(long long),int (*b)(void))
{
  int i;
  unsigned short x;
  unsigned short y;

  redraw = r;
  keypress = k;
  background = b;

  /* XXX: catch various X errors */

  top = XtOpenApplication(&a,"draw",0,0,&argc,argv,0,applicationShellWidgetClass,0,0);
  XtMakeResizeRequest(top,WINDOWWIDTH,WINDOWHEIGHT,&x,&y);
  draw = XtVaCreateWidget("draw",xmDrawingAreaWidgetClass,top,NULL);
  display = XtDisplay(draw);
  screen = XtScreen(draw);
  root = RootWindowOfScreen(screen);
  gcv.foreground = BlackPixelOfScreen(XtScreen(draw));
  gc = XCreateGC(display,root,GCForeground,&gcv);
  pix = XCreatePixmap(display,root,WINDOWWIDTH,WINDOWHEIGHT,DefaultDepthOfScreen(screen));
  colormap = DefaultColormap(display,DefaultScreen(display));

  for (i = 0;i < COLORS;++i) {
    color[i].flags = DoRed | DoGreen | DoBlue;
    switch(i) {
      case 1: color[i].red = 65535; color[i].green = 65535; color[i].blue = 65535; break;
      case 2: color[i].red = 65535; color[i].green = 0; color[i].blue = 0; break;
      case 3: color[i].red = 0; color[i].green = 40000; color[i].blue = 0; break;
      case 4: color[i].red = 0; color[i].green = 0; color[i].blue = 65535; break;
      case 5: color[i].red = 40000; color[i].green = 0; color[i].blue = 40000; break;
      case 6: color[i].red = 0; color[i].green = 50000; color[i].blue = 50000; break;
      default: color[i].red = 0; color[i].green = 0; color[i].blue = 0;
    }
    XAllocColor(display,colormap,&color[i]);
  }

  XSetFont(display,gc,XLoadFont(display,FONT));

  XtAddCallback(draw,XmNexposeCallback,(XtCallbackProc) expose,0);
  XtAddCallback(draw,XmNinputCallback,(XtCallbackProc) input,0);
  XtAppAddTimeOut(a,TIMEOUT,(XtTimerCallbackProc) timeout,0);

  XtManageChild(draw);
  XtRealizeWidget(top);

  redrawcopy();

  XtAppMainLoop(a);
  exit(111);
}
