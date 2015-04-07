/******************************************************************************
**  libDXFrw - Library to read/write DXF files (ascii & binary)              **
**                                                                           **
**  Copyright (C) 2011-2015 José F. Soriano, rallazz@gmail.com               **
**                                                                           **
**  This library is free software, licensed under the terms of the GNU       **
**  General Public License as published by the Free Software Foundation,     **
**  either version 2 of the License, or (at your option) any later version.  **
**  You should have received a copy of the GNU General Public License        **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.    **
******************************************************************************/

#ifndef DWGREADER21_H
#define DWGREADER21_H

#include <map>
#include <list>
#include "drw_textcodec.h"
#include "dwgbuffer.h"
#include "dwgreader.h"

//reader for AC1021 aka v2007, chapter 5
class dwgReader21 : public dwgReader {
public:
    dwgReader21(std::ifstream *stream, dwgR *p):dwgReader(stream, p){
        objData = NULL;
    }
    virtual ~dwgReader21(){
        if (objData != NULL)
            delete[] objData;
    }
    bool readMetaData();
    bool readFileHeader();
    bool readDwgHeader(DRW_Header& hdr);
    bool readDwgClasses();
    bool readDwgHandles();
    bool readDwgTables(DRW_Header& hdr);
    bool readDwgBlocks(DRW_Interface& intfa);
    virtual bool readDwgEntities(DRW_Interface& intfa){
        bool ret = true;
        dwgBuffer dataBuf(objData, uncompSize, &decoder);
        ret = dwgReader::readDwgEntities(intfa, &dataBuf);
        return ret;
    }
//bool readDwgEntity(objHandle& obj, DRW_Interface& intfa){
//    return false;
//}

private:
    duint8 *objData;
    duint64 uncompSize;

};

#endif // DWGREADER21_H
