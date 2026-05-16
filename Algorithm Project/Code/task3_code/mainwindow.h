#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QTextEdit>
#include <QSlider>
#include <QTableWidget>
#include "hanoi.h"
#include "hanoiwidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onGenerate();
    void onPlay();
    void onPause();
    void onReset();
    void onStep();
    void onTimerTick();
    void onSpeedChanged(int value);

private:
    void setupUI();
    void buildDPTable();
    void logMove(const Move &mv, int index);
    void updateStats();
    void updateDPTableHighlight();

    // ── solver ────────────────────────────────────────────────────────────────
    HanoiSolver  *m_solver     = nullptr;
    HanoiWidget  *m_hanoiWidget = nullptr;

    // ── left panel controls ───────────────────────────────────────────────────
    QSpinBox     *m_diskSpin   = nullptr;
    QComboBox    *m_algoCombo  = nullptr;
    QPushButton  *m_generateBtn = nullptr;

    // ── live statistics labels ────────────────────────────────────────────────
    QLabel       *m_movesSoFarVal  = nullptr;
    QLabel       *m_optimalTotalVal = nullptr;
    QLabel       *m_optimalKVal    = nullptr;
    QLabel       *m_statusVal      = nullptr;

    // ── playback buttons ──────────────────────────────────────────────────────
    QPushButton  *m_playBtn   = nullptr;
    QPushButton  *m_pauseBtn  = nullptr;
    QPushButton  *m_resetBtn  = nullptr;
    QPushButton  *m_stepBtn   = nullptr;
    QSlider      *m_speedSlider = nullptr;
    QLabel       *m_speedLabel  = nullptr;

    // ── bottom panels ─────────────────────────────────────────────────────────
    QTextEdit    *m_log        = nullptr;
    QTableWidget *m_dpTable    = nullptr;

    // ── right panel: research notes ───────────────────────────────────────────
    QTextEdit    *m_researchNotes = nullptr;

    // ── animation ────────────────────────────────────────────────────────────
    QTimer *m_timer     = nullptr;
    int     m_moveIndex = 0;
    bool    m_playing   = false;
};

#endif // MAINWINDOW_H
