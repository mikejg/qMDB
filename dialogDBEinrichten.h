#ifndef DIALOGDBEINRICHTEN_H
#define DIALOGDBEINRICHTEN_H

#include <QDialog>
#include <QTreeView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDir>
#include <QDebug>
#include <QSettings>

#include "jobs.h"
#include "database.h"

class DialogDBEinrichten : public QDialog
{
    Q_OBJECT

public:
    JobInsertMetaPaket* jobInsertMetaPaket;
    JobDeleteMetaPaket* jobDeleteMetaPaket;

    QTreeView* treeView;
    QStandardItemModel* model;
    QDir* rootDir;
    QPushButton* buttonAbbrechen;
    QPushButton* buttonAnwenden;
    QPushButton* buttonNeuEinlesen;
    QVBoxLayout* layoutMaster;
    DataBase* database;

    DialogDBEinrichten(QProgressBar*, DataBase*, QWidget *parent = 0, Qt::WindowFlags f = 0);

    void disableAll();
    void findCheckedItems(QStringList*, QStandardItem*);
    void findTracks(QDir, QStringList*);
    void readSettings();
    QStandardItem* setItem(QStandardItem*, QString);

public slots:
    void anwenden();
    void itemChanged(QStandardItem*);
    void neuEinlesen();
    void treeViewClicked(QModelIndex);
    void writeSettings();
    void enableAll();
};

#endif // DIALOGDBEINRICHTEN_H
