#include "hanoi.h"
#include <climits>

HanoiSolver::HanoiSolver(int n) : m_n(n)
{
    buildFrameStewartTable();
}

void HanoiSolver::buildFrameStewartTable()
{
    m_dp.resize(m_n + 1, 0);
    m_k.resize(m_n + 1, 0);
    m_dp[0] = 0; m_k[0] = 0;
    if (m_n < 1) return;
    m_dp[1] = 1; m_k[1] = 0;

    for (int n = 2; n <= m_n; ++n) {
        long long best = LLONG_MAX;
        int bestK = 1;
        for (int k = 1; k < n; ++k) {
            long long t = 2LL * m_dp[k] + ((1LL << (n - k)) - 1LL);
            if (t < best) { best = t; bestK = k; }
        }
        m_dp[n] = (int)best;
        m_k[n]  = bestK;
    }
}

void HanoiSolver::hanoi3(int n, int src, int dst, int aux, int diskBase)
{
    if (n <= 0) return;
    hanoi3(n - 1, src, aux, dst, diskBase - 1);
    m_moves.append({diskBase, src, dst});
    hanoi3(n - 1, aux, dst, src, diskBase - 1);
}

void HanoiSolver::frameStewart4(int n, int src, int dst, int h1, int h2, int diskBase)
{
    if (n <= 0) return;
    if (n == 1) { m_moves.append({diskBase, src, dst}); return; }

    int k       = m_k[n];
    int topBase = diskBase - (n - k);  // largest label among the top-k disks

    // Step 1: move top k disks to h1 using all 4 pegs
    frameStewart4(k,     src, h1, dst, h2, topBase);
    // Step 2: move bottom (n-k) disks to dst using 3 pegs (h1 is frozen)
    hanoi3(n - k,        src, dst, h2,    diskBase);
    // Step 3: move k disks from h1 to dst using all 4 pegs
    frameStewart4(k,     h1, dst, src, h2, topBase);
}

void HanoiSolver::solve(const QString &algorithm)
{
    m_moves.clear();
    if (algorithm.contains("3-peg") || algorithm.contains("Classic")) {
        hanoi3(m_n, 0, 3, 1, m_n);
    } else {
        frameStewart4(m_n, 0, 3, 1, 2, m_n);
    }
}

QString HanoiSolver::validate() const
{
    QVector<QStack<int>> pegs(4);
    for (int d = m_n; d >= 1; --d) pegs[0].push(d);

    for (int i = 0; i < m_moves.size(); ++i) {
        const Move &mv = m_moves[i];
        if (pegs[mv.from].isEmpty())
            return QString("Move %1: peg %2 is empty").arg(i+1).arg(mv.from+1);
        if (pegs[mv.from].top() != mv.disk)
            return QString("Move %1: wrong disk on peg %2").arg(i+1).arg(mv.from+1);
        if (!pegs[mv.to].isEmpty() && pegs[mv.to].top() < mv.disk)
            return QString("Move %1: ILLEGAL – disk %2 on disk %3")
                       .arg(i+1).arg(mv.disk).arg(pegs[mv.to].top());
        pegs[mv.from].pop();
        pegs[mv.to].push(mv.disk);
    }
    if (pegs[3].size() != m_n)
        return QString("Final: peg 4 has %1 disks, expected %2")
                   .arg(pegs[3].size()).arg(m_n);
    return {};
}
