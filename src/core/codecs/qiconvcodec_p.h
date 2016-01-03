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

#ifndef QICONVCODEC_P_H
#define QICONVCODEC_P_H

#include <qtextcodec.h>

#if defined(Q_OS_UNIX) && !defined(QT_NO_ICONV)

#ifdef Q_OS_MAC
typedef void *iconv_t;
#else
#include <iconv.h>
#endif

QT_BEGIN_NAMESPACE

class QIconvCodec: public QTextCodec
{
 private:
   mutable QTextCodec *utf16Codec;

 public:
   QIconvCodec();
   ~QIconvCodec();

   QString convertToUnicode(const char *, int, ConverterState *) const;
   QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const;

   QByteArray name() const;
   int mibEnum() const;

   static iconv_t createIconv_t(const char *to, const char *from);

   class IconvState
   {
    public:
      IconvState(iconv_t x);
      ~IconvState();
      ConverterState internalState;
      char *buffer;
      int bufferLen;
      iconv_t cd;

      char array[8];

      void saveChars(const char *c, int count);
   };
};

QT_END_NAMESPACE

#endif // Q_OS_UNIX && !QT_NO_ICONV

#endif // QICONVCODEC_P_H
