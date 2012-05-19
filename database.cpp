#include <database.h>
#include <qdebug.h>
DataBase::DataBase()
{
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("192.168.2.101");
    db.setDatabaseName("qmdb");
    db.setUserName("qmdbuser");
    db.setPassword("qmdb");
    if(db.open()) qDebug() << "DB offen :)";
}

DataBase::~DataBase(){ db.close();}

void DataBase::cleanAlbum()
{
    QSqlQuery query(db);
    QList<int> albumID;

    query.exec("SELECT t_album.id FROM t_album");
    while(query.next())
    {
        albumID << query.value(0).toInt();
    }

    foreach(int id, albumID)
    {
        query.exec("SELECT t_title.id, t_title.album FROM t_title "
                   "WHERE "
                   "t_title.album = " + QString("%1").arg(id));

        if(!query.first())
        {
           query.exec("DELETE FROM t_album "
                      "WHERE "
                      "t_album.id = " + QString("%1").arg(id));
        }
    }
}

void DataBase::cleanArtist()
{
    QSqlQuery query(db);
    QList<int> artistID;

    query.exec("SELECT t_artist.id FROM t_artist");
    while(query.next())
    {
        artistID << query.value(0).toInt();
    }

    foreach(int id, artistID)
    {
        query.exec("SELECT t_title.id, t_title.artist FROM t_title "
                   "WHERE "
                   "t_title.artist = " + QString("%1").arg(id));

        if(!query.first())
        {
           query.exec("DELETE FROM t_artist "
                      "WHERE "
                      "t_artist.id = " + QString("%1").arg(id));
        }
    }
}

QString DataBase::convert(QString s)
{
    //SQL mag manche zeichen nicht
    s = s.replace("'", "''");
    return s;
}

void DataBase::dbNeuErstellen()
{
    QSqlQuery sqlQuery(db);
    QString query;

    query = "DROP TABLE IF EXISTS t_album, t_artist, t_genre, t_title, t_year, "
            "t_verzeichnis, t_playlist, t_playlisttitle ";

    sqlQuery.exec(query);

    query = "CREATE TABLE t_playlist (id INTEGER, "
                                   "name VARCHAR(128),"
                                   "PRIMARY KEY (id))";
    sqlQuery.exec(query);

    query = "CREATE TABLE t_playlisttitle (id INTEGER NOT NULL, "
                                  "path VARCHAR(255), "
                                  "playlist INTEGER, "
                                  "FOREIGN KEY (playlist) REFERENCES t_playlist (id),"
                                  "PRIMARY KEY (id))";
    sqlQuery.exec(query);

    query = "CREATE TABLE t_verzeichnis (id INTEGER, "
                                   "name VARCHAR(255),"
                                   "PRIMARY KEY (id))";
    sqlQuery.exec(query);

    query = "CREATE TABLE t_artist (id INTEGER, "
                                   "name VARCHAR(128),"
                                   "PRIMARY KEY (id))";
    sqlQuery.exec(query);

    query = "CREATE TABLE t_album (id INTEGER NOT NULL, "
                                  "name VARCHAR(128), "
                                  "artist INTEGER, "
                                  "FOREIGN KEY (artist) REFERENCES t_artist (id),"
                                  "imagePath VARCHAR(255), "
                                  "image MEDIUMBLOB, "
                                  "hasImage TINYINT, "
                                  "PRIMARY KEY (id))";
    sqlQuery.exec(query);

    query = "CREATE TABLE t_genre (id INTEGER NOT NULL, "
                                   "name VARCHAR(128),"
                                   "PRIMARY KEY (id))";
    sqlQuery.exec(query);

    query = "CREATE TABLE t_year (id INTEGER NOT NULL, "
                                   "name VARCHAR(20),"
                                   "PRIMARY KEY (id))";
    sqlQuery.exec(query);

    query = "CREATE TABLE t_title (id INTEGER NOT NULL, "
                                   "title VARCHAR(128), "
                                   "artist INTEGER, "
                                   "album INTEGER, "
                                   "genre INTEGER,"
                                   "year INTEGER, "
                                   "tracknr INTEGER, "
                                   "playcounter INTEGER,"
                                   "erfasst INTEGER,"
                                   "gespielt INTEGER,"
                                   "wertung INTEGER,"
                                   "pfad VARCHAR(255),"
                                   "PRIMARY KEY (id),"
                                   "FOREIGN KEY (artist) REFERENCES t_artist (id),"
                                   "FOREIGN KEY (album) REFERENCES t_album (id),"
                                   "FOREIGN KEY (genre) REFERENCES t_genre (id),"
                                   "FOREIGN KEY (year) REFERENCES t_year (id))";

    sqlQuery.exec(query);
}

void DataBase::deleteAlbum(int albumID)
{
    QSqlQuery query(db);
    query.exec("DELETE FROM t_album "
               "WHERE t_album.id = "
               + QString("%1").arg(albumID));
}

void DataBase::deleteArtist(int artistID)
{
    QSqlQuery query(db);

    //erstmal gucken ob artistID noch benutzt wird
    query.exec("SELECT artist FROM t_title "
               "WHERE artist = "
               + QString("%1").arg(artistID));

    if(!query.first())
    {
        query.exec("DELETE FROM t_artist "
                   "WHERE t_artist.id = "
                   + QString("%1").arg(artistID));
    }
    //qDebug() << query.lastQuery();
    //qDebug() << query.lastError().text();
}

void DataBase::deletePlaylist(QString playlist)
{
    int idKey = key(playlist, QString("t_playlist"));

    QSqlQuery query(db);
    query.exec("DELETE FROM t_playlisttitle "
               "WHERE t_playlisttitle.playlist = "
               + QString("%1").arg(idKey));

    query.exec("DELETE FROM t_playlist "
               "WHERE "
               "t_playlist.id = " + QString("%1").arg(idKey));

}

void DataBase::deleteTitleByUrl(QString url)
{
    url = url.replace("'", "''");
    QSqlQuery query(db);

    query.exec("DELETE FROM t_title "
               "WHERE "
               "t_title.pfad = '" + url + "'");

}

QStringList DataBase::getAlbumsFromInterpret(QString artist)
{
    artist = artist.replace("'", "''");
    QStringList list;
    QSqlQuery query(db);
    //DISTINCT filtert doppelte datensätze raus!!

    query.exec("SELECT DISTINCT t_album.name, t_artist.name FROM t_title "
               "INNER JOIN t_artist ON t_title.artist = t_artist.id "
               "INNER JOIN t_album ON t_title.album = t_album.id "
               "WHERE "
               "t_artist.name = '" + artist + "' ");

    while(query.next())
    {
        list << query.value(0).toString();
    }

    return list;
}

QList<int> DataBase::getAlbumIDFromArtist(QString artist)
{
    artist = artist.replace("'", "''");
    QList<int> list;
    QSqlQuery query(db);
    //DISTINCT filtert doppelte datensätze raus!!

    query.exec("SELECT DISTINCT album, t_artist.name FROM t_title "
               "INNER JOIN t_artist ON t_title.artist = t_artist.id "
               "WHERE "
               "t_artist.name = '" + artist + "' ");

    while(query.next())
    {
        list << query.value(0).toInt();
    }

    return list;
}

int DataBase::getAlbumIDFromArtist(QString artist, QString album)
{
    artist = artist.replace("'", "''");
    album = album.replace("'", "''");

    QSqlQuery query(db);
    query.exec("SELECT t_album.id, t_album.name, t_artist.name FROM t_album "
               "INNER JOIN t_artist ON t_album.artist = t_artist.id "
               "WHERE "
               "t_artist.name = '" + artist + "' "
               "AND "
               "t_album.name = '" + album + "' ");

    if (query.first())
    {
        return query.value(0).toInt();
    }

    return 0;
}

QPixmap DataBase::getCover(QString artist, QString album)
{
    QSqlQuery query(db);
    QPixmap pixmap;
    query.exec("SELECT hasImage, image, t_album.name, t_artist.name FROM t_album "
               "INNER JOIN t_artist ON t_album.artist = t_artist.id "
               "WHERE "
               "t_album.name = '" + album + "' "
               "AND "
               "t_artist.name = '" + artist + "'");

    if(query.first())
    {
        if(query.value(0).toInt() == 1)
        {
            qDebug("hasImage");
            QString image = query.value(1).toString();
            QByteArray xcode;

            if(!image.contains("|end_stream|"))
            {
                qDebug("error sql - limit");
                pixmap.load(":/images/noCover.png", "PNG");
                return pixmap;
            }

            QStringList dat = image.split("|");
            if (dat.size() > 0)
            {
                   qDebug("/* ok complete ... */");
                   xcode.append(dat.at(0));
                   pixmap.loadFromData(QByteArray::fromBase64(xcode));
                   return pixmap;
            }
        }
    }
    pixmap.load(":/images/noCover.png", "PNG");
    return pixmap;
}

QString DataBase::getCoverPath(QString artist, QString album)
{
    QString coverPath;
    QSqlQuery query(db);
    query.exec("SELECT imagePath, t_album.name, t_artist.name FROM t_album "
               "INNER JOIN t_artist ON t_album.artist = t_artist.id "
               "WHERE "
               "t_album.name = '" + album + "' "
               "AND "
               "t_artist.name = '" + artist + "'");

    if(query.first())
    {
        qDebug() << "cover gefunden";
        coverPath = query.value(0).toString();
    }
    else
    {
        coverPath = "noCover";
    }

    return coverPath;
}

QStringList DataBase::getPlaylist()
{
    QStringList list;
    QSqlQuery query(db);
    query.exec("SELECT name FROM t_playlist ORDER BY name");
    while(query.next())
    {
        list << query.value(0).toString();
    }
    return list;
}

QStringList DataBase::getInterpreten()
{
    QStringList list;
    QSqlQuery query(db);
    query.exec("SELECT name FROM t_artist ORDER BY name");
    while(query.next())
    {
        list << query.value(0).toString();
    }
    return list;
}

QString DataBase::getInterpret(QString track, QString album)
{
    QString interpret;
    QSqlQuery query(db);
    query.exec("SELECT t_artist.name, t_album.name, t_title.title FROM t_title "
               "INNER JOIN t_artist ON t_title.artist = t_artist.id "
               "INNER JOIN t_album ON t_title.album = t_album.id "
               "WHERE "
               "t_title.title = '" + track + "' "
               "AND "
               "t_album.name = '" + album + "'");

    if(query.first())
    {
        interpret = query.value(0).toString();
    }

    return interpret;
}

QStringList DataBase::getSampler()
{
    QStringList list;
    QSqlQuery query(db);
    query.exec("SELECT t_album.name, t_album.artist FROM t_album "
               "WHERE t_album.artist = 0");

    while(query.next())
    {
        list << query.value(0).toString();
    }

    return list;
}

QStringList DataBase::getSongInterpretAlbum(QString url)
{
    url = url.replace("'", "''");
    QStringList list;
    QSqlQuery query(db);

    query.exec("SELECT pfad, title, t_album.name, t_artist.name FROM t_title "
               "INNER JOIN t_artist ON t_title.artist = t_artist.id "
               "INNER JOIN t_album ON t_title.album = t_album.id "
               "WHERE "
               "pfad = '" + url +"'");

    if(query.first())
    {
        list << query.value(1).toString();
        list << query.value(3).toString();
        list << query.value(2).toString();
    }
    return list;
}

QList<int> DataBase::getTitleIDFormAlbumID_Artist(int albumID, QString artist)
{
    QList<int> list;
    artist = artist.replace("'", "''");
    QSqlQuery query(db);
    query.exec("SELECT t_title.id, album, t_artist.name FROM t_title "
               "INNER JOIN t_artist ON t_title.artist = t_artist.id "
                "WHERE "
                "t_artist.name = '" + artist + "' "
                "AND album = " + QString("%1").arg(albumID) + " ");

    //qDebug() << query.lastQuery();
    while(query.next())
    {
        list << query.value(0).toInt();
    }

    return list;
}

int DataBase::getTitleIDFromTitle_Album_Artist(QString title, QString album, QString artist)
{
    title = title.replace("'", "''");
    album = album.replace("'", "''");
    artist = artist.replace("'", "''");

    QSqlQuery query(db);
    query.exec("SELECT t_title.id, title, t_artist.name, t_album.name FROM t_title "
               "INNER JOIN t_artist ON t_title.artist = t_artist.id "
               "INNER JOIN t_album ON t_title.album = t_album.id "
               "WHERE "
               "t_artist.name = '" + artist + "' "
               "AND t_album.name = '" + album + "' "
               "AND title = '" + title + "'");

    if(query.first())
    {
        return query.value(0).toInt();
    }

    return 0;
}

QStringList DataBase::getTracksFromAlbum(QString artist, QString album)
{
    QStringList list;
    artist = artist.replace("'", "''");
    album = album.replace("'", "''");

    QSqlQuery query(db);
    if(artist == QString("Sampler"))
    {
       query.exec("SELECT tracknr, title, t_album.name FROM t_title "
                  "INNER JOIN t_album ON t_title.album = t_album.id "
                  "WHERE "
                  "t_album.name = '" + album + "' "
                  "ORDER BY tracknr, title");
    }
    else
    {
    query.exec("SELECT tracknr, title, t_artist.name, t_album.name From t_title "
               "INNER JOIN t_artist ON t_title.artist = t_artist.id "
                "INNER JOIN t_album ON t_title.album = t_album.id "
                "WHERE "
                "t_artist.name = '" + artist + "' "
                "AND t_album.name = '" + album + "' "
                 "ORDER BY tracknr, title");
    }
    while(query.next())
    {
        list << query.value(1).toString();
    }

    return list;
}

QStringList DataBase::getUrl()
{
    QStringList list;
    QSqlQuery query(db);
    query.exec("SELECT pfad FROM t_title");

    while(query.next())
    {
        list << query.value(0).toString();
    }
    return list;
}

QStringList DataBase::getUrlAlbumID_ArtistID(int albumID, int artistID)
{
    QStringList list;
    QSqlQuery query(db);
    query.exec("SELECT pfad FROM t_title "
               "WHERE t_title.artist = "
               + QString("%1").arg(artistID) +
               " AND t_title.album = "
               + QString("%1").arg(albumID));

    qDebug() << query.lastError().text();
    qDebug() << query.lastQuery();
    while(query.next())
    {
        list << query.value(0).toString();
    }
    return list;
}

QStringList DataBase::getUrlArtistID(int artistID)
{
    QStringList list;
    QSqlQuery query(db);
    query.exec("SELECT pfad FROM t_title "
               "WHERE t_title.artist = "
               + QString("%1").arg(artistID));

    qDebug() << query.lastError().text();
    qDebug() << query.lastQuery();
    while(query.next())
    {
        list << query.value(0).toString();
    }
    return list;
}

QStringList DataBase::getUrlFromErfasst()
{
    QDateTime dateTime = QDateTime(QDate::currentDate());
    dateTime = dateTime.addMonths(-1);

    int erfasst = dateTime.toTime_t();

    QStringList list;
    QSqlQuery query(db);
    query.exec("SELECT pfad, erfasst FROM t_title "
               "WHERE erfasst > " + QString("%1").arg(erfasst));

    qDebug() << query.lastQuery();
    qDebug() << query.lastError();
    while(query.next())
    {
        list << query.value(0).toString();
    }
    return list;
}

QStringList DataBase::getUrlFromPlaylist(QString playlist)
{
    QSqlQuery query(db);
    QStringList urls;

    query.exec("SELECT path, t_playlist.name FROM t_playlisttitle "
               "INNER JOIN t_playlist ON t_playlisttitle.playlist = t_playlist.id "
               "WHERE "
               "t_playlist.name = '" + playlist + "'");

    while(query.next())
    {
        urls << query.value(0).toString();
    }

    return urls;
}

QString DataBase::getUrl(QString artist, QString album, QString title)
{
    QString file;
    artist = artist.replace("'", "''");
    album = album.replace("'", "''");
    title = title.replace("'", "''");
    QSqlQuery query(db);
    query.exec("SELECT pfad, title, t_artist.name, t_album.name FROM t_title "
               "INNER JOIN t_artist ON t_title.artist = t_artist.id "
               "INNER JOIN t_album ON t_title.album = t_album.id "
               "WHERE "
               "t_artist.name = '" + artist + "' "
               "AND t_album.name = '" + album + "' "
               "AND title = '" + title + "'");

    while(query.next())
    {
        file = query.value(0).toString();
    }

    return file;
}

QString DataBase::getUrlTitleID(int titleID)
{
    QString file;
    QSqlQuery query(db);
    query.exec("SELECT pfad, t_title.id FROM t_title "
               "WHERE t_title.id = "
               + QString("%1").arg(titleID));

    if(query.first())
        file = query.value(0).toString();

    return file;
}

int DataBase::getYearFromAlbum(QString album, QString artist)
{
    album = album.replace("'", "''");
    QSqlQuery query(db);
    query.exec("SELECT t_year.name, t_album.name FROM t_title "
               "INNER JOIN t_year ON t_title.year = t_year.id "
               "INNER JOIN t_artist ON t_title.artist = t_artist.id "
               "INNER JOIN t_album ON t_title.album = t_album.id "
               "WHERE "
               "t_album.name = '" + album + "' "
               "AND t_artist.name = '" + artist + "' ");

    if(query.first())
    {
        return query.value(0).toInt();
    }

    return 0;
}

void DataBase::insertMetaPaket(MetaPaket mp)
{
    int keyArtist = key(mp.artist, "t_artist");
    int keyAlbum = key(mp.album, "t_album");
    int keyGenre = key(mp.genre, "t_genre");
    int keyYear = key(mp.year, "t_year");
    mp.pfad = mp.pfad.toLatin1();
    mp.title = mp.title.toLatin1();

    int id = 0;
    QSqlQuery query(db);

    //In t_album den key von artist reinschreiben
    query.exec("UPDATE t_album SET t_album.artist = "
               + QString("%1").arg(keyArtist) +
               " WHERE t_album.id = "
               + QString("%1").arg(keyAlbum));

    //Nach Cover sehen
    query.exec("SELECT t_album.id, t_album.hasImage FROM t_album "
               "WHERE "
               "t_album.id = " + QString("%1").arg(keyAlbum));

    if(query.first())
    {
        if(!query.value(1).toInt())
        {
            qDebug("Kein Cover");
            QString file = mp.pfad;
            QByteArray fileName = QFile::encodeName( file );
            const char *encodedName = fileName.constData();

            TagLib::MPEG::File mpegFile(encodedName);
            TagLib::ID3v2::Tag* tag = mpegFile.ID3v2Tag();

            QImage image;
            TagLib::ID3v2::FrameList l = tag->frameList("APIC");
            if(!l.isEmpty())
              {    qDebug("lese cover");
                   TagLib::ID3v2::AttachedPictureFrame *f =
                   static_cast<TagLib::ID3v2::AttachedPictureFrame *>(l.front());
                   image.loadFromData((const uchar *) f->picture().data(), f->picture().size());
                   QByteArray pixmapBytes;
                   QBuffer pixmapBuffer(&pixmapBytes);
                   pixmapBuffer.open(QIODevice::WriteOnly);
                   image.save(&pixmapBuffer, "PNG");
                   const QString blobimage = pixmapBytes.toBase64()+"|end_stream|";
                   query.exec("UPDATE t_album SET t_album.image = '"
                              + blobimage + "' "
                              " WHERE t_album.id = "
                              + QString("%1").arg(keyAlbum));

                   //qDebug() << query.lastQuery();
                   qDebug() << query.lastError();

                   query.exec("UPDATE t_album SET t_album.hasImage = 1 "
                              "WHERE t_album.id = "
                              + QString("%1").arg(keyAlbum));
               }
        }
    }
   /* if(mp.coverPfad != "noCover")
    {
        query.exec("SELECT id, imagePath FROM t_album "
                   "WHERE id = "
                   + QString("%1").arg(keyAlbum));
        if(query.first())
        {
            if(mp.coverPfad != query.value(1).toString())
            {
                query.exec("UPDATE t_album SET t_album.imagePath = '"
                           + mp.coverPfad +
                           "' WHERE t_album.id = "
                           + QString("%1").arg(keyAlbum));
            }
        }
    }*/

    //Testen ob die Tabelle leer ist
    query.exec("SELECT id FROM t_title");
    if(query.isActive() && query.isSelect())
    {
        if(!query.first())
            id = 1;
    }

    //Es gibt schon einträge, also höchste id suchen
    if(!id)
    {
        query.exec("SELECT id FROM t_title ORDER BY id");
        if(query.isActive() && query.isSelect())
        {
            QSqlRecord record = query.record();
            if (query.last())
                id = query.value(record.indexOf("id")).toInt() + 1;
        }
    }

    query.prepare("INSERT INTO t_title (id, title, artist, album, genre, year, tracknr, playcounter, "
                                       "erfasst, gespielt, wertung, pfad) "
                                       "VALUES (:id, :title, :artist, :album, :genre, "
                                       ":year, :tracknr, :playcounter, "
                                       ":erfasst, :gespielt, :wertung, :pfad) ");
    query.bindValue(":id", id);
    query.bindValue(":title", mp.title);
    query.bindValue(":artist", keyArtist);
    query.bindValue(":album", keyAlbum);
    query.bindValue(":genre", keyGenre);
    query.bindValue(":year", keyYear);
    query.bindValue(":tracknr", mp.tracknr);
    query.bindValue(":playcounter", 0);
    query.bindValue(":erfasst", mp.erfasst);
    query.bindValue(":gespielt", mp.gespielt);
    query.bindValue(":wertung", mp.wertung);
    query.bindValue(":pfad", mp.pfad);

    query.exec();
}

void DataBase::insertPlaylist(QString playlist, QStringList urls)
{
    int idKey = key(playlist, QString("t_playlist"));
    int id = 0;
    QSqlQuery query(db);

    //Testen ob die Tabelle leer ist
    query.exec("SELECT id FROM t_playlisttitle");
    if(query.isActive() && query.isSelect())
    {
        if(!query.first())
            id = 1;
    }

    //Es gibt schon einträge, also höchste id suchen
    if(!id)
    {
        query.exec("SELECT id FROM t_playlisttitle ORDER BY id");
        if(query.isActive() && query.isSelect())
        {
            QSqlRecord record = query.record();
            if (query.last())
                id = query.value(record.indexOf("id")).toInt() + 1;
        }
    }
    qDebug() << QString("playlisturls = %1").arg(urls.count());
    foreach(QString url, urls)
    {
        query.prepare("INSERT INTO t_playlisttitle (id, path, playlist) "
                                           "VALUES (:id, :path, :playlist) ");

        query.bindValue(":id", id);
        query.bindValue(":path", url);
        query.bindValue(":playlist", idKey);
        query.exec();
        id++;
    }
}

int DataBase::key(QString s, QString table)
{
    //s = s.toLocal8Bit();
    s = s.replace("'", "''");

    QSqlQuery query(db);

    //Testen ob die Tabelle leer ist
    query.exec("SELECT id FROM " + table);
    if(query.isActive() && query.isSelect())
    {
        if(!query.first())
        {
            if(table == "t_album")
            {
                query.exec("INSERT INTO " + table + " (id, name, imagePath) "
                           "VALUES (1, '" + s +"', 'noCover')");
                return 1;
            }
            query.exec("INSERT INTO " + table + " (id, name) "
                          "VALUES (1, '" + s + "')");
            return 1;
        }
    }

    //Testen ob der key in der Tabelle ist
    query.exec("SELECT id, name FROM " + table + " WHERE "
               "name ='" + s + "'");
    QSqlRecord record = query.record();
    if (query.next())
        return query.value(record.indexOf("id")).toInt();

    //Key ist nicht in der Tabelle -> Eintrag erstellen
    //erstmal die letzte bzw höchste id suchen
    query.exec("SELECT id FROM " + table + " ORDER BY id");
    if(query.isActive() && query.isSelect())
    {
        record = query.record();
        if (query.last())
        {
            int id = query.value(record.indexOf("id")).toInt() + 1;
            if(table == "t_album")
            {
                //query.exec("INSERT INTO " + table + " (id, name, imagePath) "
                //           "VALUES (" + QString("%1").arg(id)  + ", '" + s +"', 'noCover')");

                query.exec("INSERT INTO " + table + " (id, name, hasImage) "
                           "VALUES (" + QString("%1").arg(id)  + ", '" + s +"', 0)");
                return id;
            }
            query.exec("INSERT INTO " + table + " (id, name) "
                       "VALUES (" + QString("%1").arg(id)  + ", '" + s + "')");
            return id;
        }
    }
}

MetaPaket DataBase::metaPaket(QString title, QString artist, QString album)
{
    MetaPaket mp;
    artist = artist.replace("'", "''");
    album = album.replace("'", "''");
    title = title.replace("'", "''");

    QSqlQuery query(db);
    query.exec("SELECT *, t_artist.name, t_album.name, t_year.name, t_genre.name "
               "FROM t_title "
               "INNER JOIN t_artist ON t_title.artist = t_artist.id "
               "INNER JOIN t_album ON t_title.album = t_album.id "
               "INNER JOIN t_year ON t_title.year = t_year.id "
               "INNER JOIN t_genre ON t_title.genre = t_genre.id "
               "WHERE "
               "t_artist.name = '" + artist + "' "
               "AND t_album.name = '" + album + "' "
               "AND title = '" + title + "'");
    //qDebug() << query.lastError().text();
    //qDebug() << query.lastQuery();

    while(query.next())
    {
        mp.id = query.value(0).toInt();
        mp.title = title;
        mp.artist = artist;
        mp.album = album;
        mp.genre = query.value(21).toString();
        mp.year = query.value(19).toString();
        mp.tracknr = query.value(6).toInt();
        mp.playcounter = query.value(7).toInt();
        mp.erfasst = query.value(8).toInt();
        mp.gespielt = query.value(9).toInt();
        mp.wertung = query.value(10).toInt();
        mp.pfad = query.value(11).toString();
        //for(int i = 0; i < 20 ; i++)
        //    qDebug() << i << ":" << query.value(i).toString();
    }
    return mp;
}

QStringList DataBase::readPath()
{
    QSqlQuery query(db);
    QStringList list;
    query.exec("SELECT t_verzeichnis.name FROM t_verzeichnis");

    while(query.next())
    {
        list << query.value(0).toString();
    }

    return list;
}

void DataBase::renamePlaylist(QString oldName, QString newName)
{
    qDebug("renamePlaylist");
    qDebug() << oldName;
    qDebug() << newName;
    if(oldName == QString(""))
    {
        qDebug("gleicher string");
        return;
    }


     QSqlQuery query(db);
     query.exec("SELECT t_playlist.name FROM t_playlist "
                "WHERE "
                "t_playlist.name ='" + newName + "'");

     //newName exestiert noch nicht
     if(!query.first())
     {
         query.exec("UPDATE t_playlist "
                    "SET "
                    "t_playlist.name = '" + newName + "' "
                    "WHERE "
                    "t_playlist.name = '" + oldName + "'");

         return;
     }

     int newId = key(newName, QString("t_playlist"));
     int oldId = key(oldName, QString("t_playlist"));

     query.exec("DELETE FROM t_playlisttitle "
                "WHERE "
                "t_playlisttitle.playlist = " + QString("%1").arg(newId));
     qDebug() << query.lastQuery();
     qDebug() << query.lastError();

     query.exec("UPDATE t_playlisttitle "
                "SET "
                "t_playlisttitle.playlist = " + QString("%1").arg(newId) + " "
                "WHERE "
                "t_playlisttitle.playlist = " + QString("%1").arg(oldId));
     qDebug() << query.lastQuery();
     qDebug() << query.lastError();

     query.exec("DELETE FROM t_playlist "
                "WHERE "
                "t_playlist.name = '" + oldName +"'");

     qDebug() << query.lastQuery();
     qDebug() << query.lastError();
}

void DataBase::setSampler(int albumID)
{
    QSqlQuery query(db);
    query.exec("UPDATE t_album SET t_album.artist = 0 "
               "WHERE "
               "t_album.id = " + QString("%1").arg(albumID));
}

void DataBase::updateAlbumFromTitle(int titleID, int albumID)
{
    QSqlQuery query(db);
    query.exec("UPDATE t_title SET t_title.album = "
               + QString("%1").arg(albumID) +
               " WHERE t_title.id = "
               + QString("%1").arg(titleID));
    qDebug() << query.lastQuery();
    qDebug() << query.lastError().text();
}

void DataBase::updateArtistFromAlbum(int albumID, int artistID)
{
    QSqlQuery query(db);
    //Hier muss noch eine Abfrage wegen Samplern rein
    //Bei Sampler ist t_album.artis = NULL

    query.exec("UPDATE t_album SET t_album.artist = "
               + QString("%1").arg(artistID) +
               " WHERE t_album.id = "
               + QString("%1").arg(albumID));
    //qDebug() << query.lastQuery();
    //qDebug() << query.lastError().text();
}

void DataBase::updateArtistFromTitle(int titleID, int artistID)
{
    QSqlQuery query(db);
    query.exec("UPDATE t_title SET t_title.artist = "
               + QString("%1").arg(artistID) +
               " WHERE t_title.id = "
               + QString("%1").arg(titleID));
    //qDebug() << query.lastQuery();
    //qDebug() << query.lastError().text();
}

void DataBase::updateImageFromAlbum(int albumID, QString blobimage)
{
    QSqlQuery query(db);
    query.exec("UPDATE t_album SET hasImage = 1, "
               "t_album.image = '"
               + blobimage +
               "' WHERE t_album.id = "
               + QString("%1").arg(albumID));
}

void DataBase::updateImagePathFromAlbum(int albumID, QString path)
{
    path = path.replace("'", "''");
    QSqlQuery query(db);
    query.exec("UPDATE t_album SET t_album.imagePath = '"
               + path +
               "' WHERE t_album.id = "
               + QString("%1").arg(albumID));
}

void DataBase::updateTitle_Track_Year_FromTitle(int titleID, QString title, int track, int yearID)
{
    title = title.replace("'", "''");
    QSqlQuery query(db);
    query.exec("UPDATE t_title SET t_title.year = "
               + QString("%1").arg(yearID) +
               ", t_title.tracknr = "
               + QString("%1").arg(track) +
               ", t_title.title = '" +
               title +
               "' WHERE t_title.id = "
               + QString("%1").arg(titleID));

    qDebug() << query.lastQuery();
    qDebug() << query.lastError().text();

}

void DataBase::updateYearFromTitle(int titleID, int yearID)
{
    QSqlQuery query(db);
    query.exec("UPDATE t_title SET t_title.year = "
               + QString("%1").arg(yearID) +
               " WHERE t_title.id = "
               + QString("%1").arg(titleID));
    qDebug() << query.lastQuery();
    qDebug() << query.lastError().text();
}

void DataBase::writePath(QStringList path)
{
    QSqlQuery query(db);
    query.exec("DROP TABLE IF EXISTS t_verzeichnis");

    query.exec("CREATE TABLE t_verzeichnis (id INTEGER, "
               "name VARCHAR(255),"
               "PRIMARY KEY (id))");

    int id = 1;

    foreach(QString p, path)
    {
        query.prepare("INSERT INTO t_verzeichnis (id, name) "
                   "VALUES (:id, :name) ");
        query.bindValue(":id", id);
        query.bindValue(":name", p);
        query.exec();
        id++;
    }
}
