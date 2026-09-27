#include <stdio.h>
#include <string.h>
#include <math.h>
#include "driver.h"
#include "core.h"
#include "retriever.h"

// every lane runs every instruction for every face, so frame time scales
// with WINDOW_WIDTH * WINDOW_HEIGHT * number of faces
#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 300
#define MAX_INPUT 80

#define HALF_FOV 0.6f // horizontal, radians
#define NEAR 0.01f
#define ROTATE_STEP 0.2f
#define PI 3.14159265f

struct model {
  int numVerts;
  int numFaces;
  float *verts;   // 3 per vertex
  uint32_t *faces; // v0, v1, v2, color per face
};

struct camera {
  float center[3]; // point the camera orbits around
  float distance;
  float yaw;
  float pitch;
};

// parses lines of the form 'v x y z' and 'f v0 v1 v2 r g b'. vertices are
// numbered from 1 in the order they appear. returns 0 on success
int parse_model(const char *path, struct model *model) {
  FILE *file;
  char line[MAX_INPUT];
  int lineNum = 0;
  int capVerts = 64, capFaces = 64;
  float x, y, z;
  int v[3], rgb[3];

  file = fopen(path, "r");
  if (!file) {
    fprintf(stderr, "Could not open %s\n", path);
    return 1;
  }

  model->numVerts = 0;
  model->numFaces = 0;
  model->verts = malloc(sizeof(float) * 3 * capVerts);
  model->faces = malloc(sizeof(uint32_t) * 4 * capFaces);

  while (fgets(line, MAX_INPUT, file)) {
    lineNum++;
    if (line[0] == 'v') {
      if (sscanf(line, "v %f %f %f", &x, &y, &z) != 3) {
        fprintf(stderr, "%s:%d: expected 'v x y z'\n", path, lineNum);
        fclose(file);
        return 1;
      }
      if (model->numVerts == capVerts) {
        capVerts *= 2;
        model->verts = realloc(model->verts, sizeof(float) * 3 * capVerts);
      }
      model->verts[3 * model->numVerts] = x;
      model->verts[3 * model->numVerts + 1] = y;
      model->verts[3 * model->numVerts + 2] = z;
      model->numVerts++;
    } else if (line[0] == 'f') {
      if (sscanf(line, "f %d %d %d %d %d %d",
            &v[0], &v[1], &v[2], &rgb[0], &rgb[1], &rgb[2]) != 6) {
        fprintf(stderr, "%s:%d: expected 'f v0 v1 v2 r g b'\n", path, lineNum);
        fclose(file);
        return 1;
      }
      for (int i = 0; i < 3; i++) {
        if (v[i] < 1 || v[i] > model->numVerts) {
          fprintf(stderr, "%s:%d: vertex %d is not defined above this face\n",
              path, lineNum, v[i]);
          fclose(file);
          return 1;
        }
        if (rgb[i] < 0 || rgb[i] > 255) {
          fprintf(stderr, "%s:%d: color values must be 0-255\n", path, lineNum);
          fclose(file);
          return 1;
        }
      }
      if (model->numFaces == capFaces) {
        capFaces *= 2;
        model->faces = realloc(model->faces, sizeof(uint32_t) * 4 * capFaces);
      }
      // the kernel wants 0 based indices and colors in the framebuffer's format
      for (int i = 0; i < 3; i++) {
        model->faces[4 * model->numFaces + i] = v[i] - 1;
      }
      model->faces[4 * model->numFaces + 3] =
        ((uint32_t)rgb[0] << 24) | ((uint32_t)rgb[1] << 16) | ((uint32_t)rgb[2] << 8);
      model->numFaces++;
    }
    // anything else (comments, blank lines) is ignored
  }

  fclose(file);
  if (model->numFaces == 0) {
    fprintf(stderr, "%s has no faces\n", path);
    return 1;
  }
  return 0;
}

// points the camera at the middle of the model's bounding box, far enough
// back that the whole model fits on screen
void fit_camera(struct model *model, struct camera *camera) {
  float min[3], max[3], radius = 0;

  for (int i = 0; i < 3; i++) {
    min[i] = max[i] = model->verts[i];
  }
  for (int v = 0; v < model->numVerts; v++) {
    for (int i = 0; i < 3; i++) {
      min[i] = fminf(min[i], model->verts[3 * v + i]);
      max[i] = fmaxf(max[i], model->verts[3 * v + i]);
    }
  }
  for (int i = 0; i < 3; i++) {
    camera->center[i] = (min[i] + max[i]) / 2;
    radius += (max[i] - min[i]) * (max[i] - min[i]) / 4;
  }
  radius = sqrtf(radius);

  // the vertical field of view is the narrower one
  float halfFovY = atanf(tanf(HALF_FOV) * WINDOW_HEIGHT / WINDOW_WIDTH);
  camera->distance = 1.1f * radius / sinf(halfFovY);
  camera->yaw = 0.6f;
  camera->pitch = -0.4f;
}

void write_f(uint8_t *mem, int addr, float value) {
  memcpy(mem + addr, &value, 4);
}

void write_u(uint8_t *mem, int addr, uint32_t value) {
  memcpy(mem + addr, &value, 4);
}

int main(int argc, char **argv) {
  core_t *core;
  struct Game game;
  struct model model;
  struct camera camera;
  FILE *kernel_file;
  uint32_t binary[MAX_B_SIZE];
  uint32_t constants[] = R_CONSTANTS;
  uint32_t pixel;
  uint8_t *dataMem;
  int redraw = 1;

  if (argc != 2) {
    fprintf(stderr, "usage: %s model.obj\n", argv[0]);
    return 1;
  }
  if (parse_model(argv[1], &model)) {
    return 1;
  }
  fit_camera(&model, &camera);

  // memory: header, vertices, faces, framebuffer, depth buffer
  int vertAddr = R_HEADER_SIZE;
  int faceAddr = vertAddr + 12 * model.numVerts;
  int fbAddr = faceAddr + 16 * model.numFaces;
  int zbAddr = fbAddr + 4 * WINDOW_WIDTH * WINDOW_HEIGHT;
  int memSize = zbAddr + 4 * WINDOW_WIDTH * WINDOW_HEIGHT;
  if (memSize > S_MEM_SIZE) {
    fprintf(stderr, "Model is too big: needs %d bytes of shared memory, have %d\n",
        memSize, S_MEM_SIZE);
    return 1;
  }

  // calloc so the framebuffer (black background) and depth buffer start zeroed.
  // the kernel only writes to the core's copy, so they stay zeroed here
  dataMem = calloc(memSize, 1);
  memcpy(dataMem + vertAddr, model.verts, 12 * model.numVerts);
  memcpy(dataMem + faceAddr, model.faces, 16 * model.numFaces);
  memcpy(dataMem + R_CONSTANTS_ADDR, constants, sizeof(constants));
  write_u(dataMem, R_FB_ADDR, fbAddr);
  write_u(dataMem, R_ZB_ADDR, zbAddr);
  write_u(dataMem, R_FACE_PTR, faceAddr);
  write_u(dataMem, R_VERT_ADDR, vertAddr);
  write_f(dataMem, R_WIDTH, WINDOW_WIDTH);
  write_f(dataMem, R_INV_WIDTH, 1.0f / WINDOW_WIDTH);
  write_f(dataMem, R_HALF_WIDTH, WINDOW_WIDTH / 2.0f);
  write_f(dataMem, R_HALF_HEIGHT, WINDOW_HEIGHT / 2.0f);
  write_f(dataMem, R_FOCAL, (WINDOW_WIDTH / 2.0f) / tanf(HALF_FOV));
  write_f(dataMem, R_NEAR, NEAR);

  // the core holds 4mb of shared memory, too much for the stack
  core = malloc(sizeof(core_t));
  core_init(core, WINDOW_WIDTH * WINDOW_HEIGHT);
  init_display(&game, WINDOW_WIDTH, WINDOW_HEIGHT);

  kernel_file = fopen("build/retriever_kernel", "r");
  if (!kernel_file) {
    fprintf(stderr, "Could not open build/retriever_kernel\n");
    return 1;
  }
  size_t binary_size = fread(binary, 4, MAX_B_SIZE, kernel_file);
  fclose(kernel_file);

  printf("%d vertices, %d faces. arrow keys orbit, w/s zoom\n",
      model.numVerts, model.numFaces);

  for (;;) {
    while (SDL_PollEvent(&(game.event))) {
      if (game.event.type == SDL_QUIT) {
        destroy_display(&game);
        return 0;
      }
      if (game.event.type == SDL_KEYDOWN) {
        redraw = 1;
        switch (game.event.key.keysym.sym) {
          case SDLK_LEFT:
            camera.yaw -= ROTATE_STEP;
            break;
          case SDLK_RIGHT:
            camera.yaw += ROTATE_STEP;
            break;
          case SDLK_UP:
            camera.pitch = fminf(camera.pitch + ROTATE_STEP, 1.5f);
            break;
          case SDLK_DOWN:
            camera.pitch = fmaxf(camera.pitch - ROTATE_STEP, -1.5f);
            break;
          case SDLK_w:
            camera.distance *= 0.8f;
            break;
          case SDLK_s:
            camera.distance *= 1.25f;
            break;
          default:
            redraw = 0;
        }
      }
    }

    if (!redraw) {
      SDL_Delay(16);
      continue;
    }
    redraw = 0;

    // the kernel's taylor series only works for angles in [-pi, pi]
    if (camera.yaw > PI) {
      camera.yaw -= 2 * PI;
    }
    if (camera.yaw < -PI) {
      camera.yaw += 2 * PI;
    }

    // back the camera away from the center, opposite the way it faces
    float forward[3] = {
      sinf(camera.yaw) * cosf(camera.pitch),
      sinf(camera.pitch),
      cosf(camera.yaw) * cosf(camera.pitch)
    };
    write_f(dataMem, R_CAM_X, camera.center[0] - camera.distance * forward[0]);
    write_f(dataMem, R_CAM_Y, camera.center[1] - camera.distance * forward[1]);
    write_f(dataMem, R_CAM_Z, camera.center[2] - camera.distance * forward[2]);
    write_f(dataMem, R_YAW, camera.yaw);
    write_f(dataMem, R_PITCH, camera.pitch);

    // one launch per face
    printf("rendering...\n");
    launch_threads(core, model.numFaces, binary_size * 4, binary, memSize, dataMem);

    for (int x = 0; x < WINDOW_WIDTH; x++) {
      for (int y = 0; y < WINDOW_HEIGHT; y++) {
        memcpy(&pixel, &(core->sharedMem[fbAddr + 4 * (y * WINDOW_WIDTH + x)]), 4);
        SDL_SetRenderDrawColor(game.renderer,
            (pixel >> 24) & 0xFF, (pixel >> 16) & 0xFF, (pixel >> 8) & 0xFF, 0xFF);
        SDL_RenderDrawPoint(game.renderer, x, y);
      }
    }
    SDL_RenderPresent(game.renderer);
    printf("done\n");
  }
}
