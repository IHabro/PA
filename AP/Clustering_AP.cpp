#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include <omp.h>
#include <stdlib.h>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <limits>
#include <fstream>
#include <unistd.h>

using namespace std;

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
		return true;vector<Pair> newClusters = vector<Pair>();

	}

	return false;	
}

int rows = -1, cols = -1;	//rows could also be called numberOfPoints
int diffCount = 0, newDiffcount = 0;
vector<vector<int>> Sml = vector<vector<int>>();
vector<vector<int>> Rsp = vector<vector<int>>();
vector<vector<int>> Avl = vector<vector<int>>();
vector<vector<int>> Crt = vector<vector<int>>();
vector<Pair> clusters = vector<Pair>();
vector<Pair> newClusters = vector<Pair>();
std::vector<std::vector<int>> testData =
{
	{1, 2, 0},
	{2, 1, 0},
	{5, 8, 2},
	{8, 8, 1},
	{1, 0, 0},
	{9, 11, 3},
	{8, 2, 1},
	{10, 2, 2},
	{9, 3, 0},
	{3, 3, 1},
	{4, 6, 2},
	{7, 6, 1},
	{5, 3, 0},
	{6, 8, 2},
	{2, 7, 1}
};

void LoadData(string filePath)
{
	vector<int> mdn = vector<int>();
	string line;
	ifstream file;
	file.open(filePath);
	int iter = 0;

	// Remove first row with labels
	getline(file, line);
	while (getline(file, line))
	{
		iter++;
		vector<int> tmp = vector<int>();
		stringstream ss(line); 
    	string word;

		while (getline(ss, word, ','))
		{
			int i = stoi(word);
			tmp.push_back(i);
			mdn.push_back(i);
    	}

		vector<int> empty(tmp.size(), 0);
		Sml.push_back(tmp);
		Rsp.push_back(empty);
		Avl.push_back(empty);
		Crt.push_back(empty);
	}
	file.close();

	// Set medians
	int mdnSize = mdn.size(), median = 0;
	std::sort(mdn.begin(), mdn.end());
	if (mdnSize % 2 == 0)
	{
		median = 0.5 * (mdn[(mdnSize / 2) - 1] + mdn[mdnSize / 2]);
	}
	else
	{
		median = mdn[mdnSize / 2];
	}

#pragma omp parallel for
	for (int i = 0; i < rows; i++)
	{
		Sml[i][i] = median;
	}

	rows = Sml.size();
	cols = Sml[0].size();
}

void PrintAffinityData(vector<vector<double>>& data)
{
	std::cout << std::fixed;
    std::cout << std::setprecision(1);

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
}

void PrintClusters()
{
	int i;
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

double Distance(vector<double>& a, vector<double>& b)
{
	double result = 0.0;

#pragma omp paralle for reduction(+:result)
	for(int i = 0; i < cols; i++)
	{
		result += (a[i] - b[i]) * (a[i] - b[i]);
	}

	return -1 * sqrt(result);
}

// Opimalizace vytvorenim 1 promenne maximum
int GetMaxAvlSml(int i, int k)
{
	int result = -numeric_limits<double>::infinity(), tmp;

#pragma omp parallel for
	for(int kNoted = 0; kNoted < rows; kNoted++)
	{
		if (kNoted != k)
		{
			// max(A[i, k'] + S[i, k']); {k' != k}
			tmp = Avl[i][kNoted] + Sml[i][kNoted];

			if (tmp > result)
			{
				result = tmp;
			}
		}
	}

	return result;
}

// + Sum of max(0, R[i', k])
int GetSumRsp(int i, int k)
{
	int result = 0;

#pragma omp paralell for reduction(+:result)
	for(int iNoted = 0; iNoted < rows; iNoted++)
	{
		if (iNoted != i)
		{
			// sum += max(0.0, R[i', k])
			result += max(0, Rsp[iNoted][k]);
		}
	}

	return result;
}

void CalculateSimilarity(vector<vector<double>>& data)
{
	// Update Similarity
	vector<double> mdn = vector<double>();
	int mdnSize = 0;
	// int i, j;
	double median = 0.0;

#pragma omp parallel for //collapse(2)
	for(int i = 0; i < rows; i++)
	{
		vector<int> tmp;
		vector<int> empty(rows, 0);

		for(int j = 0; j < rows; j++)
		{
			double dst = Distance(data[i], data[j]);
			
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

#pragma omp parallel for
	for (int i = 0; i < rows; i++)
	{
		Sml[i][i] = median;
	}
}

void UpdateResponsibility()
{
	// Update Responsibilities -> Step 3
#pragma omp parallel for //collapse(2)
	for(int i = 0; i < rows; i++)
	{
		for(int j = 0; j < rows; j++)
		{
			// R[i, k] = S[i, k] - max(A[i, k'] + S[i, k']); {k' != k}
			Rsp[i][j] = Sml[i][j] - GetMaxAvlSml(i, j);
		}
	}
}

void UpdateAvailability()
{
	// Update Availability -> Step 3
#pragma omp parallel for //collapse(2)
	for(int i = 0; i < rows; i++)
	{
		for(int j = 0;j < rows;j++)
		{
			if (i != j)
			{
				Avl[i][j] = min(0, Rsp[j][j] + GetSumRsp(i, j));
			}
			else
			{
				// Upravid drobnz preklep v sume i' != k, ne i
				Avl[i][j] = GetSumRsp(i, j);
			}
		}
	}
}

bool UpdateClusters()
{
	newClusters.clear();
	newClusters.resize(rows);

	// Calculate Criteria Matrix C
//#pragma omp parallel for //collapse(2)
	for(int i = 0; i < rows; i++)
	{
		//cout << "i: " << i << ", [";
		for(int j = 0;j < rows;j++)
		{
			Crt[i][j] = Rsp[i][j] + Avl[i][j];
			// cout << j << " ";
		}
		//cout << "]" << endl;

		Pair p;
		//cout << "New p" << endl;
		// p.clusterNum = distance(Crt[i].begin(), max_element(Crt[i].begin(), Crt[i].end())) + 1;
		p.clusterNum = 0;
		p.pointNumber = 1;
		// << "p set" << endl;
		// << newClusters.size() << endl;
		newClusters.push_back(p);
		// << "p pushed" << endl;
	}
	cout << "Crt Updated" << endl;

	if (clusters.size() != newClusters.size())
	{
		clusters = newClusters;
		cout << "Clusters set" << endl;

		return false;
	}
	

	cout << "newDiffCount calculation" << endl;
#pragma omp parallel for	
	for (int i = 0; i < clusters.size(); i++)
	{
		if (clusters[i].clusterNum != newClusters[i].clusterNum)
		{
			newDiffcount++;
		}
	}

	if (newDiffcount == 0)
	{
		// Nenasel jsme zmenu v Clusterech, muzu rovnou vypsat a ukoncit
		return true;
	}

	diffCount = newDiffcount;

	return false;
}

void DelegateAndIterate(int maxIterations)
{
	for (int i = 0; i < maxIterations; i++)
	{
		cout << "Responsibilities" << endl;
		UpdateResponsibility();
		cout << "Availabilities" << endl;
		UpdateAvailability();
		
		cout << "Cluster check" << endl;
		bool val = UpdateClusters();

		if (val)
		{
			break;
		}
	}

	PrintMatricies();
	PrintClusters();
}
#pragma endregion

int main(int argc, char* argv[])
{
	LoadData("archive/mnist_test.csv");

	cout << "Sml: [" << Sml.size() << ", " << Sml[0].size() << "]" << endl;
	cout << "Rsp: [" << Rsp.size() << ", " << Rsp[0].size() << "]" << endl;
	cout << "Avl: [" << Avl.size() << ", " << Avl[0].size() << "]" << endl;
	cout << "Crt: [" << Crt.size() << ", " << Crt[0].size() << "]" << endl;

	// vector<Pair> pairs = vector<Pair>();
	// int i = 0;
	// while(true)
	// {
	// 	Pair p;
	// 	p.clusterNum = i;
	// 	p.pointNumber = i*2;

	// 	pairs.push_back(p);
	// 	cout << i++ << endl;
	// }
	// return 0;

	DelegateAndIterate(1000);

	return 0;
}

// Simple paralizace maticovych operaci -> Operace 3 a 4
// Jak funguje omp parallel + vlastne chci aby se pole Clustr nezmenilo

/*
Omp parallel je alternativa vlaken
Na pozadi je stejne pouziva
Viceme nahrada Task v C#

pragma omp paralell for vezme vsechny iterace cyklu for, rozdeli je do skupin a nasledne kazdou skupinu spusti nad samostatnym vlaknem 
Buron doporucuje pragma
*/