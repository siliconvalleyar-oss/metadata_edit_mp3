#include "mp3tablemodel.h"
#include "mp3file.h"
#include <QColor>

MP3TableModel::MP3TableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_headers = QStringList()
        << tr("Filename")
        << tr("Artist")
        << tr("Album")
        << tr("Title")
        << tr("Year")
        << tr("Track")
        << tr("Genre")
        << tr("Size");
}

int MP3TableModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_files.size();
}

int MP3TableModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return ColumnCount;
}

QVariant MP3TableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_files.size())
        return {};

    MP3File *file = m_files[index.row()];

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case ColFileName: return file->fileName();
        case ColArtist:   return file->artist();
        case ColAlbum:    return file->album();
        case ColTitle:    return file->title();
        case ColYear:     return file->year();
        case ColTrack:    return file->track();
        case ColGenre:    return file->genre();
        case ColSize:
            if (role == Qt::DisplayRole) {
                qint64 sz = file->fileSize();
                if (sz < 1024) return QString("%1 B").arg(sz);
                if (sz < 1024*1024) return QString("%1 KB").arg(sz / 1024);
                return QString("%1 MB").arg(sz / (1024*1024));
            }
            return file->fileSize();
        }
    }

    if (role == Qt::ForegroundRole && index.column() == ColFileName) {
        if (file->isModified())
            return QColor(0, 120, 200);
    }

    if (role == Qt::ToolTipRole) {
        if (index.column() == ColFileName)
            return file->filePath();
        if (index.column() == ColSize) {
            qint64 sz = file->fileSize();
            return QString("%1 bytes").arg(sz);
        }
    }

    return {};
}

QVariant MP3TableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};
    if (section >= 0 && section < m_headers.size())
        return m_headers[section];
    return {};
}

bool MP3TableModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || index.row() >= m_files.size())
        return false;

    MP3File *file = m_files[index.row()];

    if (role == Qt::EditRole) {
        switch (index.column()) {
        case ColFileName: file->setNewFileName(value.toString()); break;
        case ColArtist:   file->setArtist(value.toString()); break;
        case ColAlbum:    file->setAlbum(value.toString()); break;
        case ColTitle:    file->setTitle(value.toString()); break;
        case ColYear:     file->setYear(value.toString()); break;
        case ColTrack:    file->setTrack(value.toString()); break;
        case ColGenre:    file->setGenre(value.toString()); break;
        default: return false;
        }
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole, Qt::ForegroundRole});
        return true;
    }

    return false;
}

Qt::ItemFlags MP3TableModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags f = QAbstractTableModel::flags(index);
    if (index.column() != ColSize)
        f |= Qt::ItemIsEditable;
    return f;
}

void MP3TableModel::addFile(MP3File *file) {
    beginInsertRows(QModelIndex(), m_files.size(), m_files.size());
    m_files.append(file);
    endInsertRows();
}

void MP3TableModel::clear() {
    beginResetModel();
    qDeleteAll(m_files);
    m_files.clear();
    endResetModel();
}

MP3File *MP3TableModel::fileAt(int row) const {
    if (row < 0 || row >= m_files.size()) return nullptr;
    return m_files[row];
}

void MP3TableModel::removeFile(int row) {
    if (row < 0 || row >= m_files.size()) return;
    beginRemoveRows(QModelIndex(), row, row);
    delete m_files[row];
    m_files.removeAt(row);
    endRemoveRows();
}

void MP3TableModel::sort(int column, Qt::SortOrder order) {
    if (m_files.isEmpty()) return;

    emit layoutAboutToBeChanged();

    std::sort(m_files.begin(), m_files.end(), [column, order](MP3File *a, MP3File *b) {
        int cmp = 0;
        switch (column) {
        case ColFileName: cmp = a->fileName().compare(b->fileName(), Qt::CaseInsensitive); break;
        case ColArtist:   cmp = a->artist().compare(b->artist(), Qt::CaseInsensitive); break;
        case ColAlbum:    cmp = a->album().compare(b->album(), Qt::CaseInsensitive); break;
        case ColTitle:    cmp = a->title().compare(b->title(), Qt::CaseInsensitive); break;
        case ColYear: {
            int ya = a->year().toInt();
            int yb = b->year().toInt();
            cmp = (ya < yb) ? -1 : (ya > yb) ? 1 : 0;
            break;
        }
        case ColTrack: {
            int ta = a->track().toInt();
            int tb = b->track().toInt();
            cmp = (ta < tb) ? -1 : (ta > tb) ? 1 : 0;
            break;
        }
        case ColGenre: cmp = a->genre().compare(b->genre(), Qt::CaseInsensitive); break;
        case ColSize: {
            qint64 sa = a->fileSize();
            qint64 sb = b->fileSize();
            cmp = (sa < sb) ? -1 : (sa > sb) ? 1 : 0;
            break;
        }
        }
        return order == Qt::AscendingOrder ? (cmp < 0) : (cmp > 0);
    });

    emit layoutChanged();
}
