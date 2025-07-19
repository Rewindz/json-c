#include <stdio.h>

#include "../src/json.h"


const char* types_str[] = {"Number", "String", "Boolean", "Array", "Object", "Null"};

const char* json_file = "./json/example-large.json";

void printkey(JSON_KeyPair *kp, int tabs)
{
  if(tabs > 0){
    for(int i = 0; i < tabs; i++)printf("\t");
  }
  printf("Reading key %s from kvp at %p\n", kp->key, (void*)kp);
  if(tabs > 0){
    for(int i = 0; i < tabs; i++)printf("\t");
  }
  if(!kp->data){
    printf("Key %s:%s is empty!\n", types_str[kp->type], kp->key);
    return;
  }
  switch(kp->type){
  case Number:
    printf("Key %s:%s has value: %zd with length: %zd\n",
	   types_str[kp->type], kp->key, (size_t)*kp->data, kp->data_len);
    break;
  case String:
    printf("Key %s:%s has value: %s with length: %zd\n",
	   types_str[kp->type], kp->key, kp->data, kp->data_len);
    break;
  case Boolean:
    printf("Key %s:%s has value: %s with length: %zd\n",
	   types_str[kp->type], kp->key, (*kp->data ? "true" : "false"), kp->data_len);
    break;
  case Null:
    printf("Key %s:%s has value: null\n",
	   types_str[kp->type], kp->key);
    break;
  case Array:
    JSON_Array *array = (JSON_Array*)kp->data;
    printf("Key %s:%s is an array which contains:\n", types_str[kp->type], kp->key);
    for(size_t z = 0; z < array->arr_len; z++){
      printkey(&array->kvp_arr[z], tabs+1);
    }
    break;
  case Object:
    JSON_Object *child = (JSON_Object*)kp->data;
    printf("Key %s:%s is an object which has keys:\n", types_str[kp->type], kp->key);
    for(size_t z = 0; z < child->val_count; z++){
      printkey(&child->val_arr[z], tabs + 1);
    }
    break;
  default:
    break;
  }
}

int main(int argc, char **argv)
{

  printf("Hello from %s!\n", argv[0]);
  __print_version();
  (void)argc;

  JSON_File jf = open_json_file(json_file, "r");
  printf("\n\nReading file %s\n", json_file);
  parse_json_file(&jf);

  JSON_Object *parent = jf.main_object;
  printf("Parent object has child count: %zd\nValue count: %zd\n", parent->child_count, parent->val_count);

  for(size_t i = 0; i < parent->val_count; i++){
    JSON_KeyPair *kp = &parent->val_arr[i];
    printkey(kp, 0);
  }

  free_json_file(&jf);
  
  return 0;
}
