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

#define UIC_VERSION_STR "1.0.0"

#include "uic.h"
#include "option.h"
#include "driver.h"

#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtCore/QTextCodec>

QT_BEGIN_NAMESPACE

static const char *error = 0;

void showHelp(const char *appName)
{
   fprintf(stderr, "Qt User Interface Compiler version %s\n", UIC_VERSION_STR);
   if (error) {
      fprintf(stderr, "%s: %s\n", appName, error);
   }

   fprintf(stderr, "Usage: %s [options] <uifile>\n\n"
           "  -h, -help                 display this help and exit\n"
           "  -v, -version              display version\n"
           "  -d, -dependencies         display the dependencies\n"
           "  -o <file>                 place the output into <file>\n"
           "  -tr <func>                use func() for i18n\n"
           "  -p, -no-protection        disable header protection\n"
           "  -n, -no-implicit-includes disable generation of #include-directives\n"
           "                            for forms generated by uic3\n"
           "  -g <name>                 change generator\n"
           "\n", appName);
}

int runUic(int argc, char *argv[])
{
   Driver driver;

   const char *fileName = 0;

   int arg = 1;
   while (arg < argc) {
      QString opt = QString::fromLocal8Bit(argv[arg]);
      if (opt == QLatin1String("-h") || opt == QLatin1String("-help")) {
         showHelp(argv[0]);
         return 0;
      } else if (opt == QLatin1String("-d") || opt == QLatin1String("-dependencies")) {
         driver.option().dependencies = true;
      } else if (opt == QLatin1String("-v") || opt == QLatin1String("-version")) {
         fprintf(stderr, "CopperSpice User Interface Compiler version %s\n", UIC_VERSION_STR);
         return 0;
      } else if (opt == QLatin1String("-o") || opt == QLatin1String("-output")) {
         ++arg;
         if (!argv[arg]) {
            showHelp(argv[0]);
            return 1;
         }
         driver.option().outputFile = QFile::decodeName(argv[arg]);
      } else if (opt == QLatin1String("-p") || opt == QLatin1String("-no-protection")) {
         driver.option().headerProtection = false;
      } else if (opt == QLatin1String("-n") || opt == QLatin1String("-no-implicit-includes")) {
         driver.option().implicitIncludes = false;
      } else if (opt == QLatin1String("-postfix")) {
         ++arg;
         if (!argv[arg]) {
            showHelp(argv[0]);
            return 1;
         }
         driver.option().postfix = QLatin1String(argv[arg]);
      } else if (opt == QLatin1String("-tr") || opt == QLatin1String("-translate")) {
         ++arg;
         if (!argv[arg]) {
            showHelp(argv[0]);
            return 1;
         }
         driver.option().translateFunction = QLatin1String(argv[arg]);
      } else if (opt == QLatin1String("-g") || opt == QLatin1String("-generator")) {
         ++arg;
         if (!argv[arg]) {
            showHelp(argv[0]);
            return 1;
         }
         QString name = QString::fromLocal8Bit(argv[arg]).toLower ();
         driver.option().generator = (name == QLatin1String ("java")) ? Option::JavaGenerator : Option::CppGenerator;
      } else if (!fileName) {
         fileName = argv[arg];
      } else {
         showHelp(argv[0]);
         return 1;
      }

      ++arg;
   }

   QString inputFile;
   if (fileName) {
      inputFile = QString::fromLocal8Bit(fileName);
   } else {
      driver.option().headerProtection = false;
   }

   if (driver.option().dependencies) {
      return !driver.printDependencies(inputFile);
   }

   QTextStream *out = 0;
   QFile f;
   if (driver.option().outputFile.size()) {
      f.setFileName(driver.option().outputFile);
      if (!f.open(QIODevice::WriteOnly | QFile::Text)) {
         fprintf(stderr, "Could not create output file\n");
         return 1;
      }
      out = new QTextStream(&f);
      out->setCodec(QTextCodec::codecForName("UTF-8"));
   }

   bool rtn = driver.uic(inputFile, out);
   delete out;

   if (!rtn) {
      if (driver.option().outputFile.size()) {
         f.close();
         f.remove();
      }
      fprintf(stderr, "File '%s' is not valid\n", inputFile.isEmpty() ? "<stdin>" : inputFile.toLocal8Bit().constData());
   }

   return !rtn;
}

QT_END_NAMESPACE

int main(int argc, char *argv[])
{
   return QT_PREPEND_NAMESPACE(runUic)(argc, argv);
}
