#include "json.h"

#include <ctype.h>

#define JSON_C_VERSION "0.01a"

void __print_version()
{
  printf("JSON-C Version %s\n", JSON_C_VERSION);
}

JSON_File open_json_file(const char* file, const char *mode)
{
  JSON_File jf = {0}; 
  if( (jf.json_f = fopen(file, mode)) == NULL){
    fprintf(stderr, "Could not open file %s with mode %s\nError: %s",
	    file, mode, strerror(errno));
    return jf;
  }

  return jf;
}

void close_json_file(JSON_File *jf)
{
  if(jf->json_f) fclose(jf->json_f);
  jf->json_f = NULL;
}

struct ParseState{
  uint8_t inkey;
  int inblock;
  uint8_t instring;
  uint8_t invalue;
  int inarray;
  int buf_sz;
  char *str_buf;
  JSON_Array *current_arr;
  JSON_Object *current_obj;
  JSON_KeyPair *current_kvp;
};

static void parsing_error(JSON_File *jf, char *buf){
  fprintf(stderr,
	  "Could not parse JSON file!\nBuffer status: %s\n",
	  buf);
  free(buf);
  close_json_file(jf);
}

//TODO: Create free/cleanup functions

void parse_json_file(JSON_File *jf)
{
  if(!jf->json_f){
    fprintf(stderr, "No JSON File open!\n");
    return;
  }

  int c_i = EOF;

  struct ParseState state = {0};
  state.str_buf = calloc(2048, sizeof(char));
  state.buf_sz = 2048;
  
  while( (c_i = fgetc(jf->json_f)) != EOF ){
    unsigned char c = (unsigned char) c_i;
    if( c == '"' && state.current_obj){
      if(!state.instring){
	state.instring = 1;
	continue;
      }
      else{
	state.instring = 0;
	if(state.inkey){
	  state.current_obj->val_arr = reallocarray(state.current_obj->val_arr,
						    ++state.current_obj->val_count, sizeof(JSON_KeyPair));
	  state.current_kvp = &state.current_obj->val_arr[state.current_obj->val_count - 1];
	  state.current_kvp->key = strdup(state.str_buf);
	}else if(state.invalue){
	  state.current_kvp->data = (unsigned char*)strdup(state.str_buf);
	  state.current_kvp->data_len = strlen(state.str_buf) + 1;
	  state.current_kvp->type = String;
	}else{
	  parsing_error(jf, strdup(state.str_buf));
	  fprintf(stderr, "Trying to eval a string, but not in either a key nor a value!\n");
	  free(state.str_buf);
	  return;
	}
	memset(state.str_buf, 0, state.buf_sz);
      }
    }else if(c == '"' && !state.current_obj){
      fprintf(stderr, "Got \" when not in an object. Did you forget to add a main object?\n");
      parsing_error(jf, strdup(state.str_buf));
      free(state.str_buf);
      return;
    }
    if(!state.instring){
      switch(c){
      case '{': // Open Block
	if(!state.current_obj){ // Should be first or 'main' object
 	  state.current_obj = calloc(1, sizeof(JSON_Object));
	  jf->main_object = state.current_obj;
	}else{ // New Objects
	  if(state.invalue){
	    state.current_kvp->type = Object;
	    state.current_kvp->data = calloc(1, sizeof(JSON_Object));
	    state.current_kvp->data_len = sizeof(JSON_Object);
	    ((JSON_Object*)state.current_kvp->data)->parent = state.current_obj;
	    state.current_obj = (JSON_Object*)state.current_kvp->data;
	  }else{
	    JSON_Object *obj = calloc(1, sizeof(JSON_Object));
	    obj->parent = state.current_obj;
	    state.current_obj->child_arr =
	      reallocarray(state.current_obj->child_arr,
			   ++state.current_obj->child_count, sizeof(JSON_Object*));
	    state.current_obj->child_arr[state.current_obj->child_count-1] = obj;
	    state.current_obj = obj;
	  }
	}
	if(state.inarray)state.inblock++;
	state.invalue = 0;
	state.inkey = 1;
	break;
      case '}': // Close Block
	state.current_obj = (state.current_obj->parent) ? state.current_obj->parent : state.current_obj;
	if(!state.inarray){
	  state.inkey = 1;
	  state.invalue = 0;
	}else{
	  state.inkey = 0;
	  state.invalue = 1;
	  state.inblock--;
	}
	break;
      case '[': // Open Array
	if(!state.invalue) break;
	state.inarray++;
	state.current_kvp->type = Array;
	state.current_kvp->data = calloc(1, sizeof(JSON_Array));
	((JSON_Array*)state.current_kvp->data)->parent_arr = state.current_arr;
	state.current_arr = ((JSON_Array*)state.current_kvp->data);
	state.current_arr->kvp_arr = calloc(1, sizeof(JSON_KeyPair));
	state.current_arr->kvp_arr[0].key = strdup("array");
	state.current_arr->arr_len = 1;
	state.current_kvp = state.current_arr->kvp_arr;
	break;
      case ']': // Close Array
	if(!state.invalue) break;
	state.inarray--;
	state.current_arr = (state.current_arr->parent_arr ? state.current_arr->parent_arr : NULL);
	break;
      case ',': // Next Key
	if(state.inarray){
	  if(!state.inblock){
	    state.current_arr->kvp_arr =
	      reallocarray(state.current_arr->kvp_arr, ++state.current_arr->arr_len, sizeof(JSON_KeyPair));
	    state.current_kvp = &state.current_arr->kvp_arr[state.current_arr->arr_len - 1];
	    state.current_kvp->key = strdup("array");
	    state.inkey = 0;
	    state.invalue = 1;
	  }else{
	    state.inkey = 1;
	    state.invalue = 0;
	  }
	  
	  break;
	}
	state.inkey = 1;
	state.invalue = 0;
	break;
      case ':': // Start Value Assignment
	state.invalue = 1;
	state.inkey = 0;
	break;
      default: // Numbers and bool [true, false, null, NaN(?)]
	if(!state.invalue) break;
	if(isspace(c) || c == '\n')break;
	if(isalpha(c)){
	  state.current_kvp->data = calloc(1, sizeof(unsigned char));
	  state.current_kvp->type = Boolean;
	  state.current_kvp->data_len = sizeof(unsigned char);
	  *state.current_kvp->data = ( (c == 't') ? 1 : 0);
	  if(c == 'n') state.current_kvp->type = Null;
	  do{
	    c_i = fgetc(jf->json_f);
	  }while(c_i != EOF && isalpha(c_i));
	  c_i = ungetc(c_i, jf->json_f);
	  break;
	}
	if (isdigit(c) || c == '-') {
	  char numbuf[32] = {0};
	  size_t i = 0;

	  numbuf[i++] = c;  // save the first char (digit or '-')

	  // Read until non-digit (or dot, if you want floats in future)
	  while ((c_i = fgetc(jf->json_f)) != EOF && i < sizeof(numbuf) - 1) {
	    c = (unsigned char)c_i;
	    if (isdigit(c)) {
	      numbuf[i++] = c;
	    } else {
	      ungetc(c_i, jf->json_f);  // push back the non-digit
	      break;
	    }
	  }

	  numbuf[i] = '\0';

	  errno = 0;
	  long val = strtol(numbuf, NULL, 10);
	  if (errno == ERANGE) {
	    fprintf(stderr, "Warning: number out of range: %s\n", numbuf);
	    // Optionally handle this better
	  }

	  state.current_kvp->data = calloc(1, sizeof(val));
	  memcpy(state.current_kvp->data, &val, sizeof(val));
	  state.current_kvp->data_len = sizeof(val);
	  state.current_kvp->type = Number;
	  printf("Saved number %ld to key %s at %p\n", val, state.current_kvp->key, (void*)state.current_kvp);

	  state.invalue = 0;
	  state.inkey = 1;
	}
	break;
      }
    }else{
      int len = strlen(state.str_buf);
      //Needs a fix can still overflow here if += 2048 still isnt big enough
      if(len + 2 > state.buf_sz){
	state.buf_sz += 2048;
	state.str_buf = reallocarray(state.str_buf, state.buf_sz, sizeof(char));
      }
      state.str_buf[len] = c;
    }
  }
  free(state.str_buf);
  
  close_json_file(jf);
}

void free_json_array(JSON_Array *arr)
{
  if(!arr)return;
  for(size_t i = 0; i < arr->arr_len; i++){
    free_json_keypair(arr->kvp_arr + i);
  }
}

void free_json_keypair(JSON_KeyPair *kvp)
{
  if(!kvp)return;
  switch(kvp->type){
  case String:
  case Number:
  case Boolean:
  case Null:
    free(kvp->data);
    break;
  case Object:
    free_json_object((JSON_Object*)kvp->data);
    free(kvp->data);
    break;
  case Array:
    free_json_array((JSON_Array*)kvp->data);
    free(kvp->data);
    break;
  default:
    fprintf(stderr, "Hit default case when freeing key pair! Aborting!\n");
    abort();
  }
  if(kvp->key)free(kvp->key);
}

void free_json_object(JSON_Object *obj)
{
  if(obj->val_arr){
    for(size_t i = 0; i < obj->val_count; i++){
      free_json_keypair(obj->val_arr + i);
    }
    free(obj->val_arr);
  }
  if(obj->child_arr){
    for(size_t i = 0; i < obj->child_count; i++){
      free_json_object(obj->child_arr[i]);
    }
    free(obj->child_arr);
  }
}


void free_json_file(JSON_File *file)
{
  free_json_object(file->main_object);
}
