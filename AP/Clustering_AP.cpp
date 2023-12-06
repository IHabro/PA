#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include <omp.h>
#include <stdlib.h>
#include <chrono>
#include <iomanip>
#include <algorithm>

using namespace std;

/*
Podpora pro pragmu simd musi byt explicitne zapnuta;
	> MSVC: https://learn.microsoft.com/cs-cz/cpp/build/reference/openmp-enable-openmp-2-0-support?view=msvc-170
	> GCC: -fopenmp -fopenmp-simd
	> clang: -openmp-simd

U GCC je jeste treba dat pozor na optimalizaci. Prekladac pri pouziti -O3 provadi vektorizaci cyklu i bez OpenMP,
takze efekt pragmy neni vyrazny.
*/
double eucl_distance_simd(const double* x, const double* y, const unsigned int n)
{
	double dist = 0;

	#pragma omp simd reduction(+:dist)
	for (unsigned int i = 0; i < n; i++)
	{
		dist += (x[i] - y[i]) * (x[i] - y[i]);
	}

	return sqrt(dist);
}

double eucl_distance(const double* x, const double* y, const unsigned int n)
{
	double dist = 0;

	for (unsigned int i = 0; i < n; i++)
	{
		dist += (x[i] - y[i]) * (x[i] - y[i]);
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
void vector_gen(double* a, unsigned int n)
{
	// #pragma omp parallel for
	for (unsigned int i = 0; i < n; i++)
	{
		a[i] = (double)rand() / RAND_MAX;
	}
}

/*
// void vector_gen_p(double* a, unsigned int n)
// {
// #pragma omp parallel
// 	{
// 		unsigned int seed = time(NULL) << omp_get_thread_num();

// #pragma omp for
// 		for (unsigned int i = 0; i < n; i++)
// 		{
// 			a[i] = (double)rand_r(&seed) / RAND_MAX;
// 		}
// 	}
// }

// Nekolik "slozitych" operaci pro simulovani workloadu.
// Je dodrzena nezavislost iteraci, takze je mozno paralelizovat pomoci
// 	>	#omp parallel for
// void vector_add(double* a, double* b, unsigned int n)
// {
// #pragma omp parallel for
// 	for (unsigned int i = 0; i < n; i++)
// 	{
// 		a[i] = a[i] * sin(2 * M_PI * n);
// 		b[i] = b[i] * sin(M_PI * n + M_PI / 2);
// 		a[i] += b[i];
// 	}
// }
*/

void step_1(const unsigned int m, const unsigned int n)
{
	say_hello();

	double* a = new double[m * n];
	double* b = new double[m * n];
	memset(a, 0, sizeof(double) * m * n);
	memset(b, 0, sizeof(double) * m * n);

	auto start = chrono::steady_clock::now();
	//vector_gen_p(a, m * n);
	auto stop = chrono::steady_clock::now();
	chrono::duration<double> elapsed = stop - start;
	cout << "gen 1: " << elapsed.count() << endl;

	start = chrono::steady_clock::now();
	vector_gen(b, m * n);
	stop = chrono::steady_clock::now();
	elapsed = stop - start;
	cout << "gen 2: " << elapsed.count() << endl;

	start = chrono::steady_clock::now();
	//vector_add(a, b, m * n);
	stop = chrono::steady_clock::now();
	elapsed = stop - start;
	cout << "  add: " << elapsed.count() << endl;

	delete[] a;
	delete[] b;
}

void step_2(const unsigned int m, const unsigned int n, const bool use_simd)
{
	double* data = new double[m * n];

	auto start = chrono::steady_clock::now();
	//vector_gen_p(data, m * n);
	auto stop = chrono::steady_clock::now();

	chrono::duration<double>  elapsed = stop - start;
	cout << "gen 1: " << elapsed.count() << endl;

	vector<double> dists;
	dists.resize(m);

	start = chrono::steady_clock::now();

#pragma omp parallel for
	for (unsigned int i = 0; i < m; i++)
	{
		if (use_simd)
			dists[i] = eucl_distance_simd(data, data + n * i, n);
		else
			dists[i] = eucl_distance(data, data + n * i, n);
	}

	stop = chrono::steady_clock::now();
	elapsed = stop - start;
	cout << " dist: " << elapsed.count() << endl;

	for (unsigned int i = 0; i < 10; i++)
		cout << dists[i] << endl;

	delete[] data;
}

#pragma region "Affinity Propagation"

struct Pair
{
	int pointNumber;
	int clusterNum;
};

bool cmp(Pair &a, Pair &b)
{
	if (a.clusterNum < b.clusterNum)
	{
		return true;
	}

	return false;	
}

vector<vector<double>> Sml = vector<vector<double>>();
vector<vector<double>> Rsp = vector<vector<double>>();
vector<vector<double>> Avl = vector<vector<double>>();
vector<vector<double>> Crt = vector<vector<double>>();
int rows = -1, cols = -1;	//rows could also be called numberOfPoints

vector<Pair> clusters = vector<Pair>();
std::vector<std::vector<double>> testData =
{
	{1.0, 2.0, 0.5},
	{1.5, 1.8, 0.6},
	{5.0, 8.0, 2.0},
	{8.0, 8.0, 1.5},
	{1.0, 0.6, 0.3},
	{9.0, 11.0, 3.0},
	{8.0, 2.0, 1.0},
	{10.0, 2.0, 2.0},
	{9.0, 3.0, 0.8},
	{3.0, 3.0, 1.2},
	{4.0, 6.0, 2.5},
	{7.0, 6.0, 1.8},
	{5.0, 3.0, 0.7},
	{6.0, 8.0, 2.3},
	{2.0, 7.0, 1.2}
};

void PrintAffinityData(vector<vector<double>>& data)
{
	std::cout << std::fixed;
    std::cout << std::setprecision(1);

	rows = data.size();
	cols = data[0].size();

	cout << "Data: \n";
	cout << "---------------------------------------------------------------" << endl;
	for(int i = 0; i < rows; i++)
	{
		for(int j = 0; j < cols; j++)
		{
			cout << data[i][j] << " ";
		}

		cout << endl;
	}
	cout << "---------------------------------------------------------------\n" << endl;
}

void PrintMatricies()
{
	std::cout << std::fixed;
    std::cout << std::setprecision(1);

	int i = 0, j = 0;

	cout << "Sml: \n";
	cout << "---------------------------------------------------------------" << endl;
	for (i = 0;i < rows; i++)
	{
		for(j = 0; j < rows; j++)
		{
			cout << Sml[i][j] << " ";
		}

		cout << "\n";
	}
	cout << "---------------------------------------------------------------\n" << endl;;

	cout << "Rsp: \n";
	cout << "---------------------------------------------------------------" << endl;
	for (i = 0;i < rows; i++)
	{
		for(j = 0; j < rows; j++)
		{
			cout << Rsp[i][j] << " ";
		}

		cout << "\n";
	}
	cout << "---------------------------------------------------------------\n" << endl;

	cout << "Avl: \n";
	cout << "---------------------------------------------------------------" << endl;
	for (i = 0;i < rows; i++)
	{
		for(j = 0; j < rows; j++)
		{
			cout << Avl[i][j] << " ";
		}

		cout << "\n";
	}
	cout << "---------------------------------------------------------------\n" << endl;

	cout << "Crt: \n";
	cout << "---------------------------------------------------------------" << endl;
	for (i = 0;i < rows; i++)
	{
		for(j = 0; j < rows; j++)
		{
			cout << Crt[i][j] << " ";
		}

		cout << "\n";
	}
	cout << "---------------------------------------------------------------\n" << endl;

	sort(clusters.begin(), clusters.end(), cmp);

	cout << "Points: ";
	for(i = 0; i < clusters.size();i++)
	{
		cout << clusters[i].pointNumber << " ";
	}
	cout << endl << "Clustr: ";
	for(i = 0; i < clusters.size();i++)
	{
		cout << clusters[i].clusterNum << " ";
	}
	cout << endl;
}

double DistanceAffinity(vector<double>& a, vector<double>& b)
{
	double result = 0.0;

	for(int i = 0; i < cols; i++)
	{
		result += (a[i] - b[i]) * (a[i] - b[i]);
	}

	return -1 * sqrt(result);
}

double GetSumAvlSml(int i, int k)
{
	vector<double> mxm = vector<double>();

	for(int kNoted = 0; kNoted < rows; kNoted++)
	{
		if (kNoted != k)
		{
			// max(A[i, k'] + S[i, k']); {k' != k}
			mxm.push_back(Avl[i][kNoted] + Sml[i][kNoted]);
		}
	}

	return *max_element(mxm.begin(), mxm.end());
}

// + Sum of max(0, R[i', k])
double GetSumRsp(int i, int k)
{
	double result = 0.0;

	for(int iNoted = 0; iNoted < rows; iNoted++)
	{
		if (iNoted != k)
		{
			// sum += max(0.0, R[i', k])
			result += max(0.0, Rsp[iNoted][k]);
		}
	}

	return result;
}

void InitMatricies(vector<vector<double>>& data)
{
	// Rows and Cols should be the same number
	int i = 0, j = 0;

	// Update Similarity
	vector<double> mdn = vector<double>();
	int mdnSize = 0;
	double median = 0.0;

	for(i = 0; i < rows; i++)
	{
		vector<double> tmp;
		vector<double> empty(rows, 0.0);

		for(j = 0; j < rows; j++)
		{
			double dst = DistanceAffinity(data[i], data[j]);
			
			tmp.push_back(dst);
			mdn.push_back(dst);
		}

		Sml.push_back(tmp);
		Rsp.push_back(empty);
		Avl.push_back(empty);
		Crt.push_back(empty);
	}

	mdnSize = mdn.size();
	std::sort(mdn.begin(), mdn.end());
	if (mdnSize % 2 == 0)
	{
		median = 0.5 * (mdn[(mdnSize / 2) - 1] + mdn[mdnSize / 2]);
	}
	else
	{
		median = mdn[mdnSize / 2];
	}
	
	for (i = 0; i < rows; i++)
	{
		Sml[i][i] = median;
	}

	// Update Responsibilities -> Step 3
	for(i = 0; i < rows; i++)
	{
		for(j = 0; j < rows; j++)
		{
			// R[i, k] = S[i, k] - max(A[i, k'] + S[i, k']); {k' != k}
			Rsp[i][j] = Sml[i][j] - GetSumAvlSml(i, j);
		}
	}

	// Update Availability -> Step 3
	for(i = 0; i < rows; i++)
	{
		for(j = 0;j < rows;j++)
		{
			if (i != j)
			{
				Avl[i][j] = min(0.0, Rsp[j][j] + GetSumRsp(i, j));
			}
			else
			{
				Avl[i][j] = GetSumRsp(i, j);
			}
		}
	}

	// Calculate Criteria Matrix C
	for(i = 0; i < rows; i++)
	{
		for(j = i;j < rows;j++)
		{
			Crt[i][j] = Crt[j][i] = Rsp[i][j] + Avl[i][j];
		}

		Pair p;
		p.clusterNum = distance(Crt[i].begin(), max_element(Crt[i].begin(), Crt[i].end())) + 1;
		p.pointNumber = i;
		clusters.push_back(p);
	}
}

#pragma endregion


int main(int argc, char* argv[])
{
	const unsigned int rows = testData.size();
	const unsigned int cols = testData[0].size();

	PrintAffinityData(testData);
	InitMatricies(testData);
	PrintMatricies();

	return 0;
}
