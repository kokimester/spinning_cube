#include <curses.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /*  for sleep()  */

static int MAX_X = 0, MAX_Y = 0;
const char BACKGROUND_CHAR = '.';
const char LINE_CHAR = 'x';
const float MAX_DISTANCE_FROM_LINE = 0.5f;

void fill_background() {
  erase();
  for (int row = 0; row < MAX_Y; ++row) {
    for (int col = 0; col < MAX_X; ++col) {
      addch(BACKGROUND_CHAR);
    }
  }
}

bool is_point_part_of_line(int starty, int startx, int endy, int endx,
                           int pointy, int pointx) {
  // early return if point is outside the rectangle defined by stand and end
  if (!(((pointy <= endy) && (pointy >= starty)) ||
        ((pointy >= endy) && (pointy <= starty))))
    return false;
  if (!(((pointx <= endx) && (pointx >= startx)) ||
        ((pointx >= endx) && (pointx <= startx))))
    return false;

  float length = sqrt((startx - endx) * (startx - endx) +
                      (starty - endy) * (starty - endy));

  float twice_area =
      fabsf((float)((endy - starty) * pointx - (endx - startx) * pointy +
                    endx * starty - endy * startx));

  float distance = twice_area / length;

  return distance < MAX_DISTANCE_FROM_LINE ? true : false;
}

void draw_line(int starty, int startx, int endy, int endx) {
  for (int row = 0; row < MAX_Y; ++row) {
    for (int col = 0; col < MAX_X; ++col) {
      if (is_point_part_of_line(starty, startx, endy, endx, row, col)) {
        mvaddch(row, col, LINE_CHAR);
      }
    }
  }
}

int main(void) {

  WINDOW *mainwin;

  /*  Initialize ncurses  */

  if ((mainwin = initscr()) == NULL) {
    fprintf(stderr, "Error initialising ncurses.\n");
    exit(EXIT_FAILURE);
  }

  /*  Display "Hello, world!" in the centre of the
      screen, call refresh() to show our changes, and
      sleep() for a few seconds to get the full screen effect  */

  getmaxyx(mainwin, MAX_Y, MAX_X);
  const char *text = "Hello, world!";
  int counter = 0;
  while (1) {
    fill_background();
    counter = (counter + 1) % MAX_X;
    draw_line(0, counter, 10, MAX_X - counter);
    mvaddstr(MAX_Y / 2, MAX_X / 2 - strlen(text) / 2, text);
    refresh();
    usleep(1000 * 10);
  }

  /*  Clean up after ourselves  */

  delwin(mainwin);
  endwin();
  refresh();

  return EXIT_SUCCESS;
}
