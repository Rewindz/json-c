#ifndef JSON_H_
#define JSON_H_

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct JSON_Object JSON_Object;
typedef struct JSON_KeyPair JSON_KeyPair;


typedef enum
  {
    Number,
    String,
    Boolean,
    Array,
    Object,
    Null
  } JSON_Types;

struct JSON_KeyPair
{
  JSON_Types type;
  char *key;
  size_t data_len;
  unsigned char *data;
};

struct JSON_Object
{
  size_t val_count;
  size_t child_count;
  JSON_KeyPair *val_arr;
  JSON_Object **child_arr;
  JSON_Object *parent;
};

typedef struct
{
  FILE *json_f;
  JSON_Object *main_object;
} JSON_File;


void __print_version();
JSON_File open_json_file(const char* file, const char *mode);
void parse_json_file(JSON_File *jf);


#endif //JSON_H_
