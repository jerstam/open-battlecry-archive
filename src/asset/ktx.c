#include "ktx.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define KTX_ETC1_RGB8_OES                             0x8D64
#define KTX_COMPRESSED_R11_EAC                        0x9270
#define KTX_COMPRESSED_SIGNED_R11_EAC                 0x9271
#define KTX_COMPRESSED_RG11_EAC                       0x9272
#define KTX_COMPRESSED_SIGNED_RG11_EAC                0x9273
#define KTX_COMPRESSED_RGB8_ETC2                      0x9274
#define KTX_COMPRESSED_SRGB8_ETC2                     0x9275
#define KTX_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2  0x9276
#define KTX_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9277
#define KTX_COMPRESSED_RGBA8_ETC2_EAC                 0x9278
#define KTX_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC          0x9279
#define KTX_COMPRESSED_RGB_PVRTC_4BPPV1_IMG           0x8C00
#define KTX_COMPRESSED_RGB_PVRTC_2BPPV1_IMG           0x8C01
#define KTX_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG          0x8C02
#define KTX_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG          0x8C03
#define KTX_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG          0x9137
#define KTX_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG          0x9138
#define KTX_COMPRESSED_RGB_S3TC_DXT1_EXT              0x83F0
#define KTX_COMPRESSED_RGBA_S3TC_DXT1_EXT             0x83F1
#define KTX_COMPRESSED_RGBA_S3TC_DXT3_EXT             0x83F2
#define KTX_COMPRESSED_RGBA_S3TC_DXT5_EXT             0x83F3
#define KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT       0x8C4D
#define KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT       0x8C4E
#define KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT       0x8C4F
#define KTX_COMPRESSED_LUMINANCE_LATC1_EXT            0x8C70
#define KTX_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT      0x8C72
#define KTX_COMPRESSED_RGBA_BPTC_UNORM_ARB            0x8E8C
#define KTX_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB      0x8E8D
#define KTX_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB      0x8E8E
#define KTX_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB    0x8E8F
#define KTX_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT          0x8A54
#define KTX_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT          0x8A55
#define KTX_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT    0x8A56
#define KTX_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT    0x8A57
#define KTX_ATC_RGB_AMD                               0x8C92
#define KTX_ATC_RGBA_EXPLICIT_ALPHA_AMD               0x8C93
#define KTX_ATC_RGBA_INTERPOLATED_ALPHA_AMD           0x87EE
#define KTX_COMPRESSED_RGBA_ADDSKTX_4x4_KHR              0x93B0
#define KTX_COMPRESSED_RGBA_ADDSKTX_5x5_KHR              0x93B2
#define KTX_COMPRESSED_RGBA_ADDSKTX_6x6_KHR              0x93B4
#define KTX_COMPRESSED_RGBA_ADDSKTX_8x5_KHR              0x93B5
#define KTX_COMPRESSED_RGBA_ADDSKTX_8x6_KHR              0x93B6
#define KTX_COMPRESSED_RGBA_ADDSKTX_10x5_KHR             0x93B8
#define KTX_COMPRESSED_SRGB8_ALPHA8_ADDSKTX_4x4_KHR      0x93D0
#define KTX_COMPRESSED_SRGB8_ALPHA8_ADDSKTX_5x5_KHR      0x93D2
#define KTX_COMPRESSED_SRGB8_ALPHA8_ADDSKTX_6x6_KHR      0x93D4
#define KTX_COMPRESSED_SRGB8_ALPHA8_ADDSKTX_8x5_KHR      0x93D5
#define KTX_COMPRESSED_SRGB8_ALPHA8_ADDSKTX_8x6_KHR      0x93D6
#define KTX_COMPRESSED_SRGB8_ALPHA8_ADDSKTX_10x5_KHR     0x93D8

#define KTX_A8                                        0x803C
#define KTX_R8                                        0x8229
#define KTX_R16                                       0x822A
#define KTX_RG8                                       0x822B
#define KTX_RG16                                      0x822C
#define KTX_R16F                                      0x822D
#define KTX_R32F                                      0x822E
#define KTX_RG16F                                     0x822F
#define KTX_RG32F                                     0x8230
#define KTX_RGBA8                                     0x8058
#define KTX_RGBA16                                    0x805B
#define KTX_RGBA16F                                   0x881A
#define KTX_R32UI                                     0x8236
#define KTX_RG32UI                                    0x823C
#define KTX_RGBA32UI                                  0x8D70
#define KTX_RGBA32F                                   0x8814
#define KTX_RGB565                                    0x8D62
#define KTX_RGBA4                                     0x8056
#define KTX_RGB5_A1                                   0x8057
#define KTX_RGB10_A2                                  0x8059
#define KTX_R8I                                       0x8231
#define KTX_R8UI                                      0x8232
#define KTX_R16I                                      0x8233
#define KTX_R16UI                                     0x8234
#define KTX_R32I                                      0x8235
#define KTX_R32UI                                     0x8236
#define KTX_RG8I                                      0x8237
#define KTX_RG8UI                                     0x8238
#define KTX_RG16I                                     0x8239
#define KTX_RG16UI                                    0x823A
#define KTX_RG32I                                     0x823B
#define KTX_RG32UI                                    0x823C
#define KTX_R8_SNORM                                  0x8F94
#define KTX_RG8_SNORM                                 0x8F95
#define KTX_RGB8_SNORM                                0x8F96
#define KTX_RGBA8_SNORM                               0x8F97
#define KTX_R16_SNORM                                 0x8F98
#define KTX_RG16_SNORM                                0x8F99
#define KTX_RGB16_SNORM                               0x8F9A
#define KTX_RGBA16_SNORM                              0x8F9B
#define KTX_SRGB8                                     0x8C41
#define KTX_SRGB8_ALPHA8                              0x8C43
#define KTX_RGBA32UI                                  0x8D70
#define KTX_RGB32UI                                   0x8D71
#define KTX_RGBA16UI                                  0x8D76
#define KTX_RGB16UI                                   0x8D77
#define KTX_RGBA8UI                                   0x8D7C
#define KTX_RGB8UI                                    0x8D7D
#define KTX_RGBA32I                                   0x8D82
#define KTX_RGB32I                                    0x8D83
#define KTX_RGBA16I                                   0x8D88
#define KTX_RGB16I                                    0x8D89
#define KTX_RGBA8I                                    0x8D8E
#define KTX_RGB8                                      0x8051
#define KTX_RGB8I                                     0x8D8F
#define KTX_RGB9_E5                                   0x8C3D
#define KTX_R11F_G11F_B10F                            0x8C3A

#define KTX_ZERO                                      0
#define KTX_RED                                       0x1903
#define KTX_ALPHA                                     0x1906
#define KTX_RGB                                       0x1907
#define KTX_RGBA                                      0x1908
#define KTX_BGRA                                      0x80E1
#define KTX_RG                                        0x8227

#define KTX_BYTE                                      0x1400
#define KTX_UNSIGNED_BYTE                             0x1401
#define KTX_SHORT                                     0x1402
#define KTX_UNSIGNED_SHORT                            0x1403
#define KTX_INT                                       0x1404
#define KTX_UNSIGNED_INT                              0x1405
#define KTX_FLOAT                                     0x1406
#define KTX_HALF_FLOAT                                0x140B
#define KTX_UNSIGNED_INT_5_9_9_9_REV                  0x8C3E
#define KTX_UNSIGNED_SHORT_5_6_5                      0x8363
#define KTX_UNSIGNED_SHORT_4_4_4_4                    0x8033
#define KTX_UNSIGNED_SHORT_5_5_5_1                    0x8034
#define KTX_UNSIGNED_INT_2_10_10_10_REV               0x8368
#define KTX_UNSIGNED_INT_10F_11F_11F_REV              0x8C3B

typedef struct ktx_header_t
{
	uint8_t identifier[12];
	uint32_t endianness;
	uint32_t type;
	uint32_t type_size;
	uint32_t format;
	uint32_t internal_format;
	uint32_t base_internal_format;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	uint32_t array_count;
	uint32_t face_count;
	uint32_t mip_count;
	uint32_t metadata_size;
} ktx_header_t;

typedef struct ktx2_header_t
{
	uint8_t identifier[12];
	uint32_t vk_format;
	uint32_t type_size;
	uint32_t pixel_width;
	uint32_t pixel_height;
	uint32_t pixel_depth;
	uint32_t layer_count;
	uint32_t face_count;
	uint32_t level_count;
	uint32_t supercompression_scheme;
	uint32_t dfd_byte_offset;
	uint32_t dfd_byte_length;
	uint32_t kvd_byte_offset;
	uint32_t kvd_byte_length;
	uint64_t sgd_byte_offset;
	uint64_t sgd_byte_length;
} ktx2_header_t;

typedef struct memory_t
{
	const uint8_t* buffer;
	uint32_t total;
	uint32_t offset;
} memory_t;

typedef struct block_info_t
{
	uint8_t bytes_per_pixel;
	uint8_t block_width;
	uint8_t block_height;
	uint8_t block_size;
	uint8_t min_block_x;
	uint8_t min_block_y;
	uint8_t depth_bits;
	uint8_t stencil_bits;
	uint8_t r_bits;
	uint8_t g_bits;
	uint8_t b_bits;
	uint8_t a_bits;
	uint8_t encoding;
} block_info_t;

typedef enum encode_type_t
{
    ENCODE_UNORM,
    ENCODE_SNORM,
    ENCODE_FLOAT,
    ENCODE_INT,
    ENCODE_UINT,
    ENCODE_COUNT
} encode_type_t;

typedef struct ktx_format_info_t
{
    uint32_t internal_format;
    uint32_t internal_format_srgb;
    uint32_t format;
    uint32_t type;
} ktx_format_info_t;

static const uint8_t ktx_identifier[12] = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };
static const uint8_t ktx2_identifier[12] = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };

static const block_info_t block_infos[] =
{
    //  +-------------------------------------------- bits per pixel
    //  |  +----------------------------------------- block width
    //  |  |  +-------------------------------------- block height
    //  |  |  |   +---------------------------------- block size
    //  |  |  |   |  +------------------------------- min blocks x
    //  |  |  |   |  |  +---------------------------- min blocks y
    //  |  |  |   |  |  |   +------------------------ depth bits
    //  |  |  |   |  |  |   |  +--------------------- stencil bits
    //  |  |  |   |  |  |   |  |   +---+---+---+----- r, g, b, a bits
    //  |  |  |   |  |  |   |  |   r   g   b   a  +-- encoding type
    //  |  |  |   |  |  |   |  |   |   |   |   |  |
    {   4, 4, 4,  8, 1, 1,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // BC1
    {   8, 4, 4, 16, 1, 1,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // BC2
    {   8, 4, 4, 16, 1, 1,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // BC3
    {   4, 4, 4,  8, 1, 1,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // BC4
    {   8, 4, 4, 16, 1, 1,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // BC5
    {   8, 4, 4, 16, 1, 1,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_FLOAT) }, // BC6H
    {   8, 4, 4, 16, 1, 1,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // BC7
    {   4, 4, 4,  8, 1, 1,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // ETC1
    {   4, 4, 4,  8, 1, 1,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // ETC2
    {   8, 4, 4, 16, 1, 1,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // ETC2A
    {   4, 4, 4,  8, 1, 1,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // ETC2A1
    {   2, 8, 4,  8, 2, 2,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // PTC12
    {   4, 4, 4,  8, 2, 2,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // PTC14
    {   2, 8, 4,  8, 2, 2,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // PTC12A
    {   4, 4, 4,  8, 2, 2,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // PTC14A
    {   2, 8, 4,  8, 2, 2,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // PTC22
    {   4, 4, 4,  8, 2, 2,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // PTC24
    {   4, 4, 4,  8, 1, 1,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // ATC
    {   8, 4, 4, 16, 1, 1,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // ATCE
    {   8, 4, 4, 16, 1, 1,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // ATCI
    {   8, 4, 4, 16, 1, 1,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // ASTC4x4
    {   6, 5, 5, 16, 1, 1,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // ASTC5x5
    {   4, 6, 6, 16, 1, 1,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // ASTC6x6
    {   4, 8, 5, 16, 1, 1,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // ASTC8x5
    {   3, 8, 6, 16, 1, 1,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // ASTC8x6
    {   3, 10, 5, 16, 1, 1, 0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // ASTC10x5
    {   0, 0, 0,  0, 0, 0,  0, 0,  0,  0,  0,  0, (uint8_t)(ENCODE_COUNT) }, // Unknown
    {   8, 1, 1,  1, 1, 1,  0, 0,  0,  0,  0,  8, (uint8_t)(ENCODE_UNORM) }, // A8
    {   8, 1, 1,  1, 1, 1,  0, 0,  8,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // R8
    {  32, 1, 1,  4, 1, 1,  0, 0,  8,  8,  8,  8, (uint8_t)(ENCODE_UNORM) }, // RGBA8
    {  32, 1, 1,  4, 1, 1,  0, 0,  8,  8,  8,  8, (uint8_t)(ENCODE_SNORM) }, // RGBA8S
    {  32, 1, 1,  4, 1, 1,  0, 0, 16, 16,  0,  0, (uint8_t)(ENCODE_UNORM) }, // RG16
    {  24, 1, 1,  3, 1, 1,  0, 0,  8,  8,  8,  0, (uint8_t)(ENCODE_UNORM) }, // RGB8
    {  16, 1, 1,  2, 1, 1,  0, 0, 16,  0,  0,  0, (uint8_t)(ENCODE_UNORM) }, // R16
    {  32, 1, 1,  4, 1, 1,  0, 0, 32,  0,  0,  0, (uint8_t)(ENCODE_FLOAT) }, // R32F
    {  16, 1, 1,  2, 1, 1,  0, 0, 16,  0,  0,  0, (uint8_t)(ENCODE_FLOAT) }, // R16F
    {  32, 1, 1,  4, 1, 1,  0, 0, 16, 16,  0,  0, (uint8_t)(ENCODE_FLOAT) }, // RG16F
    {  32, 1, 1,  4, 1, 1,  0, 0, 16, 16,  0,  0, (uint8_t)(ENCODE_SNORM) }, // RG16S
    {  64, 1, 1,  8, 1, 1,  0, 0, 16, 16, 16, 16, (uint8_t)(ENCODE_FLOAT) }, // RGBA16F
    {  64, 1, 1,  8, 1, 1,  0, 0, 16, 16, 16, 16, (uint8_t)(ENCODE_UNORM) }, // RGBA16
    {  32, 1, 1,  4, 1, 1,  0, 0,  8,  8,  8,  8, (uint8_t)(ENCODE_UNORM) }, // BGRA8
    {  32, 1, 1,  4, 1, 1,  0, 0, 10, 10, 10,  2, (uint8_t)(ENCODE_UNORM) }, // RGB10A2
    {  32, 1, 1,  4, 1, 1,  0, 0, 11, 11, 10,  0, (uint8_t)(ENCODE_UNORM) }, // RG11B10F
    {  16, 1, 1,  2, 1, 1,  0, 0,  8,  8,  0,  0, (uint8_t)(ENCODE_UNORM) }, // RG8
    {  16, 1, 1,  2, 1, 1,  0, 0,  8,  8,  0,  0, (uint8_t)(ENCODE_SNORM) }  // RG8S
};

static const ktx_format_info_t translate_ktx_format[] = {
    { KTX_COMPRESSED_RGBA_S3TC_DXT1_EXT,            KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,        KTX_COMPRESSED_RGBA_S3TC_DXT1_EXT,            KTX_ZERO,                         }, // BC1
    { KTX_COMPRESSED_RGBA_S3TC_DXT3_EXT,            KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,        KTX_COMPRESSED_RGBA_S3TC_DXT3_EXT,            KTX_ZERO,                         }, // BC2
    { KTX_COMPRESSED_RGBA_S3TC_DXT5_EXT,            KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,        KTX_COMPRESSED_RGBA_S3TC_DXT5_EXT,            KTX_ZERO,                         }, // BC3
    { KTX_COMPRESSED_LUMINANCE_LATC1_EXT,           KTX_ZERO,                                       KTX_COMPRESSED_LUMINANCE_LATC1_EXT,           KTX_ZERO,                         }, // BC4
    { KTX_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT,     KTX_ZERO,                                       KTX_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT,     KTX_ZERO,                         }, // BC5
    { KTX_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB,     KTX_ZERO,                                       KTX_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB,     KTX_ZERO,                         }, // BC6H
    { KTX_COMPRESSED_RGBA_BPTC_UNORM_ARB,           KTX_ZERO,                                       KTX_COMPRESSED_RGBA_BPTC_UNORM_ARB,           KTX_ZERO,                         }, // BC7
    { KTX_ETC1_RGB8_OES,                            KTX_ZERO,                                       KTX_ETC1_RGB8_OES,                            KTX_ZERO,                         }, // ETC1
    { KTX_COMPRESSED_RGB8_ETC2,                     KTX_ZERO,                                       KTX_COMPRESSED_RGB8_ETC2,                     KTX_ZERO,                         }, // ETC2
    { KTX_COMPRESSED_RGBA8_ETC2_EAC,                KTX_COMPRESSED_SRGB8_ETC2,                      KTX_COMPRESSED_RGBA8_ETC2_EAC,                KTX_ZERO,                         }, // ETC2A
    { KTX_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, KTX_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2,  KTX_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, KTX_ZERO,                         }, // ETC2A1
    { KTX_COMPRESSED_RGB_PVRTC_2BPPV1_IMG,          KTX_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT,           KTX_COMPRESSED_RGB_PVRTC_2BPPV1_IMG,          KTX_ZERO,                         }, // PTC12
    { KTX_COMPRESSED_RGB_PVRTC_4BPPV1_IMG,          KTX_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT,           KTX_COMPRESSED_RGB_PVRTC_4BPPV1_IMG,          KTX_ZERO,                         }, // PTC14
    { KTX_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG,         KTX_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT,     KTX_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG,         KTX_ZERO,                         }, // PTC12A
    { KTX_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG,         KTX_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT,     KTX_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG,         KTX_ZERO,                         }, // PTC14A
    { KTX_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG,         KTX_ZERO,                                       KTX_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG,         KTX_ZERO,                         }, // PTC22
    { KTX_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG,         KTX_ZERO,                                       KTX_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG,         KTX_ZERO,                         }, // PTC24
    { KTX_ATC_RGB_AMD,                              KTX_ZERO,                                       KTX_ATC_RGB_AMD,                              KTX_ZERO,                         }, // ATC
    { KTX_ATC_RGBA_EXPLICIT_ALPHA_AMD,              KTX_ZERO,                                       KTX_ATC_RGBA_EXPLICIT_ALPHA_AMD,              KTX_ZERO,                         }, // ATCE
    { KTX_ATC_RGBA_INTERPOLATED_ALPHA_AMD,          KTX_ZERO,                                       KTX_ATC_RGBA_INTERPOLATED_ALPHA_AMD,          KTX_ZERO,                         }, // ATCI
    { KTX_COMPRESSED_RGBA_ADDSKTX_4x4_KHR,          KTX_COMPRESSED_SRGB8_ALPHA8_ADDSKTX_4x4_KHR,    KTX_COMPRESSED_RGBA_ADDSKTX_4x4_KHR,          KTX_ZERO,                         }, // ASTC4x4
    { KTX_COMPRESSED_RGBA_ADDSKTX_5x5_KHR,          KTX_COMPRESSED_SRGB8_ALPHA8_ADDSKTX_5x5_KHR,    KTX_COMPRESSED_RGBA_ADDSKTX_5x5_KHR,          KTX_ZERO,                         }, // ASTC5x5
    { KTX_COMPRESSED_RGBA_ADDSKTX_6x6_KHR,          KTX_COMPRESSED_SRGB8_ALPHA8_ADDSKTX_6x6_KHR,    KTX_COMPRESSED_RGBA_ADDSKTX_6x6_KHR,          KTX_ZERO,                         }, // ASTC6x6
    { KTX_COMPRESSED_RGBA_ADDSKTX_8x5_KHR,          KTX_COMPRESSED_SRGB8_ALPHA8_ADDSKTX_8x5_KHR,    KTX_COMPRESSED_RGBA_ADDSKTX_8x5_KHR,          KTX_ZERO,                         }, // ASTC8x5
    { KTX_COMPRESSED_RGBA_ADDSKTX_8x6_KHR,          KTX_COMPRESSED_SRGB8_ALPHA8_ADDSKTX_8x6_KHR,    KTX_COMPRESSED_RGBA_ADDSKTX_8x6_KHR,          KTX_ZERO,                         }, // ASTC8x6
    { KTX_COMPRESSED_RGBA_ADDSKTX_10x5_KHR,         KTX_COMPRESSED_SRGB8_ALPHA8_ADDSKTX_10x5_KHR,   KTX_COMPRESSED_RGBA_ADDSKTX_10x5_KHR,         KTX_ZERO,                         }, // ASTC10x5
    { KTX_ZERO,                                     KTX_ZERO,                                       KTX_ZERO,                                     KTX_ZERO,                         }, // Unknown
    { KTX_ALPHA,                                    KTX_ZERO,                                       KTX_ALPHA,                                    KTX_UNSIGNED_BYTE,                }, // A8
    { KTX_R8,                                       KTX_ZERO,                                       KTX_RED,                                      KTX_UNSIGNED_BYTE,                }, // R8
    { KTX_RGBA8,                                    KTX_SRGB8_ALPHA8,                               KTX_RGBA,                                     KTX_UNSIGNED_BYTE,                }, // RGBA8
    { KTX_RGBA8_SNORM,                              KTX_ZERO,                                       KTX_RGBA,                                     KTX_BYTE,                         }, // RGBA8S
    { KTX_RG16,                                     KTX_ZERO,                                       KTX_RG,                                       KTX_UNSIGNED_SHORT,               }, // RG16
    { KTX_RGB8,                                     KTX_SRGB8,                                      KTX_RGB,                                      KTX_UNSIGNED_BYTE,                }, // RGB8
    { KTX_R16,                                      KTX_ZERO,                                       KTX_RED,                                      KTX_UNSIGNED_SHORT,               }, // R16
    { KTX_R32F,                                     KTX_ZERO,                                       KTX_RED,                                      KTX_FLOAT,                        }, // R32F
    { KTX_R16F,                                     KTX_ZERO,                                       KTX_RED,                                      KTX_HALF_FLOAT,                   }, // R16F
    { KTX_RG16F,                                    KTX_ZERO,                                       KTX_RG,                                       KTX_FLOAT,                        }, // RG16F
    { KTX_RG16_SNORM,                               KTX_ZERO,                                       KTX_RG,                                       KTX_SHORT,                        }, // RG16S
    { KTX_RGBA16F,                                  KTX_ZERO,                                       KTX_RGBA,                                     KTX_HALF_FLOAT,                   }, // RGBA16F
    { KTX_RGBA16,                                   KTX_ZERO,                                       KTX_RGBA,                                     KTX_UNSIGNED_SHORT,               }, // RGBA16
    { KTX_BGRA,                                     KTX_SRGB8_ALPHA8,                               KTX_BGRA,                                     KTX_UNSIGNED_BYTE,                }, // BGRA8
    { KTX_RGB10_A2,                                 KTX_ZERO,                                       KTX_RGBA,                                     KTX_UNSIGNED_INT_2_10_10_10_REV,  }, // RGB10A2
    { KTX_R11F_G11F_B10F,                           KTX_ZERO,                                       KTX_RGB,                                      KTX_UNSIGNED_INT_10F_11F_11F_REV, }, // RG11B10F
    { KTX_RG8,                                      KTX_ZERO,                                       KTX_RG,                                       KTX_UNSIGNED_BYTE,                }, // RG8
    { KTX_RG8_SNORM,                                KTX_ZERO,                                       KTX_RG,                                       KTX_BYTE,                         }  // RG8S
};

static inline int read(memory_t* memory, void* buffer, int size)
{
    int read_bytes = (memory->offset + size) <= memory->total ? size : (memory->total - memory->offset);
    memcpy(buffer, memory->buffer + memory->offset, read_bytes);
    memory->offset += read_bytes;
    return read_bytes;
}

void ktx_parse(const void* data, int size, ktx_info_t* info)
{
    assert(info);
    memset(info, 0x0, sizeof(ktx_info_t));

    // TODO: Support KTX2

    memory_t memory = { (const uint8_t*)data, size, sizeof(uint32_t) };
    ktx_header_t header = { 0 };
    if (read(&memory, &header, sizeof(header)) != sizeof(ktx_header_t)) {
        fprintf(stderr, "ktx; header size does not match");
    }

    if (memcmp(header.identifier, ktx_identifier, sizeof(header.identifier)) != 0) {
        fprintf(stderr, "ktx: invalid file header");
    }

    if (header.endianness != 0x04030201) {
        fprintf(stderr, "ktx: big-endian format is not supported");
    }

    info->metadata_offset = memory.offset;
    info->metadata_size = (int)header.metadata_size;
    memory.offset += (int)header.metadata_size;

    ktx_format_t format = KTX_FORMAT_COUNT;

    int count = sizeof(translate_ktx_format) / sizeof(ktx_format_info_t);
    for (int i = 0; i < count; i++) {
        if (translate_ktx_format[i].internal_format == header.internal_format) {
            format = (ktx_format_t)i;
            break;
        }
    }

    if (format == KTX_FORMAT_COUNT) {
        count = sizeof(k__translate_ktx_fmt2) / sizeof(ktx_format_info2);
        for (int i = 0; i < count; i++) {
            if (k__translate_ktx_fmt2[i].internal_fmt == header.internal_format) {
                format = (ddsktx_format)k__translate_ktx_fmt2[i].format;
                break;
            }
        }
    }

    if (format == KTX_FORMAT_COUNT) {
        fprintf(stderr, "ktx: unsupported format");
    }

    if (header.face_count > 1 && header.face_count != 6) {
        fprintf(stderr, "ktx: incomplete cubemap");
    }

    info->data_offset = memory.offset;
    info->size_bytes = memory.total - memory.offset;
    info->format = format;
    info->width = (int)header.width;
    info->height = (int)header.height;
    info->depth = max((int)header.depth, 1);
    info->num_layers = max((int)header.array_count, 1);
    info->num_mips = max((int)header.mip_count, 1);
    info->bytes_per_pixel = block_infos[format].bytes_per_pixel;

    if (header.face_count == 6)
        info->flags |= DDSKTX_TEXTURE_FLAG_CUBEMAP;
    info->flags |= k__formats_info[format].has_alpha ? DDSKTX_TEXTURE_FLAG_ALPHA : 0;
}
