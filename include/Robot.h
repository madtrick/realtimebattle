/*
RealTimeBattle, a robot programming game for Unix
Copyright (C) 1998-2000  Erik Ouchterlony and Ragnar Ouchterlony

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef __ROBOT__
#define __ROBOT__

#ifndef NO_GRAPHICS
# include <gdk/gdk.h>
#endif

#include <fstream>
#include <sys/types.h>
#include <unistd.h>

using namespace std;


#include "Messagetypes.h"

#include "Structs.h"
#include "MovingObject.h"
#include "String.h"
#include "List.h"

#include "fdstream.h"

class Robot : public MovingObject
{
public:
  Robot(const String& filename);
  Robot(const int r_id, const long int col, const String& name);
  ~Robot();

  //arenaobject_t get_arenaobject_t() { return ROBOT; }
  
  void move(const double timestep);  
  void change_velocity(const double timestep);  
  void update_radar_and_cannon(const double timestep);  
  bool update_rotation(rotation_t& angle, const double timestep);
  void bounce_on_wall(const double bounce_c, const double hardness_c, const Vector2D& normal);
  friend void bounce_on_robot(Robot& robot1, Robot& robot2, const Vector2D& normal);
  void change_energy(const double energy_diff);
  void change_position( const double x, const double y, 
                        const double robot_a, const double cannon_a, 
                        const double radar_a, const double en );

  void check_name_uniqueness();
  void get_messages();
  void send_message(enum message_to_robot_type ...);
  void set_values_before_game(const Vector2D& pos, double angle);
  void set_values_at_process_start_up();
  void set_stats(const int robots_killed_same_time, const bool timeout=false);
  void set_stats(const double pnts, const int pos, const double time_survived,
                 const bool make_stats);
  bool is_dead_but_stats_not_set() { return dead_but_stats_not_set; }
  bool is_networked() { return network_robot; }

  void start_process();
  bool is_process_running();

  void check_process();
  void update_cpu_time_used();
  void reset_cpu_time();

  void send_signal();
  void end_process();
  void delete_pipes();
  void kill_process_forcefully();

  void live();
  void die();

  String get_robot_name() { return robot_name; }
  String get_robot_filename() { return robot_filename; }
  bool is_alive() { return alive; }
  double get_energy() { return energy; }
  pid_t get_pid() { return pid; }
  List<stat_t>* get_statistics() { return &statistics; }
  boost::fdostream* get_outstreamp() { return outstreamp; }  
  int get_position_this_game() { return position_this_game; }
  double get_total_points();
  //  void add_points(double pts) { points_this_game += pts; total_points += pts; }
  //  bool get_died_this_round() { return died_this_round; }
  int get_last_position();

  bool is_colour_given() { return colour_given; }
  void set_colour_given( const bool c ) { colour_given = c; }
  bool is_name_given() { return name_given; }
  

  ListIterator<stat_t> get_current_game_stats();
  

  rotation_t get_robot_angle() { return robot_angle; }

  bool set_and_get_has_competed() 
    { if( has_competed ) return true; else { has_competed = true; return false; } }

#ifndef NO_GRAPHICS

  int get_row_in_score_clist() { return row_in_score_clist; }
  void get_score_pixmap( GdkWindow* win, GdkPixmap*& pixm, GdkBitmap*& bitm );
  void get_stat_pixmap( GdkWindow* win, GdkPixmap*& pixm, GdkBitmap*& bitm );

  void set_row_in_score_clist( int row ) { row_in_score_clist = row; }
  void reset_last_displayed();
  void display_score();
  void draw_radar_and_cannon();

#endif

private:
  message_from_robot_type name2msg_from_robot_type(char*);
  void move(const double timestep, int iterstep, const double eps);
  bool check_state_for_message(const message_from_robot_type msg_t, const enum state_t state1,
                               const enum state_t state2 = NO_STATE);
  //  void save_data(const bool bin, const bool rewrite);
  //  void load_data(const bool bin);
  
  bool get_default_non_blocking_state();
  void set_non_blocking_state(const bool use_non_blocking);

  bool alive;
  bool process_running;

  bool send_usr_signal;
  int signal_to_send;
  int  send_rotation_reached;

  bool has_competed;

  double energy; 
  double extra_air_resistance;
  double brake_percent;

  rotation_t robot_angle;
  rotation_t cannon_angle;
  rotation_t radar_angle;

  double acceleration;
  double shot_energy;

  Vector2D start_pos;
  double start_angle;

  Vector2D last_drawn_robot_center;
  double last_drawn_radar_angle;
  double last_drawn_cannon_angle;

  List<stat_t> statistics;

  int robot_name_uniqueness_number;
  class String plain_robot_name;      // Name given by robot
  class String robot_name;            // plain_robot_name + uniqueness number

  class String robot_filename;        // Filename with path  
  class String robot_plain_filename;  // Filename without path

  bool colour_given;
  bool name_given;
  

  //class String robot_dir;
  //  double total_points;
  //  double points_this_game;
  int position_this_game;
  double time_survived_in_sequence;
  bool dead_but_stats_not_set;

  double cpu_used;
  double cpu_limit;
  double cpu_warning_limit;
  double cpu_timeout;
  double last_cpu;

  boost::fdistream* instreamp;
  boost::fdostream* outstreamp;

  int pipes[2];
  pid_t pid;    

  bool network_robot;
  List<String> message_list;  // For use by a network robot

  ListIterator<stat_t> current_game_stats;

  bool use_non_blocking;

  int row_in_score_clist;

  int last_displayed_energy;
  int last_displayed_place;
  int last_displayed_last_place;
  long last_displayed_score;

#ifndef NO_GRAPHICS
  pixmap_t score_pixmap;
  pixmap_t stat_pixmap;
#endif
};

#endif