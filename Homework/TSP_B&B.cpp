#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <vector>
#include <algorithm>
#include <cmath>

// B&B struktura
struct skipData
{
	bool skip;
	int index;
	int valueAtIndex;
	double value;
};

using namespace std;

int sz = INT_MIN;
double bestVal = INT_MAX;
vector<double> distances;

int factorial(int n)
{
	if (n == 0 || n == 1)
	{
		return 1;
	}

	return n * factorial(n - 1);
}

void print_distances(vector<double>& dist, unsigned int n)
{
	for (unsigned int i = 0; i < dist.size(); i++)
	{
		cout << round(dist[i] * 1000) / 1000 << "\t";
		if ((i + 1) % n == 0)
			cout << endl;
	}
}

double compute_distance(double x1, double  y1, double  x2, double  y2)
{
	return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

bool read_tsp_file(char* fname)
{
	std::ifstream file(fname);

	if (file.is_open())
	{
		vector<double> xs, ys;

		std::string line;

		//std::getline(file, line);
		//std::getline(file, line);
		//std::getline(file, line);
		//std::getline(file, line);
		//std::getline(file, line);
		//std::getline(file, line);
		//std::getline(file, line);

		while (std::getline(file, line)) {
			if (line[0] == 'E')
				break;

			stringstream sin(line);
			int id;
			double x, y;
			sin >> id >> x >> y;

			xs.push_back(x);
			ys.push_back(y);
		}

		sz = (int) xs.size();

		distances.resize(sz * sz);

		for (unsigned int i = 0; i < sz; i++)
		{
			for (unsigned int j = i; j < sz; j++)
			{
				double dist = compute_distance(xs[i], ys[i], xs[j], ys[j]);
				distances[i * sz + j] = dist;
				distances[j * sz + i] = dist;
			}
		}

		print_distances(distances, sz);
		file.close();
	}
	else
	{
		cout << fname << " file not open" << endl;
	}

	return false;
}

vector<int> emptyPerm(int n)
{
	vector<int> result = vector<int>();

	// 1 -> n bo indexy zacinaji od 1
	for (size_t i = 1; i <= n; i++)
	{
		result.push_back(i);
	}

	return result;
}

skipData calculateDistance(vector<int>& indexes)
{
	skipData result;
	result.skip = false;
	result.index = -1;
	result.valueAtIndex = -1;
	result.value = 0.0;

	int n = sz, from, to;

	// Projdi indexy a vypocitej z nich cestu
	for (int i = 0; i < indexes.size() - 1; i++)
	{
		from = indexes[i]-1;
		to = indexes[i + 1]-1;
		result.value += distances[from*sz + to];

		if (result.value > bestVal)
		{
			// Skip from here
			result.skip = true;
			result.index = i;
			result.valueAtIndex = indexes[i];
			result.value = -1;
		}
	}

	// Dokonci smycku zpatky na zacatek
	from = indexes[indexes.size() - 1];
	to = indexes[0];
	result.value += distances[from, to];

	if (result.value > bestVal)
	{
		// Skip last one
		result.skip = true;
		result.index = indexes.size() - 1;
		result.valueAtIndex = indexes[indexes.size() - 1];
		result.value = -1;
	}

	return result;
}

double justDistance(vector<int> &indexes)
{
	double result = 0.0;
	int n = sz, from, to;

	// Projdi indexy a vypocitej z nich cestu
	for (int i = 0; i < indexes.size() - 1; i++)
	{
		from = indexes[i] - 1;
		to = indexes[i + 1] - 1;
		result += distances[from * sz + to];
	}

	// Dokonci smycku zpatky na zacatek
	from = indexes[indexes.size() - 1];
	to = indexes[0];
	result += distances[from, to];

	return result;
}

void printPermWithVal(vector<int> &data)
{
	for (size_t i = 0; i < data.size(); i++)
	{
		cout << data[i] << " ";
	}

	cout << " = " << calculateDistance(data).value << endl;
}

void printPerm(vector<int> &data)
{
	for (size_t i = 0; i < data.size(); i++)
	{
		cout << data[i] << " ";
	}

	cout << justDistance(data) << endl;
}

void work(int id, int lvl, int source, int target, vector<int> perm)
{
	if (id != 0)
	{
		swap(perm[source], perm[target]);
		sort(perm.begin() + source + 1, perm.end());
	}

	for (size_t i = 0; i < factorial(sz - lvl); i++)
	{
		// Zpracuj permutation
		skipData data = calculateDistance(perm);

		if (data.skip)
		{
			next_permutation(perm.begin() + 1, perm.end());

			i++;

			if (perm[data.index] != data.valueAtIndex)
				continue;
		}
		else
		{
			printPermWithVal(perm);

			if (data.value < bestVal)
			{
				bestVal = data.value;
			}

			next_permutation(perm.begin() + 1, perm.end());
		}
	}
}

void permutations()
{
	time_t start, end;

	vector<int> empty = emptyPerm(sz);
	bestVal = calculateDistance(empty).value;

	time(&start);
	for (size_t i = 0; i < sz; i++)
	{
		work(i, 1, 0, i, empty);
	}
	time(&end);
	cout << "First program: " << fixed << double(end - start) << endl;


	bestVal = INT_MAX;
	empty = emptyPerm(sz);
	cout << endl << "-------------------------------------------------" << endl;
	time(&start);
	for (size_t i = 0; i < factorial(sz); i++)
	{
		printPermWithVal(empty);
		next_permutation(empty.begin(), empty.end());
	}
	printPermWithVal(empty);
	time(&end);
	cout << "Second program: " << fixed << double(end - start) << endl;
	cout << "-------------------------------------------------" << endl;
}

int main()
{
	char name[] = "TSP_input.txt";

	read_tsp_file(name);
	permutations();

	return 0;
}
