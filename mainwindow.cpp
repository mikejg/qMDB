#include "mainwindow.h"
#include <QApplication>
#include <QHeaderView>
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QDir>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
  QDesktopWidget* d = QApplication::desktop ();
  setGeometry(d->availableGeometry(d->screenNumber(this)));
  database = new DataBase();
  model = 0;
  player = new Player(this);

  createMenu();
  createLayout();
  createPlaylistFolder();

  d_DBEinrichten = new DialogDBEinrichten(progressBar, database, this);
  d_Info = new DialogInfo(database, this);

  jobDBUpdate = new JobDBUpdate(progressBar, database, this);

  exportProcess = new QProcess(this);

  readArtists();

  connect(treeViewCollection, SIGNAL(clicked(QModelIndex)),
          this, SLOT(treeViewCollectionClicked(QModelIndex)));
  connect(tableViewSongs, SIGNAL(clicked(QModelIndex)),
          this, SLOT(tableViewSongsClicked(QModelIndex)));
  connect(d_DBEinrichten->jobInsertMetaPaket, SIGNAL(done()),
          this, SLOT(readArtists()));
  connect(d_DBEinrichten->jobDeleteMetaPaket, SIGNAL(done()),
          this, SLOT(readArtists()));
  connect(actionDBUpdate, SIGNAL(triggered()), jobDBUpdate, SLOT(startJob()));
  connect(actionDBEinrichten, SIGNAL(triggered()), d_DBEinrichten, SLOT(show()));
  connect(jobDBUpdate, SIGNAL(done()), this, SLOT(readArtists()));
  connect(d_Info, SIGNAL(done()), this, SLOT(readArtists()));
  connect(player, SIGNAL(nextSong()), this, SLOT(nextSong()));
  connect(player, SIGNAL(setTotalTime(qint64)), this, SLOT(setTotalTime(qint64)));
  connect(player, SIGNAL(sendCurrentTime(qint64)), this, SLOT(setSliderValue(qint64)));
  connect(sliderTime, SIGNAL(sliderMoved(int)), player, SLOT(seek(int)));
  connect(playlistWidget, SIGNAL(ladePlaylist(QString)), this, SLOT(ladePlaylist(QString)));
  connect(btnNext, SIGNAL(clicked()), this, SLOT(nextSong()));
  connect(btnPrev, SIGNAL(clicked()), this, SLOT(prevSong()));
  connect(btnPlayPause, SIGNAL(clicked()), player, SLOT(playpause()));
  connect(exportProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(exportfinished(int,QProcess::ExitStatus)));

  QTimer::singleShot(200, this, SLOT(setTableWidth()));
}

MainWindow::~MainWindow()
{
 delete database;
 delete d_DBEinrichten;
}

void MainWindow::createLayout()
{
    QVBoxLayout* layoutMaster = new QVBoxLayout(this);
    QVBoxLayout* layoutMain = new QVBoxLayout();

    QSplitter* splitter = new QSplitter(this);
    tabWidget = new QTabWidget(splitter);
    splitter->addWidget(tabWidget);
    splitter->setStretchFactor(splitter->indexOf(tabWidget),1);
    treeViewCollection = new TreeView(tabWidget);
    treeViewCollection->setAcceptDrops(false);
    treeViewCollection->setDragEnabled(true);

    treeViewCollection->setDropIndicatorShown(true);
    treeViewCollection->setHeaderHidden ( true );
    //tabWidget->setIconSize(QSize(30,30));
    playlistWidget = new PlaylistWidget(database, this);
    QStringList playlisten = database->getPlaylist();
    foreach(QString playlist, playlisten)
    {
        playlistWidget->insertItem(playlist);
    }

    tabWidget->addTab(treeViewCollection,"Sammlung");
    tabWidget->addTab(playlistWidget, "Playlisten");

    QFrame* frameRight = new QFrame(splitter);
    splitter->addWidget(frameRight);
    splitter->setStretchFactor(splitter->indexOf(frameRight),2);
    QVBoxLayout* layoutFrame = new QVBoxLayout(frameRight);
    QHBoxLayout* layoutFrameTop = new QHBoxLayout();
    btnPrev = new QPushButton(frameRight);
    btnPrev->setFixedSize(QSize(30,30));
    btnPrev->setFlat(true);
    btnPrev->setStyleSheet("border-style: none; "
                           "background-image: url(:/images/zurueck.png);");
    btnNext = new QPushButton(frameRight);
    btnNext->setFixedSize(QSize(30,30));
    btnNext->setFlat(true);
    btnNext->setStyleSheet("border-style: none; "
                           "background-image: url(:/images/vor.png);");
    btnPlayPause = new QPushButton(frameRight);
    btnPlayPause->setFixedSize(QSize(30,30));
    btnPlayPause->setFlat(true);
    btnPlayPause->setStyleSheet("border-style: none; "
                                "background-image: url(:/images/play-klein.png);");
    sliderTime = new QSlider(frameRight);
    sliderTime->setOrientation(Qt::Horizontal);
    sliderTime->setStyleSheet("QSlider::groove:horizontal {"
                              "border: 1px solid #999999; "
                              "height: 8px; "
                              "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #B1B1B1, stop:1 #c4c4c4); "
                              "margin: 2px 0; } "
                              "QSlider::handle:horizontal {"
                              "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f); "
                              "border: 1px solid #5c5c5c; "
                              "width: 8px; "
                              "margin: -2px 0; "
                              "border-radius: 3px; }"
                              );
    sliderTime->setMinimum(0);
    sliderTime->setMaximum(100);
    sliderTime->setValue(50);

    tableViewSongs = new TableView(frameRight);
    playlistModel = new CollectionModel(0,3,this);
    playlistModel->setDataBase(database);
    tableViewSongs->setModel(playlistModel);

    progressBar = new QProgressBar(frameRight);
    tableViewSongs->setAcceptDrops(true);
    tableViewSongs->setDragEnabled(true);
    tableViewSongs->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableViewSongs->verticalHeader()->hide();
    tableViewSongs->horizontalHeader()->setStretchLastSection(true);
    tableViewSongs->setShowGrid(false);
    //tableViewSongs->horizontalHeader()->setResizeMode(0,QHeaderView::Stretch);
    tableViewSongs->setSelectionMode(QAbstractItemView::SingleSelection);
    tableViewSongs->setAlternatingRowColors ( true );

    //tableViewSongs->setDropIndicatorShown(true);

    layoutFrameTop->addWidget(btnPrev);
    layoutFrameTop->addWidget(btnPlayPause);
    layoutFrameTop->addWidget(btnNext);
    layoutFrameTop->addWidget(sliderTime);

    layoutFrame->addLayout(layoutFrameTop);
    layoutFrame->addWidget(tableViewSongs);
    layoutFrame->addWidget(progressBar);

    splitter->addWidget(tabWidget);
    splitter->addWidget(frameRight);


    layoutMain->addWidget(splitter);
    layoutMaster->addWidget(menuBar);
    layoutMaster->addLayout(layoutMain,10);

    //tableViewSongs->setColumnWidth(0, tableViewSongs->width()/2);
}

void MainWindow::createMenu()
{
    menuBar = new QMenuBar(this);

    menuQMDB = menuBar->addMenu("QMDB");

    actionDBUpdate = new QAction("DB Update", this);
    actionDBEinrichten = new QAction("DB Einrichten", this);

    menuPlaylist = menuBar->addMenu("Playlist");
    menuPlaylist->addAction("Playlist speichern", this, SLOT(playlist_speichern()));
    menuPlaylist->addAction("Playlist leeren", this, SLOT(playlist_leeren()));
    menuPlaylist->addAction("Vor 4 Wochen", this, SLOT(playlist_vor_4_wochen()));
    menuPlaylist->addAction("Export", this, SLOT(exportPlaylist()));
    random = new QAction("Zufall", this);
    random->setCheckable(true);
    connect(random, SIGNAL(changed()), this, SLOT(randomChanged()));
    menuPlaylist->addAction(random);

    menuQMDB->addAction(actionDBEinrichten);
    menuQMDB->addAction(actionDBUpdate);
    menuQMDB->addSeparator();
    menuQMDB->addAction("Beenden", this, SLOT(close()));
}

void MainWindow::createPlaylistFolder()
{
    QDir dir (QDir::homePath());

    QFileInfo info (QDir::homePath() + "/.config");
    if(!info.exists())
        dir.mkdir(".config");
    dir.cd(".config");
    info.setFile(dir, "qmdb");
    if(!info.exists())
        dir.mkdir("qmdb");
    dir.cd("qmdb");
    info.setFile(dir, "Playlists");
    if(!info.exists())
        dir.mkdir("Playlists");
}

void MainWindow::exportfinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    exportPlaylist();
}

void MainWindow::exportPlaylist()
{
    static int row = 0;
    QString stringSong;
    QString stringInterpret;
    QString stringAlbum;
    QString stringUrl;
    QString outputString;

    QStandardItem* item;
    QDir exportDir(QString("/home/drue/Export"));
    QStringList argumentList;
    argumentList.append(QString("--mp3input"));
    argumentList.append(QString("-b 128"));

    if(playlistModel->hasIndex(row,0))
    {
        item = playlistModel->item(row,0);
        stringSong = item->text();

        item = playlistModel->item(row,1);
        stringInterpret = item->text();

        item = playlistModel->item(row,2);
        stringAlbum = item->text();
        qDebug() << stringSong << stringInterpret << stringAlbum;
        exportDir.mkdir(stringInterpret);
        exportDir.cd(stringInterpret);
        exportDir.mkdir(stringAlbum);

        stringUrl = database->getUrl(stringInterpret, stringAlbum, stringSong);
        argumentList.append(stringUrl);
        outputString = QString("/home/drue/Export/") + stringInterpret + QString("/") + stringAlbum + QString("/")
                + stringSong + QString(".mp3");

        argumentList.append(outputString);

        exportProcess->start(QString("lame"), argumentList);
        row++;
        return;
    }

    row = 0;
}

void MainWindow::ladePlaylist(QString playlist)
{
 qDebug("LadePlaylist");
  QStringList urls = database->getUrlFromPlaylist(playlist);
  QStringList list;
  foreach(QString line, urls)
  {
       list.clear();
       list = database->getSongInterpretAlbum(line);

       QList<QStandardItem*> items;
       QStandardItem* item;
       items.clear();
       item = new QStandardItem(list.at(0));
       item->setWhatsThis("song");
       items << item;
       item = new QStandardItem(list.at(1));
       item->setWhatsThis("interpret");
       items << item;
       item = new QStandardItem(list.at(2));
       item->setWhatsThis("album");
       items << item;
       playlistModel->appendRow(items);
    }
  }

void MainWindow::nextSong()
{
    QStandardItem *rootItem = playlistModel->invisibleRootItem();
    int rows = playlistModel->rowCount(rootItem->index());
    int row = tableViewSongs->currentIndex().row();
    static QList<int> listPlayed;
    QList<int> listToPlay;
    if (random->isChecked())
    {
        listPlayed.append(row);
        for(int i = 0; i < rows; i++)
        {
            if(!listPlayed.contains(i))
            {
                listToPlay.append(i);
                //qDebug() << QString("%1").arg(i);
            }
        }

        if(listToPlay.isEmpty())
        {
            listPlayed.clear();
            for(int i = 0; i < rows; i++)
            {
                if(!listPlayed.contains(i))
                    listToPlay.append(i);
            }
        }
        row = listToPlay.at((rand()%listToPlay.count()));
        if(playlistModel->hasIndex(row,0))
        {
            tableViewSongs->selectRow(row);
            tableViewSongsClicked(playlistModel->index(row,0));
        }
        btnNext->clearFocus();
        return;
    }

    row = tableViewSongs->currentIndex().row() + 1;
    if(playlistModel->hasIndex(row,0))
    {
        tableViewSongs->selectRow(row);
        tableViewSongsClicked(playlistModel->index(row,0));
    }
    btnNext->clearFocus();
}

void MainWindow::playlist_leeren()
{
    playlistModel->clear();
    playlistModel->setColumnCount(3);
    setTableWidth();
}

void MainWindow::playlist_speichern()
{
    QString song;
    QString album;
    QString interpret;
    QString file;
    QString dataString;

    QDateTime time(QDateTime::currentDateTime());

    //QDir dir(QDir::homePath());

    //QFile playlistFile(QDir::homePath() + "/.config/qmdb/Playlists/" + time.toString() + ".m3u");
    //playlistFile.open(QIODevice::WriteOnly);

    QStringList urls;
    QStandardItem *rootItem = playlistModel->invisibleRootItem();
    int rows = playlistModel->rowCount(rootItem->index());
    for(int i = 0; i < rows; i++)
    {
        song = playlistModel->item(i,0)->text();
        interpret = playlistModel->item(i,1)->text();
        album = playlistModel->item(i,2)->text();
        file = database->getUrl(interpret,album,song);
        urls.append(file);
        //dataString = "#extinf " + interpret + " " + song + "\n";
        //playlistFile.write(dataString.toAscii().data());
        //playlistFile.write(file.toAscii().data());
    }

    //playlistFile.close();

    database->insertPlaylist(time.toString(), urls);
    playlistWidget->insertItem(time.toString());
}

void MainWindow::playlist_vor_4_wochen()
{
    QStringList list;
    QStringList urls = database->getUrlFromErfasst();

    foreach(QString line, urls)
    {
         list.clear();
         list = database->getSongInterpretAlbum(line);
   qDebug() << line;
         QList<QStandardItem*> items;
         QStandardItem* item;
         items.clear();
         item = new QStandardItem(list.at(0));
         item->setWhatsThis("song");
         items << item;
         item = new QStandardItem(list.at(1));
         item->setWhatsThis("interpret");
         items << item;
         item = new QStandardItem(list.at(2));
         item->setWhatsThis("album");
         items << item;
         playlistModel->appendRow(items);
      }
}

void MainWindow::prevSong()
{
    int row = tableViewSongs->currentIndex().row() - 1;
    if(playlistModel->hasIndex(row,0))
    {
        tableViewSongs->selectRow(row);
        tableViewSongsClicked(playlistModel->index(row,0));
    }
    btnPrev->clearFocus();
}

void MainWindow::randomChanged()
{
    qDebug("Random");
}

void MainWindow::readArtists()
{
    QStringList listArtists = database->getInterpreten();
    QStringList listSampler = database->getSampler();

    if(model) delete model;

    model = new CollectionModel(this);

    QStandardItem *parentItem = model->invisibleRootItem();
    if(!listSampler.isEmpty())
    {
        QStandardItem *item = new QStandardItem("Sampler");
        item->setWhatsThis(QString("Interpret"));
        parentItem->appendRow(item);

        foreach(QString sampler, listSampler)
        {
            QStandardItem *samplerItem = new QStandardItem(sampler);
            samplerItem->setWhatsThis(QString("Album"));
            item->appendRow(samplerItem);
        }
    }

    for (int i = 0; i < listArtists.size(); ++i) {
         QStandardItem *item = new QStandardItem(listArtists.at(i));
         item->setWhatsThis(QString("Interpret"));
         parentItem->appendRow(item);
           }
    treeViewCollection->setModel(model);
}

void MainWindow::setSliderValue(qint64 t)
{
    sliderTime->setValue((int)(t/1000));
}

void MainWindow::setTableWidth()
{
    tableViewSongs->setColumnWidth(0, tableViewSongs->width()/2);
    tableViewSongs->setColumnWidth(1, tableViewSongs->width()/4);
    QStringList labels;
    labels << "Title" << "Artis" << "Album";
    playlistModel->setHorizontalHeaderLabels(labels);
    tableViewSongs->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
}

void MainWindow::setTotalTime(qint64 totalTime)
{
    sliderTime->setMinimum(0);
    sliderTime->setMaximum((int)(totalTime/1000));
}

void MainWindow::showInfoFromItem(QStandardItem* item)
{
    if (item->whatsThis() == QString("Interpret"))
    {
        d_Info->setInfo(item->text());
    }

    if (item->whatsThis() == QString("Album"))
    {
        QString artist = item->parent()->text();
        d_Info->setInfo(artist, item->text());
    }

    if (item->whatsThis() == QString("Track"))
        {
            QString album = item->parent()->text();
            QString artist = item->parent()->parent()->text();
            d_Info->setInfo(artist,album, item->text());
        }
}

void MainWindow::treeViewCollectionClicked(QModelIndex index)
{
    //if(treeViewCollection->button == Qt::LeftButton) qDebug("linke Maus");

     QStandardItem* parentItem = model->itemFromIndex(index);

     if(treeViewCollection->button == Qt::RightButton)
     {
        showInfoFromItem(parentItem);
        d_Info->setStandardItem(parentItem);
        return;
     }

     QStringList listResults;

     if (parentItem->whatsThis() == QString("Interpret"))
        {
         if(!parentItem->hasChildren())
           {
             listResults = database->getAlbumsFromInterpret(parentItem->text());
             for (int i = 0; i < listResults.size(); ++i)
                 {
                  QStandardItem *item = new QStandardItem(listResults.at(i));
                  item->setWhatsThis(QString("Album"));
                  parentItem->appendRow(item);
                 }
           }
          (treeViewCollection->isExpanded(index)) ? treeViewCollection->setExpanded(index, false) :
                                                   treeViewCollection->setExpanded(index, true);
          return;
        }

     if (parentItem->whatsThis() == QString("Album"))
        {
         if(!parentItem->hasChildren())
           {
             QString stringInterpret = parentItem->parent()->text();
             listResults = database->getTracksFromAlbum(stringInterpret, parentItem->text());
             for (int i = 0; i < listResults.size(); ++i)
                 {
                  QStandardItem *item = new QStandardItem(listResults.at(i));
                  item->setWhatsThis(QString("Track"));
                  parentItem->appendRow(item);
                 }
           }
          (treeViewCollection->isExpanded(index)) ? treeViewCollection->setExpanded(index, false) :
                                                   treeViewCollection->setExpanded(index, true);
          return;
        }
}

void MainWindow::tableViewSongsClicked(QModelIndex index)
{
    QString song;
    QString album;
    QString interpret;
    QString file;

    QModelIndexList list = tableViewSongs->selectionModel()->selectedIndexes();

    if(tableViewSongs->button == Qt::RightButton)
    {
      if(!list.isEmpty())
        playlistModel->takeRow(list.first().row());
      return;
    }

    foreach(QModelIndex item, list)
    {
        QStandardItem* parentItem = playlistModel->itemFromIndex(item);
        if (parentItem->whatsThis() == "song")
            song = parentItem->text();
        if (parentItem->whatsThis() == "album")
            album = parentItem->text();
        if (parentItem->whatsThis() == "interpret")
            interpret = parentItem->text();
    }

    file = database->getUrl(interpret,album,song);
    player->play(file);
    qDebug() << song << " " << interpret << " " << album;
    qDebug() << file;
}
