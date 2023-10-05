using System.Xml.Linq;
using System;
using System.Net.Mime;
using System.Runtime;

namespace TSP
{
    internal class Program
    {
        public class City
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

                id = int.Parse(lines[0]);
                x = float.Parse(lines[1]);
                y = float.Parse(lines[2]);
            }

            public override string ToString()
            {
                return "City[" + id.ToString("000") + "](" + x.ToString("00.00") + ", " + y.ToString("00.00") + ")";
            }
        }

        public class Cities
        {
            public List<City> cities { get; set; }
            public int size { get; set; }
            public double[,] distances { get; set; }

            public Cities()
            { }

            public Cities(List<City> lst)
            {
                cities = lst;
            }

            public Cities(string inp)
            {
                cities = DocumentRead(inp);

                size = cities.Count();
                distances = new double[size, size];

                for (int i = 0; i < size; i++)
                {
                    for (int j = i; j < size; j++)
                    {
                        double dist = Distance(cities[i], cities[j]);
                        distances[i, j] = dist;
                        distances[j, i] = dist;
                    }
                }
            }

            public void Print()
            {
                foreach (City ct in cities)
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
            Cities cities = new Cities("../../../data/ulysses22.tsp.txt");

            cities.Print();

            cities.PrintDistances();
        }
    }
}
