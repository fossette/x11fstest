/*
 * File:        x11fstest.c
 *
 * Author:      fossette
 *
 * Date:        2019/04/21
 *
 * Version:     1.0
 *
 * Description: This is the usual example found in X11 tutorials,
 *              but modified to be FULLSCREEN.  To exit, click inside
 *              the window.  Tested under FreeBSD 11.2 and Debian 10,
 *              using libX11 1.6.7, Gnome 3.30.2 and qvwm 1.1.12_2015.
 *              Should be easy to port because there are only X11
 *              dependencies.
 *
 *              Note that since FULLSCREEN mode is not well implemented
 *              and/or documented in X11 and Window Managers, the technique
 *              used here is simply to offset the window decorations out
 *              of the visible screen area.
 *
 *              Also, it would be a good thing to add further error checking
 *              on X11 function return.  However, the author of this little
 *              piece of source code requires better X11 documentation.
 *              Constructive comments are welcome.  ;-)
 *
 * Parameter:   None.
 *
 * Web:         https://github.com/fossette/x11fstest/wiki
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/extensions/xf86vmode.h>


int
main(int argc, char* argv[])
{
   int                  iDotClock,
                        iOffsetX,
                        iOffsetY,
                        iRet,
                        iX11Screen;
   long                 iEventMask;
   unsigned int         iScreenX,
                        iScreenY;
   unsigned long        iX11Black,
                        iX11White;
   char                 szErr[200];
   Atom                 aMotifWmHints,
                        aWmState,
                        aWmStateFs;
   Display              *pX11Display;
   GC                   gc = 0;
   Window               w = 0,
                        w2,
                        wRoot;
   XEvent               ev;
   XF86VidModeModeLine  modeLine;

 
   *szErr = 0;

   //
   // Get the default X11 display
   //
   pX11Display = XOpenDisplay(NULL);
   if (!pX11Display)
      strcat(szErr, "XOpenDisplay(NULL) failed!");

   if (!(*szErr))
   {
      printf("X11 Protocol Version %d, Revision %d.\n",
             ProtocolVersion(pX11Display), ProtocolRevision(pX11Display));

      //
      // Fetch X11 macro values
      //
      iX11Screen =   DefaultScreen(pX11Display);
      iX11Black =    BlackPixel(pX11Display, iX11Screen);
      iX11White =    WhitePixel(pX11Display, iX11Screen);
      wRoot =        DefaultRootWindow(pX11Display);

      //
      // Find out the screen size using XFree86-VidModeExtension
      //
      memset(&modeLine, 0, sizeof(modeLine));
      XF86VidModeGetModeLine(pX11Display, iX11Screen,
                                  &iDotClock, &modeLine);
      iScreenX = modeLine.hdisplay;
      iScreenY = modeLine.vdisplay;
      if (modeLine.private)
      {
         XFree(modeLine.private);
         modeLine.privsize = 0;
         modeLine.private = NULL;
      }
      printf("X11 Screen Size: %dx%d.\n", iScreenX, iScreenY);

      //
      // To be sure, set the viewport to origin using
      // XFree86-VidModeExtension
      //
      XF86VidModeSetViewPort(pX11Display, iX11Screen, 0, 0);

      //
      // Create the window
      //
      w = XCreateSimpleWindow(pX11Display, wRoot, 0, 0,
             iScreenX, iScreenY, 0, iX11Black, iX11White);
      if (!w)
         sprintf(szErr, "XCreateSimpleWindow(root=0x%lX) failed!", wRoot);
   }
   if (!(*szErr))
   {
      //
      // Remove the window decorations
      //
      struct
      {
         unsigned long flags;
         unsigned long functions;
         unsigned long decorations;
         long input_mode;
         unsigned long status;
      } sMotifWmHints;
      sMotifWmHints.flags = 2;   // MWM_HINTS_DECORATIONS;
      sMotifWmHints.decorations = 0;
      aMotifWmHints = XInternAtom(pX11Display, "_MOTIF_WM_HINTS", False);
      XChangeProperty(pX11Display, w, aMotifWmHints, aMotifWmHints, 32,
                      PropModeReplace, (unsigned char *)&sMotifWmHints,
                      sizeof(sMotifWmHints) / sizeof(long));

      //
      // Map the window on top of every other windows
      //
      XMapRaised(pX11Display, w);

      //
      // Wait for the window to be created
      //
      iEventMask = StructureNotifyMask;
      XSelectInput(pX11Display, w, iEventMask);
      do
      {
         XNextEvent(pX11Display,     &ev);
      }
      while(ev.type != MapNotify) ;

      //
      // Notify the parent window
      //
      aWmState = XInternAtom(pX11Display, "_NET_WM_STATE", False);
      aWmStateFs = XInternAtom(pX11Display, "_NET_WM_STATE_FULLSCREEN", False);
      memset(&ev, 0, sizeof(ev));
      ev.type = ClientMessage;
      ev.xclient.window = w;
      ev.xclient.message_type = aWmState;
      ev.xclient.format = 32;
      ev.xclient.data.l[0] = 1;
      ev.xclient.data.l[1] = aWmStateFs;
      ev.xclient.data.l[2] = 0;
      if (!XSendEvent(pX11Display, wRoot, 0, SubstructureNotifyMask, &ev))
         printf("WARNING: XSendEvent(_NET_WM_STATE) aborted!\n");

      //
      // Move the window decorations out of the viewport
      //
      if (XTranslateCoordinates(pX11Display, w, wRoot, 0, 0,
                                     &iOffsetX, &iOffsetY, &w2))
      {
         if (iOffsetX > 0 || iOffsetY > 0)
            iRet = XMoveWindow(pX11Display, w, -iOffsetX, -iOffsetY);
      }
      else
         printf("WARNING: X11 Screen Issue, Can't go fullscreen!\n");

      //
      // Draw the X in the top-left corner
      //
      sleep(1);
      gc = XCreateGC(pX11Display, w, 0, NULL);
      if (!gc)
         sprintf(szErr, "XCreateGC(w=0x%lX) failed!", w);
   }
   if (!(*szErr))
   {
      XSetForeground(pX11Display, gc, iX11Black);
      XDrawLine(pX11Display, w, gc, 10, 10,190,190);
      XDrawLine(pX11Display, w, gc, 10,190,190, 10);

      //
      // Wait for the button to be clicked
      //
      iEventMask = ButtonPressMask | ButtonReleaseMask;
      XSelectInput(pX11Display, w, iEventMask);
      do
      {
         XNextEvent(pX11Display,     &ev);
      }
      while(ev.type != ButtonRelease) ;
   }

   if (*szErr)
      printf("ERROR: %s\n", szErr);

   if (gc)
      XFreeGC(pX11Display, gc);
   if (w)
      XDestroyWindow(pX11Display, w);
   if (pX11Display)
      XCloseDisplay(pX11Display);

   return 0;
}
