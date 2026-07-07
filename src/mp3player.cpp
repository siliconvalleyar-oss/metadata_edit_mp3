#include "mp3player.h"
#include "mp3file.h"

Mp3Player::Mp3Player(QObject *parent)
    : QObject(parent)
    , m_player(new QMediaPlayer(this))
{
    connect(m_player, &QMediaPlayer::stateChanged, this, &Mp3Player::stateChanged);
    connect(m_player, &QMediaPlayer::positionChanged, this, &Mp3Player::positionChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &Mp3Player::durationChanged);
}

void Mp3Player::setFiles(const QVector<MP3File*> &files) {
    m_files = files;
}

void Mp3Player::playFile(MP3File *file) {
    if (!file) return;
    m_currentFile = file;
    m_currentIndex = m_files.indexOf(file);
    m_player->setMedia(QUrl::fromLocalFile(file->filePath()));
    m_player->play();
    emit currentFileChanged(file);
}

void Mp3Player::play() {
    if (m_player->state() == QMediaPlayer::PausedState) {
        m_player->play();
    } else if (m_currentFile) {
        playFile(m_currentFile);
    } else if (!m_files.isEmpty()) {
        playFile(m_files.first());
    }
}

void Mp3Player::pause() {
    m_player->pause();
}

void Mp3Player::stop() {
    m_player->stop();
}

void Mp3Player::next() {
    if (m_files.isEmpty()) return;
    int nextIdx = m_currentIndex + 1;
    if (nextIdx >= m_files.size())
        nextIdx = 0;
    playFile(m_files[nextIdx]);
}

void Mp3Player::previous() {
    if (m_files.isEmpty()) return;
    int prevIdx = m_currentIndex - 1;
    if (prevIdx < 0)
        prevIdx = m_files.size() - 1;
    playFile(m_files[prevIdx]);
}

void Mp3Player::setPosition(qint64 ms) {
    m_player->setPosition(ms);
}
