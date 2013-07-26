int scan(int degree, int resolution);
int cannon(int degree, int range);
void drive(int degree, int speed);
void cycle(void);

int damage(void);
int speed(void);
int loc_x(void);
int loc_y(void);
int elapsed(void);
void get_all(int *OUTPUT, int *OUTPUT, int *OUTPUT, int *OUTPUT, int *OUTPUT);

/* server config */
extern int game_type;
extern int shot_speed;
extern int max_cycles;

%include "typemaps.i"
%{
#include "../lib/netrobots.h"
%}

%inline %{
struct list {
  int count;
  char ** data;
};
%}

// This tells SWIG to treat char ** as a special case
%typemap(in) struct list * {
  AV *tempav;
  I32 len;
  int i;
  SV  **tv;
  if (!SvROK($input))
    croak("Argument $argnum is not a reference.");
  if (SvTYPE(SvRV($input)) != SVt_PVAV)
    croak("Argument $argnum is not an array.");
  tempav = (AV*)SvRV($input);
  len = av_len(tempav);
  struct list * lst = malloc(sizeof(struct list));
  lst->data = (char **) malloc((len+2)*sizeof(char *));
  for (i = 0; i <= len; i++) {
    tv = av_fetch(tempav, i, 0);
    lst->data[i] = (char *) SvPV(*tv,PL_na);
  }
  lst->data[i] = NULL;
  lst->count = len;
  $1 = lst;
}

// This cleans up the char ** array after the function call
%typemap(freearg) struct list * {
  free($1->data);
  free($1);
}

%ignore init;

%inline %{
void nr_init(char *img_path, struct list *args) {
  init_custom(img_path, args->count, args->data);
}
%}

%perlcode %{
sub init(;$) {
  my ($pic) = @_;
  Netrobotsc::nr_init($pic, [ $0, @ARGV ]);
}
%}

%typemap(argout) int *OUTPUT {
  $result = sv_newmortal();
  sv_setnv($result, *$input);
  argvi++;
}

%typemap(in,numinputs=0) int *OUTPUT(int junk) {
  $1 = &junk;
}

