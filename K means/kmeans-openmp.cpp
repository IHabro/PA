#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include <omp.h>

#include <chrono>
#include <algorithm>

using namespace std;

void print_data(double * data, const unsigned int m, const unsigned int n)
{
	if (m > 10 || n > 100)
	{
		cout << "printing suppressed ... " << endl;
	}
	for (unsigned int i = 0; i < n; i++)
	{
		for (unsigned int j = 0; j < m; j++)
		{
			cout << data[i*m + j] << " ";
		}
		cout << endl;
	}
	cout << endl;
}

void gen_data_p(double * a, unsigned int n)
{
	#pragma omp parallel
	{
		unsigned int seed = time(NULL) << omp_get_thread_num();

		#pragma omp for
		for (unsigned int i = 0; i < n; i++)
		{
			a[i] = (double) rand_r(&seed) / RAND_MAX;
		}
	}
}

double eucl_distance_simd(const double * x, const double * y, const unsigned int n)
{
	double dist = 0;

	#pragma omp simd reduction(+:dist)
	for (unsigned int i = 0; i < n; i++)
	{
		dist += (x[i] - y[i])*(x[i] - y[i]);
	}

	return sqrt(dist);
}

void assign_clusters(double * data, unsigned int m, unsigned int n, double * centers, unsigned int k, unsigned int * cluster_lbls) 
{	
	double dists[k] = {0, };	

	for (unsigned int i = 0; i < n; i++)
	{
		double * current = &data[i*m];

		// compute distance to each ceneter
		for(unsigned int l = 0; l < k; l++)
		{
			dists[l] = eucl_distance_simd(current, &centers[l*m], m);
		}

		// compute argmin
		cluster_lbls[i] = std::distance(dists, std::min_element(dists, dists + k));
	}
}

void assign_clusters_p(double * data, unsigned int m, unsigned int n, double * centers, unsigned int k, unsigned int * cluster_lbls) 
{	
	double dists[k] = {0, };

	// Pocitani odkud kam ma kadzdy thread pocitat

	for (unsigned int i = 0; i < n; i++)
	{
		double * current = &data[i*m];

		// compute distance to each ceneter
		for(unsigned int l = 0; l < k; l++)
		{
			dists[l] = eucl_distance_simd(current, &centers[l*m], m);
		}

		// compute argmin
		cluster_lbls[i] = std::distance(dists, std::min_element(dists, dists + k));
	}
}

void refresh_centers(double * data, unsigned int m, unsigned int n, double * centers, double * new_centers, unsigned int k, unsigned int * cluster_lbls) 
{
	unsigned int counts[k] = {0, };
	fill(new_centers, new_centers +  m*k, 0);

	for (unsigned int i = 0; i < n; i++)
	{
		unsigned int cls = cluster_lbls[i];
		counts[cls] += 1;

		for(unsigned int j = 0; j < m; j++)
		{
			new_centers[cls * m + j] += data[i*m + j];
		}
	}

	for (unsigned int i = 0; i < k; i++)
	{
		for(unsigned int j = 0; j < m; j++)
		{
			new_centers[i * m + j] /= counts[i];
		}
	}
}

bool check_convergence(double * centers, unsigned int m, unsigned int k, double * new_centers, double epsilon)
{
	double dist = 0;
	for (unsigned int i = 0; i < k; i++) {
		dist += eucl_distance_simd(&centers[i*m], &new_centers[i*m], m);
	}
	return (dist / k) <= epsilon;
}

int main(int argc, char *argv[])
{
	const unsigned int m = 2;
	const unsigned int n = 50;
	const unsigned int k = 3;

	double *data = new double[m*n];
	double *centers = new double[m*k];
	double *new_centers = new double[m*k];

	unsigned int * cluster_lbls = new unsigned int[k];

	gen_data_p(data, m*n);
	gen_data_p(centers, m*k);

	print_data(data, m, n);
	cout << "Initial centers" << endl;
	print_data(centers, m, k);

	while (true)
	{
		assign_clusters(data, m, n, centers, k, cluster_lbls);
		refresh_centers(data, m, n, centers, new_centers, k, cluster_lbls);

		if (check_convergence(centers, m, k, new_centers, 1e-8))
		{
			// has convereged
			cout << "Final centers" << endl;
			print_data(centers, m, k);
			print_data(new_centers, m, k);

			break;
		}
		else 
		{
			// not yet, swap the ptrs.
			double * tmp = new_centers;
			new_centers = centers;
			centers = tmp;
			cout << "iter" << endl;
		}
	}

	delete [] data;
	delete [] centers;
	delete [] new_centers;
	delete [] cluster_lbls;

    return 0;
}
