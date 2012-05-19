#include "tableView.h"

#include <QMouseEvent>

TableView::TableView(QWidget *parent) : QTableView(parent)
{

}

void TableView::mousePressEvent ( QMouseEvent * event ) //: QTreeView::mouseReleaseEvent(event)
{
    button = event->button();
    QTableView::mousePressEvent(event);
}
