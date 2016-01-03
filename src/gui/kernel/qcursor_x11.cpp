/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qdebug.h>
#include <qdatastream.h>
#include <qcursor_p.h>
#include <qt_x11_p.h>
#include <qapplication_p.h>
#include <qbitmap.h>
#include <qcursor.h>
#include <X11/cursorfont.h>
#include <qlibrary.h>

#ifndef QT_NO_XCURSOR
#  include <X11/Xcursor/Xcursor.h>
#endif

#ifndef QT_NO_XFIXES
#ifndef Status
#define Status int
#endif
#  include <X11/extensions/Xfixes.h>
#endif

#include <qx11info_x11.h>
#include <qpixmap_x11_p.h>

QT_BEGIN_NAMESPACE

// Define QT_USE_APPROXIMATE_CURSORS when compiling if you REALLY want to use the ugly X11 cursors.

/*****************************************************************************
  Internal QCursorData class
 *****************************************************************************/

QCursorData::QCursorData(Qt::CursorShape s)
   : cshape(s), bm(0), bmm(0), hx(0), hy(0), hcurs(0), pm(0), pmm(0)
{
   ref = 1;
}

QCursorData::~QCursorData()
{
   Display *dpy = X11 ? X11->display : (Display *)0;

   // Add in checking for the display too as on HP-UX
   // we seem to get a core dump as the cursor data is
   // deleted again from main() on exit...
   if (hcurs && dpy) {
      XFreeCursor(dpy, hcurs);
   }
   if (pm && dpy) {
      XFreePixmap(dpy, pm);
   }
   if (pmm && dpy) {
      XFreePixmap(dpy, pmm);
   }
   delete bm;
   delete bmm;
}

#ifndef QT_NO_CURSOR
QCursor::QCursor(Qt::HANDLE cursor)
{
   if (!QCursorData::initialized) {
      QCursorData::initialize();
   }
   d = new QCursorData(Qt::CustomCursor);
   d->hcurs = cursor;
}

#endif

QCursorData *QCursorData::setBitmap(const QBitmap &bitmap, const QBitmap &mask, int hotX, int hotY)
{
   if (!QCursorData::initialized) {
      QCursorData::initialize();
   }
   if (bitmap.depth() != 1 || mask.depth() != 1 || bitmap.size() != mask.size()) {
      qWarning("QCursor: Cannot create bitmap cursor; invalid bitmap(s)");
      QCursorData *c = qt_cursorTable[0];
      c->ref.ref();
      return c;
   }
   QCursorData *d = new QCursorData;
   d->ref = 1;

   extern QPixmap qt_toX11Pixmap(const QPixmap & pixmap); // qpixmap_x11.cpp
   d->bm  = new QBitmap(qt_toX11Pixmap(bitmap));
   d->bmm = new QBitmap(qt_toX11Pixmap(mask));

   d->hcurs = 0;
   d->cshape = Qt::BitmapCursor;
   d->hx = hotX >= 0 ? hotX : bitmap.width() / 2;
   d->hy = hotY >= 0 ? hotY : bitmap.height() / 2;
   d->fg.red   = 0x0000;
   d->fg.green = 0x0000;
   d->fg.blue  = 0x0000;
   d->bg.red   = 0xffff;
   d->bg.green = 0xffff;
   d->bg.blue  = 0xffff;
   return d;
}



#ifndef QT_NO_CURSOR
Qt::HANDLE QCursor::handle() const
{
   if (!QCursorData::initialized) {
      QCursorData::initialize();
   }
   if (!d->hcurs) {
      d->update();
   }
   return d->hcurs;
}
#endif

QPoint QCursor::pos()
{
   Window root;
   Window child;
   int root_x, root_y, win_x, win_y;
   uint buttons;
   Display *dpy = X11->display;
   for (int i = 0; i < ScreenCount(dpy); ++i) {
      if (XQueryPointer(dpy, QX11Info::appRootWindow(i), &root, &child, &root_x, &root_y,
                        &win_x, &win_y, &buttons))

      {
         return QPoint(root_x, root_y);
      }
   }
   return QPoint();
}

/*! \internal
*/
#ifndef QT_NO_CURSOR
int QCursor::x11Screen()
{
   Window root;
   Window child;
   int root_x, root_y, win_x, win_y;
   uint buttons;
   Display *dpy = X11->display;
   for (int i = 0; i < ScreenCount(dpy); ++i) {
      if (XQueryPointer(dpy, QX11Info::appRootWindow(i), &root, &child, &root_x, &root_y,
                        &win_x, &win_y, &buttons)) {
         return i;
      }
   }
   return -1;
}
#endif

void QCursor::setPos(int x, int y)
{
   QPoint current, target(x, y);

   // this is copied from pos(), since we need the screen number for the correct
   // root window in the XWarpPointer call
   Window root;
   Window child;
   int root_x, root_y, win_x, win_y;
   uint buttons;
   Display *dpy = X11->display;
   int screen;
   for (screen = 0; screen < ScreenCount(dpy); ++screen) {
      if (XQueryPointer(dpy, QX11Info::appRootWindow(screen), &root, &child, &root_x, &root_y,
                        &win_x, &win_y, &buttons)) {
         current = QPoint(root_x, root_y);
         break;
      }
   }

   if (screen >= ScreenCount(dpy)) {
      return;
   }

   // Need to check, since some X servers generate null mouse move
   // events, causing looping in applications which call setPos() on
   // every mouse move event.
   //
   if (current == target) {
      return;
   }

   XWarpPointer(X11->display, XNone, QX11Info::appRootWindow(screen), 0, 0, 0, 0, x, y);
}


/*!
    \internal

    Creates the cursor.
*/

void QCursorData::update()
{
   if (!QCursorData::initialized) {
      QCursorData::initialize();
   }
   if (hcurs) {
      return;
   }

   Display *dpy = X11->display;
   Window rootwin = QX11Info::appRootWindow();

   if (cshape == Qt::BitmapCursor) {
      extern QPixmap qt_toX11Pixmap(const QPixmap & pixmap); // qpixmap_x11.cpp
#ifndef QT_NO_XRENDER
      if (!pixmap.isNull() && X11->use_xrender) {
         pixmap = qt_toX11Pixmap(pixmap);
         hcurs = XRenderCreateCursor (X11->display, pixmap.x11PictureHandle(), hx, hy);
      } else
#endif
      {
         hcurs = XCreatePixmapCursor(dpy, bm->handle(), bmm->handle(), &fg, &bg, hx, hy);
      }
      return;
   }

   static const char *cursorNames[] = {
      "left_ptr",
      "up_arrow",
      "cross",
      "wait",
      "ibeam",
      "size_ver",
      "size_hor",
      "size_bdiag",
      "size_fdiag",
      "size_all",
      "blank",
      "split_v",
      "split_h",
      "pointing_hand",
      "forbidden",
      "whats_this",
      "left_ptr_watch",
      "openhand",
      "closedhand",
      "copy",
      "move",
      "link"
   };

#ifndef QT_NO_XCURSOR
   if (X11->ptrXcursorLibraryLoadCursor) {
      // special case for non-standard dnd-* cursors
      switch (cshape) {
         case Qt::DragCopyCursor:
            hcurs = X11->ptrXcursorLibraryLoadCursor(dpy, "dnd-copy");
            break;
         case Qt::DragMoveCursor:
            hcurs = X11->ptrXcursorLibraryLoadCursor(dpy, "dnd-move");
            break;
         case Qt::DragLinkCursor:
            hcurs = X11->ptrXcursorLibraryLoadCursor(dpy, "dnd-link");
            break;
         default:
            break;
      }
      if (!hcurs) {
         hcurs = X11->ptrXcursorLibraryLoadCursor(dpy, cursorNames[cshape]);
      }
   }
   if (hcurs) {
      return;
   }
#endif // QT_NO_XCURSOR

   static const uchar cur_blank_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
   };

   // Non-standard X11 cursors are created from bitmaps

#ifndef QT_USE_APPROXIMATE_CURSORS
   static const uchar cur_ver_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0xc0, 0x03, 0xe0, 0x07, 0xf0, 0x0f,
      0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0xf0, 0x0f,
      0xe0, 0x07, 0xc0, 0x03, 0x80, 0x01, 0x00, 0x00
   };
   static const uchar mcur_ver_bits[] = {
      0x00, 0x00, 0x80, 0x03, 0xc0, 0x07, 0xe0, 0x0f, 0xf0, 0x1f, 0xf8, 0x3f,
      0xfc, 0x7f, 0xc0, 0x07, 0xc0, 0x07, 0xc0, 0x07, 0xfc, 0x7f, 0xf8, 0x3f,
      0xf0, 0x1f, 0xe0, 0x0f, 0xc0, 0x07, 0x80, 0x03
   };
   static const uchar cur_hor_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x08, 0x30, 0x18,
      0x38, 0x38, 0xfc, 0x7f, 0xfc, 0x7f, 0x38, 0x38, 0x30, 0x18, 0x20, 0x08,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
   };
   static const uchar mcur_hor_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0x40, 0x04, 0x60, 0x0c, 0x70, 0x1c, 0x78, 0x3c,
      0xfc, 0x7f, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfc, 0x7f, 0x78, 0x3c,
      0x70, 0x1c, 0x60, 0x0c, 0x40, 0x04, 0x00, 0x00
   };
   static const uchar cur_bdiag_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x3e,
      0x00, 0x37, 0x88, 0x23, 0xd8, 0x01, 0xf8, 0x00, 0x78, 0x00, 0xf8, 0x00,
      0xf8, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
   };
   static const uchar mcur_bdiag_bits[] = {
      0x00, 0x00, 0xc0, 0x7f, 0x80, 0x7f, 0x00, 0x7f, 0x00, 0x7e, 0x04, 0x7f,
      0x8c, 0x7f, 0xdc, 0x77, 0xfc, 0x63, 0xfc, 0x41, 0xfc, 0x00, 0xfc, 0x01,
      0xfc, 0x03, 0xfc, 0x07, 0x00, 0x00, 0x00, 0x00
   };
   static const uchar cur_fdiag_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x01, 0xf8, 0x00, 0x78, 0x00,
      0xf8, 0x00, 0xd8, 0x01, 0x88, 0x23, 0x00, 0x37, 0x00, 0x3e, 0x00, 0x3c,
      0x00, 0x3e, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00
   };
   static const uchar mcur_fdiag_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0xfc, 0x07, 0xfc, 0x03, 0xfc, 0x01, 0xfc, 0x00,
      0xfc, 0x41, 0xfc, 0x63, 0xdc, 0x77, 0x8c, 0x7f, 0x04, 0x7f, 0x00, 0x7e,
      0x00, 0x7f, 0x80, 0x7f, 0xc0, 0x7f, 0x00, 0x00
   };
   static const uchar *cursor_bits16[] = {
      cur_ver_bits, mcur_ver_bits, cur_hor_bits, mcur_hor_bits,
      cur_bdiag_bits, mcur_bdiag_bits, cur_fdiag_bits, mcur_fdiag_bits,
      0, 0, cur_blank_bits, cur_blank_bits
   };

   static const uchar vsplit_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x80, 0x00, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xe0, 0x03, 0x00,
      0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
      0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x00,
      0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
      0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00,
      0x00, 0xc0, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
   };
   static const uchar vsplitm_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
      0x00, 0xc0, 0x01, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0xf0, 0x07, 0x00,
      0x00, 0xf8, 0x0f, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x01, 0x00,
      0x00, 0xc0, 0x01, 0x00, 0x80, 0xff, 0xff, 0x00, 0x80, 0xff, 0xff, 0x00,
      0x80, 0xff, 0xff, 0x00, 0x80, 0xff, 0xff, 0x00, 0x80, 0xff, 0xff, 0x00,
      0x80, 0xff, 0xff, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x01, 0x00,
      0x00, 0xc0, 0x01, 0x00, 0x00, 0xf8, 0x0f, 0x00, 0x00, 0xf0, 0x07, 0x00,
      0x00, 0xe0, 0x03, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
   };
   static const uchar hsplit_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00,
      0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00,
      0x00, 0x41, 0x82, 0x00, 0x80, 0x41, 0x82, 0x01, 0xc0, 0x7f, 0xfe, 0x03,
      0x80, 0x41, 0x82, 0x01, 0x00, 0x41, 0x82, 0x00, 0x00, 0x40, 0x02, 0x00,
      0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00,
      0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
   };
   static const uchar hsplitm_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00,
      0x00, 0xe0, 0x07, 0x00, 0x00, 0xe2, 0x47, 0x00, 0x00, 0xe3, 0xc7, 0x00,
      0x80, 0xe3, 0xc7, 0x01, 0xc0, 0xff, 0xff, 0x03, 0xe0, 0xff, 0xff, 0x07,
      0xc0, 0xff, 0xff, 0x03, 0x80, 0xe3, 0xc7, 0x01, 0x00, 0xe3, 0xc7, 0x00,
      0x00, 0xe2, 0x47, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00,
      0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
   };
   static const uchar whatsthis_bits[] = {
      0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x05, 0xf0, 0x07, 0x00,
      0x09, 0x18, 0x0e, 0x00, 0x11, 0x1c, 0x0e, 0x00, 0x21, 0x1c, 0x0e, 0x00,
      0x41, 0x1c, 0x0e, 0x00, 0x81, 0x1c, 0x0e, 0x00, 0x01, 0x01, 0x07, 0x00,
      0x01, 0x82, 0x03, 0x00, 0xc1, 0xc7, 0x01, 0x00, 0x49, 0xc0, 0x01, 0x00,
      0x95, 0xc0, 0x01, 0x00, 0x93, 0xc0, 0x01, 0x00, 0x21, 0x01, 0x00, 0x00,
      0x20, 0xc1, 0x01, 0x00, 0x40, 0xc2, 0x01, 0x00, 0x40, 0x02, 0x00, 0x00,
      0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   };
   static const uchar whatsthism_bits[] = {
      0x01, 0x00, 0x00, 0x00, 0x03, 0xf0, 0x07, 0x00, 0x07, 0xf8, 0x0f, 0x00,
      0x0f, 0xfc, 0x1f, 0x00, 0x1f, 0x3e, 0x1f, 0x00, 0x3f, 0x3e, 0x1f, 0x00,
      0x7f, 0x3e, 0x1f, 0x00, 0xff, 0x3e, 0x1f, 0x00, 0xff, 0x9d, 0x0f, 0x00,
      0xff, 0xc3, 0x07, 0x00, 0xff, 0xe7, 0x03, 0x00, 0x7f, 0xe0, 0x03, 0x00,
      0xf7, 0xe0, 0x03, 0x00, 0xf3, 0xe0, 0x03, 0x00, 0xe1, 0xe1, 0x03, 0x00,
      0xe0, 0xe1, 0x03, 0x00, 0xc0, 0xe3, 0x03, 0x00, 0xc0, 0xe3, 0x03, 0x00,
      0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   };
   static const uchar busy_bits[] = {
      0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
      0x09, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00,
      0x41, 0xe0, 0xff, 0x00, 0x81, 0x20, 0x80, 0x00, 0x01, 0xe1, 0xff, 0x00,
      0x01, 0x42, 0x40, 0x00, 0xc1, 0x47, 0x40, 0x00, 0x49, 0x40, 0x55, 0x00,
      0x95, 0x80, 0x2a, 0x00, 0x93, 0x00, 0x15, 0x00, 0x21, 0x01, 0x0a, 0x00,
      0x20, 0x01, 0x11, 0x00, 0x40, 0x82, 0x20, 0x00, 0x40, 0x42, 0x44, 0x00,
      0x80, 0x41, 0x4a, 0x00, 0x00, 0x40, 0x55, 0x00, 0x00, 0xe0, 0xff, 0x00,
      0x00, 0x20, 0x80, 0x00, 0x00, 0xe0, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
   };
   static const uchar busym_bits[] = {
      0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
      0x0f, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00,
      0x7f, 0xe0, 0xff, 0x00, 0xff, 0xe0, 0xff, 0x00, 0xff, 0xe1, 0xff, 0x00,
      0xff, 0xc3, 0x7f, 0x00, 0xff, 0xc7, 0x7f, 0x00, 0x7f, 0xc0, 0x7f, 0x00,
      0xf7, 0x80, 0x3f, 0x00, 0xf3, 0x00, 0x1f, 0x00, 0xe1, 0x01, 0x0e, 0x00,
      0xe0, 0x01, 0x1f, 0x00, 0xc0, 0x83, 0x3f, 0x00, 0xc0, 0xc3, 0x7f, 0x00,
      0x80, 0xc1, 0x7f, 0x00, 0x00, 0xc0, 0x7f, 0x00, 0x00, 0xe0, 0xff, 0x00,
      0x00, 0xe0, 0xff, 0x00, 0x00, 0xe0, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
   };

   static const uchar *const cursor_bits32[] = {
      vsplit_bits, vsplitm_bits, hsplit_bits, hsplitm_bits,
      0, 0, 0, 0, whatsthis_bits, whatsthism_bits, busy_bits, busym_bits
   };

   static const uchar forbidden_bits[] = {
      0x00, 0x00, 0x00, 0x80, 0x1f, 0x00, 0xe0, 0x7f, 0x00, 0xf0, 0xf0, 0x00, 0x38, 0xc0, 0x01,
      0x7c, 0x80, 0x03, 0xec, 0x00, 0x03, 0xce, 0x01, 0x07, 0x86, 0x03, 0x06, 0x06, 0x07, 0x06,
      0x06, 0x0e, 0x06, 0x06, 0x1c, 0x06, 0x0e, 0x38, 0x07, 0x0c, 0x70, 0x03, 0x1c, 0xe0, 0x03,
      0x38, 0xc0, 0x01, 0xf0, 0xe0, 0x00, 0xe0, 0x7f, 0x00, 0x80, 0x1f, 0x00, 0x00, 0x00, 0x00
   };

   static const uchar forbiddenm_bits[] = {
      0x80, 0x1f, 0x00, 0xe0, 0x7f, 0x00, 0xf0, 0xff, 0x00, 0xf8, 0xff, 0x01, 0xfc, 0xf0, 0x03,
      0xfe, 0xc0, 0x07, 0xfe, 0x81, 0x07, 0xff, 0x83, 0x0f, 0xcf, 0x07, 0x0f, 0x8f, 0x0f, 0x0f,
      0x0f, 0x1f, 0x0f, 0x0f, 0x3e, 0x0f, 0x1f, 0xfc, 0x0f, 0x1e, 0xf8, 0x07, 0x3e, 0xf0, 0x07,
      0xfc, 0xe0, 0x03, 0xf8, 0xff, 0x01, 0xf0, 0xff, 0x00, 0xe0, 0x7f, 0x00, 0x80, 0x1f, 0x00
   };

   static const uchar openhand_bits[] = {
      0x80, 0x01, 0x58, 0x0e, 0x64, 0x12, 0x64, 0x52, 0x48, 0xb2, 0x48, 0x92,
      0x16, 0x90, 0x19, 0x80, 0x11, 0x40, 0x02, 0x40, 0x04, 0x40, 0x04, 0x20,
      0x08, 0x20, 0x10, 0x10, 0x20, 0x10, 0x00, 0x00
   };
   static const uchar openhandm_bits[] = {
      0x80, 0x01, 0xd8, 0x0f, 0xfc, 0x1f, 0xfc, 0x5f, 0xf8, 0xff, 0xf8, 0xff,
      0xf6, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xfe, 0x7f, 0xfc, 0x7f, 0xfc, 0x3f,
      0xf8, 0x3f, 0xf0, 0x1f, 0xe0, 0x1f, 0x00, 0x00
   };
   static const uchar closedhand_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb0, 0x0d, 0x48, 0x32, 0x08, 0x50,
      0x10, 0x40, 0x18, 0x40, 0x04, 0x40, 0x04, 0x20, 0x08, 0x20, 0x10, 0x10,
      0x20, 0x10, 0x20, 0x10, 0x00, 0x00, 0x00, 0x00
   };
   static const uchar closedhandm_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb0, 0x0d, 0xf8, 0x3f, 0xf8, 0x7f,
      0xf0, 0x7f, 0xf8, 0x7f, 0xfc, 0x7f, 0xfc, 0x3f, 0xf8, 0x3f, 0xf0, 0x1f,
      0xe0, 0x1f, 0xe0, 0x1f, 0x00, 0x00, 0x00, 0x00
   };

   static const uchar *const cursor_bits20[] = {
      forbidden_bits, forbiddenm_bits
   };

   if ((cshape >= Qt::SizeVerCursor && cshape < Qt::SizeAllCursor)
         || cshape == Qt::BlankCursor) {
      XColor bg, fg;
      bg.red   = 255 << 8;
      bg.green = 255 << 8;
      bg.blue  = 255 << 8;
      fg.red   = 0;
      fg.green = 0;
      fg.blue  = 0;
      int i = (cshape - Qt::SizeVerCursor) * 2;
      pm  = XCreateBitmapFromData(dpy, rootwin, reinterpret_cast<const char *>(cursor_bits16[i]), 16, 16);
      pmm = XCreateBitmapFromData(dpy, rootwin, reinterpret_cast<const char *>(cursor_bits16[i + 1]), 16, 16);
      hcurs = XCreatePixmapCursor(dpy, pm, pmm, &fg, &bg, 8, 8);
   } else if ((cshape >= Qt::SplitVCursor && cshape <= Qt::SplitHCursor)
              || cshape == Qt::WhatsThisCursor || cshape == Qt::BusyCursor) {
      XColor bg, fg;
      bg.red   = 255 << 8;
      bg.green = 255 << 8;
      bg.blue  = 255 << 8;
      fg.red   = 0;
      fg.green = 0;
      fg.blue  = 0;
      int i = (cshape - Qt::SplitVCursor) * 2;
      pm  = XCreateBitmapFromData(dpy, rootwin, reinterpret_cast<const char *>(cursor_bits32[i]), 32, 32);
      pmm = XCreateBitmapFromData(dpy, rootwin, reinterpret_cast<const char *>(cursor_bits32[i + 1]), 32, 32);
      int hs = (cshape == Qt::PointingHandCursor || cshape == Qt::WhatsThisCursor
                || cshape == Qt::BusyCursor) ? 0 : 16;
      hcurs = XCreatePixmapCursor(dpy, pm, pmm, &fg, &bg, hs, hs);
   } else if (cshape == Qt::ForbiddenCursor) {
      XColor bg, fg;
      bg.red   = 255 << 8;
      bg.green = 255 << 8;
      bg.blue  = 255 << 8;
      fg.red   = 0;
      fg.green = 0;
      fg.blue  = 0;
      int i = (cshape - Qt::ForbiddenCursor) * 2;
      pm  = XCreateBitmapFromData(dpy, rootwin, reinterpret_cast<const char *>(cursor_bits20[i]), 20, 20);
      pmm = XCreateBitmapFromData(dpy, rootwin, reinterpret_cast<const char *>(cursor_bits20[i + 1]), 20, 20);
      hcurs = XCreatePixmapCursor(dpy, pm, pmm, &fg, &bg, 10, 10);
   } else if (cshape == Qt::OpenHandCursor || cshape == Qt::ClosedHandCursor) {
      XColor bg, fg;
      bg.red   = 255 << 8;
      bg.green = 255 << 8;
      bg.blue  = 255 << 8;
      fg.red   = 0;
      fg.green = 0;
      fg.blue  = 0;
      bool open = cshape == Qt::OpenHandCursor;
      pm  = XCreateBitmapFromData(dpy, rootwin, reinterpret_cast<const char *>(open ? openhand_bits : closedhand_bits), 16,
                                  16);
      pmm = XCreateBitmapFromData(dpy, rootwin, reinterpret_cast<const char *>(open ? openhandm_bits : closedhandm_bits), 16,
                                  16);
      hcurs = XCreatePixmapCursor(dpy, pm, pmm, &fg, &bg, 8, 8);
   } else if (cshape == Qt::DragCopyCursor || cshape == Qt::DragMoveCursor
              || cshape == Qt::DragLinkCursor) {
      XColor bg, fg;
      bg.red   = 255 << 8;
      bg.green = 255 << 8;
      bg.blue  = 255 << 8;
      fg.red   = 0;
      fg.green = 0;
      fg.blue  = 0;
      QImage image = QApplicationPrivate::instance()->getPixmapCursor(cshape).toImage();
      pm = QX11PixmapData::createBitmapFromImage(image);
      pmm = QX11PixmapData::createBitmapFromImage(image.createAlphaMask().convertToFormat(QImage::Format_MonoLSB));
      hcurs = XCreatePixmapCursor(dpy, pm, pmm, &fg, &bg, 8, 8);
   }

   if (hcurs) {
#ifndef QT_NO_XFIXES
      if (X11->use_xfixes && X11->ptrXFixesSetCursorName) {
         X11->ptrXFixesSetCursorName(dpy, hcurs, cursorNames[cshape]);
      }
#endif /* ! QT_NO_XFIXES */
      return;
   }

#endif /* ! QT_USE_APPROXIMATE_CURSORS */

   uint sh;
   switch (cshape) {                        // map Q cursor to X cursor
      case Qt::ArrowCursor:
         sh = XC_left_ptr;
         break;
      case Qt::UpArrowCursor:
         sh = XC_center_ptr;
         break;
      case Qt::CrossCursor:
         sh = XC_crosshair;
         break;
      case Qt::WaitCursor:
         sh = XC_watch;
         break;
      case Qt::IBeamCursor:
         sh = XC_xterm;
         break;
      case Qt::SizeAllCursor:
         sh = XC_fleur;
         break;
      case Qt::PointingHandCursor:
         sh = XC_hand2;
         break;
#ifdef QT_USE_APPROXIMATE_CURSORS
      case Qt::SizeBDiagCursor:
         sh = XC_top_right_corner;
         break;
      case Qt::SizeFDiagCursor:
         sh = XC_bottom_right_corner;
         break;
      case Qt::BlankCursor:
         XColor bg, fg;
         bg.red   = 255 << 8;
         bg.green = 255 << 8;
         bg.blue  = 255 << 8;
         fg.red   = 0;
         fg.green = 0;
         fg.blue  = 0;
         pm  = XCreateBitmapFromData(dpy, rootwin, cur_blank_bits, 16, 16);
         pmm = XCreateBitmapFromData(dpy, rootwin, cur_blank_bits, 16, 16);
         hcurs = XCreatePixmapCursor(dpy, pm, pmm, &fg, &bg, 8, 8);
         return;
         break;
      case Qt::SizeVerCursor:
      case Qt::SplitVCursor:
         sh = XC_sb_v_double_arrow;
         break;
      case Qt::SizeHorCursor:
      case Qt::SplitHCursor:
         sh = XC_sb_h_double_arrow;
         break;
      case Qt::WhatsThisCursor:
         sh = XC_question_arrow;
         break;
      case Qt::ForbiddenCursor:
         sh = XC_circle;
         break;
      case Qt::BusyCursor:
         sh = XC_watch;
         break;
      case Qt::DragCopyCursor:
         sh = XC_tcross;
         break;
      case Qt::DragLinkCursor:
         sh = XC_center_ptr;
         break;
      case Qt::DragMoveCursor:
         sh = XC_top_left_arrow;
         break;
#endif /* QT_USE_APPROXIMATE_CURSORS */
      default:
         qWarning("QCursor::update: Invalid cursor shape %d", cshape);
         return;
   }
   hcurs = XCreateFontCursor(dpy, sh);

#ifndef QT_NO_XFIXES
   if (X11->use_xfixes && X11->ptrXFixesSetCursorName) {
      X11->ptrXFixesSetCursorName(dpy, hcurs, cursorNames[cshape]);
   }
#endif /* ! QT_NO_XFIXES */
}

QT_END_NAMESPACE
