/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2010 R. van Twisk (librecad@rvt.dds.nl)
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file gpl-2.0.txt included in the
** packaging of this file.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** This copyright notice MUST APPEAR in all copies of the script!
**
**********************************************************************/
#include <clocale>
#include "main.h"

#include <QDebug>
#include <QApplication>
#include <QSplashScreen>

#include "rs_fontlist.h"
#include "rs_patternlist.h"
#include "rs_scriptlist.h"
#include "rs_settings.h"
#include "rs_system.h"
#include "rs_fileio.h"
#include "qg_dlginitial.h"

#include "qc_applicationwindow.h"
#include "rs_debug.h"

#ifndef QC_SPLASH_TXTCOL
    #define QC_SPLASH_TXTCOL Qt::black
#endif

// for image mime resources from png files
extern void QINITIMAGES_LIBRECAD();

/**
 * Main. Creates Application window.
 */
int main(int argc, char** argv)
{
    RS_DEBUG->setLevel(RS_Debug::D_WARNING);

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("LibreCAD");
    QCoreApplication::setApplicationName("/LibreCAD");
    QCoreApplication::setApplicationVersion("master");

    QSplashScreen* splash = new QSplashScreen;

    RS_SETTINGS->beginGroup("Appearance");
    bool show_splash = RS_SETTINGS->readNumEntry("/ShowSplash", 1);
    RS_SETTINGS->endGroup();

    if (show_splash)
    {
        QPixmap pixmap(":/main/splash_librecad.png");
        splash->setPixmap(pixmap);
        splash->setAttribute(Qt::WA_DeleteOnClose);
        splash->show();
        splash->showMessage(QObject::tr("Loading.."),
                            Qt::AlignRight|Qt::AlignBottom, QC_SPLASH_TXTCOL);
        app.processEvents();
        RS_DEBUG->print("main: splashscreen: OK");
    }

    #if defined(Q_OS_MAC) && QT_VERSION > 0x050000
        //need stylesheet for Qt5 on mac
        app.setStyleSheet(
"QToolButton:checked"
"{"
"    background-color: rgb(160,160,160);"
"    border-style: inset;"
"}"
""
"QToolButton"
"{"
"    background-color: transparent;"
"}"
""
"QToolButton:hover"
"{"
"    background-color: rgb(255,255,255);"
"    border-style: outset;"
"}"
        );
    #endif

    const QString lpDebugSwitch0("-d"),lpDebugSwitch1("--debug") ;
    const QString help0("-h"), help1("--help");
    bool allowOptions=true;
    QList<int> argClean;
    for (int i=0; i<argc; i++)
    {
        QString argstr(argv[i]);
        if(allowOptions&&QString::compare("--", argstr)==0)
        {
            allowOptions=false;
            continue;
        }
        if (allowOptions && (help0.compare(argstr, Qt::CaseInsensitive)==0 ||
                             help1.compare(argstr, Qt::CaseInsensitive)==0 ))
        {
            qDebug()<<"librecad::usage: <options> <dxf file>";
            qDebug()<<"-h, --help\tdisplay this message";
            qDebug()<<"";
            qDebug()<<" --help\tdisplay this message";
            qDebug()<<"-d, --debug <level>";
            RS_DEBUG->print( RS_Debug::D_NOTHING, "possible debug levels:");
            RS_DEBUG->print( RS_Debug::D_NOTHING, "    %d Nothing", RS_Debug::D_NOTHING);
            RS_DEBUG->print( RS_Debug::D_NOTHING, "    %d Critical", RS_Debug::D_CRITICAL);
            RS_DEBUG->print( RS_Debug::D_NOTHING, "    %d Error", RS_Debug::D_ERROR);
            RS_DEBUG->print( RS_Debug::D_NOTHING, "    %d Warning", RS_Debug::D_WARNING);
            RS_DEBUG->print( RS_Debug::D_NOTHING, "    %d Notice", RS_Debug::D_NOTICE);
            RS_DEBUG->print( RS_Debug::D_NOTHING, "    %d Informational", RS_Debug::D_INFORMATIONAL);
            RS_DEBUG->print( RS_Debug::D_NOTHING, "    %d Debugging", RS_Debug::D_DEBUGGING);
            exit(0);
        }
        if ( allowOptions&& (argstr.startsWith(lpDebugSwitch0, Qt::CaseInsensitive) ||
                             argstr.startsWith(lpDebugSwitch1, Qt::CaseInsensitive) ))
        {
            argClean<<i;

            // to control the level of debugging output use --debug with level 0-6, e.g. --debug3
            // for a list of debug levels use --debug?
            // if no level follows, the debugging level is set
            argstr.remove(QRegExp("^"+lpDebugSwitch0));
            argstr.remove(QRegExp("^"+lpDebugSwitch1));
            char level;
            if(argstr.size()==0)
            {
                if(i+1<argc)
                {
                    if(QRegExp("\\d*").exactMatch(argv[i+1]))
                    {
                        ++i;
                        qDebug()<<"reading "<<argv[i]<<" as debugging level";
                        level=argv[i][0];
                        argClean<<i;
                    }
                    else
                        level='3';
                }
                else
                    level='3'; //default to D_WARNING
            }
            else
                level=argstr.toStdString()[0];

            switch(level)
            {
            case '?' :
                RS_DEBUG->print( RS_Debug::D_NOTHING, "possible debug levels:");
                RS_DEBUG->print( RS_Debug::D_NOTHING, "    %d Nothing", RS_Debug::D_NOTHING);
                RS_DEBUG->print( RS_Debug::D_NOTHING, "    %d Critical", RS_Debug::D_CRITICAL);
                RS_DEBUG->print( RS_Debug::D_NOTHING, "    %d Error", RS_Debug::D_ERROR);
                RS_DEBUG->print( RS_Debug::D_NOTHING, "    %d Warning", RS_Debug::D_WARNING);
                RS_DEBUG->print( RS_Debug::D_NOTHING, "    %d Notice", RS_Debug::D_NOTICE);
                RS_DEBUG->print( RS_Debug::D_NOTHING, "    %d Informational", RS_Debug::D_INFORMATIONAL);
                RS_DEBUG->print( RS_Debug::D_NOTHING, "    %d Debugging", RS_Debug::D_DEBUGGING);
                return 0;

            case '0' + RS_Debug::D_NOTHING :
                RS_DEBUG->setLevel( RS_Debug::D_NOTHING);
                ++i;
                break;

            case '0' + RS_Debug::D_CRITICAL :
                RS_DEBUG->setLevel( RS_Debug::D_CRITICAL);
                ++i;
                break;

            case '0' + RS_Debug::D_ERROR :
                RS_DEBUG->setLevel( RS_Debug::D_ERROR);
                ++i;
                break;

            case '0' + RS_Debug::D_WARNING :
                RS_DEBUG->setLevel( RS_Debug::D_WARNING);
                ++i;
                break;

            case '0' + RS_Debug::D_NOTICE :
                RS_DEBUG->setLevel( RS_Debug::D_NOTICE);
                ++i;
                break;

            case '0' + RS_Debug::D_INFORMATIONAL :
                RS_DEBUG->setLevel( RS_Debug::D_INFORMATIONAL);
                ++i;
                break;

            case '0' + RS_Debug::D_DEBUGGING :
                RS_DEBUG->setLevel( RS_Debug::D_DEBUGGING);
                ++i;
                break;

            default :
                RS_DEBUG->setLevel(RS_Debug::D_DEBUGGING);
                break;
            }
        }
    }
    RS_DEBUG->print("param 0: %s", argv[0]);

    QFileInfo prgInfo( QFile::decodeName(argv[0]) );
    QString prgDir(prgInfo.absolutePath());
    RS_SETTINGS->init(XSTR(QC_COMPANYKEY), XSTR(QC_APPKEY));
    RS_SYSTEM->init(XSTR(QC_APPNAME), XSTR(QC_VERSION), XSTR(QC_APPDIR), prgDir);

    // parse command line arguments that might not need a launched program:
    QStringList fileList = handleArgs(argc, argv, argClean);

    QString lang;
    QString langCmd;
    QString unit;

    RS_SETTINGS->beginGroup("/Defaults");
    #ifndef QC_PREDEFINED_UNIT
        unit = RS_SETTINGS->readEntry("/Unit", "Invalid");
    #else
        unit = RS_SETTINGS->readEntry("/Unit", QC_PREDEFINED_UNIT);
    #endif
    RS_SETTINGS->endGroup();

    // show initial config dialog:
    if (unit=="Invalid")
    {
        RS_DEBUG->print("main: show initial config dialog..");
        QG_DlgInitial di(nullptr);
        QPixmap pxm(":/main/intro_librecad.png");
        di.setPixmap(pxm);
        if (di.exec())
        {
            RS_SETTINGS->beginGroup("/Defaults");
            unit = RS_SETTINGS->readEntry("/Unit", "None");
            RS_SETTINGS->endGroup();
        }
        RS_DEBUG->print("main: show initial config dialog: OK");
    }

    RS_DEBUG->print("main: init fontlist..");
    RS_FONTLIST->init();
    RS_DEBUG->print("main: init fontlist: OK");

    RS_DEBUG->print("main: init patternlist..");
    RS_PATTERNLIST->init();
    RS_DEBUG->print("main: init patternlist: OK");

    RS_DEBUG->print("main: init scriptlist..");
    RS_SCRIPTLIST->init();
    RS_DEBUG->print("main: init scriptlist: OK");

    RS_DEBUG->print("main: loading translation..");
    RS_SETTINGS->beginGroup("/Appearance");
    #ifdef QC_PREDEFINED_LOCALE
        lang = RS_SETTINGS->readEntry("/Language", "");
        if (lang.isEmpty())
        {
            lang=QC_PREDEFINED_LOCALE;
            RS_SETTINGS->writeEntry("/Language", lang);
        }
        langCmd = RS_SETTINGS->readEntry("/LanguageCmd", "");
        if (langCmd.isEmpty())
        {
            langCmd=QC_PREDEFINED_LOCALE;
            RS_SETTINGS->writeEntry("/LanguageCmd", langCmd);
        }
    #else
        lang = RS_SETTINGS->readEntry("/Language", "en");
        langCmd = RS_SETTINGS->readEntry("/LanguageCmd", "en");
    #endif
    RS_SETTINGS->endGroup();

    RS_SYSTEM->loadTranslation(lang, langCmd);
    RS_DEBUG->print("main: loading translation: OK");

    RS_DEBUG->print("main: creating main window..");
    QC_ApplicationWindow appWin;
    RS_DEBUG->print("main: setting caption");
    appWin.setWindowTitle(XSTR(QC_APPNAME));

    RS_DEBUG->print("main: show main window");

    RS_SETTINGS->beginGroup("/Geometry");
    int windowWidth = RS_SETTINGS->readNumEntry("/WindowWidth", 0);
    int windowHeight = RS_SETTINGS->readNumEntry("/WindowHeight", 0);
    int windowX = RS_SETTINGS->readNumEntry("/WindowX", 30);
    int windowY = RS_SETTINGS->readNumEntry("/WindowY", 30);
    RS_SETTINGS->endGroup();

    if (windowWidth != 0)
        appWin.resize(windowWidth, windowHeight);

    appWin.move(windowX, windowY);

    RS_SETTINGS->beginGroup("Defaults");
    bool maximize = RS_SETTINGS->readNumEntry("/Maximize", 0);
    RS_SETTINGS->endGroup();
    if (maximize || windowWidth == 0)
        appWin.showMaximized();
    else
        appWin.show();

    RS_DEBUG->print("main: set focus");
    appWin.setFocus();
    RS_DEBUG->print("main: creating main window: OK");

    if (show_splash)
    {
        RS_DEBUG->print("main: updating splash");
        splash->raise();
        splash->showMessage(QObject::tr("Loading..."),
                Qt::AlignRight|Qt::AlignBottom, QC_SPLASH_TXTCOL);
        RS_DEBUG->print("main: processing events");
        qApp->processEvents();
        RS_DEBUG->print("main: updating splash: OK");
    }

    // Set LC_NUMERIC so that entering numeric values uses . as the decimal seperator
    setlocale(LC_NUMERIC, "C");

    RS_DEBUG->print("main: loading files..");
    bool files_loaded = false;
    for (QStringList::Iterator it = fileList.begin(); it != fileList.end(); ++it )
    {
        if (show_splash)
        {
            splash->showMessage(QObject::tr("Loading File %1..")
                    .arg(QDir::toNativeSeparators(*it)),
            Qt::AlignRight|Qt::AlignBottom, QC_SPLASH_TXTCOL);
            qApp->processEvents();
        }
        appWin.slotFileOpen(*it, RS2::FormatUnknown);
        files_loaded = true;
    }
    RS_DEBUG->print("main: loading files: OK");

    RS_DEBUG->print("main: app.exec()");

    if (!files_loaded)
    {
        appWin.slotFileNewNew();
    }

    if (show_splash)
        splash->finish(&appWin);
    else
        delete splash;

    int return_code = app.exec();

    RS_DEBUG->print("main: exited Qt event loop");

    return return_code;
}


/**
 * Handles command line arguments that might not require a GUI.
 *
 * @return list of files to load on startup.
 */
QStringList handleArgs(int argc, char** argv, const QList<int>& argClean)
{
    RS_DEBUG->print("main: handling args..");
    QStringList ret;

    bool doexit = false;

    for (int i=1; i<argc; i++)
    {
        if(argClean.indexOf(i)>=0) continue;
        if (!QString(argv[i]).startsWith("-"))
        {
            QString fname = QDir::toNativeSeparators(
            QFileInfo(QFile::decodeName(argv[i])).absoluteFilePath());
            ret.append(fname);
        }
        else if (QString(argv[i])=="--exit")
        {
            doexit = true;
        }
    }
    if (doexit)
    {
        exit(0);
    }
    RS_DEBUG->print("main: handling args: OK");
    return ret;
}

