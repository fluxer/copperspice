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

#include "qbuiltintypes_p.h"
#include "qgenericsequencetype_p.h"
#include "qmultiitemtype_p.h"
#include "qtypechecker_p.h"

#include "qvalidate_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

Expression::Ptr Validate::create(const Expression::Ptr &operandNode,
                                 const Mode validationMode,
                                 const StaticContext::Ptr &context)
{
   Q_ASSERT(operandNode);
   Q_ASSERT(validationMode == Lax || validationMode == Strict);
   Q_ASSERT(context);
   Q_UNUSED(validationMode);
   Q_UNUSED(context);

   ItemType::List tList;
   tList.append(BuiltinTypes::element);
   tList.append(BuiltinTypes::document);

   const SequenceType::Ptr elementOrDocument(makeGenericSequenceType(ItemType::Ptr(new MultiItemType(tList)),
         Cardinality::exactlyOne()));


   return TypeChecker::applyFunctionConversion(operandNode,
          elementOrDocument,
          context,
          ReportContext::XQTY0030);
}

QT_END_NAMESPACE
