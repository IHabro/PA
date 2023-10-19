using System.Xml.Linq;
using System;
using System.Net.Mime;
using System.Runtime;
using Combinatorics;
using Combinatorics.Collections;
using System.Runtime.CompilerServices;
using System.Collections.Generic;
using System.Text;

namespace TSP
{
    internal class Program
    {
        public class City : IComparable<City>
        {
            public int id { get; set; }
            public float x { get; set; }
            public float y { get; set; }

            public City()
            { }

            public City(int i_id, float i_x, float i_y)
            {
                id = i_id;
                x = i_x;
                y = i_y;
            }

            public City(string inpt)
            {
                inpt = inpt.Trim();
                var lines = inpt.Split(" ");

                id = int.Parse(lines[0]) - 1;
                x = float.Parse(lines[1]);
                y = float.Parse(lines[2]);
            }

            public override string ToString()
            {
                return "City[" + id.ToString("000") + "](" + x.ToString("00.00") + ", " + y.ToString("00.00") + ")";
            }

            public int CompareTo(City? obj)
            {
                if (obj == null)
                    return -1;

                return id.CompareTo(obj.id);
            }
        }

        public class Cities
        {
            public List<City> ctss { get; set; }
            public int size { get; set; }
            public double[,] distances { get; set; }
            public int threadCount { get; set; }

            public Cities()
            { }

            public Cities(List<City> lst)
            {
                ctss = lst;
            }

            public Cities(string inp, int count)
            {
                ctss = DocumentRead(inp);
                threadCount = count;

                size = ctss.Count();
                distances = new double[size, size];

                for (int i = 0; i < size; i++)
                {
                    for (int j = i; j < size; j++)
                    {
                        double dist = Distance(ctss[i], ctss[j]);
                        distances[i, j] = dist;
                        distances[j, i] = dist;
                    }
                }
            }

            public void Print()
            {
                foreach (City ct in ctss)
                {
                    Console.WriteLine(ct);
                }

                Console.WriteLine();
            }

            public void PrintDistances()
            {
                for (int i = 0; i < size; i++)
                {
                    for (int j = 0; j < size; j++)
                    {
                        Console.Write(string.Format("{0} ", distances[i, j].ToString("00.00")));
                    }

                    Console.Write(Environment.NewLine);
                }

                Console.WriteLine();
            }

            public async Task<List<string>> ThreadPart(Permutations<City> perms, City first, int number, int from)
            {
                List<string> result = new List<string>();
                List<int> indexes = new List<int>();
                StringBuilder sb = new StringBuilder();

                for (int i = from; i < (from + number); i++)
                {
                    sb.Append("{");
                    sb.Append($"{"{" + first.id.ToString("000") + "}"}");

                    indexes.Add(0);

                    foreach (var item in perms.ElementAt(i))
                    {
                        sb.Append($"{"{" + item.id.ToString("000") + "}"}");

                        indexes.Add(item.id);
                    }

                    sb.Append("} = ");
                    sb.Append(IndexDistance(indexes));
                    sb.Append("\n");

                    result.Add(sb.ToString());

                    indexes.Clear();
                    sb.Clear();
                }

                return result;
            }

            public void Compute()
            {
                City first = ctss[0];
                ctss.RemoveAt(0);

                Permutations<City> perms = new Permutations<City>(ctss.ToArray());

                // Thread Brute Force
                var instances = perms.Count() / threadCount;
                var tasks = new List<Task<List<string>>>();

                for (int i = 0; i < threadCount; i++)
                {
                    // Task<List<string>> task = Task.Run(() => ThreadPart(perms, first, instances, i * instances));
                    tasks.Add(Task.Run(() => ThreadPart(perms, first, instances, i * instances)));
                }

                //Standard Brute Force
                List<int> indexes = new List<int>();

                Console.WriteLine("Standard Brute Force");
                for (int i = 0; i < perms.Count(); i++)
                {
                    Console.Write("{");
                    Console.Write($"{"{" + first.id.ToString("000") + "}"}");

                    indexes.Add(0);

                    foreach (var item in perms.ElementAt(i))
                    {
                        Console.Write($"{"{" + item.id.ToString("000") + "}"}");

                        indexes.Add(item.id);
                    }

                    Console.Write("} = ");
                    Console.Write(IndexDistance(indexes));
                    Console.Write("\n");

                    indexes.Clear();
                }

                var rst = Task.WhenAll(tasks.ToArray()).Result;

                Console.WriteLine("\nThread Brute Force");
                foreach (var task in rst)
                {
                    foreach (string val in task)
                    {
                        Console.Write(val);
                    }
                }
            }

            public double IndexDistance(List<int> indexes)
            {
                double result = 0.0;
                int n = size;

                int from;
                int to;

                for (int i = 0; i < indexes.Count() - 1; i++)
                {
                    from = indexes[i];
                    to = indexes[i + 1];
                    result += distances[from, to];
                }

                from = indexes.Last();
                to = indexes.First();
                result += distances[from, to];

                return result;
            }
        }

        public static List<City> DocumentRead(string file)
        {
            List<City> result = new List<City>();

            file = File.ReadAllText(file);

            foreach (string ln in file.Split("\n"))
            {
                if (ln == "" || ln == string.Empty)
                {
                    continue;
                }

                var city = new City(ln);

                result.Add(city);
            }

            return result;
        }

        public static double Distance(City a, City b)
        {
            return Math.Sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
        }

        private void Perms(List<int> result, int input, int size)
        {
            if (size == 1)
            {
                result.Add(input);
            }
            else
            {
                for (int i = 0; i < size; i++)
                {
                    Perms(result, input, size - 1);
                }
            }
        }

        static void Main(string[] args)
        {
            Cities cts = new Cities("../../../data/ulysses22.tsp.txt", 2);

            cts.Print();

            cts.PrintDistances();

            cts.Compute();
        }
    }
}
