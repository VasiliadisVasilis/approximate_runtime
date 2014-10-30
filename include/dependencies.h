#ifndef __DEPENDENCIES__
#define __DEPENDENCIES__

#define IN  1
#define OUT 2
#define INOUT 3

typedef struct dependency{
  void *memory_location;
  unsigned int size;
  unsigned int type;
  unsigned int references;
}d_t;


pool_t *running_dependencies;

void add_dependency(pool_t *pool, pair_t *depend, unsigned int type);

#endif