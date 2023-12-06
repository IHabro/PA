#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include <omp.h>

#include <chrono>

using namespace std;

void print_data(vector<double> & container, const unsigned int x, const unsigned int y)
{
	for (unsigned int i = 0; i < container.size(); i++)
	{
		cout << container[i] << " ";
	}
	cout << endl;
}

/*
Podpora pro pragmu simd musi byt explicitne zapnuta; 
	> MSVC: https://learn.microsoft.com/cs-cz/cpp/build/reference/openmp-enable-openmp-2-0-support?view=msvc-170
	> GCC: -fopenmp -fopenmp-simd 
	> clang: -openmp-simd

U GCC je jeste treba dat pozor na optimalizaci. Prekladac pri pouziti -O3 provadi vektorizaci cyklu i bez OpenMP,
takze efekt pragmy neni vyrazny.

*/

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

double eucl_distance(const double * x, const double * y, const unsigned int n)
{
	double dist = 0;

	for (unsigned int i = 0; i < n; i++)
	{
		dist += (x[i] - y[i])*(x[i] - y[i]);
	}

	return sqrt(dist);
}

void say_hello()
{		
	#pragma omp parallel
	{
		unsigned int tid = omp_get_thread_num();
		cout << "Hello from thread " << tid << endl;
	}
}


/*
	Chovani funkce rand() ze standardni knihovny C zavisi na implementaci, 
	obecne neni prilis vhodna pro pouziti ve vice vlaknech. Obecne je 
	thread-safe [1], ale muze byt velmi neefektivni. V Linuxove implementaci 
	vyuziva mutex, kterym dalsi vlakna blokuje [2].
	
	Toto chovani lze vyzkouset na nasledujicim kodu - varianta s 
		>	#omp parallel for
	je mohem pomalejsi, nez varianta bez.

	Toto resi funkce rand_r [1]. U te je ale treba dat pozor na inicializaci.
	Pri pouziti klasickeho ''kouknuti na hodinky'' time(NULL) se velmi lehce stane, 
	ze se RNG ve vice vlaknech inicializuje na stejnou hodnotu a obe vlakna 
	tak generuji stejnou posloupnost cisel.


	Refs
		[1] https://man7.org/linux/man-pages/man3/srand.3.html
		[2] https://www.evanjones.ca/random-thread-safe.html
*/
void vector_gen(double * a, unsigned int n)
{
	// #pragma omp parallel for
	for (unsigned int i = 0; i < n; i++)
	{
		a[i] = (double) rand() / RAND_MAX;
	}
}

void vector_gen_p(double * a, unsigned int n)
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

/*
	Nekolik "slozitych" operaci pro simulovani workloadu.
	Je dodrzena nezavislost iteraci, takze je mozno paralelizovat pomoci 
		>	#omp parallel for
*/
void vector_add(double * a, double * b, unsigned int n)
{
	#pragma omp parallel for
	for (unsigned int i = 0; i < n; i++)
	{	
		a[i] = a[i] * sin(2 * M_PI * n );
		b[i] = b[i] * sin( M_PI * n + M_PI / 2);
		a[i] += b[i];
	}
}

void step_1(const unsigned int m, const unsigned int n) 
{
	say_hello();

	double * a = new double[m*n];
	double * b = new double[m*n];
	memset(a, 0, sizeof(double) * m * n);
	memset(b, 0, sizeof(double) * m * n);

	auto start = chrono::steady_clock::now();
	vector_gen_p(a, m*n);
	auto stop = chrono::steady_clock::now();
	chrono::duration<double> elapsed = stop - start;
	cout << "gen 1: " << elapsed.count() << endl;

	start = chrono::steady_clock::now();
	vector_gen(b, m*n);
	stop = chrono::steady_clock::now();
	elapsed = stop - start;
	cout << "gen 2: " << elapsed.count() << endl;
	
	start = chrono::steady_clock::now();
	vector_add(a, b, m*n);
	stop = chrono::steady_clock::now();
	elapsed = stop - start;
	cout << "  add: " << elapsed.count() << endl;
	
	delete [] a;
	delete [] b;
}

void step_2(const unsigned int m, const unsigned int n, const bool use_simd) 
{	
	double * data = new double[m*n];

	auto start = chrono::steady_clock::now();	
	vector_gen_p(data, m*n);
	auto stop = chrono::steady_clock::now();

	chrono::duration<double>  elapsed = stop-start;
	cout << "gen 1: " << elapsed.count() << endl;
	
	vector<double> dists;
	dists.resize(m);

	start = chrono::steady_clock::now();

	#pragma omp parallel for
	for (unsigned int i = 0; i < m; i++)
	{
		if (use_simd)
			dists[i] = eucl_distance_simd(data, data + n*i, n);
		else
			dists[i] = eucl_distance(data, data + n*i, n);
	}

	stop = chrono::steady_clock::now();
	elapsed = stop-start;
	cout << " dist: " << elapsed.count() << endl;

	for (unsigned int i = 0; i < 10; i++)
		cout << dists[i] << endl;

	delete [] data;
}

int main(int argc, char *argv[])
{
	const unsigned int m = 1000000;
	const unsigned int n = 100;

	bool use_simd = false;

	if (argc > 1)
	{
		use_simd = true;
	}

	step_1(m, n);
	step_2(m, n, use_simd);

    return 0;
}
