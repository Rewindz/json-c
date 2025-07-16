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
    Object
  } JSON_Types;

struct JSON_KeyPair
{
  JSON_Types type;
  char *key;
  int data_len;
  unsigned char *data;
};

struct JSON_Object
{
  int val_count;
  JSON_KeyPair *val_arr;
  JSON_Object *parent;
};

typedef struct
{
  FILE *json_f;
  JSON_Object *main_object;
} JSON_File;


void __print_version();



#endif //JSON_H_
