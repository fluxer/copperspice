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

#include <qtoolbarseparator_p.h>

#ifndef QT_NO_TOOLBAR

#include <qstyle.h>
#include <qstyleoption.h>
#include <qtoolbar.h>
#include <qpainter.h>

QT_BEGIN_NAMESPACE

void QToolBarSeparator::initStyleOption(QStyleOption *option) const
{
   option->initFrom(this);
   if (orientation() == Qt::Horizontal) {
      option->state |= QStyle::State_Horizontal;
   }
}

QToolBarSeparator::QToolBarSeparator(QToolBar *parent)
   : QWidget(parent), orient(parent->orientation())
{
   setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

void QToolBarSeparator::setOrientation(Qt::Orientation orientation)
{
   orient = orientation;
   update();
}

Qt::Orientation QToolBarSeparator::orientation() const
{
   return orient;
}

QSize QToolBarSeparator::sizeHint() const
{
   QStyleOption opt;
   initStyleOption(&opt);
   const int extent = style()->pixelMetric(QStyle::PM_ToolBarSeparatorExtent, &opt, parentWidget());
   return QSize(extent, extent);
}

void QToolBarSeparator::paintEvent(QPaintEvent *)
{
   QPainter p(this);
   QStyleOption opt;
   initStyleOption(&opt);
   style()->drawPrimitive(QStyle::PE_IndicatorToolBarSeparator, &opt, &p, parentWidget());
}

QT_END_NAMESPACE

#endif // QT_NO_TOOLBAR
