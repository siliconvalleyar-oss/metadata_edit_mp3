#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QSet>

class QTableView;
class QLineEdit;
class QLabel;
class QPushButton;
class QSlider;
class QSplitter;
class MP3TableModel;
class MP3File;
class Mp3Player;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onAddFiles();
    void onAddFolder();
    void onClearAll();
    void onApply();
    void onToggleSelectAll();
    void onTableSelectionChanged();
    void onCellChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void onRemoveArt();
    void onDiscardChanges();
    void onPlayPause();
    void onNext();
    void onPrevious();
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onSeek(int position);
    void onPlayerFileChanged(MP3File *file);
    void onTableRowDoubleClicked(const QModelIndex &index);

private:
    void setupUI();
    void loadFile(const QString &filePath);
    void updateDetailPanel();
    void updateStatusBar();
    void commitPendingEdits();
    QVector<MP3File*> selectedFiles() const;
    void refreshPlayerPlaylist();

    QTableView *m_tableView;
    MP3TableModel *m_model;
    Mp3Player *m_player;

    // Player controls
    QPushButton *m_prevBtn;
    QPushButton *m_playBtn;
    QPushButton *m_nextBtn;
    QSlider *m_seekSlider;
    QLabel *m_timeLabel;

    // Detail panel widgets
    QLineEdit *m_editArtist;
    QLineEdit *m_editAlbum;
    QLineEdit *m_editTitle;
    QLineEdit *m_editYear;
    QLineEdit *m_editTrack;
    QLineEdit *m_editGenre;
    QLineEdit *m_editComment;
    QLineEdit *m_editComposer;
    QLineEdit *m_editFileName;
    QLabel *m_artPreview;
    QLabel *m_fileInfoLabel;
    QPushButton *m_removeArtBtn;
    QPushButton *m_applyBtn;
    QPushButton *m_selectAllBtn;

    int m_currentRow = -1;
    bool m_updatingDetail = false;
    QSet<QString> m_editedFields;
    bool m_seeking = false;
};

#endif
