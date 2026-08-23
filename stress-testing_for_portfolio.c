#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define NUMBER_OF_SIMULATIONS 10000.0 //number of monte carlo simualtions 
#define TRADING_DAYS 42 //number of trading days simulation will take place on 
#define INITAL_PRINCIPAL 10000.0 //starting protfolio value

//Box muller transform to make Guassian noise which simulates likely realisitic price movement goal is to generate random numbers with mean 0 and std dev 1

double generate_gaussian_noise () {
	double u1 = ((double)rand() + 1)/((double)RAND_MAX + 2); // both u1 and u2 are going to be decimal numbers
	double u2 = ((double)rand() + 1)/((double)RAND_MAX + 2);


	//box muller transform
	return sqrt(-2 * log(u1)) * cos(2 * M_PI * u2);
	// the sqrt part acts like the distance from 0 (the mean) and cos makes it so movement is both negative and positive, -2 is selected as the constant inside the sqrt.


	// reasoning for -2 in sqrt is to make the std dev = 1. For this to happen the var also = 1 so therefore sqrt^2 * cos^2 must on avg = 1
	// note that log on average is 1 which is found thorugh intergation by parts
	// the average of cos^2 is 0.5 (found by expanding, known cos(x) avg is 0) and thus the avg of sqrt(2^2)=2 therefore var = 1


	// all that matters is that the std dev of this is 1 and the mean is 0
}

// compaison of outcomes (using qsort)
// if diff is positive outcome a had greater profit than b, if it is negative than b is better than a
int compare_doubles(const void*a, const void*b) {
	double diff = (*(double*)a - *(double*)b);
	if (diff > 0) return 1;
	if (diff < 0) return -1;
	return 0;
}

void run_stress_test (const char *scenario_name, double annual_drift, double annual_volatility) {
	double *pnl_array = (double* )malloc(NUMBER_OF_SIMULATIONS * sizeof(double));
	if (pnl_array == NULL) {
		printf("memory allocation failed\n");
		return;
	}

	double dt = 1.0/252.0; // assuming 252 trading days in a year
	double daily_drift = (annual_drift - 0.5 * annual_volatility * annual_volatility) * dt; // accouting for volatiltity drag, can prove by S t+1 = S t * e^x and finding x through taylor seris expansion
	double daily_volatiltiy = annual_volatility * sqrt(dt); // since std dev scales linearly with time and std = var^2 therefore the to find the daily must take the sqrt of time


	// running 10000 or however many simulations I want simulations
	for (int sim = 0; sim < NUMBER_OF_SIMULATIONS; sim++) {
		double portfolio_value = INITAL_PRINCIPAL;

		for (int day = 0; day < TRADING_DAYS; day++) {
			double z = generate_gaussian_noise(); // for each day model the GBM, ie. calling the box muller
			portfolio_value *= exp(daily_drift + daily_volatiltiy * z); // daily growth is random scaled to the guassian noise function at the start
		}
		pnl_array[sim] = portfolio_value - INITAL_PRINCIPAL;
	}

	qsort(pnl_array, NUMBER_OF_SIMULATIONS, sizeof(double), compare_doubles); // sorting my outcomes in ascending order

	int index_95 = (0.05 * NUMBER_OF_SIMULATIONS); // bottom 5th percentile
	int index_99 = (0.01 * NUMBER_OF_SIMULATIONS); // bottom 1st percentile

	double var_95 = -pnl_array[index_95]; // 95% Value at Risk
	double var_99 = -pnl_array[index_99]; // 99% Value at Risk
	double max_loss = -pnl_array[0];  // Maximum single worst loss out of 100,000


	//calculting for the worst case scenario what the shortfall will be (worst 1% outcomes)
	double tail_sum = 0.0;
	for (int i = 0; i < index_99; i++) {
		tail_sum += pnl_array[i];
	}
	double cvar_99 = -(tail_sum / index_99);



	// Print the resulults of all stress tests
	printf("================================================================\n");
	printf(" SCENARIO: %s\n", scenario_name);
	printf(" Annual Drift (mu): %.1f%% | Annual Volatility (sigma): %.1f%%\n", annual_drift * 100.0, annual_volatility * 100.0);
	printf("----------------------------------------------------------------\n");
	printf(" 95%% 2 Month VaR (Max Loss with 95%% confidence):  $%.2f\n", var_95);
	printf(" 99%% 2 Month VaR (Max Loss with 99%% confidence):  $%.2f\n", var_99);
	printf(" 99%% 2 Month VaR (Expected Loss in Worst 1%%):   $%.2f\n", cvar_99);
	printf(" Worst Simulated Catastrophic Loss (Max Loss):   $%.2f\n", max_loss);
	printf("================================================================\n\n");

	free(pnl_array); // Free dynamic memory safely
}


//main runs the stress test over 3 different scenarios

int main() {
	srand(time(NULL));

	printf("\n*** MONTE CARLO PORTFOLIO STRESS TESTING ENGINE (C) ***\n");
	printf("Initial Portfolio Value: $%.2f | Horizon: %d Trading Days\n", INITAL_PRINCIPAL, TRADING_DAYS);
	printf("Number of Monte Carlo Trajectories: %d\n\n", (int)NUMBER_OF_SIMULATIONS);


	run_stress_test("1. S&P Baseline of a Normal Market", 0.08, 0.15);
	run_stress_test("2. during a crash", -0.20, 0.45);
	run_stress_test("3. 2008 finacial crisis GFC", -0.60, 0.85);
	run_stress_test("4. ASX 200 average past 20 years", 0.095, 0.14);
	return 0;
}



