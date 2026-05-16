#ifndef HANOI_H
#define HANOI_H

#include <QVector>
#include <QString>
#include <QStack>

struct Move {
    int disk;   // 1 = smallest
    int from;   // 0-based peg
    int to;     // 0-based peg
};

class HanoiSolver {
public:
    explicit HanoiSolver(int n);

    void buildFrameStewartTable();
    void solve(const QString &algorithm);

    const QVector<Move> &moves()    const { return m_moves; }
    int optimalMoves()              const { return m_dp.isEmpty() ? 0 : m_dp[m_n]; }
    int optimalK()                  const { return m_k.isEmpty()  ? 0 : m_k[m_n];  }
    int optimalK(int n)             const { return (n < m_k.size()) ? m_k[n] : 0;   }
    int dpValue(int n)              const { return (n < m_dp.size()) ? m_dp[n] : 0; }
    int numDisks()                  const { return m_n; }

    // Returns empty string if all moves are legal, else description of first error
    QString validate() const;

private:
    int            m_n;
    QVector<int>   m_dp;
    QVector<int>   m_k;
    QVector<Move>  m_moves;

    void frameStewart4(int n, int src, int dst, int h1, int h2, int diskBase);
    void hanoi3(int n, int src, int dst, int aux, int diskBase);
};

#endif // HANOI_H
