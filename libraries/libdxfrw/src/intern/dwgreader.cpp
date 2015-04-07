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

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "dwgreader.h"
#include "drw_textcodec.h"
#include "drw_dbg.h"

dwgReader::~dwgReader(){
    for (std::map<duint32, DRW_LType*>::iterator it=ltypemap.begin(); it!=ltypemap.end(); ++it)
        delete(it->second);
    for (std::map<duint32, DRW_Layer*>::iterator it=layermap.begin(); it!=layermap.end(); ++it)
        delete(it->second);
    for (std::map<duint32, DRW_Block*>::iterator it=blockmap.begin(); it!=blockmap.end(); ++it)
        delete(it->second);
    for (std::map<duint32, DRW_Textstyle*>::iterator it=stylemap.begin(); it!=stylemap.end(); ++it)
        delete(it->second);
    for (std::map<duint32, DRW_Dimstyle*>::iterator it=dimstylemap.begin(); it!=dimstylemap.end(); ++it)
        delete(it->second);
    for (std::map<duint32, DRW_Vport*>::iterator it=vportmap.begin(); it!=vportmap.end(); ++it)
        delete(it->second);
    for (std::map<duint32, DRW_Class*>::iterator it=classesmap.begin(); it!=classesmap.end(); ++it)
        delete(it->second);
    for (std::map<duint32, DRW_Block_Record*>::iterator it=blockRecordmap.begin(); it!=blockRecordmap.end(); ++it)
        delete(it->second);
    for (std::map<duint32, DRW_AppId*>::iterator it=appIdmap.begin(); it!=appIdmap.end(); ++it)
        delete(it->second);

    delete fileBuf;
}

void dwgReader::parseAttribs(DRW_Entity* e){
    if (e != NULL){
        duint32 ltref =e->lTypeH.ref;
        duint32 lyref =e->layerH.ref;
        std::map<duint32, DRW_LType*>::iterator lt_it = ltypemap.find(ltref);
        if (lt_it != ltypemap.end()){
            e->lineType = (lt_it->second)->name;
        }
        std::map<duint32, DRW_Layer*>::iterator ly_it = layermap.find(lyref);
        if (ly_it != layermap.end()){
            e->layer = (ly_it->second)->name;
        }
    }
}

std::string dwgReader::findTableName(DRW::TTYPE table, dint32 handle){
    std::string name;
    switch (table){
    case DRW::STYLE:{
        std::map<duint32, DRW_Textstyle*>::iterator st_it = stylemap.find(handle);
        if (st_it != stylemap.end())
            name = (st_it->second)->name;
        break;}
    case DRW::DIMSTYLE:{
        std::map<duint32, DRW_Dimstyle*>::iterator ds_it = dimstylemap.find(handle);
        if (ds_it != dimstylemap.end())
            name = (ds_it->second)->name;
        break;}
    case DRW::BLOCK_RECORD:{ //use DRW_Block because name are more correct
//        std::map<duint32, DRW_Block*>::iterator bk_it = blockmap.find(handle);
//        if (bk_it != blockmap.end())
        std::map<duint32, DRW_Block_Record*>::iterator bk_it = blockRecordmap.find(handle);
        if (bk_it != blockRecordmap.end())
            name = (bk_it->second)->name;
        break;}
/*    case DRW::VPORT:{
        std::map<duint32, DRW_Vport*>::iterator vp_it = vportmap.find(handle);
        if (vp_it != vportmap.end())
            name = (vp_it->second)->name;
        break;}*/
    case DRW::LAYER:{
        std::map<duint32, DRW_Layer*>::iterator ly_it = layermap.find(handle);
        if (ly_it != layermap.end())
            name = (ly_it->second)->name;
        break;}
    case DRW::LTYPE:{
        std::map<duint32, DRW_LType*>::iterator lt_it = ltypemap.find(handle);
        if (lt_it != ltypemap.end())
            name = (lt_it->second)->name;
        break;}
    default:
        break;
    }
    return name;
}

bool dwgReader::readDwgHeader(DRW_Header& hdr, dwgBuffer *buf, dwgBuffer *hBuf){
    bool ret = hdr.parseDwg(version, buf, hBuf, maintenanceVersion);
    //RLZ: copy objectControl handles
    return ret;
}

//RLZ: TODO add check instead print
bool dwgReader::checkSentinel(dwgBuffer *buf, enum secEnum::DWGSection, bool start){
    DRW_UNUSED(start);
    for (int i=0; i<16;i++) {
        DRW_DBGH(buf->getRawChar8()); DRW_DBG(" ");
    }
    return true;
}

/*********** objects ************************/
/**
 * Reads all the object referenced in the object map section of the DWG file
 * (using their object file offsets)
 */
bool dwgReader::readDwgTables(DRW_Header& hdr, dwgBuffer *dbuf) {
    DRW_DBG("\ndwgReader::readDwgTables start\n");
    bool ret = true;
    bool ret2 = true;
    objHandle oc;
//    blockCtrl = hdr.blockCtrl;
    dint16 oType;
    duint32 bs = 0; //bit size of handle stream 2010+

    //parse linetypes, start with linetype Control
    oc = ObjectMap[hdr.linetypeCtrl];
    ObjectMap.erase(hdr.linetypeCtrl);
    if (oc.handle == 0 /*hdr.linetypeCtrl == 0*/) {
        DRW_DBG("\nWARNING: LineType control not found\n");
        ret = false;
    } else {
        DRW_DBG("\n**********Parsing LineType control*******\n");
        DRW_ObjControl ltControl;
        dbuf->setPosition(oc.loc);
        int csize = dbuf->getModularShort();
        if (version > DRW::AC1021) //2010+
            bs = dbuf->getUModularChar();
        else
            bs = 0;
        duint8 byteStr[csize];
        dbuf->getBytes(byteStr, csize);
        dwgBuffer cbuff(byteStr, csize, &decoder);
        //verify if object are correct
        oType = cbuff.getObjType(version);
        if (oType != 0x38) {
                DRW_DBG("\nWARNING: Not LineType control object, found oType ");
                DRW_DBG(oType);  DRW_DBG(" instead 0x38\n");
                ret = false;
            } else { //reset position
            cbuff.resetPosition();
            ret2 = ltControl.parseDwg(version, &cbuff, bs);
            if(ret)
                ret = ret2;
        }

        for (std::list<duint32>::iterator it=ltControl.hadlesList.begin(); it != ltControl.hadlesList.end(); ++it){
            oc.handle = 0;
            oc = ObjectMap[*it];
            ObjectMap.erase(oc.handle);
            if (oc.handle == 0) {
                DRW_DBG("\nWARNING: LineType not found\n");
                ret = false;
            } else {
                DRW_DBG("\nLineType Handle= "); DRW_DBGH(oc.handle); DRW_DBG(" loc.: "); DRW_DBG(oc.loc); DRW_DBG("\n");
                DRW_LType *lt = new DRW_LType();
                dbuf->setPosition(oc.loc);
                int lsize = dbuf->getModularShort();
                DRW_DBG("LineType size in bytes= "); DRW_DBG(lsize);
                if (version > DRW::AC1021) //2010+
                    bs = dbuf->getUModularChar();
                else
                    bs = 0;
                duint8 byteStr[lsize];
                dbuf->getBytes(byteStr, lsize);
                dwgBuffer lbuff(byteStr, lsize, &decoder);
                ret2 = lt->parseDwg(version, &lbuff, bs);
                ltypemap[lt->handle] = lt;
                if(ret)
                    ret = ret2;
            }
        }
    }

    //parse layers, start with layer Control
    oc = ObjectMap[hdr.layerCtrl];
    ObjectMap.erase(hdr.layerCtrl);
    if (oc.handle == 0) {
        DRW_DBG("\nWARNING: Layer control not found\n");
        ret = false;
    } else {
        DRW_DBG("\n**********Parsing Layer control*******\n");
        DRW_ObjControl layControl;
        dbuf->setPosition(oc.loc);
        int size = dbuf->getModularShort();
        if (version > DRW::AC1021) //2010+
            bs = dbuf->getUModularChar();
        else
            bs = 0;
        duint8 byteStr[size];
        dbuf->getBytes(byteStr, size);
        dwgBuffer buff(byteStr, size, &decoder);
        //verify if object are correct
        oType = buff.getObjType(version);
        if (oType != 0x32) {
                DRW_DBG("\nWARNING: Not Layer control object, found oType ");
                DRW_DBG(oType);  DRW_DBG(" instead 0x32\n");
                ret = false;
            } else { //reset position
            buff.resetPosition();
            ret2 = layControl.parseDwg(version, &buff, bs);
            if(ret)
                ret = ret2;
        }

        for (std::list<duint32>::iterator it=layControl.hadlesList.begin(); it != layControl.hadlesList.end(); ++it){
            oc.handle = 0;
            oc = ObjectMap[*it];
            ObjectMap.erase(oc.handle);
            if (oc.handle == 0) {
                DRW_DBG("\nWARNING: Layer not found\n");
                ret = false;
            } else {
                DRW_DBG("Layer Handle= "); DRW_DBGH(oc.handle); DRW_DBG(" "); DRW_DBG(oc.loc); DRW_DBG("\n");
                DRW_Layer *la = new DRW_Layer();
                dbuf->setPosition(oc.loc);
                int size = dbuf->getModularShort();
                if (version > DRW::AC1021) //2010+
                    bs = dbuf->getUModularChar();
                else
                    bs = 0;
                duint8 byteStr[size];
                dbuf->getBytes(byteStr, size);
                dwgBuffer buff(byteStr, size, &decoder);
                ret2 = la->parseDwg(version, &buff, bs);
                layermap[la->handle] = la;
                if(ret)
                    ret = ret2;
            }
        }
    }

    //set linetype in layer
    for (std::map<duint32, DRW_Layer*>::iterator it=layermap.begin(); it!=layermap.end(); ++it) {
        DRW_Layer *ly = it->second;
        duint32 ref =ly->lTypeH.ref;
        std::map<duint32, DRW_LType*>::iterator lt_it = ltypemap.find(ref);
        if (lt_it != ltypemap.end()){
            ly->lineType = (lt_it->second)->name;
        }
    }

    //parse text styles, start with style Control
    oc = ObjectMap[hdr.styleCtrl];
    ObjectMap.erase(hdr.styleCtrl);
    if (oc.handle == 0) {
        DRW_DBG("\nWARNING: Style control not found\n");
        ret = false;
    } else {
        DRW_DBG("\n**********Parsing Style control*******\n");
        DRW_ObjControl styControl;
        dbuf->setPosition(oc.loc);
        int size = dbuf->getModularShort();
        if (version > DRW::AC1021) //2010+
            bs = dbuf->getUModularChar();
        else
            bs = 0;
        duint8 byteStr[size];
        dbuf->getBytes(byteStr, size);
        dwgBuffer buff(byteStr, size, &decoder);
        //verify if object are correct
        oType = buff.getObjType(version);
        if (oType != 0x34) {
                DRW_DBG("\nWARNING: Not Text Style control object, found oType ");
                DRW_DBG(oType);  DRW_DBG(" instead 0x34\n");
                ret = false;
            } else { //reset position
            buff.resetPosition();
            ret2 = styControl.parseDwg(version, &buff, bs);
            if(ret)
                ret = ret2;
        }

        for (std::list<duint32>::iterator it=styControl.hadlesList.begin(); it != styControl.hadlesList.end(); ++it){
            oc.handle = 0;
            oc = ObjectMap[*it];
            ObjectMap.erase(oc.handle);
            if (oc.handle == 0) {
                DRW_DBG("\nWARNING: Style not found\n");
                ret = false;
            } else {
                DRW_DBG("Style Handle= "); DRW_DBGH(oc.handle); DRW_DBG(" "); DRW_DBG(oc.loc); DRW_DBG("\n");
                DRW_Textstyle *sty = new DRW_Textstyle();
                dbuf->setPosition(oc.loc);
                int size = dbuf->getModularShort();
                if (version > DRW::AC1021) //2010+
                    bs = dbuf->getUModularChar();
                else
                    bs = 0;
                duint8 byteStr[size];
                dbuf->getBytes(byteStr, size);
                dwgBuffer buff(byteStr, size, &decoder);
                ret2 = sty->parseDwg(version, &buff, bs);
                stylemap[sty->handle] = sty;
                if(ret)
                    ret = ret2;
            }
        }
    }

    //parse dim styles, start with dimstyle Control
    oc.handle = 0;
    oc = ObjectMap[hdr.dimstyleCtrl];
    ObjectMap.erase(hdr.dimstyleCtrl);
    if (oc.handle == 0) {
        DRW_DBG("\nWARNING: Dimension Style control not found\n");
        ret = false;
    } else {
        DRW_DBG("\n**********Parsing Dimension Style control*******\n");
        DRW_ObjControl dimstyControl;
        dbuf->setPosition(oc.loc);
        duint32 size = dbuf->getModularShort();
        if (version > DRW::AC1021) //2010+
            bs = dbuf->getUModularChar();
        else
            bs = 0;
        duint8 byteStr[size];
        dbuf->getBytes(byteStr, size);
        dwgBuffer buff(byteStr, size, &decoder);
        //verify if object are correct
        oType = buff.getObjType(version);
        if (oType != 0x44) {
                DRW_DBG("\nWARNING: Not Dim Style control object, found oType ");
                DRW_DBG(oType);  DRW_DBG(" instead 0x44\n");
                ret = false;
            } else { //reset position
            buff.resetPosition();
            ret2 = dimstyControl.parseDwg(version, &buff, bs);
            if(ret)
                ret = ret2;
        }

        for (std::list<duint32>::iterator it=dimstyControl.hadlesList.begin(); it != dimstyControl.hadlesList.end(); ++it){
            oc.handle = 0;
            oc = ObjectMap[*it];
            ObjectMap.erase(oc.handle);
            if (oc.handle == 0) {
                DRW_DBG("\nWARNING: Dimension Style not found\n");
                ret = false;
            } else {
                DRW_DBG("Dimstyle Handle= "); DRW_DBGH(oc.handle); DRW_DBG(" "); DRW_DBG(oc.loc); DRW_DBG("\n");
                DRW_Dimstyle *sty = new DRW_Dimstyle();
                dbuf->setPosition(oc.loc);
                int size = dbuf->getModularShort();
                if (version > DRW::AC1021) //2010+
                    bs = dbuf->getUModularChar();
                else
                    bs = 0;
                duint8 byteStr[size];
                dbuf->getBytes(byteStr, size);
                dwgBuffer buff(byteStr, size, &decoder);
                ret2 = sty->parseDwg(version, &buff, bs);
                dimstylemap[sty->handle] = sty;
                if(ret)
                    ret = ret2;
            }
        }
    }

    //parse vports, start with vports Control
    oc = ObjectMap[hdr.vportCtrl];
    ObjectMap.erase(hdr.vportCtrl);
    if (oc.handle == 0) {
        DRW_DBG("\nWARNING: vports control not found\n");
        ret = false;
    } else {
        DRW_DBG("\n**********Parsing vports control*******\n");
        DRW_ObjControl vportControl;
        dbuf->setPosition(oc.loc);
        int size = dbuf->getModularShort();
        if (version > DRW::AC1021) //2010+
            bs = dbuf->getUModularChar();
        else
            bs = 0;
        duint8 byteStr[size];
        dbuf->getBytes(byteStr, size);
        dwgBuffer buff(byteStr, size, &decoder);
        //verify if object are correct
        oType = buff.getObjType(version);
        if (oType != 0x40) {
                DRW_DBG("\nWARNING: Not VPorts control object, found oType: ");
                DRW_DBG(oType);  DRW_DBG(" instead 0x40\n");
                ret = false;
            } else { //reset position
            buff.resetPosition();
            ret2 = vportControl.parseDwg(version, &buff, bs);
            if(ret)
                ret = ret2;
        }

        for (std::list<duint32>::iterator it=vportControl.hadlesList.begin(); it != vportControl.hadlesList.end(); ++it){
            oc.handle = 0;
            oc = ObjectMap[*it];
            ObjectMap.erase(oc.handle);
            if (oc.handle == 0) {
                DRW_DBG("\nWARNING: vport not found\n");
                ret = false;
            } else {
                DRW_DBG("Vport Handle= "); DRW_DBGH(oc.handle); DRW_DBG(" "); DRW_DBG(oc.loc); DRW_DBG("\n");
                DRW_Vport *vp = new DRW_Vport();
                dbuf->setPosition(oc.loc);
                int size = dbuf->getModularShort();
                if (version > DRW::AC1021) //2010+
                    bs = dbuf->getUModularChar();
                else
                    bs = 0;
                duint8 byteStr[size];
                dbuf->getBytes(byteStr, size);
                dwgBuffer buff(byteStr, size, &decoder);
                ret2 = vp->parseDwg(version, &buff, bs);
                vportmap[vp->handle] = vp;
                if(ret)
                    ret = ret2;
            }
        }
    }

    //parse Block_records , start with Block_record Control
    oc.handle = 0;
    oc = ObjectMap[hdr.blockCtrl];
    ObjectMap.erase(hdr.blockCtrl);
    if (oc.handle == 0) {
        DRW_DBG("\nWARNING: Block_record control not found\n");
        ret = false;
    } else {
        DRW_DBG("\n**********Parsing Block_record control*******\n");
        DRW_ObjControl blockControl;
        dbuf->setPosition(oc.loc);
        int csize = dbuf->getModularShort();
        if (version > DRW::AC1021) //2010+
            bs = dbuf->getUModularChar();
        else
            bs = 0;
        duint8 byteStr[csize];
        dbuf->getBytes(byteStr, csize);
        dwgBuffer buff(byteStr, csize, &decoder);
        //verify if object are correct
        oType = buff.getObjType(version);
        if (oType != 0x30) {
                DRW_DBG("\nWARNING: Not Block Record control object, found oType ");
                DRW_DBG(oType);  DRW_DBG(" instead 0x30\n");
                ret = false;
            } else { //reset position
            buff.resetPosition();
            ret2 = blockControl.parseDwg(version, &buff, bs);
            if(ret)
                ret = ret2;
        }

        for (std::list<duint32>::iterator it=blockControl.hadlesList.begin(); it != blockControl.hadlesList.end(); ++it){
            oc.handle = 0;
            oc = ObjectMap[*it];
            ObjectMap.erase(oc.handle);
            if (oc.handle == 0) {
                DRW_DBG("\nWARNING: block record not found\n");
                ret = false;
            } else {
                DRW_DBG("block record Handle= "); DRW_DBGH(oc.handle); DRW_DBG(" "); DRW_DBG(oc.loc); DRW_DBG("\n");
                DRW_Block_Record *br = new DRW_Block_Record();
                dbuf->setPosition(oc.loc);
                int size = dbuf->getModularShort();
                if (version > DRW::AC1021) //2010+
                    bs = dbuf->getUModularChar();
                else
                    bs = 0;
                duint8 byteStr[size];
                dbuf->getBytes(byteStr, size);
                dwgBuffer buff(byteStr, size, &decoder);
                ret2 = br->parseDwg(version, &buff, bs);
                blockRecordmap[br->handle] = br;
                if(ret)
                    ret = ret2;
            }
        }
    }

    //parse appId , start with appId Control
    oc.handle = 0;
    oc = ObjectMap[hdr.appidCtrl];
    ObjectMap.erase(hdr.appidCtrl);
    if (oc.handle == 0) {
        DRW_DBG("\nWARNING: AppId control not found\n");
        ret = false;
    } else {
        DRW_DBG("\n**********Parsing AppId control*******\n");
        DRW_DBG("AppId Control Obj Handle= "); DRW_DBGH(oc.handle); DRW_DBG(" "); DRW_DBG(oc.loc); DRW_DBG("\n");
        DRW_ObjControl appIdControl;
        dbuf->setPosition(oc.loc);
        int size = dbuf->getModularShort();
        if (version > DRW::AC1021) //2010+
            bs = dbuf->getUModularChar();
        else
            bs = 0;
        duint8 byteStr[size];
        dbuf->getBytes(byteStr, size);
        dwgBuffer buff(byteStr, size, &decoder);
        //verify if object are correct
        oType = buff.getObjType(version);
        if (oType != 0x42) {
                DRW_DBG("\nWARNING: Not AppId control object, found oType ");
                DRW_DBG(oType);  DRW_DBG(" instead 0x42\n");
                ret = false;
            } else { //reset position
            buff.resetPosition();
            ret2 = appIdControl.parseDwg(version, &buff, bs);
            if(ret)
                ret = ret2;
        }

        for (std::list<duint32>::iterator it=appIdControl.hadlesList.begin(); it != appIdControl.hadlesList.end(); ++it){
            oc.handle = 0;
            oc = ObjectMap[*it];
            ObjectMap.erase(oc.handle);
            if (oc.handle == 0) {
                DRW_DBG("\nWARNING: AppId not found\n");
                ret = false;
            } else {
                DRW_DBG("AppId Handle= "); DRW_DBGH(oc.handle); DRW_DBG(" "); DRW_DBG(oc.loc); DRW_DBG("\n");
                DRW_AppId *ai = new DRW_AppId();
                dbuf->setPosition(oc.loc);
                int size = dbuf->getModularShort();
                if (version > DRW::AC1021) //2010+
                    bs = dbuf->getUModularChar();
                else
                    bs = 0;
                duint8 byteStr[size];
                dbuf->getBytes(byteStr, size);
                dwgBuffer buff(byteStr, size, &decoder);
                ret2 = ai->parseDwg(version, &buff, bs);
                appIdmap[ai->handle] = ai;
                if(ret)
                    ret = ret2;
            }
        }
    }


    //RLZ: parse remaining object controls, TODO: implement all
    if (DRW_DBGGL == DRW_dbg::DEBUG){
        oc = ObjectMap[hdr.viewCtrl];
        ObjectMap.erase(hdr.viewCtrl);
        if (oc.handle == 0) {
            DRW_DBG("\nWARNING: View control not found\n");
            ret = false;
        } else {
            DRW_DBG("\n**********Parsing View control*******\n");
            DRW_DBG("View Control Obj Handle= "); DRW_DBGH(oc.handle); DRW_DBG(" "); DRW_DBG(oc.loc); DRW_DBG("\n");
            DRW_ObjControl viewControl;
            dbuf->setPosition(oc.loc);
            int size = dbuf->getModularShort();
            if (version > DRW::AC1021) //2010+
                bs = dbuf->getUModularChar();
            else
                bs = 0;
            duint8 byteStr[size];
            dbuf->getBytes(byteStr, size);
            dwgBuffer buff(byteStr, size, &decoder);
            //verify if object are correct
            oType = buff.getObjType(version);
            if (oType != 0x3C) {
                    DRW_DBG("\nWARNING: Not View control object, found oType ");
                    DRW_DBG(oType);  DRW_DBG(" instead 0x3C\n");
                    ret = false;
                } else { //reset position
                buff.resetPosition();
                ret2 = viewControl.parseDwg(version, &buff, bs);
                if(ret)
                    ret = ret2;
            }
        }

        oc = ObjectMap[hdr.ucsCtrl];
        ObjectMap.erase(hdr.ucsCtrl);
        if (oc.handle == 0) {
            DRW_DBG("\nWARNING: Ucs control not found\n");
            ret = false;
        } else {
            DRW_DBG("\n**********Parsing Ucs control*******\n");
            DRW_DBG("Ucs Control Obj Handle= "); DRW_DBGH(oc.handle); DRW_DBG(" "); DRW_DBG(oc.loc); DRW_DBG("\n");
            DRW_ObjControl ucsControl;
            dbuf->setPosition(oc.loc);
            int size = dbuf->getModularShort();
            if (version > DRW::AC1021) //2010+
                bs = dbuf->getUModularChar();
            else
                bs = 0;
            duint8 byteStr[size];
            dbuf->getBytes(byteStr, size);
            dwgBuffer buff(byteStr, size, &decoder);
            //verify if object are correct
            oType = buff.getObjType(version);
            if (oType != 0x3E) {
                    DRW_DBG("\nWARNING: Not Ucs control object, found oType ");
                    DRW_DBG(oType);  DRW_DBG(" instead 0x3E\n");
                    ret = false;
                } else { //reset position
                buff.resetPosition();
                ret2 = ucsControl.parseDwg(version, &buff, bs);
                if(ret)
                    ret = ret2;
            }
        }

        if (version < DRW::AC1018) {//r2000-
            oc = ObjectMap[hdr.vpEntHeaderCtrl];
            ObjectMap.erase(hdr.vpEntHeaderCtrl);
            if (oc.handle == 0) {
                DRW_DBG("\nWARNING: vpEntHeader control not found\n");
                ret = false;
            } else {
                DRW_DBG("\n**********Parsing vpEntHeader control*******\n");
                DRW_DBG("vpEntHeader Control Obj Handle= "); DRW_DBGH(oc.handle); DRW_DBG(" "); DRW_DBG(oc.loc); DRW_DBG("\n");
                DRW_ObjControl ucsControl;
                dbuf->setPosition(oc.loc);
                int size = dbuf->getModularShort();
                if (version > DRW::AC1021) //2010+
                    bs = dbuf->getUModularChar();
                else
                    bs = 0;
                duint8 byteStr[size];
                dbuf->getBytes(byteStr, size);
                dwgBuffer buff(byteStr, size, &decoder);
                //verify if object are correct
                oType = buff.getObjType(version);
                if (oType != 0x46) {
                        DRW_DBG("\nWARNING: Not vpEntHeader control object, found oType ");
                        DRW_DBG(oType);  DRW_DBG(" instead 0x46\n");
                        ret = false;
                    } else { //reset position
                    buff.resetPosition();
/* RLZ: writeme                   ret2 = vpEntHeader.parseDwg(version, &buff, bs);
                    if(ret)
                        ret = ret2;*/
                }
            }
        }
    }

    return ret;
}

bool dwgReader::readDwgBlocks(DRW_Interface& intfa, dwgBuffer *dbuf){
    bool ret = true;
    bool ret2 = true;
    duint32 bs =0;

    for (std::map<duint32, DRW_Block_Record*>::iterator it=blockRecordmap.begin(); it != blockRecordmap.end(); ++it){
        DRW_Block_Record* bkr= it->second;
        DRW_DBG("\nParsing Block, record handle= "); DRW_DBGH(it->first); DRW_DBG(" Name= "); DRW_DBG(bkr->name); DRW_DBG("\n");
        DRW_DBG("\nFinding Block, handle= "); DRW_DBGH(bkr->block); DRW_DBG("\n");
        objHandle oc = ObjectMap[bkr->block];
        DRW_DBG("\nFound Block, handle= "); DRW_DBGH(oc.handle); DRW_DBG("\n");
        ObjectMap.erase(oc.handle);
        if (oc.handle == 0) {
            DRW_DBG("\nWARNING: block entity not found\n");
            ret = false;
            continue;
        }
        DRW_DBG("Block Handle= "); DRW_DBGH(oc.handle); DRW_DBG(" Location: "); DRW_DBG(oc.loc); DRW_DBG("\n");
        if ( !(dbuf->setPosition(oc.loc)) ){
            DRW_DBG("Bad Location reading blocks\n");
            ret = false;
            continue;
        }
        int size = dbuf->getModularShort();
        if (version > DRW::AC1021) //2010+
            bs = dbuf->getUModularChar();
        else
            bs = 0;
        duint8 byteStr[size];
        dbuf->getBytes(byteStr, size);
        dwgBuffer buff(byteStr, size, &decoder);
        DRW_Block bk;
        ret2 = bk.parseDwg(version, &buff, bs);
        ret = ret && ret2;
        parseAttribs(&bk);
        //complete block entity with block record data
        bk.basePoint = bkr->basePoint;
        bk.flags = bkr->flags;
        intfa.addBlock(bk);
        //and update block record name
        bkr->name = bk.name;

        /**read & send block entities**/
        // in dwg code 330 are not set like dxf in ModelSpace & PaperSpace, set it (RLZ: only tested in 2000)
        if (bk.parentHandle == DRW::NoHandle) {
            // in dwg code 330 are not set like dxf in ModelSpace & PaperSpace, set it
            bk.parentHandle= bkr->handle;
            //and do not send block entities like dxf
        } else {
            if (version < DRW::AC1018) { //pre 2004
                duint32 nextH = bkr->firstEH;
                while (nextH != 0){
                    oc = ObjectMap[nextH];
                    ObjectMap.erase(nextH);
                    if (oc.handle == 0) {
                        nextH = bkr->lastEH;//end while if entity not foud
                        DRW_DBG("\nWARNING: Entity of block not found\n");
                        ret = false;
                        continue;
                    } else {//foud entity reads it
                        ret2 = readDwgEntity(dbuf, oc, intfa);
                        ret = ret && ret2;
                    }
                    if (nextH == bkr->lastEH)
                        nextH = 0; //redundant, but prevent read errors
                    else
                        nextH = nextEntLink;
                }
            } else {//2004+
                for (std::vector<duint32>::iterator it = bkr->entMap.begin() ; it != bkr->entMap.end(); ++it){
                    duint32 nextH = *it;
                    oc = ObjectMap[nextH];
                    ObjectMap.erase(nextH);
                    if (oc.handle == 0) {
                        DRW_DBG("\nWARNING: Entity of block not found\n");
                        ret = false;
                        continue;
                    } else {//foud entity reads it
                        DRW_DBG("\nBlocks, parsing entity: "); DRW_DBGH(oc.handle); DRW_DBG(", pos: "); DRW_DBG(oc.loc); DRW_DBG("\n");
                        ret2 = readDwgEntity(dbuf, oc, intfa);
                        ret = ret && ret2;
                    }
                }
            }//end 2004+
        }

        //end block entity, really needed to parse a dummy entity??
        oc = ObjectMap[bkr->endBlock];
        ObjectMap.erase(oc.handle);
        if (oc.handle == 0) {
            DRW_DBG("\nWARNING: end block entity not found\n");
            ret = false;
            continue;
        }
        DRW_DBG("End block Handle= "); DRW_DBGH(oc.handle); DRW_DBG(" Location: "); DRW_DBG(oc.loc); DRW_DBG("\n");
        dbuf->setPosition(oc.loc);
        size = dbuf->getModularShort();
        if (version > DRW::AC1021) //2010+
            bs = dbuf->getUModularChar();
        else
            bs = 0;
        duint8 byteStr1[size];
        dbuf->getBytes(byteStr1, size);
        dwgBuffer buff1(byteStr1, size, &decoder);
        DRW_Block end;
        end.isEnd = true;
        ret2 = end.parseDwg(version, &buff1, bs);
        ret = ret && ret2;
        if (bk.parentHandle == DRW::NoHandle) bk.parentHandle= bkr->handle;
        parseAttribs(&end);
        intfa.endBlock();
    }

    return ret;
}

bool dwgReader::readPlineVertex(DRW_Polyline& pline, dwgBuffer *dbuf){
    bool ret = true;
    bool ret2 = true;
    objHandle oc;
    duint32 bs = 0;

    if (version < DRW::AC1018) { //pre 2004
        duint32 nextH = pline.firstEH;
        while (nextH != 0){
            oc = ObjectMap[nextH];
            ObjectMap.erase(nextH);
            if (oc.handle == 0) {
                nextH = pline.lastEH;//end while if entity not foud
                DRW_DBG("\nWARNING: pline vertex not found\n");
                ret = false;
                continue;
            } else {//foud entity reads it
                DRW_Vertex vt;
                dbuf->setPosition(oc.loc);
                //RLZ: verify if pos is ok
                int size = dbuf->getModularShort();
                if (version > DRW::AC1021) {//2010+
                    bs = dbuf->getUModularChar();
                }
                duint8 byteStr[size];
                dbuf->getBytes(byteStr, size);
                dwgBuffer buff(byteStr, size, &decoder);
                dint16 oType = buff.getObjType(version);
                buff.resetPosition();
                DRW_DBG(" object type= "); DRW_DBG(oType); DRW_DBG("\n");
                ret2 = vt.parseDwg(version, &buff, bs, pline.basePoint.z);
                pline.addVertex(vt);
                nextEntLink = vt.nextEntLink; \
                prevEntLink = vt.prevEntLink;
                ret = ret && ret2;
            }
            if (nextH == pline.lastEH)
                nextH = 0; //redundant, but prevent read errors
            else
                nextH = nextEntLink;
        }
    } else {//2004+
        for (std::list<duint32>::iterator it = pline.hadlesList.begin() ; it != pline.hadlesList.end(); ++it){
            duint32 nextH = *it;
            oc = ObjectMap[nextH];
            ObjectMap.erase(nextH);
            if (oc.handle == 0) {
                DRW_DBG("\nWARNING: Entity of block not found\n");
                ret = false;
                continue;
            } else {//foud entity reads it
                DRW_DBG("\nPline vertex, parsing entity: "); DRW_DBGH(oc.handle); DRW_DBG(", pos: "); DRW_DBG(oc.loc); DRW_DBG("\n");
                DRW_Vertex vt;
                dbuf->setPosition(oc.loc);
                //RLZ: verify if pos is ok
                int size = dbuf->getModularShort();
                if (version > DRW::AC1021) {//2010+
                    bs = dbuf->getUModularChar();
                }
                duint8 byteStr[size];
                dbuf->getBytes(byteStr, size);
                dwgBuffer buff(byteStr, size, &decoder);
                dint16 oType = buff.getObjType(version);
                buff.resetPosition();
                DRW_DBG(" object type= "); DRW_DBG(oType); DRW_DBG("\n");
                ret2 = vt.parseDwg(version, &buff, bs, pline.basePoint.z);
                pline.addVertex(vt);
                nextEntLink = vt.nextEntLink; \
                prevEntLink = vt.prevEntLink;
                ret = ret && ret2;
            }
        }
    }//end 2004+
    DRW_DBG("\nRemoved SEQEND entity: "); DRW_DBGH(pline.seqEndH.ref);DRW_DBG("\n");
    ObjectMap.erase(pline.seqEndH.ref);

    return ret;
}

bool dwgReader::readDwgEntities(DRW_Interface& intfa, dwgBuffer *dbuf){
    bool ret = true;
    bool ret2 = true;

    for (std::map<duint32, objHandle>::iterator it=ObjectMap.begin(); it != ObjectMap.end(); ++it){
        DRW_DBG("object map Handle= "); DRW_DBG(it->first); DRW_DBG(" "); DRW_DBG(it->second.loc);// DRW_DBG("\n");
        ret2 = readDwgEntity(dbuf, it->second, intfa);
        if (ret)
            ret = ret2;
    }
    return ret;
}

/**
 * Reads a dwg drawing entity (dwg object entity) given its offset in the file
 */
bool dwgReader::readDwgEntity(dwgBuffer *dbuf, objHandle& obj, DRW_Interface& intfa){
    bool ret = true;
    duint32 bs = 0;

#define ENTRY_PARSE(e) \
            ret = e.parseDwg(version, &buff, bs); \
            parseAttribs(&e); \
    nextEntLink = e.nextEntLink; \
    prevEntLink = e.prevEntLink;

    nextEntLink = prevEntLink = 0;// set to 0 to skip unimplemented entities
        dbuf->setPosition(obj.loc);
        //RLZ: verify if pos is ok
        int size = dbuf->getModularShort();
        if (version > DRW::AC1021) {//2010+
            bs = dbuf->getUModularChar();
        }
        duint8 byteStr[size];
        dbuf->getBytes(byteStr, size);
        dwgBuffer buff(byteStr, size, &decoder);
        dint16 oType = buff.getObjType(version);
        buff.resetPosition();
        DRW_DBG(" object type= "); DRW_DBG(oType); DRW_DBG("\n");

        if (oType > 499){
            std::map<duint32, DRW_Class*>::iterator it = classesmap.find(oType);
            if (it == classesmap.end()){//fail, not found in classes set error
                DRW_DBG("Class "); DRW_DBG(oType);DRW_DBG("not found, handle: "); DRW_DBG(obj.handle); DRW_DBG("\n");
                return false;
            } else {
                DRW_Class *cl = it->second;
                if (cl->dwgType != 0)
                    oType = cl->dwgType;
            }
        }

        switch (oType){
        case 17: {
            DRW_Arc e;
            ENTRY_PARSE(e)
            intfa.addArc(e);
            break; }
        case 18: {
            DRW_Circle e;
            ENTRY_PARSE(e)
            intfa.addCircle(e);
            break; }
        case 19:{
            DRW_Line e;
            ENTRY_PARSE(e)
            intfa.addLine(e);
            break;}
        case 27: {
            DRW_Point e;
            ENTRY_PARSE(e)
            intfa.addPoint(e);
            break; }
        case 35: {
            DRW_Ellipse e;
            ENTRY_PARSE(e)
            intfa.addEllipse(e);
            break; }
        case 7:
        case 8: {//minsert = 8
            DRW_Insert e;
            ENTRY_PARSE(e)
            e.name = findTableName(DRW::BLOCK_RECORD, e.blockRecH.ref);//RLZ: find as block or blockrecord (ps & ps0)
            intfa.addInsert(e);
            break; }
        case 77: {
            DRW_LWPolyline e;
            ENTRY_PARSE(e)
            intfa.addLWPolyline(e);
            break; }
        case 1: {
            DRW_Text e;
            ENTRY_PARSE(e)
            e.style = findTableName(DRW::STYLE, e.styleH.ref);
            intfa.addText(e);
            break; }
        case 44: {
            DRW_MText e;
            ENTRY_PARSE(e)
            e.style = findTableName(DRW::STYLE, e.styleH.ref);
            intfa.addMText(e);
            break; }
        case 28: {
            DRW_3Dface e;
            ENTRY_PARSE(e)
            intfa.add3dFace(e);
            break; }
        case 20: {
            DRW_DimOrdinate e;
            ENTRY_PARSE(e)
            e.style = findTableName(DRW::DIMSTYLE, e.dimStyleH.ref);
            intfa.addDimOrdinate(&e);
            break; }
        case 21: {
            DRW_DimLinear e;
            ENTRY_PARSE(e)
            e.style = findTableName(DRW::DIMSTYLE, e.dimStyleH.ref);
            intfa.addDimLinear(&e);
            break; }
        case 22: {
            DRW_DimAligned e;
            ENTRY_PARSE(e)
            e.style = findTableName(DRW::DIMSTYLE, e.dimStyleH.ref);
            intfa.addDimAlign(&e);
            break; }
        case 23: {
            DRW_DimAngular3p e;
            ENTRY_PARSE(e)
            e.style = findTableName(DRW::DIMSTYLE, e.dimStyleH.ref);
            intfa.addDimAngular3P(&e);
            break; }
        case 24: {
            DRW_DimAngular e;
            ENTRY_PARSE(e)
            e.style = findTableName(DRW::DIMSTYLE, e.dimStyleH.ref);
            intfa.addDimAngular(&e);
            break; }
        case 25: {
            DRW_DimRadial e;
            ENTRY_PARSE(e)
            e.style = findTableName(DRW::DIMSTYLE, e.dimStyleH.ref);
            intfa.addDimRadial(&e);
            break; }
        case 26: {
            DRW_DimDiametric e;
            ENTRY_PARSE(e)
            e.style = findTableName(DRW::DIMSTYLE, e.dimStyleH.ref);
            intfa.addDimDiametric(&e);
            break; }
        case 45: {
            DRW_Leader e;
            ENTRY_PARSE(e)
            e.style = findTableName(DRW::DIMSTYLE, e.dimStyleH.ref);
            intfa.addLeader(&e);
            break; }
        case 31: {
            DRW_Solid e;
            ENTRY_PARSE(e)
            intfa.addSolid(e);
            break; }
        case 78: {
            DRW_Hatch e;
            ENTRY_PARSE(e)
            intfa.addHatch(&e);
            break; }
        case 32: {
            DRW_Trace e;
            ENTRY_PARSE(e)
            intfa.addTrace(e);
            break; }
        case 34: {
            DRW_Viewport e;
            ENTRY_PARSE(e)
            intfa.addViewport(e);
            break; }
        case 36: {
            DRW_Spline e;
            ENTRY_PARSE(e)
            intfa.addSpline(&e);
            break; }
        case 40: {
            DRW_Ray e;
            ENTRY_PARSE(e)
            intfa.addRay(e);
            break; }
        case 15:    // pline 2D
        case 16:    // pline 3D
        case 29: {  // pline PFACE
            DRW_Polyline e;
            ENTRY_PARSE(e)
            readPlineVertex(e, dbuf);
            intfa.addPolyline(e);
            break; }
//        case 30: {
//            DRW_Polyline e;// MESH (not pline)
//            ENTRY_PARSE(e)
//            intfa.addRay(e);
//            break; }
        case 41: {
            DRW_Xline e;
            ENTRY_PARSE(e)
            intfa.addXline(e);
            break; }

        default:
            break;
        }
        if (!ret){
            DRW_DBG("Object type "); DRW_DBG(oType);DRW_DBG("has failed, handle: "); DRW_DBG(obj.handle); DRW_DBG("\n");
        }
    return ret;
}

bool DRW_ObjControl::parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs){
int unkData=0;
    bool ret = DRW_TableEntry::parseDwg(version, buf, NULL, bs);
    DRW_DBG("\n***************************** parsing object control entry *********************************************\n");
    if (!ret)
        return ret;
    //last parsed is: XDic Missing Flag 2004+
    int numEntries = buf->getBitLong();
    DRW_DBG(" num entries: "); DRW_DBG(numEntries); DRW_DBG("\n");
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");

//    if (oType == 68 && version== DRW::AC1015){//V2000 dimstyle seems have one unknown byte hard handle counter??
    if (oType == 68 && version > DRW::AC1014){//dimstyle seems have one unknown byte hard handle counter??
        unkData = buf->getRawChar8();
        DRW_DBG(" unknown v2000 byte: "); DRW_DBG( unkData); DRW_DBG("\n");
    }
    if (version > DRW::AC1018){//from v2007+ have a bit for strings follows (ObjControl do not have)
        int stringBit = buf->getBit();
        DRW_DBG(" string bit for  v2007+: "); DRW_DBG( stringBit); DRW_DBG("\n");
    }

    dwgHandle objectH = buf->getHandle();
    DRW_DBG(" NULL Handle: "); DRW_DBGHL(objectH.code, objectH.size, objectH.ref); DRW_DBG("\n");
    DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");

//    if (oType == 56 && version== DRW::AC1015){//linetype in 2004 seems not have XDicObjH or NULL handle
    if (xDictFlag !=1){//linetype in 2004 seems not have XDicObjH or NULL handle
        dwgHandle XDicObjH = buf->getHandle();
        DRW_DBG(" XDicObj control Handle: "); DRW_DBGHL(XDicObjH.code, XDicObjH.size, XDicObjH.ref); DRW_DBG("\n");
        DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
    }
//add 2 for modelspace, paperspace blocks & bylayer, byblock linetypes
    numEntries = ((oType == 48) || (oType == 56)) ? (numEntries +2) : numEntries;

    for (int i =0; i< numEntries; i++){
        objectH = buf->getOffsetHandle(handle);
        if (objectH.ref != 0) //in vports R14  I found some NULL handles
            hadlesList.push_back (objectH.ref);
        DRW_DBG(" objectH Handle: "); DRW_DBGHL(objectH.code, objectH.size, objectH.ref); DRW_DBG("\n");
        DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
    }

    for (int i =0; i< unkData; i++){
        objectH = buf->getOffsetHandle(handle);
        DRW_DBG(" unknown Handle: "); DRW_DBGHL(objectH.code, objectH.size, objectH.ref); DRW_DBG("\n");
        DRW_DBG("Remaining bytes: "); DRW_DBG(buf->numRemainingBytes()); DRW_DBG("\n");
    }
    return buf->isGood();
}

