#include "obj_parser.h"
#include <curses.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// to compile, use the following
/* gcc main.c -o main -lncurses -lm */

static int MAX_X = 0, MAX_Y = 0;
const char BACKGROUND_CHAR = '.';
const char LINE_CHAR = 'x';
const float MAX_DISTANCE_FROM_LINE = 0.5f;
const float MODEL_DISTANCE = 1.5f;

/*    .+------+     */
/* .'  |    .'|    */
/* +---+--+'  |   */
/* |   |  |   |   */
/* |  ,+--+---+   */
/* |.'    | .'    */
/* +------+'      */

void fill_background(void) {
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

typedef struct line {
  vec3 start;
  vec3 end;
} line;

typedef struct triangle {
  vec3 points[3];
} triangle;

typedef struct square {
  vec3 points[4];
} square;

bool is_point_part_of_line(int starty, int startx, int endy, int endx,
                           int pointy, int pointx) {
  // early return if point is outside the rectangle defined by start and end
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

int clamp_to_screen(const int coord, const int min, const int max) {
  int clamped_coord = coord;
  if (coord < min)
    clamped_coord = 0;
  if (coord > max)
    clamped_coord = max;
  return clamped_coord;
}

void draw_line(int starty, int startx, int endy, int endx) {
  starty = clamp_to_screen(starty, 0, MAX_Y);
  startx = clamp_to_screen(startx, 0, MAX_X);
  endy = clamp_to_screen(endy, 0, MAX_Y);
  endx = clamp_to_screen(endx, 0, MAX_X);
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

void draw_line_by_obj_vector(struct obj_vector start, struct obj_vector end) {
  draw_line(start.e[1], start.e[0], end.e[1], end.e[0]);
}

void draw_faces(const struct obj_scene_data *model,
                struct obj_vector *projected_vertices) {
  for (int32_t i = 0; i < model->face_count; ++i) {
    struct obj_face current_face = *model->face_list[i];
    for (int32_t j = 0; j < current_face.vertex_count; ++j) {
      struct obj_vector start =
          projected_vertices[current_face.vertex_index[j]];
      struct obj_vector end = projected_vertices
          [current_face.vertex_index[(j + 1) % current_face.vertex_count]];
      draw_line_by_obj_vector(start, end);
    }
  }
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

void center_and_scale_model(struct obj_scene_data *model, float scale) {
  printf("scale: %f\n", scale);
  float cx = 0.f, cy = 0.f, cz = 0.f;
  float maxx = 0.f, maxy = 0.f, maxz = 0.f;
  for (int32_t i = 0; i < model->vertex_count; ++i) {
    cx += model->vertex_list[i]->e[0];
    cy += model->vertex_list[i]->e[1];
    cz += model->vertex_list[i]->e[2];
    if (fabs(model->vertex_list[i]->e[0]) > fabs(maxx))
      maxx = model->vertex_list[i]->e[0];
    if (fabs(model->vertex_list[i]->e[1]) > fabs(maxy))
      maxy = model->vertex_list[i]->e[1];
    if (fabs(model->vertex_list[i]->e[2]) > fabs(maxz))
      maxz = model->vertex_list[i]->e[2];
  }
  printf("Max coordinates: %f %f %f\n", maxx, maxy, maxz);
  cx /= model->vertex_count;
  cy /= model->vertex_count;
  cz /= model->vertex_count;
  printf("Middle coordinates: %f %f %f\n", cx, cy, cz);
  for (int32_t i = 0; i < model->vertex_count; ++i) {
    model->vertex_list[i]->e[0] -= cx;
    model->vertex_list[i]->e[1] -= cy;
    model->vertex_list[i]->e[2] -= cz;
    model->vertex_list[i]->e[0] *= scale;
    model->vertex_list[i]->e[1] *= scale;
    model->vertex_list[i]->e[2] *= scale;
  }
  printf("Scaled down max coordinates: %f %f %f\n", maxx * scale, maxy * scale,
         maxz * scale);
}

int main(int argc, char **argv) {

  WINDOW *mainwin;
  if ((mainwin = initscr()) == NULL) {
    fprintf(stderr, "Error initialising ncurses.\n");
    exit(EXIT_FAILURE);
  }
  if (argc < 2) {
    fprintf(stderr, "Missing obj file.\nUsage: %s <model.obj>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  getmaxyx(mainwin, MAX_Y, MAX_X);
  // load obj file
  struct obj_scene_data model;
  int ok_code = parse_obj_scene(&model, argv[1]);
  if (!ok_code) {
    fprintf(stderr, "Error! Could not parse provided obj file %s\n", argv[1]);
    exit(EXIT_FAILURE);
  }
  struct obj_scene_data const_model;
  ok_code = parse_obj_scene(&const_model, argv[1]);
  if (!ok_code) {
    fprintf(stderr, "Error! Could not parse provided obj file %s\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  center_and_scale_model(&model, 1.f / 137.f);
  center_and_scale_model(&const_model, 1.f / 137.f);

  int vertex_count = model.vertex_count;

  struct obj_vector *projectedVert =
      malloc(sizeof(struct obj_vector) * vertex_count);
  float angle = 0;

  for (int32_t k = 0; k < vertex_count; ++k) {
    vec3 current_cube_vertex;
    current_cube_vertex.x = const_model.vertex_list[k]->e[0];
    current_cube_vertex.y = const_model.vertex_list[k]->e[1];
    current_cube_vertex.z = const_model.vertex_list[k]->e[2];
    rotate(&current_cube_vertex, 0, 0, 3.14f / 2.f);
    const_model.vertex_list[k]->e[0] = current_cube_vertex.x;
    const_model.vertex_list[k]->e[1] = current_cube_vertex.y;
    const_model.vertex_list[k]->e[2] = current_cube_vertex.z;
  }

  while (1) {
    fill_background();
    for (int32_t i = 0; i < vertex_count; ++i) {
      /* https://computergraphics.stackexchange.com/questions/8255/finding-the-projection-matrix-for-one-point-perspective
       */
      projectedVert[i].e[0] =
          model.vertex_list[i]->e[0] / model.vertex_list[i]->e[2];
      projectedVert[i].e[1] =
          model.vertex_list[i]->e[1] / model.vertex_list[i]->e[2];
      // potential error handling
      if (projectedVert[i].e[0] < -1 || projectedVert[i].e[0] > 1 ||
          projectedVert[i].e[1] < -1 || projectedVert[i].e[1] > 1) {
        continue;
      }
      projectedVert[i].e[0] = projectedVert[i].e[0] * MAX_X + (float)MAX_X / 2;
      projectedVert[i].e[1] = projectedVert[i].e[1] * MAX_Y + (float)MAX_Y / 2;
    }
    // draw faces

    draw_faces(&model, projectedVert);

    // perform rotation on cube located at origo and offset it by MODEL_DISTANCE
    for (int32_t k = 0; k < vertex_count; ++k) {
      vec3 current_cube_vertex;
      current_cube_vertex.x = const_model.vertex_list[k]->e[0];
      current_cube_vertex.y = const_model.vertex_list[k]->e[1];
      current_cube_vertex.z = const_model.vertex_list[k]->e[2];
      /* rotate(&current_cube_vertex, angle / 5, angle, angle / 3); */
      rotate(&current_cube_vertex, 0, angle, 0);
      model.vertex_list[k]->e[0] = current_cube_vertex.x;
      model.vertex_list[k]->e[1] = current_cube_vertex.y;
      model.vertex_list[k]->e[2] = current_cube_vertex.z + MODEL_DISTANCE;
    }
    angle += 0.1f;
    refresh();
    usleep(1000 * 50);
  }

  /*  Clean up after ourselves  */
  free(projectedVert);
  delete_obj_data(&model);

  delwin(mainwin);
  endwin();
  refresh();

  return EXIT_SUCCESS;
}
