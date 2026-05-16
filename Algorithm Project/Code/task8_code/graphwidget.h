#pragma once
#include <QWidget>
#include <QVector>
#include <QPointF>
#include <QColor>
#include "mincut.h"

class GraphWidget : public QWidget {
    Q_OBJECT
public:
    explicit GraphWidget(QWidget* parent = nullptr);

    void setGraph(const Graph& g);
    void setResult(const CutResult& r);
    void clearResult();

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    QSize sizeHint() const override;

private:
    Graph             graph;
    CutResult         result;
    bool              hasResult = false;
    QVector<QPointF>  positions;
    int               dragNode  = -1;
    QPointF           dragOffset;

    void layoutCircle();
    int  nodeAt(QPointF pos) const;
};
