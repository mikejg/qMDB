#include "jobs.h"
#include "database.h"
#include <fileref.h>
#include <mpegfile.h>
#include <tag.h>
#include <attachedpictureframe.h>
#include <QFile>
#include <QTextCodec>
#include <QSettings>
#include <QImage>

JobInsertMetaPaket::JobInsertMetaPaket(QProgressBar* pb, DataBase* db, QObject *parent, const char *name) : QObject(parent)
{
    Q_UNUSED(name);

    database = db;
    progressBar = pb;
    timerID = 0;
}

JobInsertMetaPaket::~JobInsertMetaPaket() {}


QString JobInsertMetaPaket::readCover(TagLib::ID3v2::Tag *tag, QString artist, QString album)
{
    QImage image;
    TagLib::ID3v2::FrameList l = tag->frameList("APIC");
    if(!l.isEmpty())
      {
           TagLib::ID3v2::AttachedPictureFrame *f =
           static_cast<TagLib::ID3v2::AttachedPictureFrame *>(l.front());
           image.loadFromData((const uchar *) f->picture().data(), f->picture().size());
           QString fileName = QDir::homePath() + "/.config/qmdb/cover/" + artist + "-" + album;
           image.save(fileName, "PNG");
           qDebug() << fileName;
           return fileName;
       }

    return QString("noCover");
}

void JobInsertMetaPaket::setFileListe(QStringList fl)
{
    fileListe = fl;
    if(fileListe.size() > 0)
    progressBar->setRange(0, fileListe.size());
}

void JobInsertMetaPaket::startJob()
{
    if(!timerID)
    {
        dateTime = QDateTime(QDate::currentDate());
        erfasst = dateTime.toTime_t();
        timerID = startTimer(250);
        progressBar->setValue(0);
    }

}

void JobInsertMetaPaket::timerEvent(QTimerEvent* e)
{
    Q_UNUSED(e);
    QString comment;
    
    if(fileListe.count() > 0)
    {
        QString file = fileListe.first();
        QByteArray fileName = QFile::encodeName( file );
        const char *encodedName = fileName.constData();

        TagLib::FileRef f(encodedName);
        if(!f.isNull())
        {
            MetaPaket mp;
            mp.title = TStringToQString(f.tag()->title()).toLatin1();
            mp.artist = TStringToQString(f.tag()->artist()).toLatin1();
            mp.album = TStringToQString(f.tag()->album()).toLatin1();
            mp.genre = TStringToQString(f.tag()->genre()).toLatin1();
            mp.year = QString("%1").arg(f.tag()->year());
            mp.tracknr = f.tag()->track();
            mp.playcounter = 0;
            mp.erfasst = erfasst;
            mp.wertung = 0;
            mp.gespielt = 0;
            comment = TStringToQString(f.tag()->comment()).toLatin1();
            if(comment.startsWith(QString("QMDB")))
            {
                mp.wertung = comment.section('#', 1,1).toInt();
                mp.gespielt = comment.section('#', 2,2).toUInt();

                qDebug() << QString("Wertung im Tag: %1").arg(mp.wertung);
            }
            mp.pfad = file.toLatin1();
            mp.coverPfad = QString("noCover");

            /*QString fileName = QDir::homePath() + "/.config/qmdb/cover/" + mp.artist + "-" + mp.album;
            QFileInfo info(fileName);
            if(info.exists())
                mp.coverPfad = fileName.toLatin1();
            else
            {
                TagLib::MPEG::File mpegFile(encodedName);
                TagLib::ID3v2::Tag* t = mpegFile.ID3v2Tag();
                if(t)
                    mp.coverPfad = readCover(t, mp.artist, mp.album).toLatin1();
            }*/

            database->insertMetaPaket(mp);
        }
        else
        {
            qDebug() << file;
        }
        fileListe.pop_front();
        progressBar->setValue(progressBar->value() + 1);
    }
    else
    {
        killTimer(timerID);
        timerID = 0;
        done();
    }
}

JobDeleteMetaPaket::JobDeleteMetaPaket(QProgressBar* pb, DataBase* db, QObject *parent, const char *name) : QObject(parent)
{
    Q_UNUSED(name);

    database = db;
    progressBar = pb;
    timerID = 0;

    jobInsertMetaPaket = new JobInsertMetaPaket(pb, db, this);

    connect(jobInsertMetaPaket, SIGNAL(done()), this, SIGNAL(done()));
}

JobDeleteMetaPaket::~JobDeleteMetaPaket() {}


void JobDeleteMetaPaket::setFileListe(QStringList fl, QStringList ifl)
{
    fileListe = fl;
    if(fileListe.size() > 0)
       progressBar->setRange(0, fileListe.size());
    jobInsertMetaPaket->setFileListe(ifl);
}

void JobDeleteMetaPaket::startJob()
{
    if(!timerID)
    {
        dateTime = QDateTime(QDate::currentDate());
        erfasst = dateTime.toTime_t();
        timerID = startTimer(250);
        progressBar->setValue(0);
    }

}

void JobDeleteMetaPaket::timerEvent(QTimerEvent* e)
{
    Q_UNUSED(e);

    if(fileListe.count() > 0)
    {
        database->deleteTitleByUrl(fileListe.first());
        fileListe.pop_front();
        progressBar->setValue(progressBar->value() + 1);
    }
    else
    {
        database->cleanAlbum();
        database->cleanArtist();
        killTimer(timerID);
        timerID = 0;
        jobInsertMetaPaket->startJob();
    }
}

JobDBUpdate::JobDBUpdate(QProgressBar* pb, DataBase* db, QObject *parent, const char *name) : QObject(parent)
{
    Q_UNUSED(name);

    database = db;
    progressBar = pb;
    timerID = 0;
}

JobDBUpdate::~JobDBUpdate() {}

void JobDBUpdate::findTracks(QDir dir, QStringList* list)
{
    //Sucht rekursiv im Verzeichnis dir nach mp3 Files die dann
    //in list abgelegt werden

    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);

    QFileInfoList infoList = dir.entryInfoList();
    foreach(QFileInfo info, infoList)
    {
        if(info.isFile())
        {
            if(info.suffix() == "mp3")
              *list << info.absoluteFilePath();
        }
        if(info.isDir())
        {
            dir.cd(info.absoluteFilePath());
            findTracks(dir, list);
        }
    }
}

void JobDBUpdate::startJob()
{
    if(!timerID)
    {
        filesHD.clear();
        filesDB.clear();
        filesInput.clear();
        filesOutput.clear();

        QStringList dirs = database->readPath();
        QDir dir;
        foreach(QString d, dirs)
        {
            dir.setPath(d);
            findTracks(dir,&filesHD);
        }

        filesDB = database->getUrl();

        foreach(QString inputFile, filesHD)
        {
            if(!filesDB.contains(inputFile))
            {
                filesInput << inputFile;
            }
        }

        foreach(QString outputFile, filesDB)
        {
            if(!filesHD.contains(outputFile))
            {
                filesOutput << outputFile;
            }
        }

        dateTime = QDateTime(QDate::currentDate());
        erfasst = dateTime.toTime_t();

        if(!filesOutput.isEmpty() || !filesInput.isEmpty())
        {
            progressBar->setRange(0, filesOutput.size() + filesInput.size());
            timerID = startTimer(50);
            progressBar->setValue(0);
        }

    }
}

void JobDBUpdate::timerEvent(QTimerEvent* e)
{
    Q_UNUSED(e);
    qDebug() << QString("JobDBUpdate timerEvent");
    if(filesOutput.count() > 0)
    {
       database->deleteTitleByUrl(filesOutput.first());
       filesOutput.pop_front();
       progressBar->setValue(progressBar->value() + 1);
       return;
    }

    if(filesInput.count() > 0)
    {
        QString file = filesInput.first();
        QByteArray fileName = QFile::encodeName( file );
        const char *encodedName = fileName.constData();

        TagLib::FileRef f(encodedName);
        if(!f.isNull())
        {
            MetaPaket mp;
            mp.title = TStringToQString(f.tag()->title()).toLatin1();
            mp.artist = TStringToQString(f.tag()->artist()).toLatin1();
            mp.album = TStringToQString(f.tag()->album()).toLatin1();
            mp.genre = TStringToQString(f.tag()->genre()).toLatin1();
            mp.year = QString("%1").arg(f.tag()->year());
            mp.tracknr = f.tag()->track();
            mp.playcounter = 0;
            mp.erfasst = erfasst;
            mp.wertung = 0;
            mp.pfad = file.toLatin1();
            mp.coverPfad = QString("noCover");
            database->insertMetaPaket(mp);
        }
        else
        {
            qDebug() << file;
        }
        filesInput.pop_front();
        progressBar->setValue(progressBar->value() + 1);
        return;
    }
    database->cleanAlbum();
    database->cleanArtist();
    killTimer(timerID);
    timerID = 0;
    done();
}
