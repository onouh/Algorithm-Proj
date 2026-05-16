#include "mainwindow.h"
#include "graphwidget.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QTextEdit>
#include <QSplitter>
#include <QFont>
#include <QString>
#include <QColor>
#include <QMessageBox>
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("CSE245 — Minimum Cut: Brute Force + Iterative Improvement + Stoer-Wagner");
    setMinimumSize(1200, 750);
    buildUI();
    onLoadExample();   // start with a ready-to-use sample graph
}

// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::buildUI() {
    QSplitter* root = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(root);

    // ══ LEFT PANEL ════════════════════════════════════════════════════════════
    QWidget*     left   = new QWidget;
    QVBoxLayout* leftLy = new QVBoxLayout(left);
    leftLy->setContentsMargins(10, 10, 6, 10);
    leftLy->setSpacing(8);

    // title
    QLabel* title = new QLabel("Weighted Graph — Minimum Cut");
    title->setFont(QFont("Arial", 13, QFont::Bold));
    title->setStyleSheet("color:#2563EB;");
    leftLy->addWidget(title);

    QLabel* sub = new QLabel("CSE245 Task 8  |  Brute Force · KL · Stoer-Wagner");
    sub->setStyleSheet("color:#6B7280; font-size:11px;");
    leftLy->addWidget(sub);

    // ── Graph builder ─────────────────────────────────────────────────────────
    QGroupBox*   gbuild   = new QGroupBox("Build Graph");
    QVBoxLayout* buildLy  = new QVBoxLayout(gbuild);

    QHBoxLayout* rN = new QHBoxLayout;
    rN->addWidget(new QLabel("Vertices (n):"));
    spinN = new QSpinBox;
    spinN->setRange(2, 12);
    spinN->setValue(6);
    rN->addWidget(spinN); rN->addStretch();
    buildLy->addLayout(rN);

    QHBoxLayout* rEdge = new QHBoxLayout;
    rEdge->addWidget(new QLabel("Edge:"));
    spinU = new QSpinBox; spinU->setRange(0,11); spinU->setValue(0);
    rEdge->addWidget(spinU);
    rEdge->addWidget(new QLabel("—"));
    spinV = new QSpinBox; spinV->setRange(0,11); spinV->setValue(1);
    rEdge->addWidget(spinV);
    rEdge->addWidget(new QLabel("w:"));
    spinW = new QDoubleSpinBox;
    spinW->setRange(0.1, 99); spinW->setValue(2.0); spinW->setDecimals(1);
    rEdge->addWidget(spinW);
    buildLy->addLayout(rEdge);

    QHBoxLayout* rBtns = new QHBoxLayout;
    btnAddEdge = new QPushButton("Add Edge");
    btnClear   = new QPushButton("Clear All");
    btnExample = new QPushButton("Load Example");
    for (auto* b : {btnAddEdge, btnClear, btnExample}) {
        b->setStyleSheet(
            "QPushButton{border:1px solid #D1D5DB;border-radius:4px;padding:4px 8px;}"
            "QPushButton:hover{background:#F3F4F6;}");
        rBtns->addWidget(b);
    }
    buildLy->addLayout(rBtns);

    // Edge table
    edgeTable = new QTableWidget;
    edgeTable->setColumnCount(3);
    edgeTable->setHorizontalHeaderLabels({"From", "To", "Weight"});
    edgeTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    edgeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    edgeTable->setMaximumHeight(130);
    edgeTable->setAlternatingRowColors(true);
    buildLy->addWidget(edgeTable);
    leftLy->addWidget(gbuild);

    // ── Algorithm selection ───────────────────────────────────────────────────
    QGroupBox*   galgo   = new QGroupBox("Algorithm");
    QVBoxLayout* algoLy  = new QVBoxLayout(galgo);

    QHBoxLayout* rAlgo = new QHBoxLayout;
    rAlgo->addWidget(new QLabel("Select:"));
    comboAlgo = new QComboBox;
    comboAlgo->addItem("Brute Force  (exact, exponential)");
    comboAlgo->addItem("Kernighan-Lin  (iterative improvement)");
    comboAlgo->addItem("Stoer-Wagner  (exact, polynomial)");
    comboAlgo->addItem("Run ALL three and compare");
    rAlgo->addWidget(comboAlgo);
    algoLy->addLayout(rAlgo);

    QHBoxLayout* rRst = new QHBoxLayout;
    rRst->addWidget(new QLabel("KL restarts:"));
    spinRestarts = new QSpinBox;
    spinRestarts->setRange(1, 20); spinRestarts->setValue(5);
    rRst->addWidget(spinRestarts); rRst->addStretch();
    algoLy->addLayout(rRst);

    btnRun = new QPushButton("Run Algorithm");
    btnRun->setStyleSheet(
        "QPushButton{background:#2563EB;color:white;border-radius:5px;"
        "padding:6px 14px;font-weight:bold;}"
        "QPushButton:hover{background:#1D4ED8;}"
        "QPushButton:pressed{background:#1E40AF;}");
    algoLy->addWidget(btnRun);
    leftLy->addWidget(galgo);

    // ── Results ───────────────────────────────────────────────────────────────
    QGroupBox*   gres   = new QGroupBox("Result");
    QVBoxLayout* resLy  = new QVBoxLayout(gres);

    auto addResRow = [&](const QString& lbl, QLabel*& out) {
        QHBoxLayout* r = new QHBoxLayout;
        QLabel* k = new QLabel(lbl);
        k->setStyleSheet("color:#6B7280;font-size:12px;");
        out = new QLabel("—");
        out->setFont(QFont("Arial", 12, QFont::Bold));
        r->addWidget(k); r->addStretch(); r->addWidget(out);
        resLy->addLayout(r);
    };
    addResRow("Min cut weight:", lblCut);
    addResRow("Set A:",          lblSetA);
    addResRow("Set B:",          lblSetB);

    lblStatus = new QLabel("Load a graph and press Run.");
    lblStatus->setStyleSheet("color:#6B7280;font-size:11px;padding:3px 0;");
    resLy->addWidget(lblStatus);
    leftLy->addWidget(gres);

    // ── Research notes ────────────────────────────────────────────────────────
    QGroupBox* gnotes = new QGroupBox("Research Notes");
    QVBoxLayout* notesLy = new QVBoxLayout(gnotes);
    researchPanel = new QTextEdit;
    researchPanel->setReadOnly(true);
    researchPanel->setMaximumHeight(145);
    researchPanel->setStyleSheet("font-size:11px;color:#374151;");
    researchPanel->setHtml(
        "<b>Minimum Cut Problem</b><br>"
        "Find partition (A,B) minimizing sum of crossing edge weights.<br><br>"
        "<b>Brute Force:</b> O(2^n * m) — exact but infeasible for n&gt;25.<br>"
        "<b>Kernighan-Lin:</b> O(n<sup>2</sup> log n) per pass — fast but finds local optimum only.<br>"
        "<b>Stoer-Wagner:</b> O(n<sup>3</sup>) — exact and polynomial. Best for undirected graphs.<br><br>"
        "<b>Research finding:</b> KL + random restarts dramatically improves quality. "
        "Stoer-Wagner always finds the true global minimum."
    );
    notesLy->addWidget(researchPanel);
    leftLy->addWidget(gnotes);
    leftLy->addStretch();

    // ══ RIGHT PANEL ════════════════════════════════════════════════════════════
    QWidget*     right   = new QWidget;
    QVBoxLayout* rightLy = new QVBoxLayout(right);
    rightLy->setContentsMargins(6, 10, 10, 10);
    rightLy->setSpacing(8);

    // Graph canvas
    QGroupBox* gcanvas = new QGroupBox("Graph Visualization  (drag nodes to rearrange)");
    QVBoxLayout* canvasLy = new QVBoxLayout(gcanvas);
    graphWidget = new GraphWidget;
    canvasLy->addWidget(graphWidget);
    rightLy->addWidget(gcanvas, 3);

    // Bottom splitter: log + comparison table
    QSplitter* bot = new QSplitter(Qt::Horizontal);

    QGroupBox* glog = new QGroupBox("Algorithm Log");
    QVBoxLayout* logLy = new QVBoxLayout(glog);
    logList = new QListWidget;
    logList->setFont(QFont("Courier New", 10));
    logList->setStyleSheet(
        "QListWidget::item:selected{background:#DBEAFE;color:#1E40AF;}");
    logLy->addWidget(logList);
    bot->addWidget(glog);

    QGroupBox* gcmp = new QGroupBox("Technique Comparison");
    QVBoxLayout* cmpLy = new QVBoxLayout(gcmp);
    compareTable = new QTableWidget;
    compareTable->setColumnCount(4);
    compareTable->setHorizontalHeaderLabels({"Technique","Cut Found","Exact?","Complexity"});
    compareTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    compareTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    compareTable->setAlternatingRowColors(true);
    compareTable->setRowCount(3);
    // pre-fill static info
    auto fillCell = [this](int r, int c, const QString& s, QColor bg=QColor("#FFFFFF")) {
        auto* it = new QTableWidgetItem(s);
        it->setTextAlignment(Qt::AlignCenter);
        it->setBackground(bg);
        compareTable->setItem(r, c, it);
    };
    fillCell(0,0,"Brute Force");     fillCell(0,2,"Yes"); fillCell(0,3,"O(2^n * m)");
    fillCell(1,0,"Kernighan-Lin");   fillCell(1,2,"No (local)"); fillCell(1,3,"O(n^2 log n)");
    fillCell(2,0,"Stoer-Wagner");    fillCell(2,2,"Yes"); fillCell(2,3,"O(n^3)");
    fillCell(0,1,"—"); fillCell(1,1,"—"); fillCell(2,1,"—");
    cmpLy->addWidget(compareTable);
    bot->addWidget(gcmp);

    rightLy->addWidget(bot, 2);

    root->addWidget(left);
    root->addWidget(right);
    root->setStretchFactor(0, 1);
    root->setStretchFactor(1, 3);

    // ── Signals ───────────────────────────────────────────────────────────────
    connect(btnRun,      &QPushButton::clicked, this, &MainWindow::onRun);
    connect(btnClear,    &QPushButton::clicked, this, &MainWindow::onClear);
    connect(btnExample,  &QPushButton::clicked, this, &MainWindow::onLoadExample);
    connect(btnAddEdge,  &QPushButton::clicked, this, &MainWindow::onAddEdge);
    connect(comboAlgo,   QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onAlgoChanged);
}

// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onLoadExample() {
    // The 6-vertex example from the Task 8 document
    spinN->setValue(6);
    edgeTable->setRowCount(0);
    graph = Graph(6);

    struct E { int u, v; double w; };
    std::vector<E> edges = {
        {0,1,2},{0,2,3},{1,2,4},{1,3,1},
        {2,4,2},{3,4,3},{3,5,5},{4,5,4}
    };
    for (auto& e : edges) {
        graph.addEdge(e.u, e.v, e.w);
        int row = edgeTable->rowCount();
        edgeTable->insertRow(row);
        auto ci = [](const QString& s) {
            auto* it = new QTableWidgetItem(s);
            it->setTextAlignment(Qt::AlignCenter);
            return it;
        };
        edgeTable->setItem(row,0,ci(QString::number(e.u)));
        edgeTable->setItem(row,1,ci(QString::number(e.v)));
        edgeTable->setItem(row,2,ci(QString::number(e.w)));
    }
    graphWidget->setGraph(graph);
    graphWidget->clearResult();
    lblCut->setText("—"); lblSetA->setText("—"); lblSetB->setText("—");
    lblStatus->setText("Example loaded (6 vertices, 8 edges). Press Run.");
    logList->clear();
    setCell(0,1,"—"); setCell(1,1,"—"); setCell(2,1,"—");
}

void MainWindow::setCell(int r, int c, const QString& s, QColor bg) {
    auto* it = compareTable->item(r, c);
    if (!it) { it = new QTableWidgetItem; compareTable->setItem(r,c,it); }
    it->setText(s);
    it->setTextAlignment(Qt::AlignCenter);
    it->setBackground(bg);
}

void MainWindow::onAddEdge() {
    int    u = spinU->value();
    int    v = spinV->value();
    double w = spinW->value();
    int    n = spinN->value();

    if (u == v) {
        lblStatus->setText("Error: self-loops not allowed.");
        return;
    }
    if (u >= n || v >= n) {
        lblStatus->setText("Error: vertex index out of range for n=" + QString::number(n));
        return;
    }

    // If the graph size changed, rebuild; otherwise just add the new edge.
    // We never shrink the graph's vertex count mid-session — only grow or stay.
    if (graph.n != n) {
        // Rebuild graph at the new size, replaying all edges currently in the table.
        graph = Graph(n);
        for (int row = 0; row < edgeTable->rowCount(); row++) {
            int    eu = edgeTable->item(row,0)->text().toInt();
            int    ev = edgeTable->item(row,1)->text().toInt();
            double ew = edgeTable->item(row,2)->text().toDouble();
            // Only replay edges whose endpoints are still valid in the new size.
            if (eu < n && ev < n) graph.addEdge(eu, ev, ew);
        }
    }

    // Check for duplicate edge (same endpoints already exist)
    if (graph.adj[u][v] > 0) {
        lblStatus->setText(QString("Edge %1—%2 already exists (w=%3). Clear first to change it.")
            .arg(u).arg(v).arg(graph.adj[u][v]));
        return;
    }

    // Add the new edge into the graph and into the table.
    graph.addEdge(u, v, w);
    int row = edgeTable->rowCount();
    edgeTable->insertRow(row);
    auto ci = [](const QString& s) {
        auto* it = new QTableWidgetItem(s);
        it->setTextAlignment(Qt::AlignCenter);
        return it;
    };
    edgeTable->setItem(row,0,ci(QString::number(u)));
    edgeTable->setItem(row,1,ci(QString::number(v)));
    edgeTable->setItem(row,2,ci(QString::number(w)));

    graphWidget->setGraph(graph);
    graphWidget->clearResult();
    lblStatus->setText(QString("Edge %1—%2 (w=%3) added. Press Run.")
        .arg(u).arg(v).arg(w));
}

void MainWindow::onClear() {
    edgeTable->setRowCount(0);
    graph = Graph(spinN->value());
    graphWidget->setGraph(graph);
    graphWidget->clearResult();
    logList->clear();
    lblCut->setText("—"); lblSetA->setText("—"); lblSetB->setText("—");
    lblStatus->setText("Graph cleared.");
    setCell(0,1,"—"); setCell(1,1,"—"); setCell(2,1,"—");
}

// ─────────────────────────────────────────────────────────────────────────────
// isConnected: BFS from vertex 0; returns true if all vertices are reachable.
// ─────────────────────────────────────────────────────────────────────────────
static bool isConnected(const Graph& G) {
    if (G.n == 0) return false;
    std::vector<bool> visited(G.n, false);
    std::vector<int>  queue = {0};
    visited[0] = true;
    int front = 0;
    while (front < (int)queue.size()) {
        int u = queue[front++];
        for (int v = 0; v < G.n; v++) {
            if (!visited[v] && G.adj[u][v] > 0) {
                visited[v] = true;
                queue.push_back(v);
            }
        }
    }
    for (bool b : visited) if (!b) return false;
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onRun() {
    if (graph.n < 2) {
        lblStatus->setText("Need at least 2 vertices.");
        return;
    }

    // Verify the graph is connected (min-cut is undefined for disconnected graphs)
    if (!isConnected(graph)) {
        lblStatus->setText("Graph is not connected. Add edges to connect all vertices.");
        return;
    }

    logList->clear();
    results.clear();
    int algo = comboAlgo->currentIndex();

    if (algo == 0 || algo == 3) {
        if (graph.n > 20) {
            logList->addItem("Brute Force skipped: n > 20 would take too long.");
        } else {
            CutResult r = bruteForce(graph);
            results.push_back(r);
            for (auto& line : r.log)
                logList->addItem(QString::fromStdString(line));
            logList->addItem(""); // spacer
        }
    }
    if (algo == 1 || algo == 3) {
        CutResult r = kernighanLin(graph, spinRestarts->value());
        results.push_back(r);
        for (auto& line : r.log)
            logList->addItem(QString::fromStdString(line));
        logList->addItem("");
    }
    if (algo == 2 || algo == 3) {
        CutResult r = stoerWagner(graph);
        results.push_back(r);
        for (auto& line : r.log)
            logList->addItem(QString::fromStdString(line));
        logList->addItem("");
    }

    if (results.empty()) return;

    // Show the best result found across all algorithms
    const CutResult* best = &results[0];
    for (const auto& r : results)
        if (r.cutWeight < best->cutWeight) best = &r;

    showResult(*best);
    graphWidget->setResult(*best);

    // Update compare table
    updateCompareTable();

    lblStatus->setText(QString("Done. Best cut = %1 (by %2)")
        .arg((int)best->cutWeight)
        .arg(QString::fromStdString(best->technique)));
}

void MainWindow::showResult(const CutResult& r) {
    lblCut->setText(QString::number((int)r.cutWeight));

    QString aStr;
    for (int v : r.setA) aStr += QString::number(v) + " ";
    QString bStr;
    for (int v : r.setB) bStr += QString::number(v) + " ";
    lblSetA->setText("{" + aStr.trimmed() + "}");
    lblSetB->setText("{" + bStr.trimmed() + "}");
}

void MainWindow::updateCompareTable() {
    // Reset cut column
    setCell(0,1,"—"); setCell(1,1,"—"); setCell(2,1,"—");

    // Find the true minimum cut from all exact algorithms (Brute Force and Stoer-Wagner).
    // We use this as the reference to colour-code Kernighan-Lin.
    double exactCut = 1e18;
    for (const auto& r : results) {
        bool isExact = (r.technique.find("Brute")  != std::string::npos) ||
                       (r.technique.find("Stoer")  != std::string::npos);
        if (isExact && r.cutWeight < exactCut)
            exactCut = r.cutWeight;
    }

    for (const auto& r : results) {
        int row = -1;
        if (r.technique.find("Brute")     != std::string::npos) row = 0;
        if (r.technique.find("Kernighan") != std::string::npos) row = 1;
        if (r.technique.find("Stoer")     != std::string::npos) row = 2;
        if (row < 0) continue;

        QColor bg("#FFFFFF");
        if (exactCut < 1e17) {
            if (std::abs(r.cutWeight - exactCut) < 0.5)
                bg = QColor("#DCFCE7");  // green — matches exact minimum
            else
                bg = QColor("#FEF2F2");  // red — suboptimal
        }
        setCell(row, 1, QString::number((int)r.cutWeight), bg);
    }
}

void MainWindow::onAlgoChanged(int idx) {
    // Enable restart spinner only when KL is selected (alone or in "All")
    spinRestarts->setEnabled(idx == 1 || idx == 3);
}
