#ifndef RETRIEVER_H
#define RETRIEVER_H

// shared memory layout for retriever_kernel. all fields are 4 bytes.
// "int" fields are uint32_t, "float" fields are IEEE floats.
//
// the kernel draws one face per launch, so launch it numFaces times.
// before the first launch:
//   - the whole region below must be zeroed except for the fields you set
//   - the framebuffer should hold the background color
//   - the depth buffer must be all zeroes (it stores 1/z, so 0 is infinitely far)
//
// camera space is +x right, +y up, +z forward. yaw turns toward +x and pitch
// turns toward +y. both angles must be in [-pi, pi].
// faces are drawn only if their vertices appear counter clockwise from the
// camera (the usual obj convention) and all three are in front of NEAR.

// set by the host
#define R_FB_ADDR     0   // int: pixel i is written to FB_ADDR + 4*i as 0xRRGGBB00
#define R_CAM_X       4   // float
#define R_CAM_Y       8   // float
#define R_CAM_Z       12  // float
#define R_YAW         16  // float, radians
#define R_PITCH       20  // float, radians
#define R_ZB_ADDR     24  // int: depth of pixel i is at ZB_ADDR + 4*i
#define R_WIDTH       28  // float: window width
#define R_INV_WIDTH   32  // float: 1 / width
#define R_HALF_WIDTH  36  // float: width / 2
#define R_HALF_HEIGHT 40  // float: height / 2
#define R_FOCAL       44  // float: focal length in pixels, (width/2) / tan(fov/2)
#define R_NEAR        48  // float: near plane distance, > 0
#define R_FACE_PTR    52  // int: address of the first face. the kernel advances it
#define R_VERT_ADDR   56  // int: address of the vertex array

// constants the kernel needs, copy R_CONSTANTS here
#define R_CONSTANTS_ADDR 64
#define R_CONSTANTS { \
  0x3F800000, /* 1.0f */ \
  0xBF800000, /* -1.0f */ \
  0x3F000000, /* 0.5f */ \
  0x4B000000, /* 8388608.0f (2^23) */ \
  0x7EF311C3, /* reciprocal initial guess magic number */ \
  0xFFFFFFFF, /* int -1 */ \
  0xBE2AAAAB, /* -1/3! */ \
  0x3C088889, /* 1/5! */ \
  0xB9500D01, /* -1/7! */ \
  0x3638EF1D, /* 1/9! */ \
  0xB2D7322B, /* -1/11! */ \
  0xBF000000, /* -1/2! */ \
  0x3D2AAAAB, /* 1/4! */ \
  0xBAB60B61, /* -1/6! */ \
  0x37D00D01, /* 1/8! */ \
  0xB493F27E, /* -1/10! */ \
  0x310F76C7, /* 1/12! */ \
}

// 132 - 255 is kernel scratch space
#define R_HEADER_SIZE 256

// the model can go anywhere after the header:
//   vertices: 3 floats (x, y, z) each, starting at VERT_ADDR
//   faces: 4 ints each (v0, v1, v2, color), starting at FACE_PTR. vertex
//   indices are 0 based, color is 0xRRGGBB00

#endif
