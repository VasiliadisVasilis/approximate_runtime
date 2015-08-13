#include "simulation.hpp"
#include <iostream>
#include <cstdio>
#include <runtime.h>


typedef struct {
	cl_float4* pos, *color, *force, *vel;
	
	float *potential, bound, dt;
	int num;

	size_t idx;
} arg_t;

cl_float4 ZERO4 = cl_float4(0.0f);

cl_float4 clamp(cl_float4 val, cl_float4 low, cl_float4 high);


void update_task(arg_t *args);
void update_task_approx(arg_t *args);

void lj_force(cl_float4 pos1, cl_float4 pos2, cl_float4* force, float* pot);

void update(cl_float4* pos, cl_float4* color,
		     cl_float4* force, cl_float4* vel, float* potential,
		     float bound, float dt, int num, double waitRatio)
{
  int i =0;
  const int work = 1;
	arg_t args;
	task_t *task;
	static int ran = 0;

	char group[1024];

	sprintf(group, "%s%d", "exec", ran++);

	args.pos = pos;
	args.color = color;
	args.force = force;
	args.vel = vel;
	args.potential = potential;
	args.bound = bound;
	args.dt = dt;
	args.num = num;

  for ( i=0; i < num; i+=work )
  {
		args.idx = i;
		
		task = new_task((TASK)update_task, &args, sizeof(arg_t), (TASK)update_task_approx, 
			NON_SIGNIFICANT+1);
		push_task(task, group);
		// update_task(pos, color, force, vel, potential, bound, dt, num, i);
  }
	wait_group(group, NULL, NULL, SYNC_RATIO,0, 0,  waitRatio, 0);
}

double calc_distance(cl_float4 &p1, cl_float4 &p2)
{
	double dist, t;

	t = p1.x - p2.x;
	t = t * t;
	dist = t;

	t = p1.y - p2.y;
	t = t * t;
	dist += t;

	t = p1.z - p2.z;
	t = t * t;
	dist += t;

	return dist;
}

void update_task_approx(arg_t *args)
{
	cl_float4* pos, *color, *force, *vel;
	
	float *potential, bound, dt;
	int num;

	size_t idx;
	pos=args->pos;
	color=args->color;
	force=args->force;
	vel=args->vel;
	potential=args->potential;
	bound=args->bound;
	dt=args->dt;
	num=args->num;
	idx=args->idx;
	// Argon molecules properties
	// σ = 3.4e-10
	// ε = 1.65e-21
	// m = 6.69e-26
	// σ(m/ε)^(1/2) = 2.17e-12
	// (ε/m)^(1/2) = 1.57e+2
	// ε/σ = 4.85e-12
	// ε/σ^3 = 4.2e+7
	// ε/κb = 120
	
	cl_float4 p = pos[idx];    // [σ]
	cl_float4 v = vel[idx];    // [(ε/m)^(1/2)]
	cl_float4 f = force[idx];    // [ε/σ]
	cl_float4 a = f;     // [ε/σ/m]
	float dt2=0.5f*dt;
	float pot = 0.0f;
	
	float distance_epsilon = 2;

  cl_float4 b = pot;
	v += a * dt2;
	p += v * dt;
	
	if (p.x >= bound || p.x <= -bound)
		v.x = -v.x;
	if (p.y >= bound || p.y <= -bound)
		v.y = -v.y;
	if (p.z >= bound || p.z <= -bound)
		v.z = -v.z;
	
	p = clamp(p, -bound, bound);
	
	cl_float4 new_f = ZERO4;
	for (int i = 0; i < num; i++) {
		if (i!=idx)
		{
			double dist = calc_distance(pos[i], pos[idx]);
			//std::cout << dist << "\t";
			if ( dist < distance_epsilon )
			{
				lj_force(p, pos[i], &new_f, &pot);
			}
		}
	}
	//std::cout << "particles " <<  particles << std::endl;
	potential[idx] = pot;
	force[idx] = new_f;
	
	a = new_f;
	v += a * dt2;
	
	pos[idx] = p;
	vel[idx] = v;
	
	color[idx] = clamp((p + bound) / (2 * bound), 0.2f, 1.f);
	color[idx].w = 1.f;  // Leave alpha alone.
}

void update_task(arg_t *args)
{
	cl_float4* pos, *color, *force, *vel;
	
	float *potential, bound, dt;
	int num;

	size_t idx;
	pos=args->pos;
	color=args->color;
	force=args->force;
	vel=args->vel;
	potential=args->potential;
	bound=args->bound;
	dt=args->dt;
	num=args->num;
	idx=args->idx;

	// Argon molecules properties
	// σ = 3.4e-10
	// ε = 1.65e-21
	// m = 6.69e-26
	// σ(m/ε)^(1/2) = 2.17e-12
	// (ε/m)^(1/2) = 1.57e+2
	// ε/σ = 4.85e-12
	// ε/σ^3 = 4.2e+7
	// ε/κb = 120
	
	cl_float4 p = pos[idx];    // [σ]
	cl_float4 v = vel[idx];    // [(ε/m)^(1/2)]
	cl_float4 f = force[idx];    // [ε/σ]
	cl_float4 a = f;     // [ε/σ/m]
	float dt2=0.5f*dt;
	float pot = 0.0f;
	

  cl_float4 b = pot;
	v += a * dt2;
	p += v * dt;
	
	if (p.x >= bound || p.x <= -bound)
		v.x = -v.x;
	if (p.y >= bound || p.y <= -bound)
		v.y = -v.y;
	if (p.z >= bound || p.z <= -bound)
		v.z = -v.z;
	
	p = clamp(p, -bound, bound);
	
	cl_float4 new_f = ZERO4;
	for (int i = 0; i < num; i++) {
		if (i!=idx)
		{
			lj_force(p, pos[i], &new_f, &pot);
		}
	}
	potential[idx] = pot;
	force[idx] = new_f;
	
	a = new_f;
	v += a * dt2;
	
	pos[idx] = p;
	vel[idx] = v;
	
	color[idx] = clamp((p + bound) / (2 * bound), 0.2f, 1.f);
	color[idx].w = 1.f;  // Leave alpha alone.
}

void lj_force(cl_float4 pos1, cl_float4 pos2, cl_float4* force, float* pot) {
	float sigma = 1.25f;
	float sigma2 = sigma*sigma;
	float epsilon = 1.0f;
	cl_float4 dxyz = pos1 - pos2;
	float r2 = dxyz.x*dxyz.x + dxyz.y*dxyz.y + dxyz.z*dxyz.z;
	
	//  if (r2 > 1600.0f)
	//    return;
	
	if (r2 < 0.5625f)
		r2 = 0.5625f;
	
	float fr2 = sigma2/r2;
	float fr6 = fr2 * fr2 * fr2;
	float fpr = 48.0f * epsilon * fr6 * (fr6 - 0.5f) / r2;
	*force += dxyz*fpr;
	*pot += 4.0f * epsilon * fr6 * (fr6 - 1.0f);
}



cl_float4 clamp(cl_float4 val, cl_float4 low, cl_float4 high)
{
  if ( val.x < low.x )
    val.x = low.x;
  if ( val.y < low.y )
    val.y = low.y;
  if ( val.z < low.z )
    val.z = low.z;
  if ( val.w < low.w )
    val.w = low.w;

  if ( val.x > high.x )
    val.x = high.x;
  if ( val.y > high.y )
    val.y = high.y;
  if ( val.z > high.z )
    val.z = high.z;
  if ( val.w > high.w )
    val.w = high.w;
	
	return val;
}
