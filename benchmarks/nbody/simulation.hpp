#ifndef __SIMULATION_HPP_
#define __SIMULATION_HPP_
#include <cstddef>
#include <vector>
#include "float4.hpp"
enum {BlockX=10, BlockY=10, BlockZ=10};


extern std::vector<int> particles[BlockZ][BlockY][BlockX];

void update(cl_float4* pos, cl_float4* color,
		     cl_float4* force, cl_float4* vel, float* potential,
		     float bound, float dt, int num, double waitRatio);

#endif

