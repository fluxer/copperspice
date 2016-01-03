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

#include "qcommonvalues_p.h"
#include "qgenericsequencetype_p.h"
#include "qschemanumeric_p.h"

#include "qnumericfns_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

Item FloorFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item num(m_operands.first()->evaluateSingleton(context));

   if (!num) {
      return Item();
   }

   return toItem(num.as<Numeric>()->floor());
}

Item AbsFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item num(m_operands.first()->evaluateSingleton(context));

   if (!num) {
      return Item();
   }

   return toItem(num.as<Numeric>()->abs());
}

Item RoundFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item num(m_operands.first()->evaluateSingleton(context));

   if (!num) {
      return Item();
   }

   return toItem(num.as<Numeric>()->round());
}

Item CeilingFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item num(m_operands.first()->evaluateSingleton(context));

   if (!num) {
      return Item();
   }

   return toItem(num.as<Numeric>()->ceiling());
}

Item RoundHalfToEvenFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item num(m_operands.first()->evaluateSingleton(context));

   if (!num) {
      return Item();
   }

   xsInteger scale = 0;

   if (m_operands.count() == 2) {
      scale = m_operands.at(1)->evaluateSingleton(context).as<Numeric>()->toInteger();
   }

   return toItem(num.as<Numeric>()->roundHalfToEven(scale));
}

QT_END_NAMESPACE
