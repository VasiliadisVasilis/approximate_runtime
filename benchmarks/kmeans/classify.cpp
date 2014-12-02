#include <map>
#include <vector>
#include <iostream>
#include <fstream>

int main(int argc, char* argv[])
{
  std::map<int, std::map<int,int> > edges[2];
  std::vector<int>  memberships[2];
  int points, points2;
  int p1, p2, i, j;

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
  }

  input[0].close();
  input[1].close();
  
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

  int err = 0;
  int total = 0;
  double rel_err = 0;


  for ( i=0; i<points; ++i)
  {
    for (j=0; j<points; ++j)
    {
      total += edges[1][i][j];
      err += ( (edges[0][i][j] != edges[1][i][j]) && i>j);
    }
  }
  
  #if 1
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

  rel_err = err*100.0/(double)(total);
  std::cout << "Err: " << err << " out of " << total << " (" << rel_err << "%)" << std::endl;


  return 0;
}
