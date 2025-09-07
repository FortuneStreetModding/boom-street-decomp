#include "runtime/MWCPlusLib.h"
#include "runtime/NMWException.h"

DestructorChain* __global_destructor_chain;

void* __register_global_object(void* object, void* destructor, void* regmem) {
  ((DestructorChain*)regmem)->next = __global_destructor_chain;
  ((DestructorChain*)regmem)->destructor = destructor;
  ((DestructorChain*)regmem)->object = object;
  __global_destructor_chain = (DestructorChain*)regmem;

  return object;
}
