#ifndef PLAYLISTWIDGET_H
#define PLAYLISTWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include "database.h"

class PlaylistWidget : public QWidget
{
    Q_OBJECT
private:
    QString currentText;
    DataBase* database;

public:
    QListWidget* playlistTable;
    QPushButton* buttonLaden;
    QPushButton* buttonLoeschen;

    PlaylistWidget ( DataBase*, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    void insertItem(QString);

public slots:
    void currentTextChanged(QString);
    void itemChanged(QListWidgetItem*);
    void ladenClicked();
    void loeschenClicked();

signals:
    void ladePlaylist(QString);

};
#endif // PLAYLISTWIDGET_H
