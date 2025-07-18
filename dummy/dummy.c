#include <stdio.h>

#include "../src/json.h"


int main(int argc, char **argv)
{

  printf("Hello from %s!\n", argv[0]);
  __print_version();

  JSON_File jf = open_json_file("./json/example2.json", "r");
  parse_json_file(&jf);

  
  return 0;
}
