#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <QTreeView>

class TreeView : public QTreeView
{
    Q_OBJECT

public:
    Qt::MouseButton button;
    TreeView(QWidget* parent);

protected:
    void mousePressEvent ( QMouseEvent * event );
};

#endif // TREEVIEW_H
