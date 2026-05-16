#include "hanoiwidget.h"
#include <QPainter>
#include <QFontMetrics>

HanoiWidget::HanoiWidget(QWidget *parent)
    : QWidget(parent), m_numDisks(8), m_pegs(4)
{
    setMinimumSize(700, 380);
    // Auto-load 8 disks so they show immediately on startup
    for (int d = 8; d >= 1; --d)
        m_pegs[0].push(d);
}

void HanoiWidget::reset(int numDisks)
{
    m_numDisks = numDisks;
    for (auto &p : m_pegs) p.clear();
    for (int d = numDisks; d >= 1; --d)
        m_pegs[0].push(d);
    update();
}

bool HanoiWidget::applyMove(const Move &mv)
{
    if (m_pegs[mv.from].isEmpty())           return false;
    if (m_pegs[mv.from].top() != mv.disk)    return false;
    if (!m_pegs[mv.to].isEmpty() && m_pegs[mv.to].top() < mv.disk) return false;
    m_pegs[mv.from].pop();
    m_pegs[mv.to].push(mv.disk);
    update();
    return true;
}

QColor HanoiWidget::diskColor(int diskNum) const
{
    // Exact colors matching the original screenshot (disk 1=smallest, 8=largest)
    static const QColor palette[] = {
        QColor(231,  76,  60),  // 1  red
        QColor(230, 126,  34),  // 2  orange-red
        QColor(241, 196,  15),  // 3  yellow
        QColor( 39, 174,  96),  // 4  green
        QColor( 52, 152, 219),  // 5  blue
        QColor(155,  89, 182),  // 6  purple
        QColor( 26, 188, 156),  // 7  teal
        QColor(236,  72, 153),  // 8  pink/magenta  (matches screenshot)
        QColor(243, 156,  18),  // 9  amber
        QColor( 46, 204, 113),  // 10 emerald
        QColor( 41, 128, 185),  // 11 steel blue
        QColor(142,  68, 173),  // 12 dark purple
        QColor( 22, 160, 133),  // 13 dark teal
        QColor(192,  57,  43),  // 14 dark red
        QColor(211,  84,   0),  // 15 dark orange
        QColor(127, 179,  71),  // 16 olive green
    };
    int idx = qMax(0, qMin(diskNum - 1, 15));
    return palette[idx];
}

QRect HanoiWidget::diskRect(int pegIdx, int slot, int diskNum) const
{
    int W = width();
    int H = height();

    int baseY    = H - 52;   // where the base platform sits
    int pegZoneW = W / 4;
    int pegCX    = pegIdx * pegZoneW + pegZoneW / 2;

    int dh = 20;

    // Width: smallest = 30px, largest fills ~80% of zone
    int minW = 30;
    int maxW = (int)(pegZoneW * 0.80);
    int dw   = minW + (int)((double)(diskNum - 1) / qMax(1, m_numDisks - 1) * (maxW - minW));

    int dy = baseY - (slot + 1) * dh;
    int dx = pegCX - dw / 2;
    return QRect(dx, dy, dw, dh);
}

void HanoiWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int W = width();
    int H = height();

    // ── Background — matches original screenshot navy
    p.fillRect(rect(), QColor(15, 20, 40));

    int baseY    = H - 52;
    int pegZoneW = W / 4;
    int baseH    = 8;

    // ── Base bar ─────────────────────────────────────────────────────────────
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(80, 80, 90));
    p.drawRect(0, baseY, W, baseH);

    // ── Pegs + labels ────────────────────────────────────────────────────────
    const QString pegLabels[] = {"Peg 1", "Peg 2", "Peg 3", "Peg 4"};
    int pegW  = 10;
    int pegH  = baseY - 20;

    QFont labelFont("Segoe UI", 9);
    p.setFont(labelFont);

    for (int i = 0; i < 4; ++i) {
        int cx = i * pegZoneW + pegZoneW / 2;

        // Rod — grey like in screenshot
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(160, 160, 170));
        p.drawRect(cx - pegW / 2, baseY - pegH, pegW, pegH);

        // Label below base
        p.setPen(QColor(180, 180, 190));
        QRect lr(i * pegZoneW, baseY + baseH + 4, pegZoneW, 24);
        p.drawText(lr, Qt::AlignCenter, pegLabels[i]);
    }

    // ── Disks ────────────────────────────────────────────────────────────────
    QFont diskFont("Segoe UI", 8, QFont::Bold);
    p.setFont(diskFont);

    for (int pi = 0; pi < 4; ++pi) {
        const QStack<int> &stk = m_pegs[pi];
        for (int slot = 0; slot < stk.size(); ++slot) {
            int dn = stk[slot];
            QRect r = diskRect(pi, slot, dn);
            QColor c = diskColor(dn);

            // Body
            p.setPen(Qt::NoPen);
            p.setBrush(c);
            p.drawRoundedRect(r, 4, 4);

            // Number label
            p.setPen(QColor(20, 20, 20));
            p.drawText(r, Qt::AlignCenter, QString::number(dn));
        }
    }
}
