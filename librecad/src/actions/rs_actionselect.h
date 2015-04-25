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

#ifndef RS_ACTIONSELECT_H
#define RS_ACTIONSELECT_H

#include "rs_actioninterface.h"


/**
 * This action class can handle user events to select entities.
 *
 * @author Andrew Mustun
 */
class RS_ActionSelect : public RS_ActionInterface {
        Q_OBJECT
public:

    RS_ActionSelect(RS_EntityContainer& container,
                    RS_GraphicView& graphicView,
                    RS2::ActionType nextAction,
					std::vector<RS2::EntityType>* entityTypeList=NULL);
	~RS_ActionSelect()=default;

    virtual void init(int status);
    virtual RS2::ActionType rtti() const{
            return RS2::ActionSelect;
    }
    virtual void resume();
    //virtual void keyPressEvent(QKeyEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void updateToolBar();
    virtual void updateMouseButtonHints();
    virtual void updateMouseCursor();
    virtual int countSelected();
    void requestFinish(bool keep=false);

private:
	std::vector<RS2::EntityType>* entityTypeList;
    RS2::ActionType nextAction;
    bool selectSingle;
};

#endif
