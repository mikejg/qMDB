#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QTableView>

class TableView : public QTableView
{
    Q_OBJECT

public:
    Qt::MouseButton button;
    TableView(QWidget* parent);

protected:
    void mousePressEvent ( QMouseEvent * event );
};
#endif // TABLEVIEW_H
