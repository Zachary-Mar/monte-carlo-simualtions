#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void shuffle(int *array, int n) { // void means to return notihng at the end and also shuffle means to randomise. int n means to set the parameter for the array ie 100
	for (int i=n-1; i>0; i--) { // n -100 means to start at the last slot, i--  means to count down from n and the i>0 means to stop at -100
		int j = rand() % (i+1); // rand() means to pick a random number and divide by i+1. The modulo operator means to give the remainder, as all remainders are 0-99
		// overall this locks into place a perfectly random shuffule.
		int temp = array [i];
		array[i] = array [j];
		array [j] = temp;
		// first line makes the temp value = i, second line makes i the vaLue of j and third line makes j tmep's value, hence i and j have swapped values.
		// this is called the fisher yates shuffle

	}
}

int run_single_trial(int n_applicants, int k_cutoff) {
	int applicants[100]; // create an array of 100 boxes
	for (int i=0; i<100; i++) { // makes it so that box 0 displays are slot 1
		applicants[i]=i+1; // i starts at 0 so i=0 corrosponds to box applicant 100
	}


	shuffle(applicants, 100); // this executes the shuffle command that I have made int he previous paragraph.



	int best_in_sample = applicants[0]; // there is some interger in the current array that is the best in sample, basically setting the benchmark
	for (int i=0; i < k_cutoff; i++) { // we count upwards from 0 until we reach the cutoff value - 1
		if (applicants[i] > best_in_sample) {
			best_in_sample = applicants[i]; // if a greater value appears in the sample than the best in sample variable is redetermined to be the new one, ie. tells me te current best in the sample
		}
	}

	for (int i = k_cutoff; i < 100; i++) { // starts from the cutt off and scan supwards to 100
		if (applicants[i] > best_in_sample) { // determines if i is greater and the best in sample
			if(applicants[i] == 100) { // determines if i is equal to 100
				return 1; // this is the condition for the win
			}
			else {
				return 0; // if not this means lose
			}
		}
	}

	if (applicants[99] == 100)
		return 1;
	else return 0;
}



int main () {

	srand(time(NULL));

	int best_k = 0;
	double max_win_rate = 0.0; // max_win_rate is my chosen variable which is a percentage

	printf("Cutoff_k, Success_rate\n");

	for (int k=0; k<100; k++) {
		int wins = 0;
		for (int trial = 0; trial < 10000; trial++) {
			wins += run_single_trial(100,k); // takes into account the current amount of wins and keeps affing
		}


		double win_rate = (double) wins / 10000.0;

		if (win_rate > max_win_rate) {
			max_win_rate = win_rate;
			best_k = k;
		}
	}


	printf("\n--- SUMMARY ---\n");
	printf("Optimal Cuttoff k*: Reject first %d candidates\n", best_k);
	printf("Max Win Rate: %.2f%%\n", max_win_rate * 100.0);


	return 0;
}






