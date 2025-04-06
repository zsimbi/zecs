# zecs
An ecs (entity component system) written from scratch in c.
The implementation may not be the best but it's okay for my games.

## Where are the _.c_ files?
This library is a header only library which means that the implementation is defined in the header and there are no .c files.

## How to use the library?
Before including the zecs.h do this **once** in a C file:
```
#define ZSTACK_IMPL
#define ZTABLE_IMPL
#define ZECS_IMPL
```
And with this the implementation is created.