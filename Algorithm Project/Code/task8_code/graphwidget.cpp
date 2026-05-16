#include "graphwidget.h"
#include <QPainter>
#include <QPen>
#include <QFont>
#include <QMouseEvent>
#include <cmath>

GraphWidget::GraphWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(400, 340);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
}

QSize GraphWidget::sizeHint() const { return QSize(520, 380); }

void GraphWidget::setGraph(const Graph& g) {
    graph     = g;
    hasResult = false;
    layoutCircle();
    update();
}

void GraphWidget::setResult(const CutResult& r) {
    result    = r;
    hasResult = true;
    update();
}

void GraphWidget::clearResult() {
    hasResult = false;
    update();
}

// ── Layout: arrange vertices in a circle centred on the widget ────────────────
// Called on setGraph and on resize so that the layout always uses the real
// widget dimensions (at construction time the widget has no size yet).
void GraphWidget::layoutCircle() {
    if (graph.n == 0) return;

    // Use the current widget size, or fall back to sizeHint if not yet shown.
    int w = (this->width()  > 0) ? this->width()  : sizeHint().width();
    int h = (this->height() > 0) ? this->height() : sizeHint().height();

    double cx = w / 2.0;
    double cy = h / 2.0;
    double r  = std::min(cx, cy) * 0.65;

    positions.resize(graph.n);
    for (int i = 0; i < graph.n; i++) {
        double angle = 2.0 * M_PI * i / graph.n - M_PI / 2.0;
        positions[i] = QPointF(cx + r * std::cos(angle),
                               cy + r * std::sin(angle));
    }
}

// ── Re-layout on resize so nodes don't cluster in one corner ─────────────────
void GraphWidget::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    // Only redo the layout if no drag is in progress and we have a graph.
    if (dragNode < 0 && graph.n > 0) layoutCircle();
    update();
}

int GraphWidget::nodeAt(QPointF pos) const {
    for (int i = 0; i < (int)positions.size(); i++) {
        QPointF d = pos - positions[i];
        if (d.x()*d.x() + d.y()*d.y() < 22*22) return i;
    }
    return -1;
}

void GraphWidget::mousePressEvent(QMouseEvent* e) {
    dragNode = nodeAt(e->pos());
    if (dragNode >= 0) dragOffset = e->pos() - positions[dragNode];
}
void GraphWidget::mouseMoveEvent(QMouseEvent* e) {
    if (dragNode >= 0) {
        positions[dragNode] = e->pos() - dragOffset;
        update();
    }
}
void GraphWidget::mouseReleaseEvent(QMouseEvent*) { dragNode = -1; }

void GraphWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor("#1A1A2E"));

    if (graph.n == 0) {
        p.setPen(QColor("#555577"));
        p.setFont(QFont("Arial", 12));
        p.drawText(rect(), Qt::AlignCenter, "No graph loaded.\nAdd vertices and edges, then click Run.");
        return;
    }

    // Which set is each vertex in?
    // 0 = unassigned, 1 = set A (blue), 2 = set B (orange)
    std::vector<int> side(graph.n, 0);
    if (hasResult) {
        for (int v : result.setA) if (v < graph.n) side[v] = 1;
        for (int v : result.setB) if (v < graph.n) side[v] = 2;
    }

    // ── draw edges ───────────────────────────────────────────────────────────
    for (int i = 0; i < graph.n; i++) {
        for (int j = i+1; j < graph.n; j++) {
            if (graph.adj[i][j] <= 0) continue;

            bool isCut = hasResult && side[i] != 0 && side[j] != 0 && side[i] != side[j];

            QColor color;
            float  lwidth;
            if (isCut) {
                color  = QColor("#FF4444");
                lwidth = 3.0f;
            } else {
                color  = QColor("#556677");
                lwidth = 1.5f;
            }

            p.setPen(QPen(color, lwidth));
            p.drawLine(positions[i], positions[j]);

            // edge weight label at midpoint
            QPointF mid = (positions[i] + positions[j]) / 2.0;
            p.setPen(isCut ? QColor("#FF8888") : QColor("#8899AA"));
            p.setFont(QFont("Arial", 9));
            p.drawText(mid + QPointF(4, -4),
                       QString::number(graph.adj[i][j], 'g', 3));
        }
    }

    // ── cut weight label in top-right corner ──────────────────────────────────
    if (hasResult) {
        p.setPen(QColor("#FF6666"));
        p.setFont(QFont("Arial", 11, QFont::Bold));
        p.drawText(rect().adjusted(0, 8, -8, 0),
                   Qt::AlignTop | Qt::AlignRight,
                   QString("Cut = %1").arg((int)result.cutWeight));
    }

    // ── draw nodes ────────────────────────────────────────────────────────────
    const int R = 18;
    for (int i = 0; i < graph.n; i++) {
        QColor fill, border;
        if (!hasResult || side[i] == 0) {
            fill   = QColor("#334455");
            border = QColor("#7788AA");
        } else if (side[i] == 1) {
            fill   = QColor("#1A3A6A");
            border = QColor("#4488DD");
        } else {
            fill   = QColor("#5A2A00");
            border = QColor("#DD8833");
        }

        p.setBrush(fill);
        p.setPen(QPen(border, 2));
        p.drawEllipse(positions[i], R, R);

        // vertex label
        p.setPen(Qt::white);
        p.setFont(QFont("Arial", 10, QFont::Bold));
        p.drawText(QRectF(positions[i].x()-R, positions[i].y()-R, 2*R, 2*R),
                   Qt::AlignCenter, QString::number(i));
    }

    // ── legend ────────────────────────────────────────────────────────────────
    if (hasResult) {
        int lx = 12, ly = height() - 60;
        p.setFont(QFont("Arial", 9));

        p.setBrush(QColor("#1A3A6A"));
        p.setPen(QPen(QColor("#4488DD"), 1.5));
        p.drawEllipse(lx, ly, 12, 12);
        p.setPen(QColor("#AABBCC"));
        p.drawText(lx+16, ly+11, "Set A");

        ly += 20;
        p.setBrush(QColor("#5A2A00"));
        p.setPen(QPen(QColor("#DD8833"), 1.5));
        p.drawEllipse(lx, ly, 12, 12);
        p.setPen(QColor("#AABBCC"));
        p.drawText(lx+16, ly+11, "Set B");

        ly += 20;
        p.setPen(QPen(QColor("#FF4444"), 2));
        p.drawLine(lx, ly+6, lx+12, ly+6);
        p.setPen(QColor("#AABBCC"));
        p.drawText(lx+16, ly+11, "Cut edges");
    }
}
