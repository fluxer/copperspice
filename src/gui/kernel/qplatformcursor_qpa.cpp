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

#include <qplatformcursor_qpa.h>

#include <QWidget>
#include <QPainter>
#include <QBitmap>
#include <QApplication>

#include <QDebug>

QT_BEGIN_NAMESPACE

QList <QWeakPointer<QPlatformCursor> > QPlatformCursorPrivate::instances;

QPlatformCursor::QPlatformCursor(QPlatformScreen *scr )
   : screen(scr)
{
   QPlatformCursorPrivate::instances.append(this);
}

extern int qt_last_x;
extern int qt_last_y;

QPoint QPlatformCursor::pos() const
{
   // As a fallback return the last mouse position seen by QApplication.
   return QPoint(qt_last_x, qt_last_y);
}

void QPlatformCursor::setPos(const QPoint &pos)
{
   Q_UNUSED(pos);
   qWarning("This plugin does not support QCursor::setPos()");
}

// End of display and pointer event handling code
// Beginning of built-in cursor graphics
// from src/gui/embedded/QGraphicsSystemCursorImage_qws.cpp

static QPlatformCursorImage *systemCursorTable[Qt::LastCursor + 1];
static bool systemCursorTableInit = false;

// 16 x 16
static const uchar cur_arrow_bits[] = {
   0x07, 0x00, 0x39, 0x00, 0xc1, 0x01, 0x02, 0x0e, 0x02, 0x10, 0x02, 0x08,
   0x04, 0x04, 0x04, 0x02, 0x04, 0x04, 0x88, 0x08, 0x48, 0x11, 0x28, 0x22,
   0x10, 0x44, 0x00, 0x28, 0x00, 0x10, 0x00, 0x00
};
static const uchar mcur_arrow_bits[] = {
   0x07, 0x00, 0x3f, 0x00, 0xff, 0x01, 0xfe, 0x0f, 0xfe, 0x1f, 0xfe, 0x0f,
   0xfc, 0x07, 0xfc, 0x03, 0xfc, 0x07, 0xf8, 0x0f, 0x78, 0x1f, 0x38, 0x3e,
   0x10, 0x7c, 0x00, 0x38, 0x00, 0x10, 0x00, 0x00
};

static const unsigned char cur_up_arrow_bits[] = {
   0x80, 0x00, 0x40, 0x01, 0x40, 0x01, 0x20, 0x02, 0x20, 0x02, 0x10, 0x04,
   0x10, 0x04, 0x08, 0x08, 0x78, 0x0f, 0x40, 0x01, 0x40, 0x01, 0x40, 0x01,
   0x40, 0x01, 0x40, 0x01, 0x40, 0x01, 0xc0, 0x01
};
static const unsigned char mcur_up_arrow_bits[] = {
   0x80, 0x00, 0xc0, 0x01, 0xc0, 0x01, 0xe0, 0x03, 0xe0, 0x03, 0xf0, 0x07,
   0xf0, 0x07, 0xf8, 0x0f, 0xf8, 0x0f, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01,
   0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01
};

static const unsigned char cur_cross_bits[] = {
   0xc0, 0x01, 0x40, 0x01, 0x40, 0x01, 0x40, 0x01, 0x40, 0x01, 0x40, 0x01,
   0x7f, 0x7f, 0x01, 0x40, 0x7f, 0x7f, 0x40, 0x01, 0x40, 0x01, 0x40, 0x01,
   0x40, 0x01, 0x40, 0x01, 0xc0, 0x01, 0x00, 0x00
};
static const unsigned char mcur_cross_bits[] = {
   0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01,
   0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01,
   0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0x00, 0x00
};

static const uchar cur_ibeam_bits[] = {
   0x00, 0x00, 0xe0, 0x03, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00,
   0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00,
   0x80, 0x00, 0xe0, 0x03, 0x00, 0x00, 0x00, 0x00
};
static const uchar mcur_ibeam_bits[] = {
   0xf0, 0x07, 0xf0, 0x07, 0xf0, 0x07, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01,
   0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01,
   0xf0, 0x07, 0xf0, 0x07, 0xf0, 0x07, 0x00, 0x00
};

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

// 20 x 20
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

// 32 x 32
static const uchar wait_data_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x7f, 0x00,
   0x00, 0x04, 0x40, 0x00, 0x00, 0xfc, 0x7f, 0x00, 0x00, 0x08, 0x20, 0x00,
   0x00, 0x08, 0x20, 0x00, 0x00, 0x08, 0x20, 0x00, 0x00, 0x08, 0x20, 0x00,
   0x00, 0x50, 0x15, 0x00, 0x00, 0xa0, 0x0a, 0x00, 0x00, 0x40, 0x05, 0x00,
   0x00, 0x80, 0x02, 0x00, 0x00, 0x40, 0x04, 0x00, 0x00, 0x20, 0x08, 0x00,
   0x00, 0x10, 0x10, 0x00, 0x00, 0x08, 0x21, 0x00, 0x00, 0x88, 0x22, 0x00,
   0x00, 0x48, 0x25, 0x00, 0x00, 0xa8, 0x2a, 0x00, 0x00, 0xfc, 0x7f, 0x00,
   0x00, 0x04, 0x40, 0x00, 0x00, 0xfc, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const uchar wait_mask_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x7f, 0x00,
   0x00, 0xfc, 0x7f, 0x00, 0x00, 0xfc, 0x7f, 0x00, 0x00, 0xf8, 0x3f, 0x00,
   0x00, 0xf8, 0x3f, 0x00, 0x00, 0xf8, 0x3f, 0x00, 0x00, 0xf8, 0x3f, 0x00,
   0x00, 0xf0, 0x1f, 0x00, 0x00, 0xe0, 0x0f, 0x00, 0x00, 0xc0, 0x07, 0x00,
   0x00, 0x80, 0x03, 0x00, 0x00, 0xc0, 0x07, 0x00, 0x00, 0xe0, 0x0f, 0x00,
   0x00, 0xf0, 0x1f, 0x00, 0x00, 0xf8, 0x3f, 0x00, 0x00, 0xf8, 0x3f, 0x00,
   0x00, 0xf8, 0x3f, 0x00, 0x00, 0xf8, 0x3f, 0x00, 0x00, 0xfc, 0x7f, 0x00,
   0x00, 0xfc, 0x7f, 0x00, 0x00, 0xfc, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00,
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
static const uchar phand_bits[] = {
   0x00, 0x00, 0x00, 0x00,        0xfe, 0x01, 0x00, 0x00,        0x01, 0x02, 0x00, 0x00,
   0x7e, 0x04, 0x00, 0x00,        0x08, 0x08, 0x00, 0x00,        0x70, 0x08, 0x00, 0x00,
   0x08, 0x08, 0x00, 0x00,        0x70, 0x14, 0x00, 0x00,        0x08, 0x22, 0x00, 0x00,
   0x30, 0x41, 0x00, 0x00,        0xc0, 0x20, 0x00, 0x00,        0x40, 0x12, 0x00, 0x00,
   0x80, 0x08, 0x00, 0x00,        0x00, 0x05, 0x00, 0x00,        0x00, 0x02, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,        0x00, 0x00, 0x00, 0x00,        0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,        0x00, 0x00, 0x00, 0x00,        0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,        0x00, 0x00, 0x00, 0x00,        0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,        0x00, 0x00, 0x00, 0x00,        0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,        0x00, 0x00, 0x00, 0x00,        0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,        0x00, 0x00, 0x00, 0x00
};
static const uchar phandm_bits[] = {
   0xfe, 0x01, 0x00, 0x00,        0xff, 0x03, 0x00, 0x00,        0xff, 0x07, 0x00, 0x00,
   0xff, 0x0f, 0x00, 0x00,        0xfe, 0x1f, 0x00, 0x00,        0xf8, 0x1f, 0x00, 0x00,
   0xfc, 0x1f, 0x00, 0x00,        0xf8, 0x3f, 0x00, 0x00,        0xfc, 0x7f, 0x00, 0x00,
   0xf8, 0xff, 0x00, 0x00,        0xf0, 0x7f, 0x00, 0x00,        0xe0, 0x3f, 0x00, 0x00,
   0xc0, 0x1f, 0x00, 0x00,        0x80, 0x0f, 0x00, 0x00,        0x00, 0x07, 0x00, 0x00,
   0x00, 0x02, 0x00, 0x00,        0x00, 0x00, 0x00, 0x00,        0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,        0x00, 0x00, 0x00, 0x00,        0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,        0x00, 0x00, 0x00, 0x00,        0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,        0x00, 0x00, 0x00, 0x00,        0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,        0x00, 0x00, 0x00, 0x00,        0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,        0x00, 0x00, 0x00, 0x00
};

static const uchar size_all_data_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x80, 0x00, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xe0, 0x03, 0x00,
   0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
   0x00, 0x80, 0x00, 0x00, 0x00, 0x81, 0x40, 0x00, 0x80, 0x81, 0xc0, 0x00,
   0xc0, 0xff, 0xff, 0x01, 0x80, 0x81, 0xc0, 0x00, 0x00, 0x81, 0x40, 0x00,
   0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
   0x00, 0x80, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0xc0, 0x01, 0x00,
   0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const uchar size_all_mask_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
   0x00, 0xc0, 0x01, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0xf0, 0x07, 0x00,
   0x00, 0xf8, 0x0f, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xc2, 0x21, 0x00,
   0x00, 0xc3, 0x61, 0x00, 0x80, 0xc3, 0xe1, 0x00, 0xc0, 0xff, 0xff, 0x01,
   0xe0, 0xff, 0xff, 0x03, 0xc0, 0xff, 0xff, 0x01, 0x80, 0xc3, 0xe1, 0x00,
   0x00, 0xc3, 0x61, 0x00, 0x00, 0xc2, 0x21, 0x00, 0x00, 0xc0, 0x01, 0x00,
   0x00, 0xf8, 0x0f, 0x00, 0x00, 0xf0, 0x07, 0x00, 0x00, 0xe0, 0x03, 0x00,
   0x00, 0xc0, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
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

// 16 x 16
static const uchar openhand_bits[] = {
   0x80, 0x01, 0x58, 0x0e, 0x64, 0x12, 0x64, 0x52, 0x48, 0xb2, 0x48, 0x92,
   0x16, 0x90, 0x19, 0x80, 0x11, 0x40, 0x02, 0x40, 0x04, 0x40, 0x04, 0x20,
   0x08, 0x20, 0x10, 0x10, 0x20, 0x10, 0x00, 0x00
};
static const uchar openhandm_bits[] = {
   0x80, 0x01, 0xd8, 0x0f, 0xfc, 0x1f, 0xfc, 0x5f, 0xf8, 0xff, 0xf8, 0xff,
   0xfe, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xfe, 0x7f, 0xfc, 0x7f, 0xfc, 0x3f,
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

void QPlatformCursorImage::createSystemCursor(int id)
{
   if (!systemCursorTableInit) {
      for (int i = 0; i <= Qt::LastCursor; i++) {
         systemCursorTable[i] = 0;
      }
      systemCursorTableInit = true;
   }
   switch (id) {
      // 16x16 cursors
      case Qt::ArrowCursor:
         systemCursorTable[Qt::ArrowCursor] =
            new QPlatformCursorImage(cur_arrow_bits, mcur_arrow_bits, 16, 16, 0, 0);
         break;

      case Qt::UpArrowCursor:
         systemCursorTable[Qt::UpArrowCursor] =
            new QPlatformCursorImage(cur_up_arrow_bits, mcur_up_arrow_bits, 16, 16, 7, 0);
         break;

      case Qt::CrossCursor:
         systemCursorTable[Qt::CrossCursor] =
            new QPlatformCursorImage(cur_cross_bits, mcur_cross_bits, 16, 16, 7, 7);
         break;

      case Qt::IBeamCursor:
         systemCursorTable[Qt::IBeamCursor] =
            new QPlatformCursorImage(cur_ibeam_bits, mcur_ibeam_bits, 16, 16, 7, 7);
         break;

      case Qt::SizeVerCursor:
         systemCursorTable[Qt::SizeVerCursor] =
            new QPlatformCursorImage(cur_ver_bits, mcur_ver_bits, 16, 16, 7, 7);
         break;

      case Qt::SizeHorCursor:
         systemCursorTable[Qt::SizeHorCursor] =
            new QPlatformCursorImage(cur_hor_bits, mcur_hor_bits, 16, 16, 7, 7);
         break;

      case Qt::SizeBDiagCursor:
         systemCursorTable[Qt::SizeBDiagCursor] =
            new QPlatformCursorImage(cur_bdiag_bits, mcur_bdiag_bits, 16, 16, 7, 7);
         break;

      case Qt::SizeFDiagCursor:
         systemCursorTable[Qt::SizeFDiagCursor] =
            new QPlatformCursorImage(cur_fdiag_bits, mcur_fdiag_bits, 16, 16, 7, 7);
         break;

      case Qt::BlankCursor:
         systemCursorTable[Qt::BlankCursor] =
            new QPlatformCursorImage(0, 0, 0, 0, 0, 0);
         break;

      // 20x20 cursors
      case Qt::ForbiddenCursor:
         systemCursorTable[Qt::ForbiddenCursor] =
            new QPlatformCursorImage(forbidden_bits, forbiddenm_bits, 20, 20, 10, 10);
         break;

      // 32x32 cursors
      case Qt::WaitCursor:
         systemCursorTable[Qt::WaitCursor] =
            new QPlatformCursorImage(wait_data_bits, wait_mask_bits, 32, 32, 15, 15);
         break;

      case Qt::SplitVCursor:
         systemCursorTable[Qt::SplitVCursor] =
            new QPlatformCursorImage(vsplit_bits, vsplitm_bits, 32, 32, 15, 15);
         break;

      case Qt::SplitHCursor:
         systemCursorTable[Qt::SplitHCursor] =
            new QPlatformCursorImage(hsplit_bits, hsplitm_bits, 32, 32, 15, 15);
         break;

      case Qt::SizeAllCursor:
         systemCursorTable[Qt::SizeAllCursor] =
            new QPlatformCursorImage(size_all_data_bits, size_all_mask_bits, 32, 32, 15, 15);
         break;

      case Qt::PointingHandCursor:
         systemCursorTable[Qt::PointingHandCursor] =
            new QPlatformCursorImage(phand_bits, phandm_bits, 32, 32, 0, 0);
         break;

      case Qt::WhatsThisCursor:
         systemCursorTable[Qt::WhatsThisCursor] =
            new QPlatformCursorImage(whatsthis_bits, whatsthism_bits, 32, 32, 0, 0);
         break;
      case Qt::BusyCursor:
         systemCursorTable[Qt::BusyCursor] =
            new QPlatformCursorImage(busy_bits, busym_bits, 32, 32, 0, 0);
         break;

      case Qt::OpenHandCursor:
         systemCursorTable[Qt::OpenHandCursor] =
            new QPlatformCursorImage(openhand_bits, openhandm_bits, 16, 16, 8, 8);
         break;
      case Qt::ClosedHandCursor:
         systemCursorTable[Qt::ClosedHandCursor] =
            new QPlatformCursorImage(closedhand_bits, closedhandm_bits, 16, 16, 8, 8);
         break;
      default:
         qWarning("Unknown system cursor %d", id);
   }
}

/*!
    \fn void QPlatformCursorImage::set(Qt::CursorShape id)

    \brief Calling this method sets the cursor image to the specified shape

    \a id is one of the defined Qt::CursorShape values.

    If id is invalid, Qt::BitmapCursor, or unknown by the implementation,
    Qt::ArrowCursor is used instead.
*/

void QPlatformCursorImage::set(Qt::CursorShape id)
{
   QPlatformCursorImage *cursor = 0;
   if (id >= 0 && id <= Qt::LastCursor) {
      if (!systemCursorTable[id]) {
         createSystemCursor(id);
      }
      cursor = systemCursorTable[id];
   }

   if (cursor == 0) {
      if (!systemCursorTable[Qt::ArrowCursor]) {
         createSystemCursor(Qt::ArrowCursor);
      }
      cursor = systemCursorTable[Qt::ArrowCursor];
   }
   cursorImage = cursor->cursorImage;
   hot = cursor->hot;
}

/*!
    Sets the cursor image to the given \a image, with the hotspot at the
    point specified by (\a hx, \a hy).
*/

void QPlatformCursorImage::set(const QImage &image, int hx, int hy)
{
   hot.setX(hx);
   hot.setY(hy);
   cursorImage = image;
}

/*!
    \fn void QPlatformCursorImage::set(const uchar *data, const uchar *mask, int width, int height, int hx, int hy)

    Sets the cursor image to the graphic represented by the combination of
    \a data and \a mask, with dimensions given by \a width and \a height and a
    hotspot at the point specified by (\a hx, \a hy).

    The image data specified by \a data must be supplied in the format
    described by QImage::Format_Indexed8.

    The corresponding mask data specified by \a mask must be supplied in a
    character array containing packed 1 bit per pixel format data, with any
    padding bits at the end of the array. Bits of value 0 represent transparent
    pixels in the image data.
*/
void QPlatformCursorImage::set(const uchar *data, const uchar *mask,
                               int width, int height, int hx, int hy)
{
   hot.setX(hx);
   hot.setY(hy);

   cursorImage = QImage(width, height, QImage::Format_Indexed8);

   if (!width || !height || !data || !mask || cursorImage.isNull()) {
      return;
   }

   cursorImage.setNumColors(3);
   cursorImage.setColor(0, 0xff000000);
   cursorImage.setColor(1, 0xffffffff);
   cursorImage.setColor(2, 0x00000000);

   int bytesPerLine = (width + 7) / 8;
   int p = 0;
   int d, m;

   int x = -1, w = 0;

   uchar *cursor_data = cursorImage.bits();
   int bpl = cursorImage.bytesPerLine();
   for (int i = 0; i < height; i++) {
      for (int j = 0; j < bytesPerLine; j++, data++, mask++) {
         for (int b = 0; b < 8 && j * 8 + b < width; b++) {
            d = *data & (1 << b);
            m = *mask & (1 << b);
            if (d && m) {
               p = 0;
            } else if (!d && m) {
               p = 1;
            } else {
               p = 2;
            }
            cursor_data[j * 8 + b] = p;

            // calc region
            if (x < 0 && m) {
               x = j * 8 + b;
            } else if (x >= 0 && !m) {
               x = -1;
               w = 0;
            }
            if (m) {
               w++;
            }
         }
      }
      if (x >= 0) {
         x = -1;
         w = 0;
      }
      cursor_data += bpl;
   }

}

/*!
    \fn QPlatformCursorImage::QPlatformCursorImage(const uchar *data, const uchar *mask, int width, int height, int hotX, int hotY)

    Sets the cursor image to the graphic represented by the combination of
    \a data and \a mask, with dimensions given by \a width and \a height and a
    hotspot at the point specified by (\a hotX, \a hotY).

    \sa set()
*/

/*!
  \fn QImage *QPlatformCursorImage::image()

  \brief Return the cursor graphic as a pointer to a QImage
*/

/*!
    \fn QPoint QPlatformCursorImage::hotspot()

    \brief Return the cursor's hotspot
*/

QT_END_NAMESPACE
