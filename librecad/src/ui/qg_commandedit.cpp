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

#include "qg_commandedit.h"
#include <QKeyEvent>
#include <QRegExp>
#include <rs_math.h>


/**
 * Default Constructor. You must call init manually if you choose
 * to use this constructor.
 */
QG_CommandEdit::QG_CommandEdit(QWidget* parent)
        : QLineEdit(parent)
        , calculator_mode(false)

{
    setStyleSheet("selection-color: white; selection-background-color: green;");
    setFrame(false);
    setFocusPolicy(Qt::StrongFocus);
}

/**
 * Bypass for key press events from the tab key.
 */
bool QG_CommandEdit::event(QEvent* e) {
	if (e->type()==QEvent::KeyPress) {
		QKeyEvent* k = (QKeyEvent*)e;
		if (k->key()==Qt::Key_Tab) {
			emit tabPressed();
			return true;
        }
	}
	
	return QLineEdit::event(e);
}

/**
 * History (arrow key up/down) support, tab.
 */
void QG_CommandEdit::keyPressEvent(QKeyEvent* e)
{
    auto input = text();

	switch (e->key()) {
            case Qt::Key_Up:
                    if (!historyList.isEmpty() && it>historyList.begin()) {
                            it--;
                            setText(*it);
                    }
                    break;

            case Qt::Key_Down:
                    if (!historyList.isEmpty() && it<historyList.end() ) {
                            it++;
                            if (it<historyList.end()) {
                                    setText(*it);
                            }
                            else {
                                    setText("");
                            }
                    }
                    break;

            case Qt::Key_Space:
            case Qt::Key_Return:
                    if (input == QObject::tr("cal"))
                    {
                        calculator_mode = !calculator_mode;
                        if(calculator_mode)
                            emit message(QObject::tr("Calculator mode: On"));
                        else
                            emit message(QObject::tr("Calculator mode: Off"));
                        clear();
                        break;
                    }
                    if (calculator_mode)
                    {
                        QRegExp regex(R"~(([\d\.]+)deg|d)~");
                        input.replace(regex, R"~(\1*pi/180)~");
                        bool ok = true;
                        double result = RS_Math::eval(input, &ok);
                        if (ok)
                            emit message(input + " = " + QString::number(result, 'g', 12));
                        else
                            emit message(QObject::tr("Calculator error for input: ") + input);
                        clear();
                        break;
                    }

                    historyList.append(input);
                    it = historyList.end();
                    if(input.compare(tr("clear"), Qt::CaseInsensitive) == 0){
                        setText("");
                        emit(clearCommandsHistory());
                    } else {
                        emit command(input);
                    }
                    break;

            case Qt::Key_Escape:
                    if (text().isEmpty()) {
                            emit escape();
                    }
                    else {
                            setText("");
                    }
                    break;

            default:
                    QLineEdit::keyPressEvent(e);
                    break;
	}
}

void QG_CommandEdit::focusInEvent(QFocusEvent *e) {
	emit focusIn();
	QLineEdit::focusInEvent(e);
}

void QG_CommandEdit::focusOutEvent(QFocusEvent *e) {
	emit focusOut();
	QLineEdit::focusOutEvent(e);
}
