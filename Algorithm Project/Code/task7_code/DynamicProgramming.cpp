#include <iostream>
using namespace std;
const int max_n = 105;
const int max_T = 10005;
const int INF = 1e9;
struct Job {
    int t, d, p;
};
Job jobs[max_n];
int dp[max_n][max_T];
void merge(Job arr[], int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    Job L[max_n], R[max_n];
    for (int i = 0; i < n1; i++)
        L[i] = arr[left + i];
    for (int j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];
    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i].d <= R[j].d) {
            arr[k++] = L[i++];
        }
        else {
            arr[k++] = R[j++];
        }
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
}
void mergeSort(Job arr[], int left, int right) {
    if (left >= right) return;
    int mid = (left + right) / 2;
    mergeSort(arr, left, mid);
    mergeSort(arr, mid + 1, right);
    merge(arr, left, mid, right);
}
void printJobs(int n) {
    cout << "\nJobs after sorting by deadline:\n";
    for (int i = 0; i < n; i++) {
        cout << "Job " << i + 1 << ": ";
        cout << "t=" << jobs[i].t
            << " d=" << jobs[i].d
            << " p=" << jobs[i].p << endl;
    }
}
void printDP(int n, int sumT) {
    cout << "\nDP Table (rows = jobs, cols = time):\n\n";
    cout << "     ";
    for (int T = 0; T <= sumT; T++) {
        cout << T << "\t";
    }
    cout << endl;
    for (int i = 0; i <= n; i++) {
        cout << "i=" << i << "  ";
        for (int T = 0; T <= sumT; T++) {
            if (dp[i][T] >= INF)
                cout << "INF\t";
            else
                cout << dp[i][T] << "\t";
        }
        cout << endl;
    }
}
int main() {
    int n;
    cout << "Enter number of jobs:\n";
    cin >> n;
    cout << "Enter each job as: time deadline penalty\n";
    for (int i = 0; i < n; i++) {
        cin >> jobs[i].t >> jobs[i].d >> jobs[i].p;
    }
    mergeSort(jobs, 0, n - 1);
    printJobs(n);
    int sumT = 0;
    for (int i = 0; i < n; i++) {
        sumT += jobs[i].t;
    }
    for (int i = 0; i <= n; i++) {
        for (int T = 0; T <= sumT; T++) {
            dp[i][T] = INF;
        }
    }
    dp[0][0] = 0;
    for (int i = 1; i <= n; i++) {
        for (int T = 0; T <= sumT; T++) {
            dp[i][T] = dp[i - 1][T] + jobs[i - 1].p; //late
            if (T >= jobs[i - 1].t && T <= jobs[i - 1].d) { //on time
                dp[i][T] = min(dp[i][T],
                    dp[i - 1][T - jobs[i - 1].t]);
            }
        }
    }
    printDP(n, sumT);
    int ans = INF;
    for (int T = 0; T <= sumT; T++) {
        ans = min(ans, dp[n][T]);
    }
    cout << "\nMinimum total penalty = " << ans << endl;
    return 0;
}