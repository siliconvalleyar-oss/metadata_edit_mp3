#ifndef MP3PLAYER_H
#define MP3PLAYER_H

#include <QObject>
#include <QMediaPlayer>
#include <QVector>

class MP3File;

class Mp3Player : public QObject {
    Q_OBJECT
public:
    explicit Mp3Player(QObject *parent = nullptr);

    void setFiles(const QVector<MP3File*> &files);
    void playFile(MP3File *file);
    void play();
    void pause();
    void stop();
    void next();
    void previous();
    void setPosition(qint64 ms);

    QMediaPlayer::State state() const { return m_player->state(); }
    qint64 position() const { return m_player->position(); }
    qint64 duration() const { return m_player->duration(); }
    MP3File *currentFile() const { return m_currentFile; }

signals:
    void stateChanged(QMediaPlayer::State state);
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void currentFileChanged(MP3File *file);

private:
    QMediaPlayer *m_player;
    QVector<MP3File*> m_files;
    MP3File *m_currentFile = nullptr;
    int m_currentIndex = -1;
};

#endif
