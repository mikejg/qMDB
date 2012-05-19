#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
//#include <QMainWindow>
#include <QSplitter>
#include <QTabWidget>
#include <QTreeView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QListView>
#include <QPushButton>
#include <QProgressBar>
#include <QSlider>
#include <QStandardItemModel>
#include <QMenuBar>
#include <QProcess>

#include "CollectionModel.h"
#include "database.h"
#include "dialogDBEinrichten.h"
#include "dialogInfo.h"
#include "treeView.h"
#include "tableView.h"
#include "player.h"
#include "playlistWidget.h"

class MainWindow : public QWidget {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    //DataBase
    DataBase* database;
    //Model for Collection
    CollectionModel* model;
    //Model for Playlist
    CollectionModel* playlistModel;
    Player* player;

    //Layout Classes
    QTabWidget* tabWidget;
    TreeView* treeViewCollection;
    TableView* tableViewSongs;
    PlaylistWidget* playlistWidget;
    QPushButton* btnPrev;
    QPushButton* btnNext;
    QPushButton* btnPlayPause;
    QProgressBar* progressBar;
    QSlider* sliderTime;

    //Menu Classes
    QMenuBar* menuBar;
    QMenu* menuQMDB;
    QMenu* menuPlaylist;

    QAction* actionDBUpdate;
    QAction* actionDBEinrichten;
    QAction* random;

    //Dialog Classes
    DialogDBEinrichten* d_DBEinrichten;
    DialogInfo* d_Info;

    // Jobs
    JobDBUpdate* jobDBUpdate;
    QProcess* exportProcess;

    void createLayout();
    void createMenu();
    void createPlaylistFolder();
    void showInfoFromItem(QStandardItem*);

public slots:
    void exportPlaylist();
    void exportfinished ( int exitCode, QProcess::ExitStatus exitStatus );
    void ladePlaylist(QString);
    void nextSong();
    void playlist_leeren();
    void playlist_speichern();
    void playlist_vor_4_wochen();
    void prevSong();
    void randomChanged();
    void readArtists();
    void setSliderValue(qint64);
    void setTableWidth();
    void setTotalTime(qint64);
    void treeViewCollectionClicked(QModelIndex);
    void tableViewSongsClicked(QModelIndex);

};

#endif // MAINWINDOW_H
