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

#ifndef RS_ACTIONDRAWARC_H
#define RS_ACTIONDRAWARC_H

#include "rs_previewactioninterface.h"

struct RS_ArcData;

/**
 * This action class can handle user events to draw 
 * simple arcs with the center, radius, start- and endangle given.
 *
 * @author Andrew Mustun
 */
class RS_ActionDrawArc : public RS_PreviewActionInterface {
	Q_OBJECT
public:
    /**
     * Action States.
     */
    enum Status {
        SetCenter,       /**< Setting the center point. */
        SetRadius,       /**< Setting the radius. */
        SetAngle1,       /**< Setting the startpoint.  */
        SetAngle2,       /**< Setting the endpoint. */
		SetIncAngle,     /**< Setting the included angle. */
		SetChordLength   /**< Setting carc chord length. */
    };

public:
    RS_ActionDrawArc(RS_EntityContainer& container,
                     RS_GraphicView& graphicView);
	~RS_ActionDrawArc();
	
	static QAction* createGUIAction(RS2::ActionType /*type*/, QObject* /*parent*/);
	
	virtual RS2::ActionType rtti() const{
		return RS2::ActionDrawArc;
	}

    void reset();

    virtual void init(int status=0);
    virtual void trigger();
	
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);

	virtual void coordinateEvent(RS_CoordinateEvent* e);
    virtual void commandEvent(RS_CommandEvent* e);
        virtual QStringList getAvailableCommands();
	
    virtual void hideOptions();
    virtual void showOptions();
	
    virtual void updateMouseButtonHints();
    virtual void updateMouseCursor();

	bool isReversed() const;

	void setReversed(bool r) const;

protected:
    /**
     * Arc data defined so far.
     */
	std::unique_ptr<RS_ArcData> data;
	/**
	 * Commands
	 */
        /*QString cmdReversed;
        QString cmdReversed2;
        QString cmdReversed3;
	*/
};

#endif
