#include "id3tag.h"
#include <QFile>
#include <QTextCodec>
#include <QImage>
#include <QBuffer>
#include <cstring>

static const int ID3V1_TAG_SIZE = 128;
static const int ID3V2_HEADER_SIZE = 10;
static const int FRAME_HEADER_SIZE = 10;

ID3Tag::ID3Tag() {}

bool ID3Tag::load(const QString &filePath) {
    m_filePath = filePath;
    m_artist.clear();
    m_album.clear();
    m_title.clear();
    m_year.clear();
    m_genre.clear();
    m_track.clear();
    m_comment.clear();
    m_composer.clear();
    m_albumArt.clear();
    m_dirty = false;
    m_removeArt = false;
    m_hasID3v1 = false;
    m_hasID3v2 = false;
    m_rawID3v2.clear();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    qint64 fileSize = file.size();
    if (fileSize < 4)
        return false;

    if (fileSize >= ID3V1_TAG_SIZE)
        readID3v1(file);

    file.seek(0);
    readID3v2(file, fileSize);

    file.close();
    return true;
}

bool ID3Tag::save(const QString &filePath) {
    if (!m_dirty)
        return true;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadWrite))
        return false;

    QByteArray existingContent = file.readAll();

    // Extract audio data: strip existing ID3v2 from beginning
    int dataStart = 0;
    if (existingContent.size() >= ID3V2_HEADER_SIZE &&
        memcmp(existingContent.constData(), "ID3", 3) == 0) {
        quint32 tagSz = syncSafeToInt((const quint8*)existingContent.constData() + 6);
        dataStart = ID3V2_HEADER_SIZE + tagSz;
        // Check for footer
        if ((quint8)existingContent[5] & 0x10)
            dataStart += 10;
    }

    QByteArray audioData = existingContent.mid(dataStart);

    // Strip ID3v1 tag from end if present
    if (audioData.size() >= ID3V1_TAG_SIZE) {
        if (memcmp(audioData.constData() + audioData.size() - 3, "TAG", 3) == 0)
            audioData.chop(ID3V1_TAG_SIZE);
    }

    // Rebuild file: [new ID3v2] + [audio] + [new ID3v1]
    file.resize(0);
    file.seek(0);

    // Write ID3v2 tag
    QByteArray v2tag = buildID3v2Tag();
    if (!v2tag.isEmpty())
        file.write(v2tag);

    // Write audio data
    file.write(audioData);

    // Write ID3v1 tag
    QByteArray v1tag;
    v1tag.append("TAG", 3);
    v1tag.append(m_title.leftJustified(30, '\0').left(30).toLatin1());
    v1tag.append(m_artist.leftJustified(30, '\0').left(30).toLatin1());
    v1tag.append(m_album.leftJustified(30, '\0').left(30).toLatin1());
    v1tag.append(m_year.leftJustified(4, '\0').left(4).toLatin1());

    // ID3v1.1: 28-byte comment + 1 zero byte + 1 track byte
    QByteArray commentBytes = m_comment.leftJustified(28, '\0').left(28).toLatin1();
    v1tag.append(commentBytes);
    v1tag.append('\0');

    bool ok;
    int trackNum = m_track.toInt(&ok);
    v1tag.append((char)(ok && trackNum > 0 && trackNum < 256 ? trackNum : 0));

    bool genreOk;
    int genreNum = m_genre.toInt(&genreOk);
    v1tag.append((char)(genreOk ? genreNum : 255));

    file.write(v1tag);
    file.close();
    return true;
}

bool ID3Tag::readID3v1(QFile &file) {
    file.seek(file.size() - ID3V1_TAG_SIZE);
    QByteArray tag = file.read(ID3V1_TAG_SIZE);
    if (tag.size() < ID3V1_TAG_SIZE)
        return false;
    if (memcmp(tag.constData(), "TAG", 3) != 0)
        return false;

    m_hasID3v1 = true;

    auto readStr = [&](int offset, int len) -> QString {
        QByteArray raw = tag.mid(offset, len);
        int nullPos = raw.indexOf('\0');
        if (nullPos >= 0)
            raw.truncate(nullPos);
        return QString::fromLatin1(raw).trimmed();
    };

    m_title = readStr(3, 30);
    m_artist = readStr(33, 30);
    m_album = readStr(63, 30);
    m_year = readStr(93, 4).trimmed();

    if ((quint8)tag[125] == 0 && (quint8)tag[126] != 0) {
        m_track = QString::number((quint8)tag[126]);
        m_comment = readStr(97, 28);
    } else {
        m_comment = readStr(97, 30);
    }

    m_genre = QString::number((quint8)tag[127]);
    return true;
}

bool ID3Tag::readID3v2(QFile &file, qint64 fileSize) {
    QByteArray header = file.read(ID3V2_HEADER_SIZE);
    if (header.size() < ID3V2_HEADER_SIZE)
        return false;
    if (memcmp(header.constData(), "ID3", 3) != 0)
        return false;

    ID3v2Header hdr;
    memcpy(&hdr, header.constData(), sizeof(hdr));
    m_hasID3v2 = true;

    bool isSyncSafe = (hdr.versionMajor >= 4);
    quint32 tagSize = syncSafeToInt((const quint8*)header.constData() + 6);

    if (tagSize > (quint32)(fileSize - ID3V2_HEADER_SIZE))
        tagSize = fileSize - ID3V2_HEADER_SIZE;

    m_rawID3v2 = file.read(tagSize);
    parseID3v2Frames(m_rawID3v2, tagSize, isSyncSafe, hdr.flags);
    return true;
}

void ID3Tag::parseID3v2Frames(const QByteArray &data, quint32 size, bool isSyncSafe, quint8 flags) {
    quint32 pos = 0;

    if (flags & 0x40) {
        if (size >= 4) {
            quint32 extSize;
            if (isSyncSafe)
                extSize = syncSafeToInt((const quint8*)data.constData());
            else
                extSize = ((quint32)(quint8)data[0] << 24) | ((quint32)(quint8)data[1] << 16) |
                          ((quint32)(quint8)data[2] << 8) | (quint32)(quint8)data[3];
            pos = extSize + 4;
            if (pos > size) pos = size;
        }
    }

    while (pos + FRAME_HEADER_SIZE <= size && (quint32)data.size() >= pos + FRAME_HEADER_SIZE) {
        FrameHeader fhdr;
        memcpy(&fhdr, data.constData() + pos, FRAME_HEADER_SIZE);

        if (fhdr.id[0] == 0)
            break;

        quint32 frameSz = frameSize((const quint8*)&fhdr.size, isSyncSafe);

        pos += FRAME_HEADER_SIZE;
        if (pos + frameSz > size || frameSz == 0)
            break;

        QByteArray frameData = data.mid(pos, frameSz);

        if (memcmp(fhdr.id, "APIC", 4) == 0) {
            if (frameData.size() > 1) {
                quint8 enc = (quint8)frameData[0];
                int offset = 1;

                int mimeEnd = frameData.indexOf('\0', offset);
                if (mimeEnd < 0) break;
                QString mimeType = QString::fromLatin1(frameData.mid(offset, mimeEnd - offset));
                offset = mimeEnd + 1;

                if (offset >= frameData.size()) break;
                quint8 picType = (quint8)frameData[offset];
                offset++;

                if (enc == 0 || enc == 3) {
                    int descEnd = frameData.indexOf('\0', offset);
                    if (descEnd < 0) break;
                    offset = descEnd + 1;
                } else {
                    if (offset + 1 >= frameData.size()) break;
                    if ((quint8)frameData[offset] == 0xFF && (quint8)frameData[offset+1] == 0xFE) {
                        offset += 2;
                    }
                    int descEnd = frameData.indexOf(QByteArray("\0\0", 2), offset);
                    if (descEnd < 0) break;
                    offset = descEnd + 2;
                }

                if (offset < frameData.size()) {
                    APICFrame apic;
                    apic.data = frameData.mid(offset);
                    apic.mimeType = mimeType;
                    apic.pictureType = picType;

                    QImage img;
                    if (img.loadFromData(apic.data) && !img.isNull()) {
                        m_albumArt = apic.data;
                    }
                }
            }
        } else {
            QString frameId = QString::fromLatin1(fhdr.id, 4);
            QString value;
            if (frameData.size() > 1) {
                quint8 enc = (quint8)frameData[0];
                int offset = 1;
                value = decodeText(frameData, offset, enc);
            }

            if (frameId == "TPE1") m_artist = value;
            else if (frameId == "TALB") m_album = value;
            else if (frameId == "TIT2") m_title = value;
            else if (frameId == "TYER" || frameId == "TDRC") m_year = value;
            else if (frameId == "TCON") m_genre = value;
            else if (frameId == "TRCK") m_track = value;
            else if (frameId == "COMM") m_comment = value;
            else if (frameId == "TCOM") m_composer = value;
        }

        pos += frameSz;
    }
}

QString ID3Tag::decodeText(const QByteArray &data, int &offset, quint8 encoding, int length) {
    if (offset >= data.size())
        return {};

    int available = (length < 0) ? (data.size() - offset) : length;

    switch (encoding) {
    case 0: // ISO-8859-1
    {
        int end = data.indexOf('\0', offset);
        if (end < 0 || end - offset > available)
            end = offset + available;
        QString result = QString::fromLatin1(data.mid(offset, end - offset));
        offset = (end < data.size() - 1) ? end + 1 : data.size();
        return result;
    }
    case 1: // UTF-16 with BOM
    case 2: // UTF-16BE
    {
        int bomSize = 0;
        if (encoding == 1 && offset + 1 < data.size()) {
            if ((quint8)data[offset] == 0xFF && (quint8)data[offset+1] == 0xFE)
                bomSize = 2;
            else if ((quint8)data[offset] == 0xFE && (quint8)data[offset+1] == 0xFF)
                bomSize = 2;
        }

        int searchStart = offset + bomSize;
        int end = searchStart;
        while (end + 1 < data.size() && (end - searchStart) < available) {
            if (data[end] == '\0' && data[end+1] == '\0') {
                break;
            }
            end += 2;
        }
        if (end >= data.size()) end = data.size() - 2;
        if (end < searchStart) end = searchStart;

        int len = end - searchStart;
        if (len > 0 && len % 2 == 0) {
            QByteArray raw = data.mid(searchStart, len);
            QTextCodec *codec = nullptr;
            if (encoding == 2 || (encoding == 1 && bomSize > 0 && (quint8)data[offset] == 0xFE))
                codec = QTextCodec::codecForName("UTF-16BE");
            else
                codec = QTextCodec::codecForName("UTF-16LE");
            if (codec)
                return codec->toUnicode(raw).trimmed();
        }
        offset = end + 2;
        return {};
    }
    case 3: // UTF-8
    {
        int end = data.indexOf('\0', offset);
        if (end < 0 || end - offset > available)
            end = offset + available;
        QString result = QString::fromUtf8(data.mid(offset, end - offset));
        offset = (end < data.size() - 1) ? end + 1 : data.size();
        return result;
    }
    }
    return {};
}

QByteArray ID3Tag::encodeText(const QString &text, quint8 encoding) {
    if (encoding == 0)
        return text.toLatin1() + '\0';
    return text.toUtf8() + '\0';
}

quint32 ID3Tag::syncSafeToInt(const quint8 *data) {
    return ((quint32)data[0] << 21) |
           ((quint32)data[1] << 14) |
           ((quint32)data[2] << 7) |
           ((quint32)data[3]);
}

void ID3Tag::intToSyncSafe(quint32 value, quint8 *data) {
    data[0] = (value >> 21) & 0x7F;
    data[1] = (value >> 14) & 0x7F;
    data[2] = (value >> 7) & 0x7F;
    data[3] = value & 0x7F;
}

quint32 ID3Tag::frameSize(const quint8 *data, bool isSyncSafe) {
    if (isSyncSafe)
        return syncSafeToInt(data);
    return (quint32)data[0] << 24 | (quint32)data[1] << 16 |
           (quint32)data[2] << 8 | (quint32)data[3];
}

QByteArray ID3Tag::buildFrame(const char *frameId, const QString &text, quint8 encoding) {
    if (text.isEmpty())
        return {};

    QByteArray content;
    content.append((char)encoding);
    content.append(encodeText(text, encoding));

    QByteArray frame;
    frame.append(frameId, 4);

    quint32 sz = content.size();
    frame.append((char)((sz >> 24) & 0xFF));
    frame.append((char)((sz >> 16) & 0xFF));
    frame.append((char)((sz >> 8) & 0xFF));
    frame.append((char)(sz & 0xFF));

    frame.append((char)0);
    frame.append((char)0);
    frame.append(content);
    return frame;
}

QByteArray ID3Tag::buildAPICFrame() {
    if (m_albumArt.isEmpty())
        return {};

    QByteArray content;
    content.append((char)0);

    QImage img;
    if (!img.loadFromData(m_albumArt))
        return {};

    QString mimeType;
    if (m_albumArt.startsWith("\xFF\xD8"))
        mimeType = "image/jpeg";
    else if (m_albumArt.startsWith("\x89\x50"))
        mimeType = "image/png";
    else
        return {};

    content.append(mimeType.toLatin1());
    content.append((char)0);
    content.append((char)3);
    content.append((char)0);
    content.append(m_albumArt);

    QByteArray frame;
    frame.append("APIC", 4);
    quint32 sz = content.size();
    frame.append((char)((sz >> 24) & 0xFF));
    frame.append((char)((sz >> 16) & 0xFF));
    frame.append((char)((sz >> 8) & 0xFF));
    frame.append((char)(sz & 0xFF));
    frame.append((char)0);
    frame.append((char)0);
    frame.append(content);
    return frame;
}

QByteArray ID3Tag::buildID3v2Tag() {
    QByteArray frames;

    QByteArray f;
    f = buildFrame("TPE1", m_artist); if (!f.isEmpty()) frames.append(f);
    f = buildFrame("TALB", m_album); if (!f.isEmpty()) frames.append(f);
    f = buildFrame("TIT2", m_title); if (!f.isEmpty()) frames.append(f);
    f = buildFrame("TDRC", m_year); if (!f.isEmpty()) frames.append(f);
    f = buildFrame("TCON", m_genre); if (!f.isEmpty()) frames.append(f);
    f = buildFrame("TRCK", m_track); if (!f.isEmpty()) frames.append(f);
    f = buildFrame("TCOM", m_composer); if (!f.isEmpty()) frames.append(f);
    f = buildFrame("COMM", m_comment); if (!f.isEmpty()) frames.append(f);

    if (!m_removeArt && !m_albumArt.isEmpty()) {
        f = buildAPICFrame();
        if (!f.isEmpty()) frames.append(f);
    }

    if (frames.isEmpty())
        return {};

    quint32 tagSize = frames.size();
    // Include extended header if needed (skipping for v2.3)
    QByteArray header;
    header.append("ID3", 3);
    header.append((char)3);
    header.append((char)0);
    header.append((char)0);
    header.append((char)((tagSize >> 21) & 0x7F));
    header.append((char)((tagSize >> 14) & 0x7F));
    header.append((char)((tagSize >> 7) & 0x7F));
    header.append((char)(tagSize & 0x7F));

    return header + frames;
}
