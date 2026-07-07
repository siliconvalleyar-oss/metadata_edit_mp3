#include "mp3file.h"
#include "id3tag.h"
#include <QFileInfo>

MP3File::MP3File() {}

MP3File::MP3File(const QString &filePath)
    : m_filePath(filePath) {}

MP3File::~MP3File() {}

bool MP3File::load() {
    QFileInfo fi(m_filePath);
    if (!fi.exists() || !fi.isFile())
        return false;

    m_fileName = fi.fileName();
    m_fileSize = fi.size();
    m_lastModified = fi.lastModified();

    ID3Tag tag;
    if (!tag.load(m_filePath))
        return false;

    m_artist = tag.artist();
    m_album = tag.album();
    m_title = tag.title();
    m_year = tag.year();
    m_genre = tag.genre();
    m_track = tag.track();
    m_comment = tag.comment();
    m_composer = tag.composer();
    m_albumArt = tag.albumArt();

    m_modified = false;
    m_removeArt = false;
    m_renameFile = false;
    m_newFileName.clear();

    if (m_title.isEmpty())
        m_title = fi.completeBaseName();

    // Save original values for discard
    m_hasBackup = true;
    m_origArtist = m_artist;
    m_origAlbum = m_album;
    m_origTitle = m_title;
    m_origYear = m_year;
    m_origGenre = m_genre;
    m_origTrack = m_track;
    m_origComment = m_comment;
    m_origComposer = m_composer;
    m_origAlbumArt = m_albumArt;

    return true;
}

bool MP3File::save() {
    if (!m_modified)
        return true;

    ID3Tag tag;
    tag.setArtist(m_artist);
    tag.setAlbum(m_album);
    tag.setTitle(m_title);
    tag.setYear(m_year);
    tag.setGenre(m_genre);
    tag.setTrack(m_track);
    tag.setComment(m_comment);
    tag.setComposer(m_composer);

    if (m_removeArt)
        tag.removeAlbumArt();
    else if (!m_albumArt.isEmpty())
        tag.setAlbumArt(m_albumArt);

    if (!tag.save(m_filePath))
        return false;

    if (m_renameFile && !m_newFileName.isEmpty()) {
        QFileInfo fi(m_filePath);
        QString newPath = fi.absolutePath() + "/" + m_newFileName;
        if (!newPath.endsWith(".mp3", Qt::CaseInsensitive) &&
            !newPath.endsWith(".MP3", Qt::CaseInsensitive))
            newPath += ".mp3";
        if (newPath != m_filePath) {
            if (QFile::exists(newPath))
                QFile::remove(newPath);
            if (!QFile::rename(m_filePath, newPath))
                return false;
            m_filePath = newPath;
            m_fileName = QFileInfo(newPath).fileName();
        }
    }

    m_modified = false;
    m_renameFile = false;
    return true;
}

void MP3File::setArtist(const QString &v) { m_artist = v; m_modified = true; }
void MP3File::setAlbum(const QString &v) { m_album = v; m_modified = true; }
void MP3File::setTitle(const QString &v) { m_title = v; m_modified = true; }
void MP3File::setYear(const QString &v) { m_year = v; m_modified = true; }
void MP3File::setGenre(const QString &v) { m_genre = v; m_modified = true; }
void MP3File::setTrack(const QString &v) { m_track = v; m_modified = true; }
void MP3File::setComment(const QString &v) { m_comment = v; m_modified = true; }
void MP3File::setComposer(const QString &v) { m_composer = v; m_modified = true; }
void MP3File::setAlbumArt(const QByteArray &v) { m_albumArt = v; m_modified = true; m_removeArt = false; }
void MP3File::removeAlbumArt() { m_albumArt.clear(); m_removeArt = true; m_modified = true; }

void MP3File::discardChanges() {
    if (!m_hasBackup)
        return;
    m_artist = m_origArtist;
    m_album = m_origAlbum;
    m_title = m_origTitle;
    m_year = m_origYear;
    m_genre = m_origGenre;
    m_track = m_origTrack;
    m_comment = m_origComment;
    m_composer = m_origComposer;
    m_albumArt = m_origAlbumArt;
    m_modified = false;
    m_removeArt = false;
    m_renameFile = false;
    m_newFileName.clear();
}
