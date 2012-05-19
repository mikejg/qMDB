#ifndef DIALOGINFO_H
#define DIALOGINFO_H

#include <QDialog>
#include <QLineEdit>
#include <QTextBlock>
#include <QSpinBox>
#include <QPushButton>
#include <QStandardItem>
#include <QTabWidget>
#include <QPixmap>
#include <QLabel>
#include <QCheckBox>
#include "database.h"

class DialogInfo : public QDialog
{
    Q_OBJECT

enum type {artist, album, title};

private:
    QTabWidget* tabWidget;
    QLineEdit* lineEditTitle;
    QLineEdit* lineEditArtist;
    QLineEdit* lineEditAlbum;
    QLineEdit* lineEditGenre;
    QSpinBox* spinBoxTrack;
    QSpinBox* spinBoxYear;
    QCheckBox* checkBoxSampler;
    QPushButton* buttonAnwenden;
    QPushButton* buttonAbbrechen;

    QWidget* coverWidget;
    QPushButton* coverLaden;
    QPushButton* coverAnwenden;
    QPushButton* coverAbbrechen;
    QPixmap* pixmapCover;
    QLabel* pixmapLabel;

    type infoType;
    QString oldArtist;
    QString oldAlbum;
    QString oldTitle;
    int oldTrack;
    int oldYear;
    QStandardItem* item;
    QStandardItemModel* model;
    DataBase* database;
    MetaPaket metaPaket;

public:

    DialogInfo(DataBase*, QWidget *parent = 0, Qt::WindowFlags f = 0);
    void anwendenAlbum();
    void anwendenArtist();
    void anwendenTrack();
    void cleanUp();
    void setInfo(QString);
    void setInfo(QString, QString);
    void setInfo(QString, QString, QString);
    void setStandardItem(QStandardItem*);

public slots:
    void anwenden();
    void anwendenCover();
    void loadCover();

signals:
    void done();
};

#endif // DIALOGINFO_H
