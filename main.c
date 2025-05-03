#include <curses.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// to compile, use the following
/* gcc main.c -o main -lncurses -lm */

#define VERTICES_COUNT 8

static int MAX_X = 0, MAX_Y = 0;
const char BACKGROUND_CHAR = '.';
const char LINE_CHAR = 'x';
const float MAX_DISTANCE_FROM_LINE = 0.5f;
const float CUBE_DISTANCE = 10.f;
const float CUBE_SIDE = 5.f;

/*    .+------+     */
/* .'  |    .'|    */
/* +---+--+'  |   */
/* |   |  |   |   */
/* |  ,+--+---+   */
/* |.'    | .'    */
/* +------+'      */

void fill_background() {
  erase();
  for (int row = 0; row < MAX_Y; ++row) {
    for (int col = 0; col < MAX_X; ++col) {
      addch(BACKGROUND_CHAR);
    }
  }
}

typedef struct vec3 {
  float x;
  float y;
  float z;
} vec3;

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

void draw_line_by_vec3(vec3 start, vec3 end) {
  draw_line(start.y, start.x, end.y, end.x);
}

// Applies yaw (Z), pitch (Y), roll (X) rotation to a point
void rotate(vec3 *point, float yaw, float pitch, float roll) {
  // Convert degrees to radians
  float cy = cosf(yaw);
  float sy = sinf(yaw);
  float cp = cosf(pitch);
  float sp = sinf(pitch);
  float cr = cosf(roll);
  float sr = sinf(roll);

  // Rotation matrix (combined yaw-pitch-roll, ZYX order)
  float R[3][3] = {{cy * cp, cy * sp * sr - sy * cr, cy * sp * cr + sy * sr},
                   {sy * cp, sy * sp * sr + cy * cr, sy * sp * cr - cy * sr},
                   {-sp, cp * sr, cp * cr}};

  // Original point
  float x = point->x;
  float y = point->y;
  float z = point->z;

  // Apply rotation
  point->x = R[0][0] * x + R[0][1] * y + R[0][2] * z;
  point->y = R[1][0] * x + R[1][1] * y + R[1][2] * z;
  point->z = R[2][0] * x + R[2][1] * y + R[2][2] * z;
}

int main(void) {

  WINDOW *mainwin;
  if ((mainwin = initscr()) == NULL) {
    fprintf(stderr, "Error initialising ncurses.\n");
    exit(EXIT_FAILURE);
  }
  getmaxyx(mainwin, MAX_Y, MAX_X);
  // vertices of origo centered cube
  const vec3 cube_vertices[VERTICES_COUNT] = {
      {.x = CUBE_SIDE / 2, .y = CUBE_SIDE / 2, .z = -CUBE_SIDE / 2},
      {.x = CUBE_SIDE / 2, .y = -CUBE_SIDE / 2, .z = -CUBE_SIDE / 2},
      {.x = -CUBE_SIDE / 2, .y = CUBE_SIDE / 2, .z = -CUBE_SIDE / 2},
      {.x = -CUBE_SIDE / 2, .y = -CUBE_SIDE / 2, .z = -CUBE_SIDE / 2},
      {.x = CUBE_SIDE / 2, .y = CUBE_SIDE / 2, .z = CUBE_SIDE / 2},
      {.x = CUBE_SIDE / 2, .y = -CUBE_SIDE / 2, .z = CUBE_SIDE / 2},
      {.x = -CUBE_SIDE / 2, .y = CUBE_SIDE / 2, .z = CUBE_SIDE / 2},
      {.x = -CUBE_SIDE / 2, .y = -CUBE_SIDE / 2, .z = CUBE_SIDE / 2}};
  // vertices of modified cube
  vec3 vertices[VERTICES_COUNT] = {
      {.x = CUBE_SIDE / 2, .y = CUBE_SIDE / 2, .z = CUBE_DISTANCE},
      {.x = CUBE_SIDE / 2, .y = -CUBE_SIDE / 2, .z = CUBE_DISTANCE},
      {.x = -CUBE_SIDE / 2, .y = CUBE_SIDE / 2, .z = CUBE_DISTANCE},
      {.x = -CUBE_SIDE / 2, .y = -CUBE_SIDE / 2, .z = CUBE_DISTANCE},
      {.x = CUBE_SIDE / 2, .y = CUBE_SIDE / 2, .z = CUBE_DISTANCE + CUBE_SIDE},
      {.x = CUBE_SIDE / 2, .y = -CUBE_SIDE / 2, .z = CUBE_DISTANCE + CUBE_SIDE},
      {.x = -CUBE_SIDE / 2, .y = CUBE_SIDE / 2, .z = CUBE_DISTANCE + CUBE_SIDE},
      {.x = -CUBE_SIDE / 2,
       .y = -CUBE_SIDE / 2,
       .z = CUBE_DISTANCE + CUBE_SIDE}};

  vec3 projectedVert[VERTICES_COUNT];
  float angle = 0;

  while (1) {
    fill_background();
    for (uint32_t i = 0; i < VERTICES_COUNT; ++i) {
      /* https://computergraphics.stackexchange.com/questions/8255/finding-the-projection-matrix-for-one-point-perspective
       */
      projectedVert[i].x = vertices[i].x / vertices[i].z;
      projectedVert[i].y = vertices[i].y / vertices[i].z;
      // potential error handling
      if (projectedVert[i].x < -1 || projectedVert[i].x > 1 ||
          projectedVert[i].y < -1 || projectedVert[i].y > 1) {
        continue;
      }
      projectedVert[i].x = projectedVert[i].x * MAX_X + (float)MAX_X / 2;
      projectedVert[i].y = projectedVert[i].y * MAX_Y + (float)MAX_Y / 2;
    }

    // draw front and back face of cube and also interconnecting lines
    draw_line_by_vec3(projectedVert[0], projectedVert[1]);
    draw_line_by_vec3(projectedVert[1], projectedVert[3]);
    draw_line_by_vec3(projectedVert[3], projectedVert[2]);
    draw_line_by_vec3(projectedVert[2], projectedVert[0]);

    draw_line_by_vec3(projectedVert[0 + 4], projectedVert[1 + 4]);
    draw_line_by_vec3(projectedVert[1 + 4], projectedVert[3 + 4]);
    draw_line_by_vec3(projectedVert[3 + 4], projectedVert[2 + 4]);
    draw_line_by_vec3(projectedVert[2 + 4], projectedVert[0 + 4]);

    draw_line_by_vec3(projectedVert[0], projectedVert[0 + 4]);
    draw_line_by_vec3(projectedVert[1], projectedVert[1 + 4]);
    draw_line_by_vec3(projectedVert[2], projectedVert[2 + 4]);
    draw_line_by_vec3(projectedVert[3], projectedVert[3 + 4]);

    // perform rotation on cube located at origo and offset it by CUBE_DISTANCE
    for (int k = 0; k < VERTICES_COUNT; ++k) {
      vec3 current_cube_vertex = cube_vertices[k];
      rotate(&current_cube_vertex, angle / 5, angle, angle / 3);
      vertices[k] = current_cube_vertex;
      vertices[k].z += CUBE_DISTANCE;
    }
    angle += 0.1f;
    refresh();
    usleep(1000 * 50);
  }

  /*  Clean up after ourselves  */

  delwin(mainwin);
  endwin();
  refresh();

  return EXIT_SUCCESS;
}
