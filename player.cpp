#include "player.h"

Player::Player(QObject* parent) : QObject(parent)
{
    m_media = new MediaObject(this);
    m_media->setTickInterval(1000);
    output = new AudioOutput(Phonon::MusicCategory, this);
    createPath(m_media, output);

    connect(m_media, SIGNAL(finished()), this, SIGNAL(nextSong()));
    connect(m_media, SIGNAL(totalTimeChanged(qint64)), this, SIGNAL(setTotalTime(qint64)));
    connect(m_media, SIGNAL(tick(qint64)), this, SLOT(slotTick()));
}

void Player::play(QString pfad)
{
    MediaSource source(pfad);
    m_media->setCurrentSource(source);
    m_media->play();
}

void Player::playpause()
{
    if(m_media->state() == Phonon::PlayingState)
    {
        m_media->pause();
        return;
    }

    if(m_media->state() == Phonon::PausedState)
    {
        m_media->play();
        return;
    }
}

void Player::seek(int i)
{
    m_media->seek(i*1000);
}

void Player::slotTick()
{
    sendCurrentTime(m_media->currentTime());
}
