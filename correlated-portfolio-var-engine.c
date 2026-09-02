#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define NUM_PATH 10000
#define NUM_DAYS 30
#define CAPITAL 3000.0
#define DT (1.0 / 252.0)


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


double generate_gaussian_noise(void) {
	// Generate uniform random decimals strictly in the open interval (0, 1)
	double u1 = ((double)rand() + 1.0) / ((double)RAND_MAX + 2.0);
	double u2 = ((double)rand() + 1.0) / ((double)RAND_MAX + 2.0);

	// Box-Muller Transform converts randomly generated numbers into standard normal
	return sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
}

typedef struct {
	char name[32];
	double weight;
	double mu;
	double sigma;
} Asset;


int compare_doubles(const void *a, const void *b) {
	double diff = (*(double *)a - *(double *)b);
	if (diff < 0) return -1;
	if (diff > 0) return 1;
	return 0;
}

void run_correlation_scenario(const char *regime_name, double rho, Asset stock1, Asset stock2) {
	//Memory Allocation
	double *portfolio_pnl = (double *)malloc(NUM_PATH * sizeof(double));
	if (portfolio_pnl == NULL) {
		printf("Error: Could not allocate memory!\n");
		return;
	}

	// pre-calculating daily parameters for both stocks
	double drift1 = (stock1.mu - 0.5 * stock1.sigma * stock1.sigma) * DT;
	double vol1   = stock1.sigma * sqrt(DT);

	double drift2 = (stock2.mu - 0.5 * stock2.sigma * stock2.sigma) * DT;
	double vol2   = stock2.sigma * sqrt(DT);

	//10000 Monte Carlo trajectories
	for (int sim = 0; sim < NUM_PATH; sim++) {
		double val1 = CAPITAL * stock1.weight;
		double val2 = CAPITAL * stock2.weight;

		for (int day = 0; day < NUM_DAYS; day++) {
			double z1 = generate_gaussian_noise();
			double z_uncorr = generate_gaussian_noise();

			// 2-variable cholesky formula
			double z2 = rho * z1 + sqrt(1.0 - rho * rho) * z_uncorr;
			// can think about this formula in terms of vectors
			// GOOGL is x axis and an unccorelated stock is the y axis
			// For SCHW which has some correlation to GOOGL the vector for SCHW would project RHO onto GOOGL's unit vector
			// so using trig on SCHW vector length = rho, magnitude = 1, we get SCHW = RHO + sqrt( 1 - RHO * RHO)
			// That is just vector addition of each component
			// I know that RHO is corrlated with GOOGL so I can multiply by z1
			// I know that the y axis projected part is unccorelated so I can multiply by z_uncorr
			// therefore z2 can be found.

			// keep in mind that mean is still 0 and var is 1
			// mean can be found as z1 and z_uncorr is 0 therefore total mean is 0
			// var of z1 and z_uncorr is 1 so when RHO and the sqrt are squared the expression is
			// RHO^2 + 1 - RHO^2, as the z1 and z_uncorr just become 1
			// therefore the total VAR(z2) = 1

			//compunding the values for both stocks

			val1 *= exp(drift1 + vol1 * z1);
			val2 *= exp(drift2 + vol2 * z2);
		}

		portfolio_pnl[sim] = (val1 + val2) - CAPITAL;
	}

	// sort outcomes in asending order
	qsort(portfolio_pnl, NUM_PATH, sizeof(double), compare_doubles);


	int index_95 = (int)(0.05 * NUM_PATH);
	int index_99 = (int)(0.01 * NUM_PATH);

	double var_95   = -portfolio_pnl[index_95];
	double var_99   = -portfolio_pnl[index_99];
	double max_loss = -portfolio_pnl[0];

	double tail_sum = 0.0;
	for (int i = 0; i < index_99; i++) {
		tail_sum += portfolio_pnl[i];
	}
	double cvar_99 = -(tail_sum / index_99);

	// print report for strategy
	printf("====================================================================================\n");
	printf(" STRATEGY: %s\n", regime_name);
	printf(" Assets:   70%% %s (Vol: %.0f%%) + 30%% %s (Vol: %.0f%%)\n",
	       stock1.name, stock1.sigma * 100, stock2.name, stock2.sigma * 100);
	printf(" Coupling: Correlation (rho) = %+.2f\n", rho);
	printf("------------------------------------------------------------------------------------\n");
	printf("  • 95.0%% 30-Day VaR (5th Percentile Loss):   -$%-9.2f  (Remaining Value: $%.2f)\n",
	       var_95, CAPITAL - var_95);
	printf("  • 99.0%% 30-Day VaR (1st Percentile Loss):   -$%-9.2f  (Remaining Value: $%.2f)\n",
	       var_99, CAPITAL - var_99);
	printf("  • 99.0%% Expected Shortfall (CVaR):          -$%-9.2f  (Remaining Value: $%.2f)\n",
	       cvar_99, CAPITAL - cvar_99);
	printf("  • Worst Catastrophic Drawdown:               -$%-9.2f  (Remaining Value: $%.2f)\n",
	       max_loss, CAPITAL - max_loss);
	printf("====================================================================================\n\n");

	//free memeory
	free(portfolio_pnl);
}


int main(void) {
	srand(time(NULL));

	// Define Portfolio Assets
	Asset GOOGL = {
		.name = "GOOGL (Alphabet)",
		.weight = 0.70,
		.mu = 0.24,
		.sigma = 0.29
	};

	Asset SCHW = {
		.name = "SCHW (Charles Schwab)",
		.weight = 0.30,
		.mu = 0.13,
		.sigma = 0.11
	};

	Asset MSFT = {
		.name = "MSFT (Tech Peer)",
		.weight = 0.30,
		.mu = 0.20,
		.sigma = 0.26
	};

	Asset GOLD = {
		.name = "GLD (Gold Commodity)",
		.weight = 0.30,
		.mu = 0.07,
		.sigma = 0.15
	};
	printf("\n####################################################################################\n");
	printf("               MULTI-ASSET CORRELATION & DIVERSIFICATION ENGINE               \n");
	printf("         Portfolio Principal: $%.2f  |  Horizon: %d Trading Days                     \n",
	       CAPITAL, NUM_DAYS);
	printf("####################################################################################\n\n");

	// Scenario 1: High Correlation Tech Concentration (GOOGL + MSFT)
	run_correlation_scenario("1. Tech Concentration (GOOGL + MSFT)", +0.85, GOOGL, MSFT);

	// Scenario 2: Cross-Sector Diversification (GOOGL + SCHW)
	run_correlation_scenario("2. Cross-Sector Diversification (GOOGL + SCHW)", +0.45, GOOGL, SCHW);

	// Scenario 3: Defensive Hedge (GOOGL + GOLD)
	run_correlation_scenario("3. Safe-Haven Hedging (GOOGL + GOLD)", -0.30, GOOGL, GOLD);


	return 0;
}

//final comments are
//expected return for each stratery is 22.8%, 20.7%, 18.9%
//although scenario 3 has the least expected return we can see that it has the least risk where
// 99% VaR is 17.57%, 14.02%, 12.43% coming from some simulation
// so Return to Var ratio for all is
// 1.3, 1.48, 1.52
//This means that ALthough regime 3 has the least expected return it's return to risk ratio makes it the most effecient.
//So i get the most return per dollar of tail risk investing in scenario 3.
// still i should evaluated my risk mandate to select if some of the other combinations suit me and the current market sentiment.



