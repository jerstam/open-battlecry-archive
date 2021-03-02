#pragma once

#include "../../macros.h"

const u32 post_effect_vert_spirv[398] = {
0x07230203,0x00010500,0x000d000a,0x0000002d,0x00000000,0x00020011,0x00000001,0x0006000b,
0x00000002,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
0x0008000f,0x00000000,0x00000005,0x6e69616d,0x00000000,0x0000000a,0x0000000d,0x0000001e,
0x00070007,0x00000001,0x74736f70,0x6666655f,0x2e746365,0x74726576,0x00000000,0x00330003,
0x00000002,0x000001c2,0x00000001,0x72657623,0x6e6f6973,0x30353420,0x616c0a0a,0x74756f79,
0x6f6c2820,0x69746163,0x3d206e6f,0x20293020,0x2074756f,0x32636576,0x74756f20,0x0a3b5655,
0x696f760a,0x616d2064,0x29286e69,0x0a7b0a20,0x74756f09,0x3d205655,0x63657620,0x67282832,
0x65565f6c,0x78657472,0x65646e49,0x3c3c2078,0x20293120,0x2c322026,0x5f6c6720,0x74726556,
0x6e497865,0x20786564,0x29322026,0x67090a3b,0x6f505f6c,0x69746973,0x3d206e6f,0x63657620,
0x756f2834,0x20565574,0x2e32202a,0x2d206630,0x302e3120,0x30202c66,0x2c66312e,0x302e3120,
0x0a3b2966,0x0000007d,0x000a0004,0x475f4c47,0x4c474f4f,0x70635f45,0x74735f70,0x5f656c79,
0x656e696c,0x7269645f,0x69746365,0x00006576,0x00080004,0x475f4c47,0x4c474f4f,0x6e695f45,
0x64756c63,0x69645f65,0x74636572,0x00657669,0x00040005,0x00000005,0x6e69616d,0x00000000,
0x00040005,0x0000000a,0x5574756f,0x00000056,0x00060005,0x0000000d,0x565f6c67,0x65747265,
0x646e4978,0x00007865,0x00060005,0x0000001c,0x505f6c67,0x65567265,0x78657472,0x00000000,
0x00060006,0x0000001c,0x00000000,0x505f6c67,0x7469736f,0x006e6f69,0x00070006,0x0000001c,
0x00000001,0x505f6c67,0x746e696f,0x657a6953,0x00000000,0x00070006,0x0000001c,0x00000002,
0x435f6c67,0x4470696c,0x61747369,0x0065636e,0x00070006,0x0000001c,0x00000003,0x435f6c67,
0x446c6c75,0x61747369,0x0065636e,0x00030005,0x0000001e,0x00000000,0x0006014a,0x72746e65,
0x6f702d79,0x20746e69,0x6e69616d,0x00000000,0x0006014a,0x65696c63,0x7620746e,0x616b6c75,
0x3030316e,0x00000000,0x0006014a,0x67726174,0x652d7465,0x7320766e,0x76726970,0x00352e31,
0x0007014a,0x67726174,0x652d7465,0x7620766e,0x616b6c75,0x322e316e,0x00000000,0x0006014a,
0x72746e65,0x6f702d79,0x20746e69,0x6e69616d,0x00000000,0x00040047,0x0000000a,0x0000001e,
0x00000000,0x00040047,0x0000000d,0x0000000b,0x0000002a,0x00050048,0x0000001c,0x00000000,
0x0000000b,0x00000000,0x00050048,0x0000001c,0x00000001,0x0000000b,0x00000001,0x00050048,
0x0000001c,0x00000002,0x0000000b,0x00000003,0x00050048,0x0000001c,0x00000003,0x0000000b,
0x00000004,0x00030047,0x0000001c,0x00000002,0x00020013,0x00000003,0x00030021,0x00000004,
0x00000003,0x00030016,0x00000007,0x00000020,0x00040017,0x00000008,0x00000007,0x00000002,
0x00040020,0x00000009,0x00000003,0x00000008,0x0004003b,0x00000009,0x0000000a,0x00000003,
0x00040015,0x0000000b,0x00000020,0x00000001,0x00040020,0x0000000c,0x00000001,0x0000000b,
0x0004003b,0x0000000c,0x0000000d,0x00000001,0x0004002b,0x0000000b,0x0000000f,0x00000001,
0x0004002b,0x0000000b,0x00000011,0x00000002,0x00040017,0x00000018,0x00000007,0x00000004,
0x00040015,0x00000019,0x00000020,0x00000000,0x0004002b,0x00000019,0x0000001a,0x00000001,
0x0004001c,0x0000001b,0x00000007,0x0000001a,0x0006001e,0x0000001c,0x00000018,0x00000007,
0x0000001b,0x0000001b,0x00040020,0x0000001d,0x00000003,0x0000001c,0x0004003b,0x0000001d,
0x0000001e,0x00000003,0x0004002b,0x0000000b,0x0000001f,0x00000000,0x0004002b,0x00000007,
0x00000021,0x40000000,0x0004002b,0x00000007,0x00000023,0x3f800000,0x0004002b,0x00000007,
0x00000026,0x3dcccccd,0x00040020,0x0000002a,0x00000003,0x00000018,0x0005002c,0x00000008,
0x0000002c,0x00000023,0x00000023,0x00050036,0x00000003,0x00000005,0x00000000,0x00000004,
0x000200f8,0x00000006,0x00040008,0x00000001,0x00000007,0x00000000,0x0004003d,0x0000000b,
0x0000000e,0x0000000d,0x000500c4,0x0000000b,0x00000010,0x0000000e,0x0000000f,0x000500c7,
0x0000000b,0x00000012,0x00000010,0x00000011,0x0004006f,0x00000007,0x00000013,0x00000012,
0x000500c7,0x0000000b,0x00000015,0x0000000e,0x00000011,0x0004006f,0x00000007,0x00000016,
0x00000015,0x00050050,0x00000008,0x00000017,0x00000013,0x00000016,0x0003003e,0x0000000a,
0x00000017,0x00040008,0x00000001,0x00000008,0x00000000,0x0004003d,0x00000008,0x00000020,
0x0000000a,0x0005008e,0x00000008,0x00000022,0x00000020,0x00000021,0x00050083,0x00000008,
0x00000025,0x00000022,0x0000002c,0x00050051,0x00000007,0x00000027,0x00000025,0x00000000,
0x00050051,0x00000007,0x00000028,0x00000025,0x00000001,0x00070050,0x00000018,0x00000029,
0x00000027,0x00000028,0x00000026,0x00000023,0x00050041,0x0000002a,0x0000002b,0x0000001e,
0x0000001f,0x0003003e,0x0000002b,0x00000029,0x000100fd,0x00010038
};
const u64 post_effect_vert_size = 1592;