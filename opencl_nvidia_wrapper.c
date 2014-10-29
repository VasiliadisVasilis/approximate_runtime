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

#include <stdio.h>
#include <dlfcn.h>
#include "CL/cl.h"
#include "opencl_wrapper.h" 


/* nvidia_runtime_init( )
 *
 * Purpose: Initialize the NVIDIA OpenCL runtime functions.
 *
 * Parameters: None.
 *
 * Returns: Error code.
 *
 * Preconditions: Nothing.
 *
 * Postconditions: Nothing.
 */
int nvidia_runtime_init() {

  /* Don't use icd now, problem with dlopen. Must know full path */
  //char *nvidia_icd = "/etc/OpenCL/vendors/nvidia.icd";  
  char *nvidia_library = "/usr/lib/nvidia-current/libOpenCL.so";

  /* Handle for libOpenCL.so */
  void *dlhandle;

  /* Try to open libOpenCL.so */
  dlhandle = dlopen( nvidia_library, RTLD_LAZY | RTLD_NOW );

  if ( !dlhandle ) { 

#ifdef DEBUG_RUNTIME

    printf( "Cuda runtime library not found.\n" );

#endif

    return -1;
  }

  /***** Initialize NVIDIA OPENCL RUNTIME functions ******/
  nvidia_ocl = ( opencl_runtime_functions *) malloc ( sizeof ( opencl_runtime_functions ) );

  nvidia_ocl->_clGetPlatformIDs = dlsym(dlhandle, "clGetPlatformIDs");
  nvidia_ocl->_clGetDeviceIDs = dlsym(dlhandle, "clGetDeviceIDs");
  nvidia_ocl->_clCreateContext = dlsym(dlhandle, "clCreateContext");
  nvidia_ocl->_clCreateCommandQueue = dlsym(dlhandle, "clCreateCommandQueue");
  nvidia_ocl->_clCreateBuffer = dlsym(dlhandle, "clCreateBuffer");
  nvidia_ocl->_clCreateProgramWithBinary = dlsym(dlhandle, "clCreateProgramWithBinary");
  nvidia_ocl->_clCreateProgramWithSource = dlsym(dlhandle, "clCreateProgramWithSource");
  nvidia_ocl->_clBuildProgram = dlsym(dlhandle, "clBuildProgram");
  nvidia_ocl->_clCreateKernel = dlsym(dlhandle, "clCreateKernel");
  nvidia_ocl->_clSetKernelArg = dlsym(dlhandle, "clSetKernelArg");
  nvidia_ocl->_clEnqueueWriteBuffer = dlsym(dlhandle, "clEnqueueWriteBuffer");
  nvidia_ocl->_clEnqueueNDRangeKernel = dlsym(dlhandle, "clEnqueueNDRangeKernel");
  nvidia_ocl->_clEnqueueReadBuffer = dlsym(dlhandle, "clEnqueueReadBuffer");
  nvidia_ocl->_clReleaseMemObject = dlsym(dlhandle, "clReleaseMemObject");
  nvidia_ocl->_clGetDeviceInfo = dlsym(dlhandle, "clGetDeviceInfo");
  nvidia_ocl->_clFinish = dlsym(dlhandle, "clFinish");
  nvidia_ocl->_clCreateImage3D = dlsym(dlhandle, "clCreateImage3D");
  nvidia_ocl->_clCreateImage2D = dlsym(dlhandle, "clCreateImage2D");
  nvidia_ocl->_clCreateImage = dlsym(dlhandle, "clCreateImage");
  nvidia_ocl->_clCreateSampler = dlsym(dlhandle, "clCreateSampler");
  nvidia_ocl->_clReleaseSampler = dlsym(dlhandle, "clReleaseSampler");
  nvidia_ocl->_clReleaseCommandQueue = dlsym(dlhandle, "clReleaseCommandQueue");
  nvidia_ocl->_clReleaseEvent = dlsym(dlhandle, "clReleaseEvent");
  nvidia_ocl->_clReleaseContext = dlsym(dlhandle, "clReleaseContext");
  nvidia_ocl->_clReleaseProgram = dlsym(dlhandle, "clReleaseProgram");
  nvidia_ocl->_clReleaseKernel = dlsym(dlhandle, "clReleaseKernel");
  nvidia_ocl->_clEnqueueMarker = dlsym(dlhandle, "clEnqueueMarker");
  nvidia_ocl->_clGetEventProfilingInfo = dlsym(dlhandle, "clGetEventProfilingInfo");
  nvidia_ocl->_clGetKernelWorkGroupInfo = dlsym(dlhandle, "clGetKernelWorkGroupInfo");

  /* OpenGL */
  nvidia_ocl->_clEnqueueAcquireGLObjects = dlsym(dlhandle,"clEnqueueAcquireGLObjects");
  nvidia_ocl->_clEnqueueReleaseGLObjects = dlsym(dlhandle,"clEnqueueReleaseGLObjects");
  nvidia_ocl->_clGetGLObjectInfo = dlsym(dlhandle,"clGetGLObjectInfo");
  nvidia_ocl->_clGetGLTextureInfo = dlsym(dlhandle,"clGetGLTextureInfo");
  nvidia_ocl->_clCreateFromGLBuffer = dlsym(dlhandle,"clCreateFromGLBuffer");

  char *error;
  error = dlerror();
  if( error != NULL) {

    	fprintf( stderr, "%s\n", error );
    	fflush( stderr );


    	return -1;
  }



  return CL_SUCCESS;


}

/* int probe_nvidia_gpus( )
 *
 * Purpose: Probe the NVIDIA GPUs.
 *
 * Parameters: device: A CUDA enabled device.
 *
 * Returns: NUmber of detected CUDA capable devices. On failure, we return 0 number of devices.
 *
 * Preconditions: Nothing.
 *
 * Postconditions: Nothing.
 */
int probe_nvidia_gpus() {

  int number_of_nvidia_gpus = 1;
  int errcode;

  /* Get platforms and available devices */
  void * cpPlatform, *cdDevice;
  
  errcode = nvidia_ocl->_clGetPlatformIDs(1, &cpPlatform, NULL);
  if ( errcode != CL_SUCCESS )
    printf("Error %d in probe clGetPlatformIDs().\n",errcode);


  errcode = nvidia_ocl->_clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 10, &cdDevice, &number_of_nvidia_gpus);
  if ( errcode != CL_SUCCESS )
    printf("Error %d in probe clGetDeviceIDs().\n",errcode);


  return number_of_nvidia_gpus;

}
