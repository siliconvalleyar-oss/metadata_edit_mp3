#ifndef ID3TAG_H
#define ID3TAG_H

#include <QString>
#include <QByteArray>
#include <QMap>
#include <QVector>
#include <QFile>

struct ID3v1Tag {
    bool valid = false;
    QString title;
    QString artist;
    QString album;
    QString year;
    QString comment;
    int genre = 255;
};

struct APICFrame {
    QByteArray data;
    QString mimeType;
    QString description;
    int pictureType = 0;
};

class ID3Tag {
public:
    ID3Tag();

    bool load(const QString &filePath);
    bool save(const QString &filePath);

    QString artist() const { return m_artist; }
    QString album() const { return m_album; }
    QString title() const { return m_title; }
    QString year() const { return m_year; }
    QString genre() const { return m_genre; }
    QString track() const { return m_track; }
    QString comment() const { return m_comment; }
    QString composer() const { return m_composer; }
    QByteArray albumArt() const { return m_albumArt; }

    void setArtist(const QString &v) { m_artist = v; m_dirty = true; }
    void setAlbum(const QString &v) { m_album = v; m_dirty = true; }
    void setTitle(const QString &v) { m_title = v; m_dirty = true; }
    void setYear(const QString &v) { m_year = v; m_dirty = true; }
    void setGenre(const QString &v) { m_genre = v; m_dirty = true; }
    void setTrack(const QString &v) { m_track = v; m_dirty = true; }
    void setComment(const QString &v) { m_comment = v; m_dirty = true; }
    void setComposer(const QString &v) { m_composer = v; m_dirty = true; }
    void setAlbumArt(const QByteArray &v) { m_albumArt = v; m_dirty = true; }
    void removeAlbumArt() { m_albumArt.clear(); m_removeArt = true; m_dirty = true; }
    bool hasAlbumArt() const { return !m_albumArt.isEmpty(); }
    bool removeArtFlag() const { return m_removeArt; }
    bool isDirty() const { return m_dirty; }

private:
    struct ID3v2Header {
        char id[3];
        quint8 versionMajor;
        quint8 versionMinor;
        quint8 flags;
        quint32 size;
    };

    struct FrameHeader {
        char id[4];
        quint32 size;
        quint16 flags;
    };

    bool readID3v1(QFile &file);
    bool readID3v2(QFile &file, qint64 fileSize);
    void parseID3v2Frames(const QByteArray &data, quint32 size, bool isSyncSafe, quint8 flags);
    QString decodeText(const QByteArray &data, int &offset, quint8 encoding, int length = -1);
    QByteArray encodeText(const QString &text, quint8 encoding);

    QByteArray buildID3v2Tag();
    QByteArray buildFrame(const char *frameId, const QString &text, quint8 encoding = 3);
    QByteArray buildAPICFrame();

    static quint32 syncSafeToInt(const quint8 *data);
    static void intToSyncSafe(quint32 value, quint8 *data);
    static quint32 frameSize(const quint8 *data, bool isSyncSafe);

    QString m_filePath;
    QString m_artist;
    QString m_album;
    QString m_title;
    QString m_year;
    QString m_genre;
    QString m_track;
    QString m_comment;
    QString m_composer;
    QByteArray m_albumArt;
    bool m_dirty = false;
    bool m_removeArt = false;
    bool m_hasID3v1 = false;
    bool m_hasID3v2 = false;
    QByteArray m_rawID3v2;
};

#endif
