#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <phonon/phononnamespace.h>
#include <phonon/mediaobject.h>
#include <phonon/audiooutput.h>
#include <phonon/backendcapabilities.h>

using namespace Phonon;

class Player : public QObject
{
 Q_OBJECT

private:
    MediaObject* m_media;
    AudioOutput* output;

 public:
    Player(QObject* parent = 0);

    void play(QString pfad);

public slots:
    void playpause();
    void seek(int);
    void slotTick();

signals:
    void nextSong(); 
    void setTotalTime(qint64);
    void sendCurrentTime(qint64);

};
#endif // PLAYER_H
