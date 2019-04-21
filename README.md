# x11fstest
X11 FULLSCREEN Test

## Purpose

This is the usual example found in X11 tutorials, but modified to be FULLSCREEN.  To exit, click inside the window.

Note that since FULLSCREEN mode is not well implemented and/or documented in X11 and Window Managers, the technique used here is simply to offset the window decorations out of the visible screen area.

Also, it would be a good thing to add further error checking on X11 function return.  However, the author of this little piece of source code requires better X11 documentation.  Constructive comments are welcome.  ;-)

## How to build and install x11fstest
1. Download the source files and store them in a directory
2. Go to that directory in a terminal window
3. To built the executable file, type `make`

## Version history
1.0 - 2019/04/21 - Initial release

## Compatibility
**x11fstest** has been tested under FreeBSD 11.2 and Debian 10, using libX11 1.6.7, Gnome 3.30.2 and qvwm 1.1.12_2015.  Should be easy to port because there are only X11 dependencies.

## Donations
Thanks for the support!  
Bitcoin: **1JbiV7rGE5kRKcecTfPv16SXag65o8aQTe**

# Have Fun!
