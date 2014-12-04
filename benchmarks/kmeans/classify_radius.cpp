#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <cstring>

#ifndef FLT_MAX
#define FLT_MAX 3.40282347e+38
#endif

float *buf;
float **attributes;
int numAttributes;
int numObjects;
int numClusters = 5;

float euclid_dist_2(float *pt1,
    float *pt2,
    int    numdims)
{
  int i;
  float ans=0.0;

  for (i=0; i<numdims; i++)
    ans += (pt1[i]-pt2[i]) * (pt1[i]-pt2[i]);

  return(ans);
}

void read_input(char *filename)
{
  char line[1024];
  int bytes, i, j;
  FILE *infile;

  if ((infile = fopen(filename, "r")) == NULL) {
      fprintf(stderr, "Error: no such file (%s)\n", filename);
      exit(1);
  }
  while (fgets(line, 1024, infile) != NULL)
      if (strtok(line, " \t\n") != 0)
          numObjects++;
  rewind(infile);
  while (fgets(line, 1024, infile) != NULL) {
      if (strtok(line, " \t\n") != 0) {
          /* ignore the id (first attribute): numAttributes = 1; */
          while (strtok(NULL, " ,\t\n") != NULL) numAttributes++;
          break;
      }
  }


  /* allocate space for attributes[] and read attributes of all objects */
  buf           = (float*) malloc(numObjects*numAttributes*sizeof(float));
  attributes    = (float**)malloc(numObjects*             sizeof(float*));
  bytes = sizeof(float)*numObjects*numAttributes;
  attributes[0] = (float*) malloc(bytes);
  for (i=1; i<numObjects; i++)
      attributes[i] = attributes[i-1] + numAttributes;
  rewind(infile);
  i = 0;
  while (fgets(line, 1024, infile) != NULL) {
      if (strtok(line, " \t\n") == NULL) continue; 
      for (j=0; j<numAttributes; j++) {
          buf[i] = atof(strtok(NULL, " ,\t\n"));
          i++;
      }
  }
  fclose(infile);
}

void calc_radius(std::vector<double> &radius, std::vector<int> &membership, 
    std::vector< std::vector<float> > &centers)
{
  int i, j;
  for ( i=0; i< numClusters; ++i)
  {
    double dist = FLT_MAX, t;
    for ( j=0; j<numObjects; ++j )
    {
      t = euclid_dist_2(centers[i].data() , attributes[i], numAttributes);
      if ( t > dist )
        dist = t;
    }
    radius.push_back(dist);
  }
}

int main(int argc, char* argv[])
{
  std::vector< std::vector<float> > centers[2];
  std::vector<double> radius[2];
  std::vector<int>  memberships[2];
  int points, points2;
  int p1, p2, i, j, faults;
  int identical = 1;
  double cluster_error = 0, cluster_tot=0.0;
  double radius_err, radius_total = 0;

  if ( argc != 4 )
  {
    std::cout << "Usage: " << argv[0] << "\"OutputBin\" \"CorrectOutputBin\" "
                 "\"Experiment\"" << std::endl;
    return 1;
  }
  
  std::ifstream input[2];
  input[0].open(argv[1], std::ifstream::in);
  input[1].open(argv[2], std::ifstream::in);
  read_input(argv[3]);
  
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


    identical &= (p1==p2);
  }
  
  for ( i=0; i<numClusters ; ++i )
  {
    std::vector<float> c[2];

    for ( j=0; j<numAttributes; ++j )
    {
      float e1, e2;
      double t;
      
      input[0].read((char*)&e1, sizeof(float));
      input[1].read((char*)&e2, sizeof(float));
      
      t = fabs(e1-e2);
      cluster_tot += fabs(e2);
      cluster_error += t;
      identical &= (*((int*)&e1) == *((int*)&e2));
      c[0].push_back(e1);
      c[1].push_back(e2);
    }
    centers[0].push_back(c[0]);
    centers[1].push_back(c[1]);
  }

  input[0].close();
  input[1].close();



 /* if ( identical )
  {
    std::cout << "strict";
    return 0;
  } */
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
  double rel_cluster_err = 0;
  double rel_radius_err = 0;


  calc_radius(radius[0], memberships[0], centers[0]);
  calc_radius(radius[1], memberships[1], centers[1]);
  
  for ( i=0; i<numClusters; ++i)
  {
    double t;

    t = fabs(radius[0][i] - radius[1][i]);
    radius_err += t;
    radius_total += radius[1][i];
  }
  
  rel_radius_err = radius_err/radius_total;

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

  //std::cout << "Err: " << err << " out of " << total << " (" << rel_err << "%)" << std::endl;
  //std::cerr << "INNFO: " << rel_err << " " << rel_cluster_err << std::endl;
  //if ( rel_err < 0.1 && rel_cluster_err < 0.1 )
  if ( rel_radius_err < 0.1 )
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
