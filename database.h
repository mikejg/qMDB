#ifndef DATABASE_H
#define DATABASE_H

#include <QtSql>
#include <QSqlDatabase>
#include <QStringList>
#include <QPixmap>
#include <QList>
#include <QDateTime>
#include <taglib/tstring.h>
#include <fileref.h>
#include <mpegfile.h>
#include <tag.h>
#include <attachedpictureframe.h>
#include <id3v2tag.h>

//#define Q4StringTotString(s) TagLib::String(s.toLatin1().data(), TagLib::String::Latin1)
//#define Q4StringToTString(s) TagLib::String(s.utf8().data(), TagLib::String::UTF8)

struct MetaPaket
{
    int id;
    QString title;
    QString artist;
    QString album;
    QString genre;
    QString year;
    uint tracknr;
    int playcounter;
    uint erfasst;
    uint gespielt;
    int wertung;
    QString pfad;
    QString coverPfad;
    QString coverData;
};



class DataBase : public QObject
{
 Q_OBJECT

 private:
 QSqlDatabase db;

 public:
 DataBase();
 ~DataBase();
 QString convert(QString);
 TagLib::String Qt4StringToString(QString s) {return TagLib::String(s.toLatin1().data(),
                                                                    TagLib::String::Latin1);}
 void cleanAlbum();
 void cleanArtist();
 void dbNeuErstellen();
 void deleteAlbum(int);
 void deleteArtist(int);
 void deletePlaylist(QString);
 void deleteTitleByUrl(QString);
 QStringList getAlbumsFromInterpret(QString);
 QList<int> getAlbumIDFromArtist (QString);
 int getAlbumIDFromArtist(QString artist, QString album);
 QPixmap getCover(QString, QString);
 QString getCoverPath (QString, QString);
 QStringList getPlaylist();
 QStringList getInterpreten();
 QString getInterpret(QString, QString);
 QStringList getSampler();
 QStringList getSongInterpretAlbum(QString);
 QList<int> getTitleIDFormAlbumID_Artist(int, QString);
 int getTitleIDFromTitle_Album_Artist(QString,QString,QString);
 QStringList getTracksFromAlbum(QString, QString);
 QStringList getUrl();
 QStringList getUrlAlbumID_ArtistID(int, int);
 QStringList getUrlArtistID(int);
 QStringList getUrlFromErfasst();
 QStringList getUrlFromPlaylist(QString);
 QString getUrlTitleID(int);
 QString getUrl(QString,QString,QString);
 int getYearFromAlbum(QString, QString);
 void insertMetaPaket(MetaPaket);
 void insertPlaylist(QString, QStringList);
 int key(QString, QString);
 MetaPaket metaPaket(QString, QString, QString);
 QStringList readPath();
 void renamePlaylist(QString, QString);
 void setSampler(int);
 void updateAlbumFromTitle(int, int);
 void updateArtistFromAlbum(int, int);
 void updateArtistFromTitle(int, int);
 void updateImageFromAlbum(int, QString);
 void updateImagePathFromAlbum(int, QString);
 void updateTitle_Track_Year_FromTitle(int, QString, int, int);
 void updateYearFromTitle(int, int);
 void writePath(QStringList);
};

#endif // DATABASE_H
