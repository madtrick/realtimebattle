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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>
#include <limits.h>

#ifndef NO_GRAPHICS
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#endif 

#include <sys/stat.h>
#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif
#include <stdlib.h>
//#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
# include <dirent.h>

using namespace std;

#include "Various.h"
#include "IntlDefs.h"
#include "String.h"
#include "Options.h"
#include "Structs.h"

extern class Options the_opts;
#ifndef NO_GRAPHICS
# include "ControlWindow.h"
extern class ControlWindow* controlwindow_p;
#endif 
extern bool no_graphics;

void
Error(const bool fatal, const String& error_msg, const String& function_name)
{
  cerr << "RealTimeBattle: " << _("Error in") << " "
       << function_name << ": " << error_msg << endl;
  //  perror("RealTimeBattle: errno message");

  if( fatal == true )
    {
      Quit(false);
    }
}

void
Quit(const bool success)
{
#ifndef NO_GRAPHICS
  if( gtk_main_level() != 0 )
  {
    gtk_main_quit();

    if( !no_graphics )
      delete controlwindow_p;
  }
#endif 

  if( !success )
    exit(EXIT_FAILURE);  
      
  exit(EXIT_SUCCESS);
}


int
factorial(const int n)
{
  double fact = 1.0;
  for(int i=1; i<=n; i++)  
    fact *= i;

  if( fact < (double)INT_MAX )
    return (int)fact;
  else
    return INT_MAX;
}

int
binomial(const int n, const int k)
{
  int k2 = min(k, n-k);
  
  double bin = 1;
  for(int i=n; i>n-k2; i--)  
    bin *= i;

  for(int i=1; i<=k2; i++)  
    bin /= i;

  if( bin < (double)INT_MAX )
    return (int)bin;
  else
    return INT_MAX;
}

void 
reorder_pointer_array(void** array, int size)
{
  int n1, n2;
  void* temp_p;
  for(int i=0; i<size*5; i++)
    {
      n1 = (int)floor(size*((double)rand() / (double)RAND_MAX)) ;
      n2 = (int)floor(size*((double)rand() / (double)RAND_MAX)) ;
      if( n1 != n2 )
        {
          temp_p = array[n1];
          array[n1] = array[n2];
          array[n2] = temp_p;
        }
    }
}

#ifndef NO_GRAPHICS
GdkColor
make_gdk_colour(const long col)
{
  GdkColormap *cmap;
  GdkColor colour;

  cmap = gdk_colormap_get_system();
  colour.red =   ((col & 0xff0000) >> 16 ) * 0x101;
  colour.green = ((col & 0x00ff00) >> 8  ) * 0x101;
  colour.blue =  (col & 0x0000ff) * 0x101;
  if( !gdk_color_alloc (cmap, &colour) )
    Error(true, "Couldn't allocate colour", "Various:make_gdk_color");

  return colour;
}

long 
gdk2hex_colour(const GdkColor& col)
{
  return  ( (col.blue & 0xff) | 
            ((col.green & 0xff) << 8) |
            ((col.red & 0xff) << 16) );
}
#endif 

void
read_dirs_from_system(List<String>& robotdirs, List<String>& arenadirs)
{
  String dirs;

  robotdirs.delete_list();
  arenadirs.delete_list();

  dirs = the_opts.get_s(OPTION_ROBOT_SEARCH_PATH);
  split_colonseparated_dirs(dirs, robotdirs);

#ifdef ROBOTDIR
  String * str = new String(ROBOTDIR "/");
  robotdirs.insert_last( str );
#endif

  dirs = the_opts.get_s(OPTION_ARENA_SEARCH_PATH);
  split_colonseparated_dirs(dirs, arenadirs);

#ifdef ARENADIR
  str = new String(ARENADIR "/");
  arenadirs.insert_last( str );
#endif
}

// This function splits a string of colonseparated directories into a glist
void
split_colonseparated_dirs(String& dirs, List<String>& str_list)
{
  String current_dir = dirs;
  int pos, lastpos = 0;
  while( (pos = dirs.find(':', lastpos)) != -1 )
    {
      current_dir = get_segment(dirs, lastpos, pos-1);
      if(current_dir[current_dir.get_length() - 1] != '/')
        current_dir += '/';

      String* str = new String(current_dir);
      str_list.insert_last( str );

      lastpos = pos+1;
    }

  if(current_dir != "")
    {
      current_dir = get_segment(dirs, lastpos, -1); 
      if(current_dir[current_dir.get_length() - 1] != '/')
        current_dir += '/';

      String* str = new String(current_dir);
      str_list.insert_last( str );
    }
}

bool
check_if_filename_is_robot( String& fname )
{ 
  struct stat filestat;
  if( stat( fname.chars(), &filestat ) != 0 ) 
    return false;


  // Check if file is a regular file that can be executed and ends with .robot
  if( S_ISREG( filestat.st_mode) && 
      ( filestat.st_mode & ( S_IXOTH | S_IXGRP | S_IXUSR )) &&
      ( String(".robot") == get_segment(fname, -6, -1) ) )
    return true;


  return false;
}

bool
check_if_filename_is_arena( String& fname )
{
  struct stat filestat;
  if( 0 == stat( fname.chars(), &filestat ) && fname.get_length() > 6 )
    // Check if file is a regular file that is readable and ends with .arena
    if( S_ISREG( filestat.st_mode) &&
        ( filestat.st_mode & ( S_IROTH | S_IRGRP | S_IRUSR ) )  &&
        ( String(".arena") == get_segment(fname, -6, -1) ) )
      return true;

  return false;
}

void
check_for_robots_and_arenas( String& word, 
                             List<start_tournament_info_t>& tour_list,
                             List<String>& dir_list, 
                             const bool check_robots )
{
  bool found = false;
  String full_file_name = "";

  if( word.get_length() > 1 )
    if( get_segment( word, -2, -1 ) == "/*" )
      {
        search_directories( get_segment( word, 0, -2 ), tour_list, check_robots );
        return;
      }
  if( word.get_length() == 1 && word[0] == '*' )
    {
      
      ListIterator<String> li;
      for( dir_list.first(li); li.ok(); li++ )
        search_directories( *li(), tour_list, check_robots );
      return;
    }
  if( word.find('/') != -1 )
    {
      if((check_robots && check_if_filename_is_robot( word )) ||
         (!check_robots && check_if_filename_is_arena( word )))
        {
          full_file_name = word;
          found = true;
        }
    }
  if( !found )
    {
      ListIterator<String> li;
      for( dir_list.first(li); li.ok(); li++ )
        {
          String temp_name = *li() + word;

          if((check_robots && check_if_filename_is_robot( temp_name )) ||
             (!check_robots && check_if_filename_is_arena( temp_name )))
            {
              full_file_name= temp_name;
              found = true;
              break;
            }
        }
    }
  if( found )
    {
      start_tournament_info_t* info;
      info = new start_tournament_info_t(0, false, full_file_name, "");
      tour_list.insert_last( info );
    }
  else
    {
      if(check_robots)
        cerr << "Couldn't find an executable robot with filename " << word << endl;
      else
        cerr << "Couldn't find an arena with filename " << word << endl;
    }
}

void
search_directories( String directory, 
                    List<start_tournament_info_t>& tour_list,
                    const bool check_robots )
{
  DIR* dir;
  if( NULL != ( dir = opendir(directory.chars()) ) )
    {
      struct dirent* entry;
      while( NULL != ( entry = readdir( dir ) ) )
        {
          String full_file_name = directory + entry->d_name;
          bool res = false;
          if(check_robots)
            res = check_if_filename_is_robot(full_file_name);
          else
            res = check_if_filename_is_arena(full_file_name);
          if(res)
            {
              start_tournament_info_t* info;
              info = new start_tournament_info_t(0, false, full_file_name, "");
              tour_list.insert_last( info );
            }
        }
      closedir(dir);
    }
}

bool
parse_tournament_file( const String& fname, const StartTournamentFunction function,
                       void* data, bool fatal_error_on_file_failure )
{
  List<String> robotdirs;
  List<String> arenadirs;

  read_dirs_from_system(robotdirs, arenadirs);

  ifstream file(fname.chars());
  if( !file )
    {
      if( fatal_error_on_file_failure )
        Error( true, "Can't open specified tournament file.",
               "parse_tournament_file" );
      else
        return false;
    }

  int games_p_s = 1;
  int robots_p_s = 2;
  int n_o_sequences = 1;
  int looking_for = 0; // 0 = keyword, 1 = robot, 2 = arena

  List<start_tournament_info_t> robot_list;
  List<start_tournament_info_t> arena_list;

  for(;;)
    {
      char buffer[200];
      if ((file >> buffer).eof()){
          
          
          int robots_counted = robot_list.number_of_elements();
          int arenas_counted = arena_list.number_of_elements();
          
          if (games_p_s == -1)
            games_p_s = arenas_counted;

          if (robots_p_s == -1)
            robots_p_s = robots_counted;

          if (n_o_sequences == -1)
            n_o_sequences=binomial(robots_counted, games_p_s);

          robots_p_s = min(robots_counted,robots_p_s);
          
          if(robots_p_s < 2)
            {
              if( fatal_error_on_file_failure )
                Error(true, "Can't start tournament with only " + String(robots_p_s) + 
                      " robots per sequence", 
                      "parse_tournament_file");
              else
                return false;
            }

          if(games_p_s < 1)
            {
              if(fatal_error_on_file_failure)
                Error(true, "Must have at least one game per sequence. " 
                      "Current value is: " + String(games_p_s),
                      "parse_tournament_file");
              else
                return false;
            }

          if(n_o_sequences < 1)
            {
              if(fatal_error_on_file_failure)
                Error(true, "Must have at least one sequence. Current value is: " + 
                      String(n_o_sequences),
                      "parse_tournament_file");
              else
                return false;
            }

          // Startup the tournament

          (*function)( robot_list, arena_list, robots_p_s, 
                       games_p_s, n_o_sequences, data );

          return true;
        }

      String word(buffer);

      if((make_lower_case(word) == "games/sequence:") || 
         (make_lower_case(word) == "g/s:"))
        {
          looking_for = 0;
          file >> buffer;
          if( buffer[0] == '*' )
            games_p_s = -1;
          else
            games_p_s = str2int( buffer );
        }
      else if((make_lower_case(word) == "robots/sequence:") || 
              (make_lower_case(word) == "r/s:"))
        {
          looking_for = 0;
          file >> buffer;
          if( buffer[0] == '*' )
            robots_p_s = -1;
          else
            robots_p_s = str2int( buffer );
        }
      else if((make_lower_case(word) == "sequences:") || 
              (make_lower_case(word) == "seq:"))
        {
          looking_for = 0;
          file >> buffer;
          if( buffer[0] == '*' )
            n_o_sequences = -1;
          else
            n_o_sequences = str2int( buffer );
        }
      else if((make_lower_case(word) == "robots:") || (make_lower_case(word) == "r:"))
        looking_for = 1;
      else if((make_lower_case(word) == "arenas:") || (make_lower_case(word) == "a:"))
        looking_for = 2;
      else
        {
        
          switch(looking_for)
            {
            case 0:
              looking_for = 0;
              cerr << "Unrecognized keyword in tournament file: " << word << endl;
              break;
            case 1:
              check_for_robots_and_arenas( word, robot_list, robotdirs, true);
              break;
            case 2:
              check_for_robots_and_arenas( word, arena_list, arenadirs, false);
              break;
            }
        }
    }
  return false;
}

void
create_tmp_rtb_dir()
{
  String dirname = the_opts.get_s( OPTION_TMP_RTB_DIR );
  struct stat filestat;
  if( 0 != stat( dirname.chars(), &filestat ) ) 
    mkdir( dirname.chars(), S_IRWXU | S_IRWXG | S_IRWXO );
}

#ifndef NO_GRAPHICS

void
entry_handler( GtkWidget * entry, entry_t * entry_info )
{
  String entry_text = gtk_entry_get_text(GTK_ENTRY(entry));
  String old_entry_text = entry_text;
  bool point_found = false;

  for(int i=0;i<entry_text.get_length();i++)
    {
      switch( entry_info->datatype )
        {
        case ENTRY_INT:
          if( !((entry_text[i] >= '0' && entry_text[i] <= '9') ||
                (entry_text[i] == '-' && i == 0 && entry_info->allow_sign ))  )
            entry_text.erase(i);
          break;
        case ENTRY_DOUBLE:
        /* if( !((entry_text[i] >= '0' && entry_text[i] <= '9') ||
                (entry_text[i] == '.') ||
                (entry_text[i] == '-' && i == 0 && entry_info->allow_sign ))  )
            entry_text.erase(i);
          if( entry_text[i] == '.' && !point_found )
            point_found = true;
          else if( entry_text[i] == '.' && point_found )
            entry_text.erase(i);*/
          break;
        case ENTRY_HEX:
          if( !(((entry_text[i] >= '0' && entry_text[i] <= '9') || 
                 (entry_text[i] >= 'a' && entry_text[i] <= 'f') ||
                 (entry_text[i] >= 'A' && entry_text[i] <= 'F')) ||
                (entry_text[i] == '-' && i == 0 ) && entry_info->allow_sign ) )
            entry_text.erase(i);
          break;
        case ENTRY_CHAR:
          break;
        case ENTRY_BOOL:
          break;
        }
    }

  if(old_entry_text != entry_text)
    gtk_entry_set_text(GTK_ENTRY(entry),entry_text.chars());
}

gint
int_compare(GtkCList* clist, gconstpointer ptr1, gconstpointer ptr2)
{
  char* text1 = NULL;
  char* text2 = NULL;

  GtkCListRow* row1 = (GtkCListRow*) ptr1;
  GtkCListRow* row2 = (GtkCListRow*) ptr2;

  switch (row1->cell[clist->sort_column].type)
    {
    case GTK_CELL_TEXT:
      text1 = GTK_CELL_TEXT (row1->cell[clist->sort_column])->text;
      break;
    case GTK_CELL_PIXTEXT:
      text1 = GTK_CELL_PIXTEXT (row1->cell[clist->sort_column])->text;
      break;
    default:
      break;
    }
 
  switch (row2->cell[clist->sort_column].type)
    {
    case GTK_CELL_TEXT:
      text2 = GTK_CELL_TEXT (row2->cell[clist->sort_column])->text;
      break;
    case GTK_CELL_PIXTEXT:
      text2 = GTK_CELL_PIXTEXT (row2->cell[clist->sort_column])->text;
      break;
    default:
      break;
    }

  if (!text2)
    return (text1 != NULL);

  if (!text1)
    return -1;

  return (str2int(text1) - str2int(text2));
}

gint
float_compare(GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2)
{
  char* text1 = NULL;
  char* text2 = NULL;

  GtkCListRow* row1 = (GtkCListRow*) ptr1;
  GtkCListRow* row2 = (GtkCListRow*) ptr2;

  switch (row1->cell[clist->sort_column].type)
    {
    case GTK_CELL_TEXT:
      text1 = GTK_CELL_TEXT (row1->cell[clist->sort_column])->text;
      break;
    case GTK_CELL_PIXTEXT:
      text1 = GTK_CELL_PIXTEXT (row1->cell[clist->sort_column])->text;
      break;
    default:
      break;
    }
 
  switch (row2->cell[clist->sort_column].type)
    {
    case GTK_CELL_TEXT:
      text2 = GTK_CELL_TEXT (row2->cell[clist->sort_column])->text;
      break;
    case GTK_CELL_PIXTEXT:
      text2 = GTK_CELL_PIXTEXT (row2->cell[clist->sort_column])->text;
      break;
    default:
      break;
    }

  if (!text2)
    return (text1 != NULL);

  if (!text1)
    return -1;

  double n1 = str2dbl(text1);
  double n2 = str2dbl(text2);

  if(n1 > n2)
    return 1;
  else if(n2 > n1)
    return -1;

  return 0;
}

gint
string_case_sensitive_compare(GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2)
{
  char *text1 = NULL;
  char *text2 = NULL;

  GtkCListRow *row1 = (GtkCListRow *) ptr1;
  GtkCListRow *row2 = (GtkCListRow *) ptr2;

  switch (row1->cell[clist->sort_column].type)
    {
    case GTK_CELL_TEXT:
      text1 = GTK_CELL_TEXT (row1->cell[clist->sort_column])->text;
      break;
    case GTK_CELL_PIXTEXT:
      text1 = GTK_CELL_PIXTEXT (row1->cell[clist->sort_column])->text;
      break;
    default:
      break;
    }
 
  switch (row2->cell[clist->sort_column].type)
    {
    case GTK_CELL_TEXT:
      text2 = GTK_CELL_TEXT (row2->cell[clist->sort_column])->text;
      break;
    case GTK_CELL_PIXTEXT:
      text2 = GTK_CELL_PIXTEXT (row2->cell[clist->sort_column])->text;
      break;
    default:
      break;
    }

  if (!text2)
    return (text1 != NULL);

  if (!text1)
    return -1;

  return strcmp (text1, text2);
}

gint
string_case_insensitive_compare(GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2)
{
  char *text1 = NULL;
  char *text2 = NULL;

  GtkCListRow *row1 = (GtkCListRow *) ptr1;
  GtkCListRow *row2 = (GtkCListRow *) ptr2;

  switch (row1->cell[clist->sort_column].type)
    {
    case GTK_CELL_TEXT:
      text1 = GTK_CELL_TEXT (row1->cell[clist->sort_column])->text;
      break;
    case GTK_CELL_PIXTEXT:
      text1 = GTK_CELL_PIXTEXT (row1->cell[clist->sort_column])->text;
      break;
    default:
      break;
    }
 
  switch (row2->cell[clist->sort_column].type)
    {
    case GTK_CELL_TEXT:
      text2 = GTK_CELL_TEXT (row2->cell[clist->sort_column])->text;
      break;
    case GTK_CELL_PIXTEXT:
      text2 = GTK_CELL_PIXTEXT (row2->cell[clist->sort_column])->text;
      break;
    default:
      break;
    }

  if (!text2)
    return (text1 != NULL);

  if (!text1)
    return -1;

  return strcasecmp (text1, text2);
}

#endif