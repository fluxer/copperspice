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

#ifndef QTreatAs_P_H
#define QTreatAs_P_H

#include <qsinglecontainer_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {

class TreatAs : public SingleContainer
{
 public:
   /**
    * Creats a TreatAs where it is checked that the expression @p operand conforms
    * to the type @p reqType.
    */
   TreatAs(const Expression::Ptr &operand,
           const SequenceType::Ptr &reqType);

   /**
    * This function rewrites always. First the type that this TreatAs expression tests for
    * is verified. Then, the type the <tt>treat as</tt> expression itself must match, @p reqType,
    * is verified.
    */
   virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType);

   /**
    * @returns always the SequenceType passed in the constructor to this class. That is, the
    * SequenceType that the operand must conform to.
    */
   virtual SequenceType::Ptr staticType() const;

   /**
    * @returns a list containing one CommonSequenceTypes::ZeroOrMoreItems
    */
   virtual SequenceType::List expectedOperandTypes() const;

   virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

 private:
   const SequenceType::Ptr m_reqType;
};
}

QT_END_NAMESPACE

#endif
