#include "license.h" // PLACED THE LICENSE HERE FOR READABILITY
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <sys/types.h>
#include <fcntl.h>
//#include <omp.h>
#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>
#include "getopt.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "kmeans.h"
#include <runtime.h>

#define RANDOM_MAX 2147483647

#ifndef FLT_MAX
#define FLT_MAX 3.40282347e+38
#endif
#ifndef RADIUS_THRESHOLD
#define RADIUS_THRESHOLD 0.8f
#endif

#ifndef THRESHOLD_RATIO
#define THRESHOLD_RATIO 0.3f
#endif

#ifndef DROP_POINTS_RATIO
#define DROP_POINTS_RATIO 0.3f
#endif

#define MOVED        1
#define NOT_MOVED    0

double RATIO = 1.0;

typedef void (*TASK) (void*);

typedef struct PROCESS_POINT_ARGS
{
	int nfeatures;
	int nclusters;
	int npoints;
	int tid;
	int work;
} process_point_args_t;


typedef struct PROCESS_CLUSTER_ARGS
{
	int nfeatures;
	int cluster;
} process_cluster_args_t;

typedef struct CLUSTER_T
{
  int *points;
  TYPE *prev_center;
  TYPE prev_center_delta;
  int new_points, old_points;
  int delta, prev_new_points;
} cluster_t;


void add_point_to_cluster(int p_id, int c_id);

void calculateRadius(
    float **cluster_coords,         /* [npts][nfeatures] */
    float **points_coords,
    int     npoints,
    int     nclusters, 
    int     nfeatures,
    float  *radius);

int granularity = 10;

volatile int      sum_delta;

TYPE    **new_centers;				/* [nclusters][nfeatures] */
TYPE    **clusters;					/* out: [nclusters][nfeatures] */
TYPE **feature;
int       *membership;
int       *membership_prev;
char      *moved;
cluster_t *cluster_points;


long this_time()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec*1000000+tv.tv_usec;
}

int find_nearest_point(TYPE *pt,          /* [nfeatures] */
    int     point_id,
    int     nfeatures,
    TYPE **pts,         /* [npts][nfeatures] */
    int     npts)
{
  int index, i;
  TYPE ret_dist;
  TYPE dist;
  index = -1;

	ret_dist = FLT_MAX;
	
  /* find the cluster center id with min distance to pt */
  for (i=0; i<npts; i++) {
    dist = euclid_dist_2(pt, pts[i], nfeatures);  /* no need square root */
		/*
		if ( point_id == 11 )
		{
			printf("dist: %g\n", dist._value().mid());
		} */
    if (dist < ret_dist) {
      ret_dist =  dist;
      index    = i;
    }
  }

	if ( index == -1 )
	{
		printf("%d\n", point_id);
	}
  assert(index!=-1);
  return(index);
}

/*----< euclid_dist_2() >----------------------------------------------------*/
/* multi-dimensional spatial Euclid distance square */
  __inline
TYPE euclid_dist_2(TYPE *pt1,
    TYPE *pt2,
    int    numdims)
{
  int i;
  TYPE ans;

	ans = 0;

  for (i=0; i<numdims; i++)
    ans = ans + (pt1[i]-pt2[i]) * (pt1[i]-pt2[i]);

  return(ans);
}

void add_point_to_cluster(int p_id, int c_id)
{
  int index;
  cluster_t *p = cluster_points + c_id;
  moved[p_id] = MOVED;
  membership[p_id] = c_id;
  index = __sync_fetch_and_add(&p->new_points, 1) + p->old_points;
  p->points[index] = p_id;
}

void clusters_init(int nclusters, int npoints, int nfeatures)
{
  int i, j;

  cluster_points = (cluster_t*) calloc(nclusters, sizeof(cluster_t));

  for ( i=0; i<nclusters; ++i )
  {
    cluster_points[i].prev_center = (TYPE*) malloc(sizeof(TYPE)*nfeatures);
    cluster_points[i].prev_center_delta = 1.0;
    cluster_points[i].points = (int*) calloc(npoints, sizeof(int));
    cluster_points[i].new_points = 0;
    cluster_points[i].old_points = 0;
    /* vasiliad: initial point is the i-th object */
    memset(cluster_points[i].points, 0xff, sizeof(int)*npoints);
    for (j=0; j<nfeatures; ++j)
    {
      clusters[i][j] = feature[i][j];
      membership[i] = i;
      membership_prev[i] = i;
    }
  }
}

/*vasiliad: we want to refrain from using global variables */
void process_point(process_point_args_t *args) 
{
	long nfeatures=args->nfeatures, nclusters=args->nclusters, 
		npoints=args->npoints, tid=args->tid, work=args->work;
  int start, end, i, index;

  start = tid*work;
  end = start+work;
  end = end > npoints ? npoints : end;

  for (i=start; i<end; i++) 
  {
    /* find the index of nestest cluster centers */					
    index = find_nearest_point(feature[i],
        i,
        nfeatures,
        clusters,
        nclusters);				
    /* if membership changes, increase delta by 1 */
    if (membership[i] != index)
    {
      membership[i] = index;
      add_point_to_cluster(i, index);
    }
  }
}

void process_cluster(process_cluster_args_t *args)
{
	long nfeatures=args->nfeatures, tid=args->cluster;
  int i, j, delta = 0,  old_points, cur;
  int pos, tail;
  int deleted = 0;
  cluster_t *p;

  p = cluster_points + tid;
  old_points = p->old_points;
  for ( j=0; j<nfeatures; ++j)
  {
    new_centers[tid][j] = 0;
  }

  /* vasiliad: calculate how much the new additions alter the cluster center */
  for ( i=0; i<p->new_points; ++i )
  {
    cur = p->points[old_points + i];
    for ( j=0; j<nfeatures; ++j )
    {
      new_centers[tid][j] += feature[cur][j];
    }
  }
  delta = p->new_points;
  /* vasiliad: compute how much the removal of old points affects the center
     and replace old points with new whenever possible */
  i = 0;
  tail = old_points + -1;
  deleted = 0;
  for ( pos=0, i=0; i<old_points; ++i )
  {
    if ( moved[p->points[pos]] == MOVED )
    {
      membership_prev[p->points[pos]] = tid;
      for ( j=0; j<nfeatures; ++j )
      {
        new_centers[tid][j] -= feature[p->points[pos]][j];
      }
      deleted ++;
      p->points[pos] = p->points[tail];
      tail--;
      p->old_points --;
    } 
    else
    {
      pos++;
    }
  }
  p->prev_new_points = p->new_points;
  /* vasiliad: copy in the remaining points */
  for ( i=0; i<p->new_points; ++i )
  {
    p->points[p->old_points+i] = p->points[old_points+i];
  }

  /* vasiliad: there might have been LOTS of faults, a cluster may have 0 points
               simply re-initialize the cluster ... */
  p->old_points = p->old_points+ p->new_points;
  p->new_points = 0;
  if ( p->old_points <= 0 )
  {
    p->old_points = 1;
    memcpy(clusters[tid], feature[tid],  sizeof(float)*nfeatures);
  }
  /* vasiliad: update the center */
  for (i=0; i<nfeatures; ++i )
  {
    new_centers[tid][i] = (old_points*clusters[tid][i] + new_centers[tid][i])
      /(float)(p->old_points);
  } 

  for ( i=0; i<nfeatures; ++i )
  {
    clusters[tid][i] = new_centers[tid][i];
  }

  p->delta = abs(p->old_points - old_points);
  __sync_fetch_and_add(&sum_delta, delta);
}

  void
calculateCenterDelta(int nfeatures, int nclusters)
{
  int i;
  TYPE delta;
  cluster_t *p;

  for (p = cluster_points, i=0; i<nclusters; ++i, ++p )
  {
    delta = euclid_dist_2(p->prev_center, clusters[i], nfeatures);
    p->prev_center_delta = delta;
    memcpy(p->prev_center, clusters[i], nfeatures*sizeof(float));
  }

}

void clear_move_status(long nclusters)
{
  int i, j;
  cluster_t *p;

  p = cluster_points;
  for ( i=0; i<nclusters; ++i, ++p )
  {
    if ( p->prev_new_points)
      for ( j=p->old_points-p->prev_new_points; j<p->old_points; ++j)
      {
        moved[p->points[j]] = NOT_MOVED;
      }
  }
}

/*----< kmeans_clustering() >---------------------------------------------*/
TYPE** kmeans_clustering(TYPE **_feature,    /* in: [npoints][nfeatures] */
    int     nfeatures,
    int     npoints,
    int     nclusters,
    int  threshold,
    int    *_membership, /* out: [npoints] */
    int     loopMax)
{
  int      i, loop=0;
  int      tid, work;
  int      ntasks;
  long bytes, page;

  ntasks   = granularity;
  feature    = _feature;
  membership = _membership;

  /* allocate space for returning variable clusters[] */
  moved         = (char*) calloc(npoints, sizeof(char));
  
  clusters  = (TYPE**) malloc(sizeof(TYPE*)*nclusters);
  clusters[0] = (TYPE*) malloc(sizeof(TYPE)*nclusters*nfeatures);
  for (i=1; i<nclusters; i++)
    clusters[i] = clusters[i-1] + nfeatures;

  membership_prev = (int*) calloc(npoints, sizeof(int));

  clusters_init(nclusters, npoints, nfeatures);

  for (i=0; i<npoints; i++)
    membership[i] = -1;

  new_centers    = (TYPE**) malloc(sizeof(TYPE*)*nclusters);
  new_centers[0] = (TYPE*) malloc(sizeof(TYPE)*nclusters * nfeatures);
  for (i=1; i<nclusters; i++)
    new_centers[i] = new_centers[i-1] + nfeatures;

  work = npoints/ntasks;
	process_point_args_t ppa;
	process_cluster_args_t pca;

	ppa.nfeatures = nfeatures;
	ppa.nclusters = nclusters;
	ppa.npoints = npoints;
	ppa.work = work;

	pca.nfeatures = nfeatures;

	task_t *task;

	int significance = 50;
	char group[1024];
  do {
    sum_delta = 0;
		sprintf(group, "points%d", loop);
    for ( tid=0; tid<ntasks; ++tid)
    {
			ppa.tid = tid;
			task = new_task((TASK)process_point, &ppa, sizeof(ppa), (TASK)process_point, significance);
			push_task(task, group);
      //process_point(nfeatures, nclusters, npoints, tid, work);
    }
		wait_group(group, NULL, NULL, SYNC_RATIO, 0, 0, RATIO, 0);
		
		sprintf(group, "clusters%d", loop);
    for ( i=0; i<nclusters; ++i )
    {	
			pca.cluster = i;
			task = new_task((TASK)process_cluster, &pca, sizeof(pca), NULL, 100);
			push_task(task, group);
    }
		wait_group(group, NULL, NULL, SYNC_RATIO, 0, 0, RATIO, 0);

    clear_move_status(nclusters);

    if ( loop == 0 )
    {
      for ( i=0; i<nclusters; ++i )
      {
        memcpy(cluster_points[i].prev_center, feature[i], nfeatures*sizeof(float));
      }
      memset(moved, NOT_MOVED, sizeof(char)*npoints);
    }
    int points = 0;
    for ( i=0; i<nclusters; ++i)
    {
      points += cluster_points[i].old_points;
    }
    calculateCenterDelta(nfeatures, nclusters);

  } while (++loop < loopMax && sum_delta > threshold );

  printf("#Delta = %d\n", sum_delta);
  printf("#Loops = %d\n", loop);

  return clusters;
}

/*---< usage() >------------------------------------------------------------*/
void usage(char *argv0) {
    const char *help =
        "Usage: %s [switches] -i filename\n"
        "       -i filename     		: file containing data to be clustered\n"
        "       -b                 	: input file is in binary format\n"
		    "       -k                 	: number of clusters (default is 5) \n"
        "       -t threshold		: threshold value\n"
		    "       -n no. of threads	: number of threads\n"
        "       -m no. of maximum loop iterations\n"
        "       -g no. of points each thread handles\n";

    fprintf(stderr, help, argv0);
    exit(-1);
}

/*---< cluster() >-----------------------------------------------------------*/
int* cluster(int      numObjects,      /* number of input objects */
    int      numAttributes,   /* size of attribute of each object */
    TYPE **attributes,      /* [numObjects][numAttributes] */            
    int      nclusters,
    int threshold,       /* in:   */
    TYPE ***cluster_centres, /* out: [best_nclusters][numAttributes] */
    int      loopMax
    )
{
  int    *membership;
  TYPE **tmp_cluster_centres;

  membership = (int*) malloc(numObjects * sizeof(int));

  srand(7);
  /* perform regular Kmeans */
  tmp_cluster_centres = kmeans_clustering(attributes,
      numAttributes,
      numObjects,
      nclusters,
      threshold,
      membership,
      loopMax);      

  if (*cluster_centres) {
    free((*cluster_centres)[0]);
    free(*cluster_centres);
  }
  *cluster_centres = tmp_cluster_centres;

  return membership;
}

/*---< main() >-------------------------------------------------------------*/
int main(int argc, char **argv) {
           int     num_omp_threads = 1;
           int     bytes  = 0;
           int     page;
           int    *membership;
           int     non_sig;
           int     opt;
    extern char   *optarg;
    extern int     optind;
           int     nclusters=5;
           char   *filename = 0;           
           TYPE **attributes;
           TYPE **cluster_centres=NULL;
           int     i, j;
                
           int     numAttributes;
           int     numObjects;        
           char    line[1024];           
           int     isBinaryFile = 0;
           int     nloops = 1;
           int     nloopsMax = 10;
           float   threshold = 0.001;
		   long timing;		   
	


	while ( (opt=getopt(argc,argv,"r:g:m:i:k:t:b:n:?"))!= EOF) {
		switch (opt) {
            case 'i': filename=optarg;
                      break;
            case 'b': isBinaryFile = 1;
                      break;
            case 't': threshold=atof(optarg);
                      break;
            case 'k': nclusters = atoi(optarg);
                      break;			
            case 'm': nloopsMax = atoi(optarg);
                      break;
            case 'n': num_omp_threads = atoi(optarg);
                      break;
            case 'g': granularity = atoi(optarg);
                      break;
						case 'r': RATIO = atof(optarg);
											break;
            case '?':
            default: usage(argv[0]);
                      break;
        }
    }
		

    if (filename == 0) usage(argv[0]);

    numAttributes = numObjects = 0;

    /* from the input file, get the numAttributes and numObjects ------------*/
   
    if (isBinaryFile) {
			assert(0 && "Not supported");
    }
    else {
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
        attributes    = (TYPE**) malloc(sizeof(TYPE*)*numObjects);
        attributes[0] = (TYPE*) malloc(sizeof(TYPE)*numObjects*numAttributes);
        for (i=1; i<numObjects; i++)
            attributes[i] = attributes[i-1] + numAttributes;
        rewind(infile);
        i = 0;
				int num_line = 0;
        while (fgets(line, 1024, infile) != NULL) {
            if (strtok(line, " \t\n") == NULL) continue; 
            for (j=0; j<numAttributes; j++) {
								double val = strtod(strtok(NULL, " ,\t\n"), NULL);
                attributes[0][i] = val;
                i++;
            }
						++num_line;
        }
        fclose(infile);
    }     

	long start, end;
	init_system(num_omp_threads);

  start = this_time();
   for (i=0; i<nloops; i++) {
        
        cluster_centres = NULL;
        membership = cluster(numObjects,
                numAttributes,
                attributes,           /* [numObjects][numAttributes] */                
                nclusters,
                threshold,
                &cluster_centres,
                nloopsMax
               );
     
    }
	end = this_time();
	shutdown_system();
#if 0
  FILE* out = fopen("out.bin", "wb");
  int pos = 0;
  for ( i=0; i<sizeof(numObjects); ++i, ++pos)
  {
    fwrite(((unsigned char*)&numObjects)+i, sizeof(char), 1, out);
  }
  for ( i=0; i<sizeof(int)*numObjects; ++i, ++pos )
  {
    fwrite(((unsigned char*)membership)+i, sizeof(char), 1, out);
  }
  for ( i=0; i<sizeof(float)*nclusters*numAttributes; ++i, ++pos)
  {
    fwrite((unsigned char*)(cluster_centres[0])+i, sizeof(char), 1, out);
  }
  fclose(out);
#endif
  free(membership);
  printf("#number of Points %d\n", numObjects);
	printf("#number of Clusters %d\n",nclusters); 
	printf("#number of Attributes %d\n\n",numAttributes); 
	
	

  return(0);
}

