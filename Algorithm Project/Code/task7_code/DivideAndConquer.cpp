// ALgoProj.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include <iostream>
#include <algorithm>
using namespace std;
struct job {
	int t;
	int d;
	int p;
};
void mergeByDeadline(job arr[], int left, int mid, int right) {
	int n1 = mid - left + 1;
	int n2 = right - mid;

	job* L = new job[n1];
	job* R = new job[n2];

	for (int i = 0; i < n1; i++) L[i] = arr[left + i];
	for (int i = 0; i < n2; i++) R[i] = arr[mid + 1 + i];

	int i = 0, j = 0, k = left;
	while (i < n1 && j < n2) {
		if (L[i].d <= R[j].d)    
			arr[k++] = L[i++];
		else
			arr[k++] = R[j++];
	}
	while (i < n1) arr[k++] = L[i++];
	while (j < n2) arr[k++] = R[j++];

	delete[] L;
	delete[] R;
}


void mergeSortByDeadline(job arr[], int left, int right) {
	if (left >= right) return;

	int mid = (left + right) / 2;
	mergeSortByDeadline(arr, left, mid);
	mergeSortByDeadline(arr, mid + 1, right);
	mergeByDeadline(arr, left, mid, right);
}
void mergeByPenalty(job arr[], int left, int mid, int right) {
	int n1 = mid - left + 1;
	int n2 = right - mid;

	job* L = new job[n1];
	job* R = new job[n2];

	for (int i = 0; i < n1; i++) L[i] = arr[left + i];
	for (int i = 0; i < n2; i++) R[i] = arr[mid + 1 + i];

	int i = 0, j = 0, k = left;
	while (i < n1 && j < n2) {
		if (L[i].p >= R[j].p)    // compare by penalty descending
			arr[k++] = L[i++];
		else
			arr[k++] = R[j++];
	}
	while (i < n1) arr[k++] = L[i++];
	while (j < n2) arr[k++] = R[j++];

	delete[] L;
	delete[] R;
}

void mergeSortByPenalty(job arr[], int left, int right) {
	if (left >= right) return;

	int mid = (left + right) / 2;
	mergeSortByPenalty(arr, left, mid);
	mergeSortByPenalty(arr, mid + 1, right);
	mergeByPenalty(arr, left, mid, right);
}


bool canfit(job jobs[], int n) {
	int time = 0, penalty = 0;
	mergeSortByDeadline(jobs, 0, n - 1);
	for (int i = 0; i < n; i++) {

		time += jobs[i].t;
		if (jobs[i].d < time) {
			penalty += jobs[i].p;
		}

	}
	return penalty == 0;
}
void merge(job jobs[], int size, job onTime[], int& ontimesize, job late[], int& lateSize) {
	mergeSortByPenalty(jobs, 0, size - 1);

	for (int i = 0; i < size; i++) {
		job* maybe= new job[size];

		for (int j = 0; j < ontimesize; j++) {
			maybe[j] = onTime[j];
		}
		maybe[ontimesize] = jobs[i];

		if (canfit(maybe, ontimesize + 1)) {
			onTime[ontimesize++] = jobs[i];
		}
		else {
			late[lateSize++] = jobs[i];
		}
		delete[] maybe;
	}
	
}
void jobschegulingDC(job jobs[], int size, job onTime[], int& ontimesize, job late[], int& latesize) {
	if (size == 1) {
		if (jobs[0].t <= jobs[0].d) {
			onTime[0] = jobs[0];
			ontimesize = 1;
			latesize = 0;
		}
		else {
			late[0] = jobs[0];
			latesize = 1;
			ontimesize = 0;
		}
		return;
	}
	int mid = size / 2;
	job* leftOn = new job[mid];
	job* leftLate = new job[mid];
	int  leftOnSize = 0;
	int  leftLateSize = 0;


	job* rightOn = new job[size - mid];
	job* rightLate = new job[size - mid];
	int  rightOnSize = 0;
	int  rightLateSize = 0;

	jobschegulingDC(jobs, mid, leftOn, leftOnSize, leftLate, leftLateSize);
	jobschegulingDC(jobs + mid, size - mid, rightOn, rightOnSize, rightLate, rightLateSize);

	int combinedSize = leftOnSize + rightOnSize;
	job* combined = new job[combinedSize];
	for (int i = 0; i < leftOnSize; i++) {
		combined[i] = leftOn[i];
	}
	for (int i = 0; i < rightOnSize; i++) {
		combined[leftOnSize + i] = rightOn[i];
	}
	latesize = 0;
	for (int i = 0; i < leftLateSize; i++) {
		late[latesize++] = leftLate[i];
	}
	for (int i = 0; i < rightLateSize; i++) {
		late[latesize++] = rightLate[i];
	}
	job* newOn = new job[combinedSize];
	job* newLate = new job[combinedSize];
	int  newOnSize = 0, newLateSize = 0;
	merge(combined, combinedSize, newOn, newOnSize, newLate, newLateSize);

	ontimesize = 0;
	for (int i = 0; i < newOnSize; i++) {
		onTime[ontimesize++] = newOn[i];
	}
	for (int i = 0; i < newLateSize; i++) {
		late[latesize++] = newLate[i];
	}
	delete[] leftOn;
	delete[] leftLate;
	delete[] rightOn;
	delete[] rightLate;
}

int main() {

	int n;
	cout << "Enter number of jobs: ";
	cin >> n;

	job* jobs = new job[n];
	for (int i = 0; i < n; i++) {
		cout << "Job " << i + 1 << " (t d p): ";
		cin >> jobs[i].t >> jobs[i].d >> jobs[i].p;
	}

	
	job* onTime = new job[n];
	job* late = new job[n];
	int onSize = 0, lateSize = 0;

	jobschegulingDC(jobs, n, onTime, onSize, late, lateSize);

	int totalPenalty = 0;
	for (int i = 0; i < lateSize; i++)
		totalPenalty += late[i].p;

	cout << "\nOn time jobs: ";
	for (int i = 0; i < onSize; i++)
		cout << "(t=" << onTime[i].t << " d=" << onTime[i].d << " p=" << onTime[i].p << ") ";

	cout << "\nLate jobs: ";
	for (int i = 0; i < lateSize; i++)
		cout << "(t=" << late[i].t << " d=" << late[i].d << " p=" << late[i].p << ") ";

	cout << "\nMinimum penalty: " << totalPenalty << endl;

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
