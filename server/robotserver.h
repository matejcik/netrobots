#ifndef ROBOTSERVER_H
#define ROBOTSERVER_H 1

#include <time.h>

#define RELOAD_RATIO 50
#define SPEED_RATIO 0.04
#define BREAK_DISTANCE (0.7 / SPEED_RATIO)

#define MAX_ROBOTS	21
#define MAX_NAME_LEN	14

struct cannon {
	int timeToReload;
	int x, y;
};

struct robot {
  // int fd; Should not be needed as it is synchronized with the array of fds
  char *name;
  double x, y;
  int damage;
  int speed;
  int break_distance;
  int target_speed;
  int degree;
  int cannon_degree;
  int radar_degree;
  int score;
  int waiting;
  time_t live_length;	/* valid only after death */
  struct cannon cannon[2];
  float color[3]; 	

  /* info on what was done... */
};

extern struct robot **all_robots;
extern int max_robots;

extern struct robot **ranking;
extern int dead_robots;
extern time_t game_start;

typedef enum game_type_t {
	GAME_SCORE,
	GAME_TIME,
} game_type_t;
extern game_type_t game_type;

/* Interface from networking code to game logic.  */

int scan (struct robot *r, int degree, int resolution);
int cannon (struct robot *r, int degree, int range);
void drive (struct robot *r, int degree, int speed);
void cycle (void);
int loc_x (struct robot *r);
int loc_y (struct robot *r);
int speed (struct robot *r);
int damage (struct robot *r);
void kill_robot (struct robot *r);
void complete_ranking(void);

#endif
