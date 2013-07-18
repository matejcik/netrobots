#ifndef NETROBOTS_H
#define NETROBOTS_H

void init(int argc, char *argv[]);
void init_custom(char *img_path, int argc, char *argv[]);

int scan(int degree, int resolution);
int cannon(int degree, int range);
void drive(int degree, int speed);
void cycle();

int damage();
int speed();
int loc_x();
int loc_y();

#endif
