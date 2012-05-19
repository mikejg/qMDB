#include "CollectionModel.h"
#include <QDebug>

CollectionModel::CollectionModel(QObject *parent) : QStandardItemModel(parent)
{

}

CollectionModel::CollectionModel ( int rows, int columns, QObject * parent ) :
        QStandardItemModel(rows, columns, parent)
{}

Qt::ItemFlags CollectionModel::flags(const QModelIndex &index)
 {


     if (index.isValid())
         return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
     else
         return Qt::ItemIsDropEnabled;
 }

Qt::DropActions CollectionModel::supportedDropActions() const
 {
     return Qt::CopyAction | Qt::MoveAction;
 }

QStringList CollectionModel::mimeTypes()
 {
     QStringList types;
     types << "application/x-qabstractitemmodeldatalist";
     return types;
 }

QMimeData* CollectionModel::mimeData(const QModelIndexList &indexes) const
 {
     qDebug("drag");
     qDebug() << QString("%1").arg(indexes.size());
     QMimeData *mimeData = new QMimeData();
     QByteArray encodedData;

     QDataStream stream(&encodedData, QIODevice::WriteOnly);

     foreach (QModelIndex index, indexes) {
         if (index.isValid())
            {
             QString text = data(index, Qt::WhatsThisRole).toString();
             stream << text;
             if( text == QString("Interpret"))
               {
                text = data(index, Qt::DisplayRole).toString();
                stream << text;
               }
             if ( text == QString("Album"))
                {
                 text = data(index.parent(), Qt::DisplayRole).toString();
                 stream << text;
                 text = data(index, Qt::DisplayRole).toString();
                 stream << text;
                }
             if ( text == QString("Track"))
             {
                 text = data(index.parent().parent(), Qt::DisplayRole).toString();
                 stream << text;
                 text = data(index.parent(), Qt::DisplayRole).toString();
                 stream << text;
                 text = data(index, Qt::DisplayRole).toString();
                 stream << text;
             }
            }
     }

     mimeData->setData("application/x-qabstractitemmodeldatalist", encodedData);
     return mimeData;
 }

bool CollectionModel::dropMimeData(const QMimeData *data,
     Qt::DropAction action, int row, int column, const QModelIndex &parent)
 {
    QByteArray encodedData = data->data("application/x-qabstractitemmodeldatalist");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QStringList newItems;
    int rows = 0;

    while (!stream.atEnd())
          {
             QString text;
             stream >> text;
             newItems << text;
             ++rows;
          }

    QStandardItem *parentItem = invisibleRootItem();

        if(newItems.at(0) == QString("Interpret"))
            appendInterpret(newItems.at(1));
        if(newItems.at(0) == QString("Album"))
           appendAlbum(newItems.at(1), newItems.at(2));
        if(newItems.at(0) == QString("Track"))
            appendTrack(newItems.at(1), newItems.at(2), newItems.at(3));

}

void CollectionModel::appendInterpret(QString interpret)
{
    QList<QStandardItem*> items;
    QStandardItem* item;
    QStringList alben = database->getAlbumsFromInterpret(interpret);
    QString artist;
    foreach (QString album, alben)
            {
              QStringList songs = database->getTracksFromAlbum(interpret, album);
              foreach(QString song, songs)
                     {
                       artist = interpret;
                       items.clear();
                       item = new QStandardItem(song);
                       item->setWhatsThis("song");
                       items << item;
                       if(interpret == QString("Sampler"))
                           artist = database->getInterpret(song, album);
                       item = new QStandardItem(artist);
                       item->setWhatsThis("interpret");
                       items << item;
                       item = new QStandardItem(album);
                       item->setWhatsThis("album");
                       items << item;
                       appendRow(items);
                     }
            }
}

void CollectionModel::appendAlbum(QString interpret, QString album)
{
    QList<QStandardItem*> items;
    QStandardItem* item;
    QStringList songs = database->getTracksFromAlbum(interpret, album);
    QString artist;
    foreach(QString song, songs)
           {
             artist = interpret;
             items.clear();
             item = new QStandardItem(song);
             item->setWhatsThis("song");
             items << item;
             if(interpret == QString("Sampler"))
             {
                 qDebug("Sampler");
                 artist = database->getInterpret(song, album);
                 qDebug() << interpret;
             }
             item = new QStandardItem(artist);
             item->setWhatsThis("interpret");
             items << item;
             item = new QStandardItem(album);
             item->setWhatsThis("album");
             items << item;
             appendRow(items);
           }
}

void CollectionModel::appendTrack(QString interpret, QString album, QString track)
{
    QList<QStandardItem*> items;
    QStandardItem* item;
    item = new QStandardItem(track);
    item->setWhatsThis("song");
    items << item;
    if(interpret == QString("Sampler"))
        interpret = database->getInterpret(track, album);
    item = new QStandardItem(interpret);
    item->setWhatsThis("interpret");
    items << item;
    item = new QStandardItem(album);
    item->setWhatsThis("album");
    items << item;
    appendRow(items);

}
