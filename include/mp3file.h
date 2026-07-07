#ifndef MP3FILE_H
#define MP3FILE_H

#include <QString>
#include <QByteArray>
#include <QDateTime>

class ID3Tag;

class MP3File {
public:
    MP3File();
    explicit MP3File(const QString &filePath);
    ~MP3File();

    bool load();
    bool save();

    QString filePath() const { return m_filePath; }
    QString fileName() const { return m_fileName; }
    qint64 fileSize() const { return m_fileSize; }
    QDateTime lastModified() const { return m_lastModified; }

    QString artist() const { return m_artist; }
    QString album() const { return m_album; }
    QString title() const { return m_title; }
    QString year() const { return m_year; }
    QString genre() const { return m_genre; }
    QString track() const { return m_track; }
    QString comment() const { return m_comment; }
    QString composer() const { return m_composer; }
    QByteArray albumArt() const { return m_albumArt; }
    bool hasAlbumArt() const { return !m_albumArt.isEmpty(); }

    void setArtist(const QString &v);
    void setAlbum(const QString &v);
    void setTitle(const QString &v);
    void setYear(const QString &v);
    void setGenre(const QString &v);
    void setTrack(const QString &v);
    void setComment(const QString &v);
    void setComposer(const QString &v);
    void setAlbumArt(const QByteArray &v);
    void removeAlbumArt();
    bool removeArtFlag() const { return m_removeArt; }

    bool isModified() const { return m_modified; }
    void setModified(bool v) { m_modified = v; }

    void discardChanges();

    QString newFileName() const { return m_newFileName; }
    void setNewFileName(const QString &v) {
        m_newFileName = v;
        m_renameFile = !v.isEmpty();
    }

private:
    QString m_filePath;
    QString m_fileName;
    qint64 m_fileSize = 0;
    QDateTime m_lastModified;

    QString m_artist;
    QString m_album;
    QString m_title;
    QString m_year;
    QString m_genre;
    QString m_track;
    QString m_comment;
    QString m_composer;
    QByteArray m_albumArt;

    bool m_modified = false;
    bool m_removeArt = false;
    bool m_renameFile = false;
    QString m_newFileName;

    // Backup of original values for discard
    bool m_hasBackup = false;
    QString m_origArtist;
    QString m_origAlbum;
    QString m_origTitle;
    QString m_origYear;
    QString m_origGenre;
    QString m_origTrack;
    QString m_origComment;
    QString m_origComposer;
    QByteArray m_origAlbumArt;
};

#endif
