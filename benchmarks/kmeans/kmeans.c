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
/**   File:         example.c                                           **/
/**   Description:  Takes as input a file:                              **/
/**                 ascii  file: containing 1 data point per line       **/
/**                 binary file: first int is the number of objects     **/
/**                              2nd int is the no. of features of each **/
/**                              object                                 **/
/**                 This example performs a fuzzy c-means clustering    **/
/**                 on the data. Fuzzy clustering is performed using    **/
/**                 min to max clusters and the clustering that gets    **/
/**                 the best score according to a compactness and       **/
/**                 separation criterion are returned.                  **/
/**   Author:  Wei-keng Liao                                            **/
/**            ECE Department Northwestern University                   **/
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <sys/types.h>
#include <fcntl.h>
#include <omp.h>
#include <sys/mman.h>
#include <unistd.h>
#include <runtime.h>
#include <unistd.h>
#include <assert.h>
#include "getopt.h"
#include "kmeans.h"

int granularity = 10;

/*---< usage() >------------------------------------------------------------*/
void usage(char *argv0) {
    char *help =
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
           float  *buf;
           float **attributes;
           float **cluster_centres=NULL;
           int     i, j;
                
           int     numAttributes;
           int     numObjects;        
           char    line[1024];           
           int     isBinaryFile = 0;
           int     nloops = 1;
           int     nloopsMax = 10;
           float   threshold = 0.001;
		   long timing;		   

	while ( (opt=getopt(argc,argv,"g:m:i:k:t:b:n:?"))!= EOF) {
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
            case '?': usage(argv[0]);
                      break;
            default: usage(argv[0]);
                      break;
        }
    }


    if (filename == 0) usage(argv[0]);

    numAttributes = numObjects = 0;

    /* from the input file, get the numAttributes and numObjects ------------*/
   
    if (isBinaryFile) {
        int infile;
        if ((infile = open(filename, O_RDONLY, "0600")) == -1) {
            fprintf(stderr, "Error: no such file (%s)\n", filename);
            exit(1);
        }
        if ( read(infile, &numObjects,    sizeof(int)) != sizeof(int) 
            || read(infile, &numAttributes, sizeof(int)) != sizeof(int) ) 
        {
          assert(0 && "Could not read header");
        }
   

        /* allocate space for attributes[] and read attributes of all objects */
        buf           = (float*) malloc(numObjects*numAttributes*sizeof(float));
        attributes    = (float**)malloc(numObjects*             sizeof(float*));
        //attributes[0] = (float*) malloc(numObjects*numAttributes*sizeof(float));
        bytes = sizeof(float)*numObjects*numAttributes;
        page = sysconf(_SC_PAGESIZE);
        bytes = ceil(bytes/(double)page) * page;
        attributes[0] = NULL;
        if ( posix_memalign((void**)&attributes[0], page, bytes) )
        {
          assert(0 && "Could not allocate memory for points");
        }

        for (i=1; i<numObjects; i++)
            attributes[i] = attributes[i-1] + numAttributes;

        if ( read(infile, buf, numObjects*numAttributes*sizeof(float))
          != numObjects*numAttributes*sizeof(float) )
        {
          assert(0 && "Could not read point features from file");
        }
        

        close(infile);
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
        buf           = (float*) malloc(numObjects*numAttributes*sizeof(float));
        attributes    = (float**)malloc(numObjects*             sizeof(float*));
        bytes = sizeof(float)*numObjects*numAttributes;
        page = sysconf(_SC_PAGESIZE);
        bytes = ceil(bytes/(double)page) * page;
        attributes[0] = NULL;
        if ( posix_memalign((void**)&attributes[0], page, bytes) )
        {
          assert(0 && "Could not allocate memory for points");
        }
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
	printf("I/O completed\n");	
	memcpy(attributes[0], buf, numObjects*numAttributes*sizeof(float));
  non_sig = num_omp_threads/2;
  if ( non_sig == 0 )
    non_sig = 1;

#ifdef PROTECT 
  mprotect(attributes[0], bytes, PROT_READ);
#endif

#ifdef GEMFI
  m5_switchcpu();
#endif
  init_system(num_omp_threads-non_sig, non_sig);
	timing = my_time();
#ifdef GEMFI
 m5_dumpreset_stats(0,0); 
#endif
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
#ifdef GEMFI
 m5_dumpreset_stats(0,0); 
#endif
  timing = my_time() - timing;

#ifdef GEMFI
  m5_writefile(numObjects, sizeof(int), 0);
  for ( i=0; i<numObjects; ++i )
  {
    m5_writefile(membership[i], sizeof(int), (1+i)*sizeof(int));
  }
  for ( i=0; i<nclusters*numAttributes; ++i)
  {
    m5_writefile(*((int*)(&membership[i])), sizeof(float),
        (1+numObjects)*sizeof(int)+i*sizeof(float));
  }
#else
  FILE* out = fopen("out.bin", "wb");
  fwrite(&numObjects, sizeof(long), 1, out);
  fwrite(membership, sizeof(long), numObjects, out);
  fwrite(cluster_centres[0], sizeof(double), nclusters*numAttributes, out);
  fclose(out);
#endif
  free(membership);
  printf("number of Points %d\n", numObjects);
	printf("number of Clusters %d\n",nclusters); 
	printf("number of Attributes %d\n\n",numAttributes); 
  /*  	printf("Cluster Centers Output\n"); 
	printf("The first number is cluster number and the following data is arribute value\n");
	printf("=============================================================================\n\n");
	
    for (i=0; i< nclusters; i++) {
		printf("%d: ", i);
        for (j=0; j<numAttributes; j++)
            printf("%.2f ", cluster_centres[i][j]);
        printf("\n\n");
    }
*/
    printf("Time for process[ms]: %f\n", timing/1000.0);

    free(attributes);
    free(cluster_centres[0]);
    free(cluster_centres);
    free(buf);
    return(0);
}

