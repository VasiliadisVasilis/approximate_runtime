#ifndef __SIMULATION_HPP_
#define __SIMULATION_HPP_
#include <cstddef>
#include "float4.hpp"

void update(cl_float4* pos, cl_float4* color,
		     cl_float4* force, cl_float4* vel, float* potential,
		     float bound, float dt, int num);

#endif

