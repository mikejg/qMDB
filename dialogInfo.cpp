#include "dialogInfo.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QFileDialog>
#include <QBuffer>
#include <mpegfile.h>
#include <id3v2tag.h>
#include <attachedpictureframe.h>
#include <taglib/tstring.h>

DialogInfo::DialogInfo(DataBase* db,QWidget* parent, Qt::WindowFlags f) : QDialog(parent, f)
{
    database = db;
    setWindowTitle("Info");
    QRect dr = parent->geometry();
    QPoint cp(dr.width()/2, dr.height()/2);
    setGeometry(QRect(cp.x()-200, cp.y()-100, 400,10));

    tabWidget = new QTabWidget(this);
    QVBoxLayout* masterLayout = new QVBoxLayout(this);
    masterLayout->addWidget(tabWidget);

    QWidget* id3tagWidget = new QWidget(tabWidget);

    QVBoxLayout* id3tagLayout = new QVBoxLayout(id3tagWidget);

    QHBoxLayout* titleLayout = new QHBoxLayout();
    QLabel* labelTitle = new QLabel("Title", id3tagWidget);
    labelTitle->setFixedWidth(100);
    lineEditTitle = new QLineEdit(id3tagWidget);
    titleLayout->addWidget(labelTitle);
    titleLayout->addWidget(lineEditTitle);

    QHBoxLayout* artistLayout = new QHBoxLayout();
    QLabel* labelArtist = new QLabel("Artist", id3tagWidget);
    labelArtist->setFixedWidth(100);
    lineEditArtist = new QLineEdit(id3tagWidget);
    artistLayout->addWidget(labelArtist);
    artistLayout->addWidget(lineEditArtist);

    QHBoxLayout* albumLayout = new QHBoxLayout();
    QLabel* labelAlbum  = new QLabel("Album", id3tagWidget);
    labelAlbum->setFixedWidth(100);
    lineEditAlbum = new QLineEdit(id3tagWidget);
    albumLayout->addWidget(labelAlbum);
    albumLayout->addWidget(lineEditAlbum);

    QHBoxLayout* genreLayout = new QHBoxLayout();
    QLabel* labelGenre  = new QLabel("Genre", id3tagWidget);
    labelGenre->setFixedWidth(100);
    lineEditGenre = new QLineEdit(id3tagWidget);
    genreLayout->addWidget(labelGenre);
    genreLayout->addWidget(lineEditGenre);

    QHBoxLayout* trackLayout = new QHBoxLayout();
    QLabel* labelTrack  = new QLabel("Track", id3tagWidget);
    labelTrack->setFixedWidth(100);
    spinBoxTrack = new QSpinBox(id3tagWidget);
    spinBoxTrack->setFixedWidth(75);
    spinBoxTrack->setMaximum(900);

    //QHBoxLayout* yearLayout = new QHBoxLayout();
    QLabel* labelYear  = new QLabel("Year", id3tagWidget);
    labelYear->setFixedWidth(75);
    spinBoxYear = new QSpinBox(id3tagWidget);
    spinBoxYear->setFixedWidth(75);
    spinBoxYear->setMaximum(2100);
    trackLayout->addWidget(labelTrack);
    trackLayout->addWidget(spinBoxTrack);
    trackLayout->addStretch(500);
    trackLayout->addWidget(labelYear);
    trackLayout->addWidget(spinBoxYear);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonAnwenden = new QPushButton("Anwenden", id3tagWidget);
    buttonAbbrechen = new QPushButton("Abbrechen", id3tagWidget);
    buttonLayout->addWidget(buttonAnwenden);
    buttonLayout->addWidget(buttonAbbrechen);

    checkBoxSampler = new QCheckBox("Sampler", id3tagWidget);
    id3tagLayout->addLayout(titleLayout);
    id3tagLayout->addLayout(artistLayout);
    id3tagLayout->addLayout(albumLayout);
    id3tagLayout->addLayout(genreLayout);
    id3tagLayout->addLayout(trackLayout);
    id3tagLayout->addWidget(checkBoxSampler);
    id3tagLayout->addLayout(buttonLayout);

    coverWidget = new QWidget(this);
    QHBoxLayout* coverLayout = new QHBoxLayout(coverWidget);
    pixmapLabel = new QLabel(coverWidget);
    pixmapCover = new QPixmap(":/images/noCover.png", "PNG");
    pixmapLabel->setPixmap(pixmapCover->scaled(QSize(150,150)));

    QVBoxLayout* coverButtonLayout = new QVBoxLayout();
    coverLaden = new QPushButton("Laden", coverWidget);
    coverAnwenden = new QPushButton("Anwenden", coverWidget);
    coverAbbrechen = new QPushButton("Abbrechen", coverWidget);
    coverButtonLayout->addWidget(coverLaden);
    coverButtonLayout->addWidget(coverAnwenden);
    coverButtonLayout->addWidget(coverAbbrechen);

    coverLayout->addWidget(pixmapLabel);
    coverLayout->addLayout(coverButtonLayout);

    tabWidget->addTab(id3tagWidget,"id3tag");
    tabWidget->addTab(coverWidget, "Cover");

    connect(buttonAbbrechen, SIGNAL(clicked()), this, SLOT(close()));
    connect(coverAbbrechen, SIGNAL(clicked()), this, SLOT(close()));
    connect(buttonAnwenden, SIGNAL(clicked()), this, SLOT(anwenden()));
    connect(coverLaden, SIGNAL(clicked()), this, SLOT(loadCover()));
    connect(coverAnwenden, SIGNAL(clicked()), this, SLOT(anwendenCover()));
}

void DialogInfo::anwenden()
{
    if(infoType == artist && oldArtist != lineEditArtist->text())
    {
        anwendenArtist();
    }

    if(infoType == album)
    {
        anwendenAlbum();
    }

    if(infoType == title)
    {
        anwendenTrack();
    }
}

void DialogInfo::anwendenAlbum()
{
    int oldArtistID = database->key(oldArtist, "t_artist");
    int oldAlbumID = database->key(oldAlbum, "t_album");


    //ID-Liste der Titel vom Album holen
    QList<int> idListTitle = database->getTitleIDFormAlbumID_Artist(oldAlbumID, oldArtist);
    //Pafde der Titel vom Album holen
    QStringList listePfad = database->getUrlAlbumID_ArtistID(oldAlbumID, oldArtistID);

    if(oldArtist != lineEditArtist->text())
    {
        //id für den neuen artist besorgen
        int newArtistID = database->key(lineEditArtist->text(), "t_artist");
        //in der tabelle t_album die spalte artist vom album ändern
        //aber nur wenn es kein Sampler ist
        if(checkBoxSampler->checkState() == Qt::Unchecked)
           database->updateArtistFromAlbum(oldAlbumID, newArtistID );

        //in der tabelle t_title für jeden titel des albums die spalte artist ändern
        foreach (int titleID, idListTitle)
        {
            database->updateArtistFromTitle(titleID, newArtistID);
        }

        database->deleteArtist(oldArtistID);
        //zum schluss noch den neuen artist in die id3tag der songs schreiben
        foreach (QString pfad, listePfad)
        {
            qDebug() << pfad;
            QByteArray fileName = QFile::encodeName( pfad );
            const char *encodedName = fileName.constData();

            TagLib::MPEG::File f(fileName);
            if(f.isValid())
            {
                qDebug() << pfad;
                TagLib::ID3v2::Tag* id3v2 = f.ID3v2Tag();
                id3v2->setArtist(database->Qt4StringToString(lineEditArtist->text()));
                f.save();
            }
        }


    }

    if(oldAlbum != lineEditAlbum->text())
    {
       //id für das neue Album besorgen
       int newAlbumID = database->key(lineEditAlbum->text(), "t_album");
       //in der tabelle t_title für jeden titel des albums die spalte album ändern
       foreach(int titleID, idListTitle)
       {
           database->updateAlbumFromTitle(titleID, newAlbumID);
       }
       database->deleteAlbum(oldAlbumID);

       //zum schluss noch den neuen artist in die id3tag der songs schreiben
       foreach (QString pfad, listePfad)
       {
           QByteArray fileName = QFile::encodeName( pfad );
           //const char *encodedName = fileName.constData();

           TagLib::MPEG::File f(fileName);
           if(f.isValid())
           {
               TagLib::ID3v2::Tag* id3v2 = f.ID3v2Tag();
               id3v2->setAlbum(database->Qt4StringToString(lineEditAlbum->text()));
               f.save();
           }
       }
    }

    if(oldYear != spinBoxYear->value())
    {
        int newYearID = database->key(QString("%1").arg(spinBoxYear->value()), "t_year");
        foreach(int titleID, idListTitle)
        {
            database->updateYearFromTitle(titleID, newYearID);
        }

        foreach (QString pfad, listePfad)
        {
            QByteArray fileName = QFile::encodeName( pfad );
            //const char *encodedName = fileName.constData();

            TagLib::MPEG::File f(fileName);
            if(f.isValid())
            {
                TagLib::ID3v2::Tag* id3v2 = f.ID3v2Tag();
                id3v2->setYear(spinBoxYear->value());
                f.save();
            }
        }
    }

    if(checkBoxSampler->checkState() == Qt::Checked)
    {
        int newAlbumID = database->key(lineEditAlbum->text(), "t_album");
        database->setSampler(newAlbumID);
    }

    done();
    //this->close();
}

void DialogInfo::anwendenArtist()
{
    qDebug() << "anwenden artist";
    QList<int> idListeTitle;
    QStringList listePfad;
    int oldArtistID = database->key(oldArtist, "t_artist");
    int newArtistID = database->key(lineEditArtist->text(), "t_artist");

    //ID-Liste der Alben von Artist holen
    QList<int> idListAlbum = database->getAlbumIDFromArtist(oldArtist);

    foreach (int albumID, idListAlbum)
    {
        //ID-Liste der Titel für jedes Album holen
        idListeTitle << database->getTitleIDFormAlbumID_Artist(albumID, oldArtist);
        //newArtistID in album schreiben
        database->updateArtistFromAlbum(albumID, newArtistID );
    }

    foreach (int titleID, idListeTitle)
    {
        database->updateArtistFromTitle(titleID, newArtistID);
    }

    //ID3Tags ändern
    listePfad = database->getUrlArtistID(newArtistID);
    foreach (QString pfad, listePfad)
    {
        QByteArray fileName = QFile::encodeName( pfad );

        TagLib::MPEG::File f(fileName);
        if(f.isValid())
        {
            //f.strip(TagLib::MPEG::File::ID3v2, false);
            TagLib::ID3v2::Tag* id3v2 = f.ID3v2Tag();
            id3v2->setArtist(database->Qt4StringToString(lineEditArtist->text()));
            f.save();
        }
    }

    database->deleteArtist(oldArtistID);
    done();
    this->close();
}

void DialogInfo::anwendenCover()
{
    int oldArtistID = database->key(oldArtist, "t_artist");
    int oldAlbumID = database->key(oldAlbum, "t_album");
    QStringList listePfad = database->getUrlAlbumID_ArtistID(oldAlbumID, oldArtistID);

    QString path = QDir::homePath()
                       + "/.config/qmdb/cover/"
                       + lineEditArtist->text()
                       + "-"
                       + lineEditAlbum->text();

    pixmapCover->save(path, "PNG");

    int albumID = database->getAlbumIDFromArtist(lineEditArtist->text(), lineEditAlbum->text());
    if (albumID)
    {
        QByteArray pixmapBytes;
        QBuffer pixmapBuffer(&pixmapBytes);
        pixmapBuffer.open(QIODevice::WriteOnly);
        pixmapCover->save(&pixmapBuffer, "PNG");
        const QString blobimage = pixmapBytes.toBase64()+"|end_stream|";
        database->updateImageFromAlbum(albumID, blobimage);
    }

    //Cover in id3tag schreiben
    foreach (QString pfad, listePfad)
    {
        QByteArray fileName = QFile::encodeName( pfad );
        //const char *encodedName = fileName.constData();

        TagLib::MPEG::File f(fileName);
        if(f.isValid())
        {
            TagLib::ID3v2::Tag* id3v2 = f.ID3v2Tag();
            TagLib::ID3v2::AttachedPictureFrame *frame = new TagLib::ID3v2::AttachedPictureFrame();
            QByteArray ba;
            QBuffer buffer( &ba );
            buffer.open( QIODevice::WriteOnly );
            pixmapCover->save( &buffer, "PNG", 100 );
            TagLib::ByteVector coverData(ba.constData(),ba.size());
            frame->setType(frame->FrontCover);
            frame->setMimeType("image/PNG");
            frame->setPicture(coverData);
            id3v2->addFrame(frame);
            buffer.close();
            f.save();
        }
    }

}

void DialogInfo::anwendenTrack()
{
  int titleID = database->getTitleIDFromTitle_Album_Artist(oldTitle,
                                                           oldAlbum,
                                                           oldArtist);
  int yearID = database->key(QString("%1").arg(spinBoxYear->value()), "t_year");
  QString pfad = database->getUrlTitleID(titleID);


  database->updateTitle_Track_Year_FromTitle(titleID,
                                             lineEditTitle->text(),
                                             spinBoxTrack->value(),
                                             yearID);

  QByteArray fileName = QFile::encodeName( pfad );

  TagLib::MPEG::File f(fileName);
  if(f.isValid())
  {
      //f.strip(TagLib::MPEG::File::ID3v2, false);
      TagLib::ID3v2::Tag* id3v2 = f.ID3v2Tag();
      id3v2->setTitle(database->Qt4StringToString(lineEditTitle->text()));
      id3v2->setTrack(spinBoxTrack->value());
      id3v2->setYear(spinBoxYear->value());
      f.save();
  }

  done();
  this->close();
}

void DialogInfo::cleanUp()
{
    lineEditTitle->setEnabled(false);
    lineEditTitle->setText("");
    lineEditAlbum->setEnabled(false);
    lineEditAlbum->setText("");
    lineEditGenre->setEnabled(false);
    lineEditGenre->setText("");
    spinBoxTrack->setEnabled(false);
    spinBoxTrack->setValue(0);
    spinBoxYear->setEnabled(false);
    spinBoxYear->setValue(0);
    coverWidget->setEnabled(false);
}

void DialogInfo::loadCover()
{
    int oldArtistID = database->key(oldArtist, "t_artist");
    int oldAlbumID = database->key(oldAlbum, "t_album");
    QStringList listePfad = database->getUrlAlbumID_ArtistID(oldAlbumID, oldArtistID);

    QFileInfo info (listePfad.first());
    qDebug() << info.absolutePath();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                     info.absolutePath(),
                                                     tr("Images (*.png *.xpm *.jpg)"));
    if(!fileName.isEmpty())
    {
        pixmapCover->load(fileName);
        pixmapLabel->setPixmap(pixmapCover->scaled(QSize(150,150)));
    }
}

void DialogInfo::setInfo(QString a)
{
    oldArtist = a;
    infoType = artist;

    cleanUp();
    lineEditArtist->setText(oldArtist);

    show();
}

void DialogInfo::setInfo(QString a, QString a1)
{
    oldArtist = a;
    oldAlbum = a1;
    oldYear = database->getYearFromAlbum(oldAlbum, oldArtist);
    infoType = album;
    QString coverPath;

    cleanUp();
    coverWidget->setEnabled(true);
    /*coverPath = database->getCoverPath(oldArtist, oldAlbum);
    if(coverPath != "noCover")
        pixmapCover->load(coverPath, "PNG");
    else
        pixmapCover->load(":/images/noCover.png", "PNG");*/
    *pixmapCover = database->getCover(oldArtist, oldAlbum);

    pixmapLabel->setPixmap(pixmapCover->scaled(QSize(150,150)));
    lineEditArtist->setText(oldArtist);
    lineEditAlbum->setEnabled(true);
    lineEditAlbum->setText(oldAlbum);
    spinBoxYear->setValue(oldYear);
    spinBoxYear->setEnabled(true);
    show();
}

void DialogInfo::setInfo(QString a, QString a1, QString t)
{
    oldArtist = a;
    oldAlbum = a1;
    oldTitle = t;
    infoType = title;

    cleanUp();
    lineEditArtist->setText(oldArtist);
    lineEditAlbum->setEnabled(true);
    lineEditAlbum->setText(oldAlbum);
    lineEditTitle->setEnabled(true);
    lineEditTitle->setText(oldTitle);
    metaPaket = database->metaPaket(oldTitle, oldArtist, oldAlbum);
    spinBoxTrack->setEnabled(true);
    spinBoxTrack->setValue(metaPaket.tracknr);
    spinBoxYear->setEnabled(true);
    spinBoxYear->setValue(metaPaket.year.toInt());
    oldTrack = metaPaket.tracknr;
    show();
}

void DialogInfo::setStandardItem(QStandardItem * i)
{
    item = i;
    model = i->model();
}
