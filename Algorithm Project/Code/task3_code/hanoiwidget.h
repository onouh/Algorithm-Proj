#ifndef HANOIWIDGET_H
#define HANOIWIDGET_H

#include <QWidget>
#include <QVector>
#include <QStack>
#include "hanoi.h"

class HanoiWidget : public QWidget {
    Q_OBJECT
public:
    explicit HanoiWidget(QWidget *parent = nullptr);

    void reset(int numDisks);
    bool applyMove(const Move &mv);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    int                  m_numDisks;
    QVector<QStack<int>> m_pegs;   // 4 pegs, each stack: bottom=largest

    QRect  diskRect(int pegIdx, int slot, int diskNum) const;
    QColor diskColor(int diskNum) const;
};

#endif // HANOIWIDGET_H
