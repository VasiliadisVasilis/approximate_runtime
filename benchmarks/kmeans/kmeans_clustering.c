/*****************************************************************************/
/*IMPORTANT:  READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.         */
/*By downloading, copying, installing or using the software you agree        */
/*to this license.  If you do not agree to this license, do not download,    */
/*install, copy or use the software.                                         */
/*                                                                           */
/*                                                                           */
/*Copyright (c) 2005 Northwestern University                                 */
/*All rights reserved.                                                       */

/*Redistribution of the software in source and binary forms,                 */
/*with or without modification, is permitted provided that the               */
/*following conditions are met:                                              */
/*                                                                           */
/*1       Redistributions of source code must retain the above copyright     */
/*        notice, this list of conditions and the following disclaimer.      */
/*                                                                           */
/*2       Redistributions in binary form must reproduce the above copyright   */
/*        notice, this list of conditions and the following disclaimer in the */
/*        documentation and/or other materials provided with the distribution.*/ 
/*                                                                            */
/*3       Neither the name of Northwestern University nor the names of its    */
/*        contributors may be used to endorse or promote products derived     */
/*        from this software without specific prior written permission.       */
/*                                                                            */
/*THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS    */
/*IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED      */
/*TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT AND         */
/*FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL          */
/*NORTHWESTERN UNIVERSITY OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,       */
/*INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES          */
/*(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR          */
/*SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)          */
/*HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,         */
/*STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN    */
/*ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE             */
/*POSSIBILITY OF SUCH DAMAGE.                                                 */
/******************************************************************************/
/*************************************************************************/
/**   File:         kmeans_clustering.c                                 **/
/**   Description:  Implementation of regular k-means clustering        **/
/**                 algorithm                                           **/
/**   Author:  Wei-keng Liao                                            **/
/**            ECE Department, Northwestern University                  **/
/**            email: wkliao@ece.northwestern.edu                       **/
/**                                                                     **/
/**   Edited by: Jay Pisharath                                          **/
/**              Northwestern University.                               **/
/**                                                                     **/
/**   ================================================================  **/
/**																		**/
/**   Edited by: Sang-Ha  Lee											**/
/**				 University of Virginia									**/
/**																		**/
/**   Description:	No longer supports fuzzy c-means clustering;	 	**/
/**					only regular k-means clustering.					**/
/**					Simplified for main functionality: regular k-means	**/
/**					clustering.											**/
/**                                                                     **/
/*************************************************************************/

#include "kmeans.h"
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <runtime.h>
#include <m5op.h>
#include <sys/mman.h>
#include <unistd.h>

#ifndef PROTECT
#define assert(X) 
#endif

#define CALCULATE_CENTER

#define OPTIMIZED_CENTER_COMPUTATION
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

extern int granularity;
extern double wtime(void);

volatile int      sum_delta;
typedef struct CLUSTER_T
{
  int *points;
#ifdef CALCULATE_CENTER
  float *prev_center;
  float prev_center_delta;
#endif
  int new_points, old_points;
  int delta, prev_new_points;
} cluster_t;


typedef struct ARG_T
{
  int nfeatures, nclusters, tid, work, npoints;
} arg_t;


float    **new_centers;				/* [nclusters][nfeatures] */
float    **clusters;					/* out: [nclusters][nfeatures] */
float    **feature;
int       *membership;
int       *membership_prev;
#ifdef CALCULATE_RADIUS
float     *radius, *radius_prev;
#endif
char      *moved;
cluster_t *cluster_points;


void add_point_to_cluster(int p_id, int c_id);

void calculateRadius(
    float **cluster_coords,         /* [npts][nfeatures] */
    float **points_coords,
    int     npoints,
    int     nclusters, 
    int     nfeatures,
    float  *radius);

int find_nearest_point(float  *pt,          /* [nfeatures] */
    int     point_id,
    int     nfeatures,
    float **pts,         /* [npts][nfeatures] */
    int     npts)
{
  int index, i;
  float ret_dist=FLT_MAX;
  float dist;
  index = -1;

  /* find the cluster center id with min distance to pt */
  for (i=0; i<npts; i++) {
    dist = euclid_dist_2(pt, pts[i], nfeatures);  /* no need square root */
    if (dist < ret_dist) {
      ret_dist =  dist;
      index    = i;
    }
  }
  assert(index!=-1);
  return(index);
}

#if 0 
int kmeans_grc(void* _args)
{
  arg_t   *args      = (arg_t*) _args;
  int      nfeatures = args->nfeatures;
  int      nclusters = args->nclusters;
  int      npoints   = args->npoints;
  int i;

  calculateRadius(clusters, feature, npoints, nclusters, nfeatures, radius);

  return SANITY_SUCCESS;
} 
#endif

#if 0
int kmeans_trc(void* _args, void *none)
{

  arg_t   *args      = (arg_t*) _args;
  int      nfeatures = args->nfeatures;
  int      nclusters = args->nclusters;
  int      npoints   = args->npoints;
  int      work      = args->work;
  int      tid       = args->tid;
  int start, end, i, index, j, my_cluster;
  float dst;

  start = tid*work;
  end = start+work;
  end = end > npoints ? npoints : end;
  for (i=start; i<end; i++) 
  {
    my_cluster = membership[i];
    dst = fabs(radius[my_cluster] - radius_prev[my_cluster]);
    if ( dst/radius_prev[my_cluster] > RADIUS_THRESHOLD )
    {
      cluster_fixme[my_cluster] = 1;

      if ( fabs(distance[i] - radius[my_cluster] )/radius[my_cluster] )
      {
        membership[i] = membership_prev[i];
        distance[i] = distance_prev[i];
        /* vasiliad: remove from current cluster */
        partial_new_centers_len[tid][my_cluster] --;
        for (j=0;j<nfeatures; ++j)
          partial_new_centers[tid][my_cluster][j] -= feature[i][j];
        /* vasiliad: add to previous cluster */
        my_cluster = membership[i];

        partial_new_centers_len[tid][index]++;				
        for (j=0; j<nfeatures; j++)
          partial_new_centers[tid][index][j] += feature[i][j];
      }
    }
  }
  return SANITY_SUCCESS;
}
#endif

/*----< euclid_dist_2() >----------------------------------------------------*/
/* multi-dimensional spatial Euclid distance square */
  __inline
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
#ifdef CALCULATE_CENTER
    cluster_points[i].prev_center = (float*) calloc(nfeatures, sizeof(float));
    cluster_points[i].prev_center_delta = 1.0;
#endif
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

#ifdef SANITY
int reset_point(void* _args, void *dummy)
{
  arg_t   *args      = (arg_t*) _args;
  int      npoints   = args->npoints;
  int      tid       = args->tid;
  int      work      = args->work;
  int start, end, i, index;

  start = tid*work;
  end = start+work;
  end = end > npoints ? npoints : end;
  for (i=start; i<end; i++) 
  {
    if (moved[i] == MOVED)
    {
      index = membership_prev[i];
      membership[i] = index;
      add_point_to_cluster(i, index);
    }
  }
  return SANITY_SUCCESS;
}
#endif

void process_point(void* _args, unsigned int task_id, unsigned int significance) 
{
  arg_t   *args      = (arg_t*) _args;
  int      nfeatures = args->nfeatures;
  int      nclusters = args->nclusters;
  int      npoints   = args->npoints;
  int      tid       = args->tid;
  int      work      = args->work;
  int start, end, i, index;

#ifdef GEMFI
  if ( significance == NON_SIGNIFICANT )
  {
    fi_activate_inst(task_id, START);
  }
#endif
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
#ifdef GEMFI
  if ( significance == NON_SIGNIFICANT )
  {
    fi_activate_inst(task_id, PAUSE);
  }
#endif
}

#ifdef CALCULATE_RADIUS
void calculateRadius(
    float **cluster_coords,         /* [npts][nfeatures] */
    float **points_coords,
    int     npoints,
    int     cluster, 
    int     nfeatures,
    float  *radius)
{
  int i;
  float dist;
  cluster_t *p;

  p = cluster_points + cluster;
  radius[cluster] = 0;

  for (i=0; i < p->old_points; i++) 
  {
    dist = euclid_dist_2(points_coords[p->points[i]], cluster_coords[cluster],
        nfeatures);
    if (dist > radius[cluster]) 
    {
      radius[cluster] = dist;
    }
  }
}
#endif

void process_cluster(void *_args, unsigned int task_id, unsigned int significance)
{
  arg_t   *args      = (arg_t*) _args;
  int      nfeatures = args->nfeatures;
  int      tid       = args->tid;
  int i, j, delta = 0,  old_points, cur;
  int pos, tail;
  int deleted = 0;
  cluster_t *p;



  p = cluster_points + tid;
  old_points = p->old_points;
#ifdef OPTIMIZED_CENTER_COMPUTATION
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
#endif
  delta = p->new_points;
  /* vasiliad: compute how much the removal of old points affects the center
     and replace old points with2.26104e+06 radius new whenever possible */
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

  p->old_points = p->old_points+ p->new_points;
  p->new_points = 0;
  if ( p->old_points <= 0 )
  {
    p->old_points = 1;
    memcpy(clusters[tid], feature[p->points[0]],  sizeof(float)*nfeatures);
  }
  assert(p->old_points>0);
  /* vasiliad: update the center */
#ifdef OPTIMIZED_CENTER_COMPUTATION
  for (i=0; i<nfeatures; ++i )
  {
    new_centers[tid][i] = (old_points*clusters[tid][i] + new_centers[tid][i])
      /(float)(p->old_points);
  } 

  for ( i=0; i<nfeatures; ++i )
  {
    clusters[tid][i] = new_centers[tid][i];
  }
#else
  for ( j=0; j<nfeatures; ++j )
  {
    new_centers[tid][j] = 0;
  }
  for ( i=0; i<p->old_points; ++i )
  {
    for ( j=0; j<nfeatures; ++j )
    {
      clusters[tid][j] += feature[p->points[i]][j];
    }
  }
  for ( j=0; j<nfeatures; ++j )
  {
    clusters[tid][j] /= (float) p->old_points;
  }
#endif
  p->delta = abs(p->old_points - old_points);
  __sync_fetch_and_add(&sum_delta, delta);
}

  void
calculateCenterDelta(int nfeatures, int nclusters)
{
  int i;
  float delta;
  cluster_t *p;

  for (p = cluster_points, i=0; i<nclusters; ++i, ++p )
  {
    delta = euclid_dist_2(p->prev_center, clusters[i], nfeatures);
//    float rel;
//    rel = fabs((delta-p->prev_center_delta)/(p->prev_center_delta+1e-15));
//    printf("Cluster %d moved %g %g\n",i, delta, rel);
    p->prev_center_delta = delta;
    memcpy(p->prev_center, clusters[i], nfeatures*sizeof(float));
  }

}


void clear_move_status(void* _args)
{
  arg_t *args = (arg_t*)_args;
  int nclusters;
  int i, j;
  cluster_t *p;

  nclusters = args->nclusters;

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

#if 0
void 
reset_last_iteration(int nclusters, int npoints, int nfeatures, int ntasks, int loop,
  int work)
{
  int tid, i;
  arg_t args;
  char group_name[30];
  
  sprintf(group_name, "pnt_'%d", loop);
  for ( tid=0; tid<ntasks; ++tid)
  {
    task_t *task;
    args.nfeatures = nfeatures;
    args.nclusters = nclusters;
    args.tid       = tid;
    args.work      = work;
    args.npoints   = npoints;

    task = new_task(reset_point, &args, sizeof(args), NULL, NULL, 0,
        SIGNIFICANT, 0);
    push_task(task, group_name);
    //process_point(&args);
  }

  wait_group(group_name, NULL, NULL, SYNC_RATIO, 0, 0, 1.0f, 0);

  task_t *task;
  for ( i=0; i<nclusters; ++i )
  {
    args.tid = i;
    args.nfeatures = nfeatures;
    args.npoints   = npoints;
    sprintf(group_name, "clstr_'%d", loop);
    task = new_task(process_cluster,  /* function */
        &args, sizeof(args),          /* args, arg_size */
        NULL, NULL, 0,                /* san_funct, args, arg_size */
        SIGNIFICANT, 0);              /* significance, redo */
    push_task(task, group_name);
  }
  args.nfeatures = nfeatures;
  args.npoints = npoints;
  args.nclusters = nclusters;
  wait_group(group_name, NULL, NULL, SYNC_RATIO, 0, 0, 1.0f, 0);
}
#endif
/*----< kmeans_clustering() >---------------------------------------------*/
float** kmeans_clustering(float **_feature,    /* in: [npoints][nfeatures] */
    int     nfeatures,
    int     npoints,
    int     nclusters,
    float   threshold,
    int    *_membership, /* out: [npoints] */
    int     loopMax)
{
  int      i, loop=0;
  int      tid, work;
  int      ntasks;
  long bytes, page;

  char group_name[256];

  ntasks   = granularity;
  feature    = _feature;
  membership = _membership;

  /* allocate space for returning variable clusters[] */
  moved         = (char*) calloc(npoints, sizeof(char));
#ifdef CALCULATE_RADIUS
  radius        = (float*) calloc(nclusters, sizeof(float));
  radius_prev   = (float*) calloc(nclusters, sizeof(float));
#endif

  clusters  = (float**) malloc(nclusters * sizeof(float*));
  bytes = sizeof(float)*nclusters*nfeatures;
  page = sysconf(_SC_PAGESIZE);
  bytes = ceil(bytes/(double)page) * page;
  posix_memalign((void**)&clusters[0], page, bytes);
  for (i=1; i<nclusters; i++)
    clusters[i] = clusters[i-1] + nfeatures;

  membership_prev = (int*) calloc(npoints, sizeof(int));


  clusters_init(nclusters, npoints, nfeatures);

  for (i=0; i<npoints; i++)
    membership[i] = -1;

  new_centers    = (float**) malloc(nclusters *            sizeof(float*));
  new_centers[0] = (float*)  calloc(nclusters * nfeatures, sizeof(float));
  for (i=1; i<nclusters; i++)
    new_centers[i] = new_centers[i-1] + nfeatures;

  work = npoints/ntasks;
  do {
    arg_t args;
    sum_delta = 0.0;
    sprintf(group_name, "pnt_%d", loop);
#ifdef PROTECT
    mprotect(clusters[0], bytes, PROT_READ);
#endif
    /*vasiliad: THIS IS THE TASK!*/
    for ( tid=0; tid<ntasks; ++tid)
    {
      task_t *task;
      args.nfeatures = nfeatures;
      args.nclusters = nclusters;
      args.tid       = tid;
      args.work      = work;
      args.npoints   = npoints;
#ifdef SANITY
      task = new_task(process_point, &args, sizeof(args), reset_point, NULL, 0,
          NON_SIGNIFICANT, 0);
#else
      task = new_task(process_point, &args, sizeof(args), NULL, NULL, 0,
          NON_SIGNIFICANT, 0);
#endif
      push_task(task, group_name);
      //process_point(&args);
    }

#ifdef PROTECT
    wait_group(group_name, NULL, NULL, SYNC_RATIO | SYNC_TIME, 16, 0, 1.0f, 0);
    mprotect(clusters[0], bytes, PROT_READ|PROT_WRITE);
#else
    wait_group(group_name, NULL, NULL, SYNC_RATIO, 0, 0, 1.0f, 0);
#endif
    /*vasiliad: END OF TASK */

    /* vasiliad: Each cluster tries to incorporate its new points. If the accumulated
       radius changes more than THRESHOLD_PERCENT of its previous value
       it disregards all remaining points. They will be returned to their
       previous cluster.*/
    task_t *task;
    for ( i=0; i<nclusters; ++i )
    {
      args.tid = i;
      args.nfeatures = nfeatures;
      args.npoints   = npoints;
      sprintf(group_name, "clstr_%d", loop);
      task = new_task(process_cluster,  /* function */
          &args, sizeof(args),          /* args, arg_size */
          NULL, NULL, 0,                /* san_funct, args, arg_size */
          SIGNIFICANT, 0);              /* significance, redo */
      push_task(task, group_name);
    }
    args.nfeatures = nfeatures;
    args.npoints = npoints;
    args.nclusters = nclusters;
    wait_group(group_name, NULL, NULL, SYNC_RATIO, 0, 0, 1.0f, 0);

    clear_move_status(&args);
//    printf("*** Changed %d\n", sum_delta);

    if ( loop == 0 )
    {
#ifdef CALCULATE_RADIUS
      for ( i=0 ;i<nclusters; ++i)
      {
        calculateRadius(clusters, feature, npoints, i, nfeatures, radius);
      }
#endif
#ifdef CALCULATE_CENTER
      for ( i=0; i<nclusters; ++i )
      {
        memcpy(cluster_points[i].prev_center, feature[i], nfeatures*sizeof(float));
      }
#endif
      memset(moved, NOT_MOVED, sizeof(char)*npoints);
    }
    int points = 0;
    for ( i=0; i<nclusters; ++i)
    {
      points += cluster_points[i].old_points;
#ifdef CALCULATE_RADIUS
      radius_prev[i] = radius[i];
      calculateRadius(clusters, feature, npoints, i, nfeatures, radius);
#endif
    }

    //printf("SumPoints %d\n", points);
    //assert(points == npoints);
#ifdef CALCULATE_RADIUS
    for ( i=0; i<nclusters; ++i)
    {
      cluster_t *p = cluster_points + i;
      float a = fabs(radius_prev[i] - radius[i])/radius_prev[i];
      a *= 100.0f;
      printf("Cluster %d contains %d and has %g %g radius %f xxx %f\n", i, cluster_points[i].old_points, radius_prev[i],radius[i], a, a*(p->delta/(float)p->old_points));
    }
#endif
#ifdef CALCULATE_CENTER
    calculateCenterDelta(nfeatures, nclusters);
#endif

  } while (sum_delta > threshold && ++loop < loopMax);

  printf("Delta = %d # %lf\n", sum_delta, threshold);
  printf("Loops = %d\n", loop);
  free(new_centers[0]);
  free(new_centers);

  return clusters;
}

