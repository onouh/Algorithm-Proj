#pragma once
#include <QMainWindow>
#include <QVector>
#include "mincut.h"

QT_BEGIN_NAMESPACE
class QSpinBox;   class QDoubleSpinBox; class QComboBox;
class QLabel;     class QPushButton;    class QListWidget;
class QTableWidget; class QTextEdit;   class QGroupBox;
QT_END_NAMESPACE

class GraphWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onRun();
    void onClear();
    void onLoadExample();
    void onAddEdge();
    void onAlgoChanged(int idx);

private:
    void buildUI();

    void showResult(const CutResult& r);
    void updateCompareTable();
    void setCell(int r, int c, const QString& s, QColor bg = QColor("#FFFFFF"));

    // ── data ──────────────────────────────────────────────────────────────
    Graph                    graph;
    std::vector<CutResult>   results;   // one per algo run

    // ── widgets ───────────────────────────────────────────────────────────
    GraphWidget*   graphWidget  = nullptr;

    QSpinBox*      spinN        = nullptr;   // num vertices
    QSpinBox*      spinU        = nullptr;   // edge from
    QSpinBox*      spinV        = nullptr;   // edge to
    QDoubleSpinBox* spinW       = nullptr;   // edge weight
    QPushButton*   btnAddEdge   = nullptr;
    QPushButton*   btnRun       = nullptr;
    QPushButton*   btnClear     = nullptr;
    QPushButton*   btnExample   = nullptr;
    QComboBox*     comboAlgo    = nullptr;
    QSpinBox*      spinRestarts = nullptr;

    QLabel*        lblCut       = nullptr;
    QLabel*        lblSetA      = nullptr;
    QLabel*        lblSetB      = nullptr;
    QLabel*        lblStatus    = nullptr;

    QTableWidget*  edgeTable    = nullptr;
    QListWidget*   logList      = nullptr;
    QTableWidget*  compareTable = nullptr;
    QTextEdit*     researchPanel= nullptr;
};
