#include "playlistWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDir>
#include <QDebug>

PlaylistWidget::PlaylistWidget(DataBase* db,QWidget* parent, Qt::WindowFlags f) : QWidget(parent, f)
{
  database = db;
  QVBoxLayout* layoutMaster = new QVBoxLayout(this);
  QHBoxLayout* layoutButton = new QHBoxLayout();

  playlistTable = new QListWidget(this);
  buttonLaden = new QPushButton("Laden", this);
  buttonLoeschen = new QPushButton("Loeschen", this);
  layoutButton->addWidget(buttonLaden);
  layoutButton->addWidget(buttonLoeschen);

  layoutMaster->addWidget(playlistTable);
  layoutMaster->addLayout(layoutButton);

 /* QDir dir(QDir::homePath() + "/.config/qmdb/Playlists");
  QFileInfoList infoList = dir.entryInfoList();
  QFileInfo fileInfo;
  QListWidgetItem* item;
  foreach(fileInfo, infoList)
  {
      if(fileInfo.isFile())
      {
          item = new QListWidgetItem(fileInfo.fileName(), playlistTable);
          item->setFlags(item->flags() | Qt::ItemIsEditable);
      }
  }*/

  connect(playlistTable, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemChanged(QListWidgetItem*)));
  connect(playlistTable, SIGNAL(currentTextChanged(QString)), this, SLOT(currentTextChanged(QString)));
  connect(buttonLaden, SIGNAL(clicked()), this, SLOT(ladenClicked()));
  connect(buttonLoeschen, SIGNAL(clicked()), this, SLOT(loeschenClicked()));
}

void PlaylistWidget::currentTextChanged(QString ct)
{
    currentText = ct;
    qDebug("-- start currentTextChanged --");
    qDebug() << "current Text " << currentText;
}


void PlaylistWidget::insertItem(QString text)
{
    currentText = QString("");
    QListWidgetItem* item = new QListWidgetItem(text, playlistTable);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
}

void PlaylistWidget::itemChanged(QListWidgetItem * item)
{
    if(currentText == QString(""))
        return;

    QString newText = item->text();
    database->renamePlaylist(currentText, newText);

    playlistTable->clear();
    QStringList playlisten = database->getPlaylist();
    foreach(QString playlist, playlisten)
    {
        insertItem(playlist);
    }
}

void PlaylistWidget::ladenClicked()
{
    ladePlaylist(playlistTable->currentItem()->text());
}

void PlaylistWidget::loeschenClicked()
{
    database->deletePlaylist(playlistTable->currentItem()->text());

    playlistTable->clear();
    QStringList playlisten = database->getPlaylist();
    foreach(QString playlist, playlisten)
    {
        insertItem(playlist);
    }
}
