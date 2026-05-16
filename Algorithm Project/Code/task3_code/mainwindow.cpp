#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QScrollBar>
#include <QMessageBox>
#include <QFont>
#include <QFrame>
#include <QSplitter>

// ─────────────────────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("CSE245 — Tower of Hanoi: D&C + DP Research Tool");
    setMinimumSize(1100, 720);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::onTimerTick);

    setupUI();
}

// ─────────────────────────────────────────────────────────────────────────────
//  UI SETUP  — mirrors the screenshot exactly
// ─────────────────────────────────────────────────────────────────────────────

static QLabel *makeStatLabel(const QString &text, QWidget *parent) {
    QLabel *l = new QLabel(text, parent);
    l->setStyleSheet("color: #aaaaaa; font-size: 13px;");
    return l;
}
static QLabel *makeStatValue(QWidget *parent) {
    QLabel *l = new QLabel("—", parent);
    l->setStyleSheet("color: #ffffff; font-size: 16px; font-weight: bold;");
    l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    return l;
}

void MainWindow::setupUI()
{
    // ── Overall: left panel | right area ─────────────────────────────────────
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QHBoxLayout *root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ════════════════════════════════════════════════════════════════════════
    //  LEFT PANEL  (fixed 280px, dark blue-grey)
    // ════════════════════════════════════════════════════════════════════════
    QWidget *left = new QWidget();
    left->setFixedWidth(280);
    left->setStyleSheet("background-color: #131929; color: #ffffff;");
    QVBoxLayout *leftLayout = new QVBoxLayout(left);
    leftLayout->setContentsMargins(16, 16, 16, 16);
    leftLayout->setSpacing(14);

    // Title
    QLabel *title = new QLabel("Tower of Hanoi — 4 Pegs");
    title->setStyleSheet("color: #5b9bd5; font-size: 18px; font-weight: bold;");
    title->setWordWrap(true);
    leftLayout->addWidget(title);

    QLabel *subtitle = new QLabel("CSE245 Project — Task 3 | Frame-Stewart D&C + DP");
    subtitle->setStyleSheet("color: #888888; font-size: 10px;");
    subtitle->setWordWrap(true);
    leftLayout->addWidget(subtitle);

    // ── Configuration group ───────────────────────────────────────────────
    QGroupBox *cfgBox = new QGroupBox("Configuration");
    cfgBox->setStyleSheet(
        "QGroupBox { color:#cccccc; border:1px solid #444; border-radius:4px; "
        "margin-top:6px; font-size:11px; }"
        "QGroupBox::title { subcontrol-origin:margin; left:8px; padding:0 4px; }");
    QGridLayout *cfgGrid = new QGridLayout(cfgBox);
    cfgGrid->setSpacing(6);

    cfgGrid->addWidget(new QLabel("Number of disks (n):"), 0, 0);
    m_diskSpin = new QSpinBox();
    m_diskSpin->setRange(1, 16);
    m_diskSpin->setValue(8);
    m_diskSpin->setStyleSheet(
        "QSpinBox { background:#2a2a3f; color:white; border:1px solid #555; "
        "border-radius:3px; padding:2px; }");
    cfgGrid->addWidget(m_diskSpin, 0, 1);

    cfgGrid->addWidget(new QLabel("Algorithm:"), 1, 0);
    m_algoCombo = new QComboBox();
    m_algoCombo->addItem("Frame-Stewart DP (4 pegs) — optimal");
    m_algoCombo->addItem("Classic 3-peg Hanoi (comparison)");
    m_algoCombo->setStyleSheet(
        "QComboBox { background:#2a2a3f; color:white; border:1px solid #555; "
        "border-radius:3px; padding:2px; }"
        "QComboBox QAbstractItemView { background:#2a2a3f; color:white; }");
    cfgGrid->addWidget(m_algoCombo, 1, 0, 1, 2);

    m_generateBtn = new QPushButton("Generate Solution");
    m_generateBtn->setStyleSheet(
        "QPushButton { background:#2980b9; color:white; border:none; "
        "border-radius:4px; padding:8px; font-size:13px; font-weight:bold; }"
        "QPushButton:hover { background:#3498db; }"
        "QPushButton:pressed { background:#1c6ea4; }");
    cfgGrid->addWidget(m_generateBtn, 2, 0, 1, 2);
    leftLayout->addWidget(cfgBox);

    // ── Live Statistics ───────────────────────────────────────────────────
    QGroupBox *statsBox = new QGroupBox("Live Statistics");
    statsBox->setStyleSheet(cfgBox->styleSheet());
    QGridLayout *statsGrid = new QGridLayout(statsBox);
    statsGrid->setSpacing(4);

    statsGrid->addWidget(makeStatLabel("Moves so far:", statsBox), 0, 0);
    m_movesSoFarVal = makeStatValue(statsBox);
    statsGrid->addWidget(m_movesSoFarVal, 0, 1);

    statsGrid->addWidget(makeStatLabel("Optimal total:", statsBox), 1, 0);
    m_optimalTotalVal = makeStatValue(statsBox);
    statsGrid->addWidget(m_optimalTotalVal, 1, 1);

    statsGrid->addWidget(makeStatLabel("Optimal k:", statsBox), 2, 0);
    m_optimalKVal = makeStatValue(statsBox);
    statsGrid->addWidget(m_optimalKVal, 2, 1);

    m_statusVal = new QLabel("Ready");
    m_statusVal->setStyleSheet("color: #27ae60; font-size: 11px; font-style: italic;");
    statsGrid->addWidget(m_statusVal, 3, 0, 1, 2);
    leftLayout->addWidget(statsBox);

    // ── Playback ─────────────────────────────────────────────────────────
    QGroupBox *pbBox = new QGroupBox("Playback");
    pbBox->setStyleSheet(cfgBox->styleSheet());
    QVBoxLayout *pbLayout = new QVBoxLayout(pbBox);
    pbLayout->setSpacing(6);

    QString btnStyle =
        "QPushButton { background:#2a2a3f; color:white; border:1px solid #555; "
        "border-radius:4px; padding:5px 10px; font-size:12px; }"
        "QPushButton:hover { background:#3a3a5f; }"
        "QPushButton:disabled { color:#555; border-color:#333; }";
    QString pauseStyle =
        "QPushButton { background:#2471a3; color:white; border:none; "
        "border-radius:4px; padding:5px 10px; font-size:12px; }"
        "QPushButton:hover { background:#2980b9; }"
        "QPushButton:disabled { color:#555; background:#1a1a2e; }";

    QHBoxLayout *btnRow = new QHBoxLayout();
    m_playBtn  = new QPushButton("▶ Play");
    m_pauseBtn = new QPushButton("⏸ Pause");
    m_resetBtn = new QPushButton("↺ Reset");
    m_stepBtn  = new QPushButton("Step →");
    m_playBtn->setStyleSheet(btnStyle);
    m_pauseBtn->setStyleSheet(pauseStyle);
    m_resetBtn->setStyleSheet(btnStyle);
    m_stepBtn->setStyleSheet(btnStyle);
    m_playBtn->setEnabled(false);
    m_pauseBtn->setEnabled(false);
    m_resetBtn->setEnabled(false);
    m_stepBtn->setEnabled(false);
    btnRow->addWidget(m_playBtn);
    btnRow->addWidget(m_pauseBtn);
    btnRow->addWidget(m_resetBtn);
    btnRow->addWidget(m_stepBtn);
    pbLayout->addLayout(btnRow);

    QHBoxLayout *speedRow = new QHBoxLayout();
    speedRow->addWidget(new QLabel("Speed:"));
    m_speedSlider = new QSlider(Qt::Horizontal);
    m_speedSlider->setRange(1, 10);
    m_speedSlider->setValue(5);
    m_speedSlider->setStyleSheet(
        "QSlider::groove:horizontal { height:4px; background:#444; border-radius:2px; }"
        "QSlider::handle:horizontal { width:14px; height:14px; background:#2980b9; "
        "border-radius:7px; margin:-5px 0; }"
        "QSlider::sub-page:horizontal { background:#2980b9; border-radius:2px; }");
    m_speedLabel = new QLabel("5");
    m_speedLabel->setFixedWidth(20);
    speedRow->addWidget(m_speedSlider);
    speedRow->addWidget(m_speedLabel);
    pbLayout->addLayout(speedRow);
    leftLayout->addWidget(pbBox);

    // ── Research Notes ────────────────────────────────────────────────────
    QGroupBox *notesBox = new QGroupBox("Research Notes");
    notesBox->setStyleSheet(cfgBox->styleSheet());
    QVBoxLayout *notesLayout = new QVBoxLayout(notesBox);
    m_researchNotes = new QTextEdit();
    m_researchNotes->setReadOnly(true);
    m_researchNotes->setStyleSheet(
        "QTextEdit { background:#12121f; color:#cccccc; border:none; font-size:11px; }");
    m_researchNotes->setMinimumHeight(140);
    m_researchNotes->setHtml(
        "<p><b>Frame-Stewart Conjecture (1941)</b><br>"
        "The 4-peg formula T(n,4) = min{ 2·T(k,4) + (2<sup>n-k</sup>-1) } "
        "gives the best known solution.</p>"
        "<p><b>Open Problem:</b> Optimality proven only for n ≤ 30 by "
        "exhaustive search. No general proof exists — this is an "
        "unsolved problem in combinatorics.</p>"
        "<p><b>Standard DP vs Frame-Stewart:</b> Standard 3-peg DP "
        "gives 2<sup>n</sup>-1 = 255 moves for n=8. Frame-Stewart gives "
        "33 moves — an 87% reduction.</p>");
    notesLayout->addWidget(m_researchNotes);
    leftLayout->addWidget(notesBox, 1);

    root->addWidget(left);

    // ════════════════════════════════════════════════════════════════════════
    //  RIGHT AREA  (visualization top, log+table bottom)
    // ════════════════════════════════════════════════════════════════════════
    QWidget *rightArea = new QWidget();
    rightArea->setStyleSheet("background-color: #0f1428;");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightArea);
    rightLayout->setContentsMargins(12, 12, 12, 12);
    rightLayout->setSpacing(10);

    // Puzzle Visualization label
    QLabel *vizLabel = new QLabel("Puzzle Visualization");
    vizLabel->setStyleSheet("color:#cccccc; font-size:12px;");
    rightLayout->addWidget(vizLabel);

    // Hanoi widget
    m_hanoiWidget = new HanoiWidget(rightArea);
    m_hanoiWidget->setMinimumHeight(300);
    rightLayout->addWidget(m_hanoiWidget, 2);

    // ── Bottom: Move Log | DP Table ───────────────────────────────────────
    QHBoxLayout *bottomRow = new QHBoxLayout();
    bottomRow->setSpacing(10);

    // Move Log
    QWidget *logPanel = new QWidget();
    QVBoxLayout *logLayout = new QVBoxLayout(logPanel);
    logLayout->setContentsMargins(0,0,0,0);
    logLayout->setSpacing(4);
    QLabel *logLabel = new QLabel("Move Log");
    logLabel->setStyleSheet("color:#cccccc; font-size:12px;");
    logLayout->addWidget(logLabel);
    m_log = new QTextEdit();
    m_log->setReadOnly(true);
    m_log->setFont(QFont("Consolas", 9));
    m_log->setStyleSheet(
        "QTextEdit { background:#0d0d1a; color:#e0e0e0; border:1px solid #333; "
        "border-radius:3px; }");
    m_log->setMinimumHeight(180);
    logLayout->addWidget(m_log);
    bottomRow->addWidget(logPanel, 1);

    // DP Table
    QWidget *tablePanel = new QWidget();
    QVBoxLayout *tableLayout = new QVBoxLayout(tablePanel);
    tableLayout->setContentsMargins(0,0,0,0);
    tableLayout->setSpacing(4);
    QLabel *tableLabel = new QLabel("DP Table — Frame-Stewart");
    tableLabel->setStyleSheet("color:#cccccc; font-size:12px;");
    tableLayout->addWidget(tableLabel);

    m_dpTable = new QTableWidget();
    m_dpTable->setColumnCount(5);
    m_dpTable->setHorizontalHeaderLabels({"n", "Opt. k", "4-peg move", "3-peg move", "Savings"});
    m_dpTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_dpTable->verticalHeader()->setVisible(false);
    m_dpTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_dpTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_dpTable->setStyleSheet(
        "QTableWidget { background:#0d0d1a; color:#e0e0e0; "
        "border:1px solid #333; gridline-color:#2a2a3f; }"
        "QHeaderView::section { background:#1a1a3f; color:#aaaaff; "
        "border:none; padding:4px; font-weight:bold; }"
        "QTableWidget::item:selected { background:#1a4a7a; }");
    m_dpTable->setMinimumHeight(180);
    tableLayout->addWidget(m_dpTable);
    bottomRow->addWidget(tablePanel, 1);

    rightLayout->addLayout(bottomRow, 1);
    root->addWidget(rightArea, 1);

    // ── Connections ───────────────────────────────────────────────────────────
    connect(m_generateBtn, &QPushButton::clicked, this, &MainWindow::onGenerate);
    connect(m_playBtn,     &QPushButton::clicked, this, &MainWindow::onPlay);
    connect(m_pauseBtn,    &QPushButton::clicked, this, &MainWindow::onPause);
    connect(m_resetBtn,    &QPushButton::clicked, this, &MainWindow::onReset);
    connect(m_stepBtn,     &QPushButton::clicked, this, &MainWindow::onStep);
    connect(m_speedSlider, &QSlider::valueChanged, this, &MainWindow::onSpeedChanged);

    // Populate the DP table for default n=8
    buildDPTable();
}

// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::buildDPTable()
{
    int n = m_diskSpin->value();
    // Build a temporary solver just for the table values
    HanoiSolver tmp(n);

    // Show rows 2..n
    int rows = qMax(0, n - 1);
    m_dpTable->setRowCount(rows);

    for (int i = 0; i < rows; ++i) {
        int ni = i + 2;
        long long fourPeg  = tmp.dpValue(ni);
        long long threePeg = (1LL << ni) - 1LL;
        double savings     = (threePeg > 0) ? (1.0 - (double)fourPeg / threePeg) * 100.0 : 0.0;

        auto *itN  = new QTableWidgetItem(QString::number(ni));
        auto *itK  = new QTableWidgetItem(QString::number(tmp.optimalK(ni)));
        auto *it4  = new QTableWidgetItem(QString::number(fourPeg));
        auto *it3  = new QTableWidgetItem(QString::number(threePeg));
        auto *itS  = new QTableWidgetItem(QString::number((int)savings) + "%");

        for (auto *it : {itN, itK, it4, it3, itS})
            it->setTextAlignment(Qt::AlignCenter);

        m_dpTable->setItem(i, 0, itN);
        m_dpTable->setItem(i, 1, itK);
        m_dpTable->setItem(i, 2, it4);
        m_dpTable->setItem(i, 3, it3);
        m_dpTable->setItem(i, 4, itS);
    }
}

void MainWindow::updateDPTableHighlight()
{
    if (!m_solver) return;
    int n = m_solver->numDisks();
    // Row index: row 0 = n=2, so row for current n is n-2
    int targetRow = n - 2;
    for (int r = 0; r < m_dpTable->rowCount(); ++r) {
        bool highlight = (r == targetRow);
        QColor bg = highlight ? QColor(0x1a, 0x4a, 0x7a) : QColor(0x0d, 0x0d, 0x1a);
        QColor fg = highlight ? QColor(0xff, 0xff, 0xff) : QColor(0xe0, 0xe0, 0xe0);
        for (int c = 0; c < m_dpTable->columnCount(); ++c) {
            if (m_dpTable->item(r, c)) {
                m_dpTable->item(r, c)->setBackground(bg);
                m_dpTable->item(r, c)->setForeground(fg);
            }
        }
    }
    if (targetRow >= 0 && targetRow < m_dpTable->rowCount())
        m_dpTable->scrollToItem(m_dpTable->item(targetRow, 0));
}

// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::onGenerate()
{
    m_timer->stop();
    m_playing   = false;
    m_moveIndex = 0;

    int n      = m_diskSpin->value();
    QString al = m_algoCombo->currentText();

    delete m_solver;
    m_solver = new HanoiSolver(n);
    m_solver->solve(al);

    // Validate before touching the UI
    QString err = m_solver->validate();
    if (!err.isEmpty()) {
        QMessageBox::critical(this, "Logic Error", err);
        m_statusVal->setText("ERROR");
        return;
    }

    m_hanoiWidget->reset(n);
    m_log->clear();
    buildDPTable();
    updateDPTableHighlight();

    int total = m_solver->moves().size();
    m_movesSoFarVal->setText("0");
    m_optimalTotalVal->setText(QString::number(m_solver->optimalMoves()));
    m_optimalKVal->setText(QString::number(m_solver->optimalK()));
    m_statusVal->setText("Ready to play...");

    m_playBtn->setEnabled(true);
    m_pauseBtn->setEnabled(false);
    m_resetBtn->setEnabled(true);
    m_stepBtn->setEnabled(total > 0);
    m_playBtn->setText("▶ Play");
}

void MainWindow::onPlay()
{
    if (!m_solver) return;
    m_playing = true;
    m_playBtn->setEnabled(false);
    m_pauseBtn->setEnabled(true);
    m_statusVal->setText("Playing...");
    int ms = 1100 - m_speedSlider->value() * 100;
    m_timer->start(ms);
}

void MainWindow::onPause()
{
    m_timer->stop();
    m_playing = false;
    m_playBtn->setEnabled(true);
    m_pauseBtn->setEnabled(false);
    m_statusVal->setText("Paused.");
}

void MainWindow::onReset()
{
    m_timer->stop();
    m_playing   = false;
    m_moveIndex = 0;
    m_playBtn->setText("▶ Play");
    m_playBtn->setEnabled(m_solver != nullptr);
    m_pauseBtn->setEnabled(false);

    if (m_solver) m_hanoiWidget->reset(m_solver->numDisks());
    m_log->clear();
    m_movesSoFarVal->setText("0");
    m_statusVal->setText("Reset.");
}

void MainWindow::onStep()
{
    if (!m_solver) return;
    const QVector<Move> &mvs = m_solver->moves();
    if (m_moveIndex >= mvs.size()) return;

    const Move &mv = mvs[m_moveIndex];
    bool ok = m_hanoiWidget->applyMove(mv);
    if (!ok) {
        m_timer->stop();
        QMessageBox::critical(this, "Error",
            QString("Illegal move #%1").arg(m_moveIndex + 1));
        return;
    }
    logMove(mv, m_moveIndex);
    ++m_moveIndex;
    updateStats();
}

void MainWindow::onTimerTick()
{
    if (!m_solver) { m_timer->stop(); return; }
    if (m_moveIndex >= m_solver->moves().size()) {
        m_timer->stop();
        m_playing = false;
        m_playBtn->setEnabled(false);
        m_pauseBtn->setEnabled(false);
        m_stepBtn->setEnabled(false);
        m_statusVal->setText("Complete! ✓");
        return;
    }
    onStep();
}

void MainWindow::onSpeedChanged(int value)
{
    m_speedLabel->setText(QString::number(value));
    if (m_playing) {
        int ms = 1100 - value * 100;
        m_timer->setInterval(ms);
    }
}

// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::logMove(const Move &mv, int index)
{
    static const char *pn[] = {"Peg 1", "Peg 2", "Peg 3", "Peg 4"};
    QString line = QString("Move %1: Disk %2   %3 → %4")
                       .arg(index + 1, 3)
                       .arg(mv.disk, 2)
                       .arg(pn[mv.from])
                       .arg(pn[mv.to]);

    // Highlight the latest move in blue (matching screenshot)
    m_log->append(QString("<span style='color:#5b9bd5;'>%1</span>").arg(line));
    m_log->verticalScrollBar()->setValue(m_log->verticalScrollBar()->maximum());
}

void MainWindow::updateStats()
{
    m_movesSoFarVal->setText(QString::number(m_moveIndex));
    int total = m_solver ? m_solver->moves().size() : 0;
    if (m_moveIndex >= total) {
        m_stepBtn->setEnabled(false);
        m_statusVal->setText("Complete! ✓");
    }
}
