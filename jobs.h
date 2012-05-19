#ifndef JOBS_H
#define JOBS_H

#include <QProgressBar>
#include <QStringList>
#include <QDebug>
#include <QDateTime>
#include <id3v2tag.h>
#include "database.h"

class JobInsertMetaPaket : public QObject
{
    Q_OBJECT

    private:
    QProgressBar* progressBar;
    QStringList fileListe;
    int timerID;
    int erfasst;
    DataBase* database;
    QDateTime dateTime;

    public:
    JobInsertMetaPaket(QProgressBar*, DataBase*, QObject* parent=0, const char* name=0);
    ~JobInsertMetaPaket();

    QString readCover(TagLib::ID3v2::Tag* tag, QString, QString);
    void setFileListe(QStringList);
    void startJob();

    protected:
    void timerEvent (QTimerEvent*);

    signals:
    void done();

};

class JobDeleteMetaPaket : public QObject
{
    Q_OBJECT

    private:
    QProgressBar* progressBar;
    QStringList fileListe;
    int timerID;
    int erfasst;
    DataBase* database;
    QDateTime dateTime;
    JobInsertMetaPaket* jobInsertMetaPaket;

    public:
    JobDeleteMetaPaket(QProgressBar*, DataBase*, QObject* parent=0, const char* name=0);
    ~JobDeleteMetaPaket();

    void setFileListe(QStringList, QStringList);
    void startJob();

    protected:
    void timerEvent (QTimerEvent*);

    signals:
    void done();

};

class JobDBUpdate : public QObject
{
    Q_OBJECT

private:
    QProgressBar* progressBar;
    QStringList filesHD;
    QStringList filesDB;
    QStringList filesInput;
    QStringList filesOutput;

    int timerID;
    DataBase* database;
    QDateTime dateTime;
    int erfasst;

public:
    JobDBUpdate(QProgressBar*, DataBase*, QObject* parent=0, const char* name=0);
    ~JobDBUpdate();

    void findTracks(QDir, QStringList*);

protected:
    void timerEvent (QTimerEvent*);

public slots:
    void startJob();

signals:
    void done();
};

#endif // JOBS_H
