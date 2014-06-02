d_t *create_dependecy(p_t *depend, unsigned int type){
  d_t *element = (d_t*) malloc (sizeof(d_t));
  assert(element);
  element->location = depend->start;
  element->size = depend->size;
}


int chk_depend(void *d1, void *d2){
  d_t *elem1 = (d_t*) d1;
  p_t *elem2 = (p_t*) d2;
  
  if(d1->start <= d2->start)
    if(d1->start + d1->size >= d2->start){
      printf("Overlapping need to recreate dependencies\n");
      return 1;
    }
    
  if(d1->start >= d2->start)
    if(d1->start <= d2->start + d2->size){
      printf("Overlapping need to recreate dependencies\n");
      return 1;
    }
    
    return 0;
}

void new_dependency(pool_t *pool, void *start, unsigned int size, unsigned int type, unsigned int ref){
  d_t *new = (d_t*) malloc (sizeof(d_t));
  assert(new);
  new->start = start;
  new->size = size;
  new->type = type;
  new->refereneces = ref; 
}

void add_dependency(pool_t *pool, pair_t *depend, unsigned int type){
  list_t elem = delete_element(pool,chk_depend, depend);
  void *start;
  void size;
  d_t *new;
  d_t *conflict;
  if(!elem){
    printf("No conflicting dependencies\n");
    return;
  }
  conflict = (d_t*) elem->args;
  
  if(conflict->start <= depend->start && conflict->start + conflict->size >= depend->start ){
    if(conflict->size + conflict->start <= depend->start + depend->size){
      start = conflict->start;
      size = start + conflict->size - depend->start;
      new_dependency(pool, start, size, conflict->type | type , conflict->refereneces);
      
      start = conflict->start + size;
      size = depend->start + conflict->size - size;
      new_dependency(pool, start, size, conflict->type | type , conflict->refereneces + 1);
      
      start = conflict->start + conflict->size;
      size = depend->size - size;
      new_dependency(pool, start, size, type , 1);
      
    }
    else{
      start = conflict->start;
      size = start + conflict->size - depend->start;
      new_dependency(pool, start, size, conflict->type | type , conflict->refereneces);
      
      start = conflict->start + size;
      size = depend->start + conflict->size - size;
      new_dependency(pool, start, size, conflict->type | type , conflict->refereneces + 1);
      
      start = start + size;
      size = conflict->start + conflict->size - start;
      new_dependency(pool, start, size, conflict->type, 1);
      
    }
    free(conflict);
  }
  
  if( conflict->start >= depend->start && depend->start + depend->size >= conflict->start ){
    if(depend->size + depend->start <= conflict->start + conflict->size){
      start = depend->start;
      size = conflict->start - start;
      new_dependency(pool, start, size, type , 1);
      
      start = conflict->start;
      size = conflict->start + conflict->size;
      new_dependency(pool, start, size, conflict->type | type , conflict->refereneces + 1);
      
      start = start + size;
      size = depend->start + depend->size - start;
      new_dependency(pool, start, size, type, 1);
    }
    else{
      start = depend->start;
      size = conflict->start - start;
      new_dependency(pool, start, size, type , 1);
      
      start = conflict->start;
      size = depend->size  - size ;
      new_dependency(pool, start, size, conflict->type | type , conflict->refereneces + 1);
      
      start = depend->start + depend->size;
      size = conflict->start + conflict->size - size;
      new_dependency(pool, start, size, conflict->type, conflict->refereneces);
    }
  }
     
}






