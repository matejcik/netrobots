%include "typemaps.i"
%{
#include "../lib/netrobots.h"
%}

%typemap(in) char **argv {
  if (Z_TYPE_PP($input) == IS_ARRAY) {
    HashTable *arr = Z_ARRVAL_PP($input);
    int len = zend_hash_num_elements(arr);
    char ** out = (char **) malloc((len+2)*sizeof(char *));
    
    HashPosition ptr;
    zval **data;
    int i = 0;
    for (zend_hash_internal_pointer_reset_ex(arr, &ptr);
         zend_hash_get_current_data_ex(arr, (void**) &data, &ptr) == SUCCESS;
         zend_hash_move_forward_ex(arr, &ptr)) {
      zval tmp = **data;
      zval_copy_ctor(&tmp);
      convert_to_string(&tmp);
      if (Z_STRVAL(tmp)) {
        out[i] = (char*)malloc(sizeof(char)*(Z_STRLEN(tmp)+1));
        strncpy(out[i], Z_STRVAL(tmp), Z_STRLEN(tmp)+1);
        i++;
      }
      zval_dtor(&tmp);
    }
    out[i] = NULL;
    $1 = out;
  }
}

%typemap(freearg) char **argv {
  free($1);
}

//%pragma(php) code="function netrobots_init($img) { netrobots::nr_init($img, $argc, $argv); }"

%typemap(argout) (int *loc_x, int *loc_y, int *damage, int *speed, int *elapsed) {
  array_init($result);

  add_next_index_long($result, *$1);
  add_next_index_long($result, *$2);
  add_next_index_long($result, *$3);
  add_next_index_long($result, *$4);
  add_next_index_long($result, *$5);
}

%typemap(in,numinputs=0) (int *loc_x, int *loc_y, int *damage, int *speed, int *elapsed) (int t[5]) {
  $1 = &t[0];
  $2 = &t[1];
  $3 = &t[2];
  $4 = &t[3];
  $5 = &t[4];
}

%inline %{
int print_args(char **argv) {
    int i = 0;
    while (argv[i]) {
         printf("argv[%d] = %s\n", i,argv[i]);
         i++;
    }
    return i;
}
%}

void init(int argc, char **argv);
void init_custom(char *img_path, int argc, char **argv);

int scan(int degree, int resolution);
int cannon(int degree, int range);
void drive(int degree, int speed);
void cycle(void);

int damage(void);
int speed(void);
int loc_x(void);
int loc_y(void);
int elapsed(void);
void get_all(int *loc_x, int *loc_y, int *damage, int *speed, int *elapsed);

/* server config */
extern int game_type;
extern int shot_speed;
extern int max_cycles;

