#include "treeView.h"
#include <QMouseEvent>

TreeView::TreeView(QWidget *parent) : QTreeView(parent)
{

}

void TreeView::mousePressEvent ( QMouseEvent * event ) //: QTreeView::mouseReleaseEvent(event)
{
    button = event->button();
    QTreeView::mousePressEvent(event);

}
