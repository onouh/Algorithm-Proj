// Task7BruteForce.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <algorithm>
#include <climits>
using namespace std;
struct job {
    int t;
    int d;
    int p;
};
int calcPenalty(job jobs[], int order[], int n) {
    int time = 0, penlty = 0;
    for (int i = 0; i < n; i++) {
        int currindex = order[i];
        time = time + jobs[currindex].t;
        if (time > jobs[currindex].d) {
            penlty += jobs[currindex].p;
        }
    }
    return penlty;
}
void permutation(job jobs[], int currindex, int& best, int order[], int bestord[], int n) {
    if (currindex == n) {
        int pen = calcPenalty(jobs, order, n);
        if (pen < best) {
            best = pen;
            for (int i = 0; i < n; i++) bestord[i] = order[i];
        }
        return;
    }
    for (int i = currindex; i < n; i++) {
        swap(order[currindex], order[i]);
        permutation(jobs, currindex + 1, best, order, bestord, n);
        swap(order[currindex], order[i]);
    }
}
void jobSchedulingbrute(job jobs[], int n) {
    int bestp = INT_MAX;
    int* order = new int[n];
    for (int i = 0; i < n; i++) {
        order[i] = i;
    }
    int* best = new int[n];
    permutation(jobs, 0, bestp, order, best, n);
    cout << "Min penalty =" << bestp << endl;
    cout << "Optimal order :";
    for (int i = 0; i < n; i++) {
        cout << "Job " << best[i] + 1 << " ";
    }
    cout << endl;




    delete[] order;
    delete[] best;
}
int main()
{
    int n;
    cout << "Enter number of jobs: ";
    cin >> n;

    job* jobs = new job[n];
    for (int i = 0; i < n; i++) {
        cout << "Job " << i + 1 << " (t d p): ";
        cin >> jobs[i].t >> jobs[i].d >> jobs[i].p;
    }

    jobSchedulingbrute(jobs, n);

    delete[] jobs;
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
