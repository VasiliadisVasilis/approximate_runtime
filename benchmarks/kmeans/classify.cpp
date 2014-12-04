#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>

int main(int argc, char* argv[])
{
  std::map<int, std::map<int,int> > edges[2];
  std::vector<int>  memberships[2];
  std::map<int,int>  populations;
  int points, points2;
  int p1, p2, i, j, faults;
  int identical = 1;
  double cluster_error = 0, cluster_tot=0.0;

  if ( argc != 3 )
  {
    std::cout << "Usage: " << argv[0] << "\"OutputBin\" "
                 "\"CorrectOutputBin\"" << std::endl;
    return 1;
  }
  
  std::ifstream input[2];
  input[0].open(argv[1], std::ifstream::in);
  input[1].open(argv[2], std::ifstream::in);
  
  input[0].read((char*)&points, sizeof(int));
  input[1].read((char*)&points2, sizeof(int));

  if ( points!=points2 )
  {
    std::cout << "Files do not match!" << std::endl;
    return 1;
  }

  for (i=0; i<points; ++i)
  {
    input[0].read((char*)&p1, sizeof(int));
    memberships[0].push_back(p1);
    input[1].read((char*)&p2, sizeof(int));
    memberships[1].push_back(p2);

    populations[p2] = populations[p2] +1;

    identical &= (p1==p2);
  }
  
  for ( ; ; )
  {
    float e1, e2;
    double t;
    
    input[0].read((char*)&e1, sizeof(float));
    input[1].read((char*)&e2, sizeof(float));
    if ( !input[0] || !input[1] )
    {
      break;
    }
    t = fabs(e1-e2);
    cluster_tot += fabs(e2);
    cluster_error += t;
    identical &= (*((int*)&e1) == *((int*)&e2));
  }

  input[0].close();
  input[1].close();

  if ( identical )
  {
    std::cout << "strict";
    return 0;
  }

#if 0
  /* This simulates faults during the execution ... cheap but seems to be working */
  faults = 0;
  srand(time(0));
  int err_rate = rand()%101;
  for ( i =0; i<points; ++i )
  {
    if ( rand()%100 < err_rate )
    {
      int e = rand()% 5;
      if ( e !=  memberships[0][i] )
        faults ++;
      memberships[0][i] = e;
    }
  }
#endif
  for ( i=0; i<points; ++i)
  {
    int m = memberships[0][i];
    for ( j=0; j<points; ++j)
    {
      if (m == memberships[0][j] && j!=i)
      {
        (edges[0])[i][j] = 1;
        (edges[0])[j][i] = 1;
      }
    }
    
    m = memberships[1][i];
    for ( j=0; j<points; ++j)
    {
      if (m == memberships[1][j] && j!=i)
      {
        edges[1][i][j] = 1;
        edges[1][j][i] = 1;
      }
    }
  }

  double err = 0;
  double total = 0;
  double rel_err = 0;
  double rel_cluster_err = 0;


  for ( i=0; i<points; ++i)
  {
    for (j=0; j<points; ++j)
    {
      total += edges[1][i][j]/(float)populations[memberships[1][i]];
      err += ( (edges[0][i][j] != edges[1][i][j]) && i>j)
          /(float)populations[memberships[1][i]];
    }
  }
  
  #if 0
  for ( i=0; i<points; ++i)
  {
    for (j=0; j<points; ++j)
    {
      std::cout << (edges[0][i][j] == 0 ? "." : "o");
//      std::cout << (int)edges[1][i][j];
    }
    std::cout << std::endl;
  }
  #endif

  rel_err = err/(double)(total);
  rel_cluster_err = cluster_error/(double)cluster_tot;
  //std::cout << "Err: " << err << " out of " << total << " (" << rel_err << "%)" << std::endl;
  std::cerr << "INNFO: " << rel_err << " " << rel_cluster_err << std::endl;
  if ( rel_err < 0.1 && rel_cluster_err < 0.1 )
  {
    std::cout << "correct";
  }
  else
  {
    std::cout << "altered";
  }
#if 0
  std::cout << "Faults: " << faults << std::endl; 
  std::cout << "Ratio: " << err/faults << std::endl;
#endif
  return 0;
}
