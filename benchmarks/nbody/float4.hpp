#ifndef __FLOAT4_INCLUDE
#define __FLOAT4_INCLUDE

typedef float fptype;

typedef struct CL_FLOAT4 cl_float4;

struct CL_FLOAT4
{
  fptype x, y, z, w;
  
  
  CL_FLOAT4()  {}

  CL_FLOAT4(fptype a)
  {
    x = y = z = w = a;
  }

  inline cl_float4 operator-()
  {
    cl_float4 ret;
    ret.x=-x;
    ret.y=-y;
    ret.z=-z;
    ret.w=-w;

    return ret;
  }

  inline cl_float4 operator-(const cl_float4 &b)
  {
    cl_float4 ret;
    ret.x=x-b.x;
    ret.y=y-b.y;
    ret.z=z-b.z;
    ret.w=w-b.w;

    return ret;
  }

  inline cl_float4 operator+(const fptype &b)
  {
    cl_float4 ret;

    ret.x=x+b;
    ret.y=y+b;
    ret.z=z+b;
    ret.w=w+b;

    return ret;
  }

  inline cl_float4 operator+(const cl_float4 &b)
  {
    cl_float4 ret;
    ret.x=x+b.x;
    ret.y=y+b.y;
    ret.z=z+b.z;
    ret.w=w+b.w;

    return ret;
  }

  inline cl_float4 &operator+=(const cl_float4 &b)
  {
    x+=b.x;
    y+=b.y;
    z+=b.z;
    w+=b.w;
    return *this;
  }


  inline cl_float4 operator*(const cl_float4 &b)
  {
    cl_float4 ret;

    ret.x=x*b.x;
    ret.y=y*b.y;
    ret.z=z*b.z;
    ret.w=w*b.w;

    return ret;
  }

  inline cl_float4 operator*(const fptype &b)
  {
    cl_float4 ret;

    ret.x=x*b;
    ret.y=y*b;
    ret.z=z*b;
    ret.w=w*b;

    return ret;
  }

  inline cl_float4 operator/(const fptype &b)
  {
    cl_float4 ret;

    ret.x=x/b;
    ret.y=y/b;
    ret.z=z/b;
    ret.w=w/b;

    return ret;
  }



};




#endif
