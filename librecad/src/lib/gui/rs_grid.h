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


#ifndef RS_GRID_H
#define RS_GRID_H

#include "rs_vector.h"

class RS_GraphicView;

/**
 * This class represents a grid. Grids can be drawn on graphic
 * views and snappers can snap to the grid points.
 *
 * @author Andrew Mustun
 */
class RS_Grid {
public:
    RS_Grid(RS_GraphicView* graphicView);
    ~RS_Grid();

    void updatePointArray();

        /**
         * @return Array of all visible grid points.
         */
	std::vector<RS_Vector> const& getPoints() const;

    /**
      *@return the closest grid point
      */
    RS_Vector snapGrid(const RS_Vector& coord) const;

        /**
         * @return Number of visible grid points.
         */
	int count() const{
        return number;
    }
    void setCrosshairType(RS2::CrosshairType chType){
        crosshairType=chType;
    }
	RS2::CrosshairType getCrosshairType() const{
        return crosshairType;
    }

        /**
         * @return Grid info for status widget.
         */
		QString getInfo() const;

        /**
         * @return Meta grid positions in X.
         */
		std::vector<double> const& getMetaX() const;

        /**
         * @return Number of visible meta grid lines in X.
         */
	int countMetaX() const{
        return numMetaX;
    }

        /**
         * @return Meta grid positions in Y.
         */
	std::vector<double> const& getMetaY() const;

        /**
         * @return Number of visible meta grid lines in Y.
         */
	int countMetaY() const{
        return numMetaY;
    }
    bool isIsometric() const{
        return isometric;
    }
    void setIsometric(bool b){
        isometric=b;
    }
    RS_Vector getMetaGridWidth() const {
        return metaGridWidth;
    }
	RS_Vector const& getCellVector() const
    {
        return cellV;
    }

protected:
    //! Graphic view this grid is connected to.
    RS_GraphicView* graphicView;

        //! Current grid spacing
        double spacing;
        //! Current meta grid spacing
        double metaSpacing;

    //! Pointer to array of grid points
	std::vector<RS_Vector> pt;
    RS_Vector baseGrid; // the left-bottom grid point
    RS_Vector cellV;// (dx,dy)
    RS_Vector metaGridWidth;
    //! Number of points in the array
    int number;
        //! Meta grid positions in X
		std::vector<double> metaX;
        //! Number of meta grid lines in X
        int numMetaX;
        //! Meta grid positions in Y
		std::vector<double> metaY;
        //! Number of meta grid lines in Y
        int numMetaY;
    bool isometric;
    RS2::CrosshairType crosshairType;

};

#endif
