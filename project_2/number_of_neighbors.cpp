#include <iostream>
#include <cmath>
#include <cstring>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>


using namespace std;

int main ()
{
  std::string line;
  ifstream file("manet100.csv");
  
  int i = 0, j = 0;
  double vec[3] = {0};
  double vecfull[100][2] = {0};
  double neigh[100]= {0};
  int rmin = 25;
  double tmp = 0;
  double sum = 0;
  
  if(file.is_open())
  {
	while(getline(file,line))
	{
		char seps[] = ",";
		char *token;
		token = strtok(&line[0], seps);		
		while(token != NULL)
		{
			vec[i] = atof(token);
			i++;
			token = strtok (NULL, ",");
			if(i == 3)
			{
				vecfull[j][0] = vec[1];
				vecfull[j][1] = vec[2];
				j++;
				i = 0;
			}
        }

	  }
	  file.close();
	}
	else
	{
	  std::cout<<"Error in csv file"<< '\n';
	}
	
	for(i = 0; i < 100; i++)
	{
		for(j = 0; j < 100; j++)
		{
			if(i == j)
			{}
			else
			{
				tmp = ((vecfull[j][1]-vecfull[i][1])*(vecfull[j][1]-vecfull[i][1])) + ((vecfull[j][0]-vecfull[i][0])*(vecfull[j][0]-vecfull[i][0]));
				if(tmp <= (rmin*rmin))
				{
					neigh[i]++;
				}
			}
		}
		std::cout << neigh[i] << "\n";
		sum += neigh[i];
	}
	sum = sum / 100;
	cout << "\nMean number of neighbors for rmin of " << rmin << "m are " << (sum / 100) << ".\n\n";
	return 0;
}
