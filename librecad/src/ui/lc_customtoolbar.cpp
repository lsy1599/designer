#include "lc_customtoolbar.h"

#include <QTextStream>
#include <QFile>
#include <QAction>

LC_CustomToolbar::LC_CustomToolbar(QString name, QWidget* parent): QToolBar(name, parent)
{
    setObjectName("lc_customtoolbar");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QAction* action = new QAction(tr("Add or Remove Action"), parent);
    action->setShortcut(QKeySequence("F2"));
    connect(action, SIGNAL(triggered()), this, SLOT(slot_add_or_remove_action()));
    addAction(action);
}

LC_CustomToolbar::~LC_CustomToolbar()
{
    if (!file_path.isNull())
    {
        QFile file(file_path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;

        QTextStream txt_stream(&file);
        foreach (QString token, state_list)
        {
            txt_stream << token << "\n";
        }
    }
}

void LC_CustomToolbar::actions_from_file(QString path, QMap<QString, QAction*> map_a)
{
    QFile file(path);    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    file_path = path;

    QTextStream txt_stream(&file);
    QString line;
    while (!txt_stream.atEnd())
    {
        line = txt_stream.readLine();

        if (line == "-")
        {
            addSeparator();
            state_list << line;
        }
        else if (map_a.contains(line))
        {
            addAction(map_a[line]);
            state_list << line;
        }
    }
}

void LC_CustomToolbar::slot_add_or_remove_action()
{
    if (most_recent_action != nullptr)
    {
        QString token = most_recent_action->data().toString();

        if (state_list.contains(token))
        {
            removeAction(most_recent_action);
            state_list.removeOne(token);
        }
        else
        {
            addAction(most_recent_action);
            state_list << token;
        }
    }
}

void LC_CustomToolbar::slot_most_recent_action(QAction* q_action)
{
    most_recent_action = q_action;
}


void LC_CustomToolbar::add_separator()
{
    addSeparator();
    state_list << "-";
}
