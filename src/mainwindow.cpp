#include "mainwindow.h"
#include "mp3tablemodel.h"
#include "mp3file.h"
#include "mp3player.h"

#include <QTableView>
#include <QHeaderView>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QScrollArea>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileInfo>
#include <QDirIterator>
#include <QImage>
#include <QPixmap>
#include <QTimer>
#include <QProgressDialog>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("MP3 Metadata Editor + Player"));
    resize(1200, 750);
    m_player = new Mp3Player(this);
    setupUI();
}

MainWindow::~MainWindow() {}

static QString formatTime(qint64 ms) {
    int sec = ms / 1000;
    int min = sec / 60;
    sec %= 60;
    return QString("%1:%2").arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0'));
}

void MainWindow::setupUI() {
    auto *mainWidget = new QWidget(this);
    setCentralWidget(mainWidget);
    auto *mainLayout = new QVBoxLayout(mainWidget);

    // Toolbar area
    auto *toolLayout = new QHBoxLayout;

    auto *addFilesBtn = new QPushButton(tr("Add Files"));
    connect(addFilesBtn, &QPushButton::clicked, this, &MainWindow::onAddFiles);
    toolLayout->addWidget(addFilesBtn);

    auto *addFolderBtn = new QPushButton(tr("Add Folder"));
    connect(addFolderBtn, &QPushButton::clicked, this, &MainWindow::onAddFolder);
    toolLayout->addWidget(addFolderBtn);

    auto *clearBtn = new QPushButton(tr("Clear All"));
    connect(clearBtn, &QPushButton::clicked, this, &MainWindow::onClearAll);
    toolLayout->addWidget(clearBtn);

    toolLayout->addStretch();

    m_selectAllBtn = new QPushButton(tr("Select All"));
    m_selectAllBtn->setCheckable(true);
    connect(m_selectAllBtn, &QPushButton::clicked, this, &MainWindow::onToggleSelectAll);
    toolLayout->addWidget(m_selectAllBtn);

    mainLayout->addLayout(toolLayout);

    // Player bar
    auto *playerLayout = new QHBoxLayout;
    playerLayout->setContentsMargins(5, 2, 5, 2);

    m_prevBtn = new QPushButton(QChar(0x23EE));
    m_prevBtn->setToolTip(tr("Previous"));
    m_prevBtn->setFixedWidth(40);
    connect(m_prevBtn, &QPushButton::clicked, this, &MainWindow::onPrevious);
    playerLayout->addWidget(m_prevBtn);

    m_playBtn = new QPushButton(QChar(0x25B6));
    m_playBtn->setToolTip(tr("Play / Pause"));
    m_playBtn->setFixedSize(50, 30);
    m_playBtn->setStyleSheet(
        "QPushButton { font-size: 16px; font-weight: bold; }"
        "QPushButton:pressed { background-color: #ccc; }"
    );
    connect(m_playBtn, &QPushButton::clicked, this, &MainWindow::onPlayPause);
    playerLayout->addWidget(m_playBtn);

    m_nextBtn = new QPushButton(QChar(0x23ED));
    m_nextBtn->setToolTip(tr("Next"));
    m_nextBtn->setFixedWidth(40);
    connect(m_nextBtn, &QPushButton::clicked, this, &MainWindow::onNext);
    playerLayout->addWidget(m_nextBtn);

    m_seekSlider = new QSlider(Qt::Horizontal);
    m_seekSlider->setRange(0, 0);
    m_seekSlider->setToolTip(tr("Seek"));
    connect(m_seekSlider, &QSlider::sliderPressed, [this]() { m_seeking = true; });
    connect(m_seekSlider, &QSlider::sliderReleased, [this]() { m_seeking = false; });
    connect(m_seekSlider, &QSlider::valueChanged, this, &MainWindow::onSeek);
    playerLayout->addWidget(m_seekSlider, 1);

    m_timeLabel = new QLabel("00:00 / 00:00");
    m_timeLabel->setMinimumWidth(100);
    m_timeLabel->setAlignment(Qt::AlignCenter);
    playerLayout->addWidget(m_timeLabel);

    auto *playerBar = new QWidget;
    playerBar->setStyleSheet("QWidget { background-color: #f0f0f0; border: 1px solid #ccc; border-radius: 3px; }");
    playerBar->setLayout(playerLayout);
    mainLayout->addWidget(playerBar);

    // Connect player signals
    connect(m_player, &Mp3Player::stateChanged, this, [this](QMediaPlayer::State state) {
        m_playBtn->setText(state == QMediaPlayer::PlayingState ? QChar(0x23F8) : QChar(0x25B6));
    });
    connect(m_player, &Mp3Player::positionChanged, this, &MainWindow::onPositionChanged);
    connect(m_player, &Mp3Player::durationChanged, this, &MainWindow::onDurationChanged);
    connect(m_player, &Mp3Player::currentFileChanged, this, &MainWindow::onPlayerFileChanged);

    // Splitter: table on left, detail on right
    auto *splitter = new QSplitter(Qt::Horizontal);

    // Table view
    m_model = new MP3TableModel(this);
    m_tableView = new QTableView;
    m_tableView->setModel(m_model);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setSortingEnabled(true);
    m_tableView->verticalHeader()->hide();
    m_tableView->horizontalHeader()->setStretchLastSection(false);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_tableView->setColumnWidth(MP3TableModel::ColFileName, 220);
    m_tableView->setColumnWidth(MP3TableModel::ColArtist, 160);
    m_tableView->setColumnWidth(MP3TableModel::ColAlbum, 160);
    m_tableView->setColumnWidth(MP3TableModel::ColTitle, 200);
    m_tableView->setColumnWidth(MP3TableModel::ColYear, 70);
    m_tableView->setColumnWidth(MP3TableModel::ColTrack, 70);
    m_tableView->setColumnWidth(MP3TableModel::ColGenre, 130);
    m_tableView->setColumnWidth(MP3TableModel::ColSize, 90);

    connect(m_tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onTableSelectionChanged);
    connect(m_model, &QAbstractItemModel::dataChanged,
            this, &MainWindow::onCellChanged);
    connect(m_tableView, &QTableView::doubleClicked,
            this, &MainWindow::onTableRowDoubleClicked);

    splitter->addWidget(m_tableView);

    // Right panel: detail editor + album art
    auto *rightPanel = new QWidget;
    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(5, 0, 0, 0);

    auto *detailGroup = new QGroupBox(tr("File Details"));
    auto *formLayout = new QFormLayout(detailGroup);

    m_fileInfoLabel = new QLabel;
    m_fileInfoLabel->setWordWrap(true);
    formLayout->addRow(tr("Info:"), m_fileInfoLabel);

    m_editFileName = new QLineEdit;
    m_editFileName->setPlaceholderText(tr("(leave empty to keep original)"));
    connect(m_editFileName, &QLineEdit::editingFinished, [this]() {
        if (m_updatingDetail || m_currentRow < 0) return;
        m_editedFields.insert("filename");
        auto *file = m_model->fileAt(m_currentRow);
        if (file) file->setNewFileName(m_editFileName->text().trimmed());
        m_model->layoutChanged();
        updateStatusBar();
    });
    formLayout->addRow(tr("New Filename:"), m_editFileName);

    m_editTitle = new QLineEdit;
    connect(m_editTitle, &QLineEdit::editingFinished, [this]() {
        if (m_updatingDetail || m_currentRow < 0) return;
        m_editedFields.insert("title");
        for (auto *file : selectedFiles())
            file->setTitle(m_editTitle->text().trimmed());
        m_model->layoutChanged();
    });
    formLayout->addRow(tr("Title:"), m_editTitle);

    m_editArtist = new QLineEdit;
    connect(m_editArtist, &QLineEdit::editingFinished, [this]() {
        if (m_updatingDetail || m_currentRow < 0) return;
        m_editedFields.insert("artist");
        for (auto *file : selectedFiles())
            file->setArtist(m_editArtist->text().trimmed());
        m_model->layoutChanged();
    });
    formLayout->addRow(tr("Artist:"), m_editArtist);

    m_editAlbum = new QLineEdit;
    connect(m_editAlbum, &QLineEdit::editingFinished, [this]() {
        if (m_updatingDetail || m_currentRow < 0) return;
        m_editedFields.insert("album");
        for (auto *file : selectedFiles())
            file->setAlbum(m_editAlbum->text().trimmed());
        m_model->layoutChanged();
    });
    formLayout->addRow(tr("Album:"), m_editAlbum);

    auto *rowLayout = new QHBoxLayout;
    m_editYear = new QLineEdit;
    m_editYear->setMaximumWidth(80);
    connect(m_editYear, &QLineEdit::editingFinished, [this]() {
        if (m_updatingDetail || m_currentRow < 0) return;
        m_editedFields.insert("year");
        for (auto *file : selectedFiles())
            file->setYear(m_editYear->text().trimmed());
        m_model->layoutChanged();
    });
    rowLayout->addWidget(new QLabel(tr("Year:")));
    rowLayout->addWidget(m_editYear);

    m_editTrack = new QLineEdit;
    m_editTrack->setMaximumWidth(80);
    connect(m_editTrack, &QLineEdit::editingFinished, [this]() {
        if (m_updatingDetail || m_currentRow < 0) return;
        m_editedFields.insert("track");
        for (auto *file : selectedFiles())
            file->setTrack(m_editTrack->text().trimmed());
        m_model->layoutChanged();
    });
    rowLayout->addWidget(new QLabel(tr("Track:")));
    rowLayout->addWidget(m_editTrack);
    rowLayout->addStretch();
    formLayout->addRow(rowLayout);

    m_editGenre = new QLineEdit;
    connect(m_editGenre, &QLineEdit::editingFinished, [this]() {
        if (m_updatingDetail || m_currentRow < 0) return;
        m_editedFields.insert("genre");
        for (auto *file : selectedFiles())
            file->setGenre(m_editGenre->text().trimmed());
        m_model->layoutChanged();
    });
    formLayout->addRow(tr("Genre:"), m_editGenre);

    m_editComposer = new QLineEdit;
    connect(m_editComposer, &QLineEdit::editingFinished, [this]() {
        if (m_updatingDetail || m_currentRow < 0) return;
        m_editedFields.insert("composer");
        for (auto *file : selectedFiles())
            file->setComposer(m_editComposer->text().trimmed());
    });
    formLayout->addRow(tr("Composer:"), m_editComposer);

    m_editComment = new QLineEdit;
    connect(m_editComment, &QLineEdit::editingFinished, [this]() {
        if (m_updatingDetail || m_currentRow < 0) return;
        m_editedFields.insert("comment");
        for (auto *file : selectedFiles())
            file->setComment(m_editComment->text().trimmed());
    });
    formLayout->addRow(tr("Comment:"), m_editComment);

    rightLayout->addWidget(detailGroup);

    // Album art section
    auto *artGroup = new QGroupBox(tr("Album Art"));
    auto *artLayout = new QVBoxLayout(artGroup);

    m_artPreview = new QLabel;
    m_artPreview->setFixedSize(200, 200);
    m_artPreview->setAlignment(Qt::AlignCenter);
    m_artPreview->setStyleSheet("QLabel { border: 1px solid #888; background: #222; color: #888; }");
    m_artPreview->setText(tr("No Image"));
    artLayout->addWidget(m_artPreview, 0, Qt::AlignCenter);

    auto *artBtnLayout = new QHBoxLayout;
    m_removeArtBtn = new QPushButton(tr("Remove Art"));
    m_removeArtBtn->setEnabled(false);
    connect(m_removeArtBtn, &QPushButton::clicked, this, &MainWindow::onRemoveArt);
    artBtnLayout->addStretch();
    artBtnLayout->addWidget(m_removeArtBtn);
    artBtnLayout->addStretch();
    artLayout->addLayout(artBtnLayout);

    rightLayout->addWidget(artGroup);
    rightLayout->addStretch();

    // Apply button
    m_applyBtn = new QPushButton(tr("Apply Changes"));
    m_applyBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #4a90d9;"
        "  color: white;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  padding: 10px 30px;"
        "  border-radius: 5px;"
        "}"
        "QPushButton:hover { background-color: #357abd; }"
        "QPushButton:disabled { background-color: #888; }"
    );
    m_applyBtn->setEnabled(false);
    connect(m_applyBtn, &QPushButton::clicked, this, &MainWindow::onApply);

    auto *discardBtn = new QPushButton(tr("Discard Changes"));
    discardBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #d9534f;"
        "  color: white;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  padding: 10px 30px;"
        "  border-radius: 5px;"
        "}"
        "QPushButton:hover { background-color: #c9302c; }"
    );
    connect(discardBtn, &QPushButton::clicked, this, &MainWindow::onDiscardChanges);

    auto *applyBtnLayout = new QHBoxLayout;
    applyBtnLayout->addWidget(m_applyBtn);
    applyBtnLayout->addWidget(discardBtn);
    rightLayout->addLayout(applyBtnLayout);

    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);
    mainLayout->addWidget(splitter);

    // Status bar
    statusBar()->showMessage(tr("Ready. Add MP3 files to begin."));
}

QVector<MP3File*> MainWindow::selectedFiles() const {
    QVector<MP3File*> sel;
    auto indexes = m_tableView->selectionModel()->selectedRows();
    for (const auto &idx : indexes) {
        MP3File *f = m_model->fileAt(idx.row());
        if (f) sel.append(f);
    }
    return sel;
}

void MainWindow::refreshPlayerPlaylist() {
    m_player->setFiles(m_model->allFiles());
}

void MainWindow::onAddFiles() {
    QStringList files = QFileDialog::getOpenFileNames(
        this, tr("Select MP3 Files"), QString(),
        tr("MP3 Files (*.mp3);;All Files (*)"));
    if (files.isEmpty()) return;

    for (const auto &path : files)
        loadFile(path);
    refreshPlayerPlaylist();
    updateStatusBar();

    if (m_model->rowCount() == 1)
        m_tableView->selectRow(0);
}

void MainWindow::onAddFolder() {
    QString dir = QFileDialog::getExistingDirectory(
        this, tr("Select Folder with MP3 Files"));
    if (dir.isEmpty()) return;

    QDirIterator it(dir, QStringList() << "*.mp3", QDir::Files, QDirIterator::Subdirectories);
    int count = 0;
    while (it.hasNext()) {
        loadFile(it.next());
        count++;
    }
    if (count == 0) {
        QMessageBox::information(this, tr("No Files"),
                                 tr("No MP3 files found in the selected folder."));
    }
    refreshPlayerPlaylist();
    updateStatusBar();

    if (m_model->rowCount() > 0 && m_currentRow < 0)
        m_tableView->selectRow(0);
}

void MainWindow::loadFile(const QString &filePath) {
    auto *file = new MP3File(filePath);
    if (!file->load()) {
        delete file;
        return;
    }
    m_model->addFile(file);
}

void MainWindow::onClearAll() {
    if (m_model->rowCount() == 0) return;
    m_player->stop();
    m_model->clear();
    m_currentRow = -1;
    refreshPlayerPlaylist();
    updateDetailPanel();
    updateStatusBar();
}

void MainWindow::commitPendingEdits() {
    if (m_currentRow < 0 || m_editedFields.isEmpty()) return;
    for (auto *file : selectedFiles()) {
        if (m_editedFields.contains("title"))    file->setTitle(m_editTitle->text().trimmed());
        if (m_editedFields.contains("artist"))   file->setArtist(m_editArtist->text().trimmed());
        if (m_editedFields.contains("album"))    file->setAlbum(m_editAlbum->text().trimmed());
        if (m_editedFields.contains("year"))     file->setYear(m_editYear->text().trimmed());
        if (m_editedFields.contains("track"))    file->setTrack(m_editTrack->text().trimmed());
        if (m_editedFields.contains("genre"))    file->setGenre(m_editGenre->text().trimmed());
        if (m_editedFields.contains("composer")) file->setComposer(m_editComposer->text().trimmed());
        if (m_editedFields.contains("comment"))  file->setComment(m_editComment->text().trimmed());
    }
    if (m_editedFields.contains("filename")) {
        auto *current = m_model->fileAt(m_currentRow);
        if (current) current->setNewFileName(m_editFileName->text().trimmed());
    }
    m_editedFields.clear();
}

void MainWindow::onApply() {
    commitPendingEdits();
    auto files = selectedFiles();
    if (files.isEmpty()) {
        QMessageBox::information(this, tr("No Files"),
                                 tr("No files selected. Select rows in the table first."));
        return;
    }

    m_applyBtn->setEnabled(false);

    QProgressDialog progress(tr("Saving changes..."), tr("Cancel"), 0, files.size(), this);
    progress.setWindowTitle(tr("Apply Changes"));
    progress.setMinimumWidth(350);
    progress.setWindowModality(Qt::WindowModal);
    progress.setValue(0);
    progress.show();
    QApplication::processEvents();

    int success = 0;
    int failed = 0;
    for (int i = 0; i < files.size(); ++i) {
        if (progress.wasCanceled())
            break;
        progress.setValue(i);
        progress.setLabelText(tr("Saving: %1").arg(files[i]->fileName()));
        QApplication::processEvents();

        if (files[i]->save())
            success++;
        else
            failed++;
    }

    progress.setValue(files.size());

    for (int i = 0; i < m_model->rowCount(); ++i) {
        auto *f = m_model->fileAt(i);
        if (f->isModified())
            f->setModified(false);
    }

    m_applyBtn->setEnabled(true);
    m_model->layoutChanged();
    refreshPlayerPlaylist();
    updateDetailPanel();
    updateStatusBar();

    QString msg = tr("Changes applied: %1 successful, %2 failed.").arg(success).arg(failed);
    statusBar()->showMessage(msg, 5000);

    if (failed > 0)
        QMessageBox::warning(this, tr("Results"), msg);
}

void MainWindow::onDiscardChanges() {
    int modified = 0;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        if (m_model->fileAt(i)->isModified())
            modified++;
    }

    if (modified == 0) {
        QMessageBox::information(this, tr("No Changes"),
                                 tr("No modified files to discard."));
        return;
    }

    if (QMessageBox::question(this, tr("Discard Changes"),
                              tr("Revert changes to %1 modified file(s)?").arg(modified),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;

    for (int i = 0; i < m_model->rowCount(); ++i) {
        auto *f = m_model->fileAt(i);
        if (f->isModified())
            f->discardChanges();
    }

    m_model->layoutChanged();
    updateDetailPanel();
    updateStatusBar();
    statusBar()->showMessage(tr("Reverted %1 file(s).").arg(modified), 5000);
}

void MainWindow::onToggleSelectAll() {
    int total = m_model->rowCount();
    if (total == 0) return;

    bool allSelected = (m_tableView->selectionModel()->selectedRows().size() == total);

    if (allSelected) {
        m_tableView->selectionModel()->clear();
        m_selectAllBtn->setText(tr("Select All"));
    } else {
        QModelIndex topLeft = m_model->index(0, 0);
        QModelIndex bottomRight = m_model->index(total - 1, m_model->columnCount() - 1);
        m_tableView->selectionModel()->select(
            QItemSelection(topLeft, bottomRight),
            QItemSelectionModel::Select | QItemSelectionModel::Rows);
        m_selectAllBtn->setText(tr("Deselect All"));
    }
    updateStatusBar();
}

void MainWindow::onTableSelectionChanged() {
    auto indexes = m_tableView->selectionModel()->selectedRows();
    int total = m_model->rowCount();
    m_editedFields.clear();

    if (indexes.isEmpty()) {
        m_currentRow = -1;
        m_selectAllBtn->setText(tr("Select All"));
        m_applyBtn->setEnabled(false);
    } else {
        m_currentRow = indexes.first().row();
        m_selectAllBtn->setText(
            indexes.size() == total ? tr("Deselect All") : tr("Select All"));
        m_applyBtn->setEnabled(true);
    }
    updateDetailPanel();
    updateStatusBar();
}

void MainWindow::onCellChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                                const QVector<int> &roles) {
    Q_UNUSED(bottomRight);
    Q_UNUSED(roles);
    if (topLeft.row() == m_currentRow)
        updateDetailPanel();
    updateStatusBar();
}

void MainWindow::onRemoveArt() {
    if (m_currentRow < 0) return;
    auto files = selectedFiles();
    if (files.isEmpty()) return;
    for (auto *file : files)
        file->removeAlbumArt();
    m_artPreview->setPixmap(QPixmap());
    m_artPreview->setText(tr("(removed on apply)"));
    m_removeArtBtn->setEnabled(false);
    updateStatusBar();
}

// --- Player slots ---

void MainWindow::onPlayPause() {
    if (m_player->state() == QMediaPlayer::PlayingState) {
        m_player->pause();
    } else {
        if (m_currentRow >= 0) {
            auto *file = m_model->fileAt(m_currentRow);
            if (file) m_player->playFile(file);
        } else {
            m_player->play();
        }
    }
}

void MainWindow::onNext() {
    m_player->next();
}

void MainWindow::onPrevious() {
    m_player->previous();
}

void MainWindow::onPositionChanged(qint64 position) {
    if (!m_seeking)
        m_seekSlider->setValue(static_cast<int>(position));
    m_timeLabel->setText(formatTime(position) + " / " + formatTime(m_player->duration()));
}

void MainWindow::onDurationChanged(qint64 duration) {
    m_seekSlider->setRange(0, static_cast<int>(duration));
}

void MainWindow::onSeek(int position) {
    if (m_seeking)
        m_player->setPosition(position);
}

void MainWindow::onPlayerFileChanged(MP3File *file) {
    if (!file) return;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        if (m_model->fileAt(i) == file) {
            m_tableView->selectRow(i);
            break;
        }
    }
}

void MainWindow::onTableRowDoubleClicked(const QModelIndex &index) {
    if (!index.isValid()) return;
    auto *file = m_model->fileAt(index.row());
    if (file) m_player->playFile(file);
}

// --- UI update ---

void MainWindow::updateDetailPanel() {
    m_updatingDetail = true;

    if (m_currentRow < 0 || m_currentRow >= m_model->rowCount()) {
        m_fileInfoLabel->setText(tr("No file selected"));
        m_editFileName->clear();
        m_editTitle->clear();
        m_editArtist->clear();
        m_editAlbum->clear();
        m_editYear->clear();
        m_editTrack->clear();
        m_editGenre->clear();
        m_editComposer->clear();
        m_editComment->clear();
        m_artPreview->setPixmap(QPixmap());
        m_artPreview->setText(tr("No Image"));
        m_removeArtBtn->setEnabled(false);
        m_updatingDetail = false;
        return;
    }

    auto *file = m_model->fileAt(m_currentRow);
    if (!file) { m_updatingDetail = false; return; }

    QFileInfo fi(file->filePath());
    m_fileInfoLabel->setText(
        tr("Path: %1\nSize: %2\nModified: %3")
            .arg(fi.absoluteFilePath())
            .arg(file->fileSize())
            .arg(file->lastModified().toString("yyyy-MM-dd hh:mm")));

    m_editFileName->setText(file->newFileName().isEmpty() ? QString() : file->newFileName());
    m_editTitle->setText(file->title());
    m_editArtist->setText(file->artist());
    m_editAlbum->setText(file->album());
    m_editYear->setText(file->year());
    m_editTrack->setText(file->track());
    m_editGenre->setText(file->genre());
    m_editComposer->setText(file->composer());
    m_editComment->setText(file->comment());

    if (file->hasAlbumArt() && !file->removeArtFlag()) {
        QPixmap pm;
        if (pm.loadFromData(file->albumArt())) {
            m_artPreview->setPixmap(pm.scaled(198, 198, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            m_removeArtBtn->setEnabled(true);
        } else {
            m_artPreview->setText(tr("(invalid image)"));
            m_removeArtBtn->setEnabled(false);
        }
    } else if (file->removeArtFlag()) {
        m_artPreview->setPixmap(QPixmap());
        m_artPreview->setText(tr("(removed on apply)"));
        m_removeArtBtn->setEnabled(false);
    } else {
        m_artPreview->setPixmap(QPixmap());
        m_artPreview->setText(tr("No Image"));
        m_removeArtBtn->setEnabled(false);
    }

    m_updatingDetail = false;
}

void MainWindow::updateStatusBar() {
    int total = m_model->rowCount();
    int selected = m_tableView->selectionModel()->selectedRows().size();
    int modified = 0;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        if (m_model->fileAt(i)->isModified())
            modified++;
    }

    if (total == 0) {
        statusBar()->showMessage(tr("Ready. Add MP3 files to begin."));
    } else {
        statusBar()->showMessage(
            tr("Files: %1 | Selected: %2 | Modified: %3%4")
                .arg(total).arg(selected).arg(modified)
                .arg(modified > 0 ? tr(" - Click Apply to save") : ""));
    }
}
