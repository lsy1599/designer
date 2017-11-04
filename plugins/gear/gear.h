/*****************************************************************************/
/*  gear.h - gear plugin for LibreCAD                                        */
/*                                                                           */
/*  Copyright (C) 2016 Cédric Bosdonnat cedric@bosdonnat.fr                  */
/*  Edited 2017 Luis Colorado <luiscoloradourcola@gmail.com>                 */
/*                                                                           */
/*  This library is free software, licensed under the terms of the GNU       */
/*  General Public License as published by the Free Software Foundation,     */
/*  either version 2 of the License, or (at your option) any later version.  */
/*  You should have received a copy of the GNU General Public License        */
/*  along with this program.  If not, see <http://www.gnu.org/licenses/>.    */
/*****************************************************************************/

#ifndef GEAR_H
#define GEAR_H

#include "qc_plugininterface.h"
#include <QSettings>
#include <QDialog>
#include <QPointF>
#include <QSpinBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QComboBox>

class lc_Geardlg : public QDialog
{
    Q_OBJECT

public:
    explicit lc_Geardlg(QWidget *parent);
    virtual ~lc_Geardlg();

public slots:
    void processAction(Document_Interface *doc, const QString& cmd, QPointF& center);
    void checkAccept();

protected:
    void closeEvent(QCloseEvent *event);

private:
    void readSettings();
    void writeSettings();

private:

    QSettings       settings;

    QDoubleSpinBox  *rotateBox;
    QSpinBox        *nteethBox;
    QDoubleSpinBox  *modulusBox;
    QDoubleSpinBox  *pressureBox;
    QDoubleSpinBox  *addendumBox;
    QDoubleSpinBox  *dedendumBox;
    QSpinBox        *n1Box; /* number of points calculated in the dedendum part */
    QSpinBox        *n2Box; /*                    ''              addendum  ''  */
    QCheckBox       *useLayersBox;
    QCheckBox       *drawAddendumCircleBox;
    QCheckBox       *drawPitchCircleBox;
    QCheckBox       *drawBaseCircleBox;
    QCheckBox       *drawRootCircleBox;
    QCheckBox       *drawPressureLineBox;
    QCheckBox       *drawPressureLimitBox;
    QCheckBox       *calcInterferenceBox;
    QSpinBox        *n3Box;
};

class LC_Gear : public QObject, QC_PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QC_PluginInterface)
    Q_PLUGIN_METADATA(IID LC_DocumentInterface_iid FILE  "gear.json")

    lc_Geardlg      *parameters_dialog;

 public:
    LC_Gear();
    ~LC_Gear();

    virtual PluginCapabilities getCapabilities() const Q_DECL_OVERRIDE;
    virtual QString name() const Q_DECL_OVERRIDE;
    virtual void execComm(Document_Interface *doc,
                          QWidget *parent, QString cmd) Q_DECL_OVERRIDE;
};

#endif /* GEAR_H */
