#ifndef MP3TABLEMODEL_H
#define MP3TABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <QStringList>

class MP3File;

class MP3TableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Column {
        ColFileName = 0,
        ColArtist,
        ColAlbum,
        ColTitle,
        ColYear,
        ColTrack,
        ColGenre,
        ColSize,
        ColumnCount
    };

    explicit MP3TableModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    void addFile(MP3File *file);
    void clear();
    MP3File *fileAt(int row) const;
    QVector<MP3File*> allFiles() const { return m_files; }
    void removeFile(int row);

private:
    QVector<MP3File*> m_files;
    QStringList m_headers;
};

#endif
