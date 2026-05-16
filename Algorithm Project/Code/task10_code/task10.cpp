#include <iostream>
#include <vector>
using namespace std;

void printState(const vector<char>& coins) {
    cout << "State: ";
    for (char c : coins) cout << c << " ";
    cout << endl;
}

bool isAllGaps(const vector<char>& coins) {
    for (char c : coins)
        if (c != '-') return false;
    return true;
}

void flip(char& coin) {
    if (coin == 'H') coin = 'T';
    else if (coin == 'T') coin = 'H';
}

int countHeads(const vector<char>& coins) {
    int count = 0;
    for (char c : coins)
        if (c == 'H') count++;
    return count;
}

int findLeftmostH(const vector<char>& coins) {
    for (int i = 0; i < coins.size(); i++)
        if (coins[i] == 'H') return i;
    return -1;
}

void processRemoval(vector<char>& coins, int index) {
    int n = coins.size();

    coins[index] = '-';

    if (index + 1 < n && coins[index + 1] != '-')
        flip(coins[index + 1]);

    if (index - 1 >= 0 && coins[index - 1] != '-')
        flip(coins[index - 1]);
}

int main() {

    int n;
    cout << "Enter number of coins: ";
    cin >> n;

    vector<char> coins(n);

    cout << "Enter coins (H/T): ";
    for (int i = 0; i < n; i++) {
        cin >> coins[i];

        // normalize input
        if (coins[i] == 'h') coins[i] = 'H';
        if (coins[i] == 't') coins[i] = 'T';
    }

    cout << "\n--- COIN PUZZLE AUTO SOLVER ---\n";

    // Check solvability
    int heads = countHeads(coins);
    if (heads % 2 == 0) {
        cout << "Unsolvable! Even number of heads.\n";
        return 0;
    }

    int moves = 0;

    while (!isAllGaps(coins)) {
        printState(coins);

        int index = findLeftmostH(coins);

        if (index == -1) {
            cout << "Deadlock reached (unexpected).\n";
            break;
        }

        cout << "Removing H at index: " << index << endl;

        processRemoval(coins, index);
        moves++;
    }

    printState(coins);
    cout << "\nSUCCESS: All coins removed!\n";
    cout << "Total moves = " << moves << endl;

    return 0;
}