#include "json.h"

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
  uint8_t inblock;
  uint8_t instring;
  uint8_t invalue;
  uint8_t inarray;
  int buf_sz;
  char *str_buf;
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
      if(!state.instring) state.instring = 1;
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
	state.inkey = 1;
	if(!state.current_obj){ // Should be first or 'main' object
 	  state.current_obj = calloc(1, sizeof(JSON_Object));
	}else{ // New Objects
	  JSON_Object *obj = calloc(1, sizeof(JSON_Object));
	  obj->parent = state.current_obj;
	  state.current_obj = obj;
	}
	break;
      case '}': // Close Block
	state.current_obj = state.current_obj->parent;
	break;
      case '[': // Open Array
	state.inarray = 1;
	break;
      case ']': // Close Array
	state.inarray = 0;
	break;
      case ',': // Next Key
	state.inkey = 1;
	state.invalue = 0;
	break;
      case ':': // Start Value Assignment
	state.invalue = 1;
	state.inkey = 0;
	break;
      default: // Numbers and bool [true, false, null, NaN(?)]
	break;
      }
    }else{
      int len = strlen(state.str_buf);
      if(len + 1 > state.buf_sz){
	state.buf_sz += 2048;
	state.str_buf = reallocarray(state.str_buf, state.buf_sz, sizeof(char));
      }
      state.str_buf[len] = c;
    }
  }
  
  
  close_json_file(jf);
}
