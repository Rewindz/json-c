#include <stdio.h>

#include "../src/json.h"


int main(int argc, char **argv)
{
  printf("Hello from %s!\n", argv[0]);
  __print_version();
  
  return 0;
}
