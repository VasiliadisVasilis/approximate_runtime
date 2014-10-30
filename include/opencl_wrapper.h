/***************************************************************************
 *   Copyright (C) 2012-2013 by Spyrou Michalis                            *
 *   mispyrou@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "CL/cl.h"

/* This header contains all the function prototypes used by OpenCL 1.2.
 * We use them to load the functions adresses from the driver of each vendor. */

#ifndef _OPENCL_WRAPPER_H_
#define _OPENCL_WRAPPER_H_

struct _opencl_runtime_functions {

  /* clGetPlatformIDs */
  cl_int (*_clGetPlatformIDs)(cl_uint, void *, cl_uint *);

  /* clGetPlatformInfo */
  cl_int (*_clGetPlatformInfo)(void *,	cl_platform_info param_name,  size_t, void *,	size_t *);  

  /* clGetDeviceIDs */
  cl_int (*_clGetDeviceIDs)(void *, cl_device_type, cl_uint , void *, void *);

  /* clCreateContext */
  cl_context (*_clCreateContext)(cl_context_properties *, cl_uint ,const void *,	void *pfn_notify (const char *,const void *,size_t cb,void *),void *, 	cl_int *);

  /* clCreateCommandQueue */
  cl_command_queue (*_clCreateCommandQueue)( void *,	void *,cl_command_queue_properties,	cl_int *);

  /* clCreateBuffer */
  cl_mem (*_clCreateBuffer)( void *,cl_mem_flags ,	size_t size,	void *,	cl_int *);

  /* clCreateProgramWithBinary */
  cl_program (*_clCreateProgramWithBinary) ( void *,cl_uint ,	const void *, 	const size_t *,	const unsigned char **,	cl_int *, cl_int *);

  /* clCreateProgramWithSource */
  cl_program (*_clCreateProgramWithSource) ( void *,cl_uint ,	const char **, 	const size_t *,	cl_int *);

  /* clBuildProgram */
  cl_int (*_clBuildProgram) (void *,	cl_uint,const void *,const char *,void (*pfn_notify)(void *, void *),void *);

  /* clCreateKernel */
  cl_kernel (*_clCreateKernel)( void *,	const char *,	cl_int *);

  /* clSetKernelArg */
  cl_int (*_clSetKernelArg)( void *,	cl_uint ,	size_t ,	const void *);

  /* clEnqueueWriteBuffer */
  cl_int (*_clEnqueueWriteBuffer)(void *,void *,cl_bool ,	size_t ,size_t ,const void *,cl_uint ,const void *,	void *);

  /* clEnqueueNDRangeKernel */
  cl_int (*_clEnqueueNDRangeKernel)(void *,	void *,	cl_uint ,	const size_t *,	const size_t *,const size_t *,	cl_uint ,const void *,void *);

  /* clEnqueueReadBuffer */
  cl_int (*_clEnqueueReadBuffer)( void *,void *,	cl_bool ,size_t ,	size_t ,	void *,cl_uint ,const cl_event *,cl_event *);

  /* clReleaseMemObject */
  cl_int (*_clReleaseMemObject)( void * );
  
  /* clGetDeviceInfo */
  cl_uint (*_clGetDeviceInfo)( void *, cl_device_info, size_t ,	void *, size_t *);

  /* clFinish */
  cl_int (*_clFinish)(void *);

  /* clCreateSampler */
  cl_sampler (*_clCreateSampler)(void *, cl_bool ,cl_addressing_mode ,	cl_filter_mode ,	cl_int *);

  /* clCreateImage3D */
  cl_mem (*_clCreateImage3D)(void *,	cl_mem_flags ,	const cl_image_format *, size_t , size_t ,	size_t ,	size_t ,	size_t ,	void *, cl_int *);
  
  /* clCreateImage2D */
  cl_mem (*_clCreateImage2D)(void *, cl_mem_flags, const cl_image_format *, size_t ,	size_t , size_t , void *, cl_int *);

  /* clCreateImage */
  cl_mem (*_clCreateImage)(void *,	cl_mem_flags , const cl_image_format *, const cl_image_desc *, void *,	cl_int *);

  /* clReleaseCommandQueue */
  cl_int (*_clReleaseCommandQueue)(void *);

  /* clGetCommandQueueInfo */
  cl_int (*_clGetCommandQueueInfo)(void *, cl_command_queue_info , size_t ,	void *, size_t *);

  /* clEnqueueMarker */
  cl_int (*_clEnqueueMarker)( void *,void *);

  /* clReleaseEvent */
  cl_int (*_clReleaseEvent)(void *);

  /* clReleaseSampler */
  cl_int (*_clReleaseSampler)(void *);

  /* clReleaseContext */
  cl_int (*_clReleaseContext)(void *);

  /* clReleaseProgram */
  cl_int (*_clReleaseProgram)(void *);

  /* clReleaseKernel */
  cl_int (*_clReleaseKernel)(void *);

  /* clEnqueueAcquireGLObjects */
  cl_int (*_clEnqueueAcquireGLObjects) (void * ,cl_uint ,const void *,cl_uint ,	const void *, void *);

  /* clEnqueueReleaseGLObjects */
  cl_int (*_clEnqueueReleaseGLObjects) (void *,cl_uint ,const void *,	cl_uint ,const void *,void *);

  /* clGetGLObjectInfo */
  cl_int (*_clGetGLObjectInfo)(void *,  	void *,	void *);

  /* clGetGLTextureInfo */
  cl_int (*_clGetGLTextureInfo)(void *,  void *,	size_t ,void *,	size_t *);

  /* clCreateFromGLBuffer */
  cl_int (*_clCreateFromGLBuffer)( void *,cl_mem_flags ,void * ,cl_int * );

  /* clGetEventProfilingInfo */
  cl_int (*_clGetEventProfilingInfo)( void *,	void *,	size_t, 	void *,	size_t *);

  /* clGetKernelWorkGroupInfo */
  cl_int (*_clGetKernelWorkGroupInfo)( void *, void *,	cl_kernel_work_group_info ,	size_t,	void *,	size_t *);

};

typedef struct _opencl_runtime_functions opencl_runtime_functions;

/* Global arrays for vendors functions */
opencl_runtime_functions *nvidia_ocl, *intel_ocl;

/* Functions prototypes */

#endif /* _OPENCL_WRAPPER_H_ */
