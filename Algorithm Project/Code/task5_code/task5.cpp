#include <iostream>
#include <vector>
using namespace std;

vector<int> shootSequence(int l, int r) {
    if (l == r) {
        return { l };
    }

    int mid = (l + r) / 2;

    vector<int> left = shootSequence(l, mid);
    vector<int> right = shootSequence(mid + 1, r);

    vector<int> result;
    int i = 0;

    while (i < left.size() || i < right.size()) {
        if (i < left.size())
            result.push_back(left[i]);

        if (i < right.size())
            result.push_back(right[i]);

        i++;
    }

    return result;
}

int main() {
    int n;
    cout << "Enter number of positions: ";
    cin >> n;

    vector<int> sequence = shootSequence(0, n - 1);

    cout << "Shooting sequence:\n";
    for (int x : sequence) {
        cout << x << " ";
    }

    return 0;
}
