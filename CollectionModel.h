#ifndef COLLECTIONMODEL_H
#define COLLECTIONMODEL_H

#include <QStandardItemModel>
#include <QMimeData>
#include "database.h"

class CollectionModel : public QStandardItemModel
{
    Q_OBJECT
private:
    DataBase* database;

public:
    CollectionModel( QObject * parent = 0 );
    CollectionModel ( int rows, int columns, QObject * parent = 0 );
    Qt::ItemFlags flags(const QModelIndex &index);
    QStringList mimeTypes();
    QMimeData* mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data,
         Qt::DropAction action, int row, int column, const QModelIndex &parent);
    Qt::DropActions supportedDropActions() const;

    void appendInterpret(QString interpret);
    void appendAlbum(QString interpret, QString album);
    void appendTrack(QString interpret, QString album, QString track);
    void setDataBase(DataBase* db) { database = db; }
};

#endif // COLLECTIONMODEL_H
