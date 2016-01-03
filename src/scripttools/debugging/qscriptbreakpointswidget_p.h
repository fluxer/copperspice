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

#ifndef QSCRIPTBREAKPOINTSWIDGET_P_H
#define QSCRIPTBREAKPOINTSWIDGET_P_H

#include "qscriptbreakpointswidgetinterface_p.h"

QT_BEGIN_NAMESPACE

class QScriptBreakpointsWidgetPrivate;

class QScriptBreakpointsWidget : public QScriptBreakpointsWidgetInterface
{
   SCRIPT_T_CS_OBJECT(QScriptBreakpointsWidget)

 public:
   QScriptBreakpointsWidget(QWidget *parent = 0);
   ~QScriptBreakpointsWidget();

   QScriptBreakpointsModel *breakpointsModel() const;
   void setBreakpointsModel(QScriptBreakpointsModel *model);

   QScriptDebuggerScriptsModel *scriptsModel() const;
   void setScriptsModel(QScriptDebuggerScriptsModel *model);

 protected:
   void keyPressEvent(QKeyEvent *e);

 private:
   CS_SLOT_1(Private, void _q_newBreakpoint())
   CS_SLOT_2(_q_newBreakpoint)

   CS_SLOT_1(Private, void _q_deleteBreakpoint())
   CS_SLOT_2(_q_deleteBreakpoint)
   
   CS_SLOT_1(Private, void _q_onCurrentChanged(const QModelIndex &un_named_arg1))
   CS_SLOT_2(_q_onCurrentChanged)

   CS_SLOT_1(Private, void _q_onNewBreakpointRequest(const QString &un_named_arg1, int un_named_arg2))
   CS_SLOT_2(_q_onNewBreakpointRequest)
  
   Q_DECLARE_PRIVATE(QScriptBreakpointsWidget)
   Q_DISABLE_COPY(QScriptBreakpointsWidget)
};

QT_END_NAMESPACE

#endif
