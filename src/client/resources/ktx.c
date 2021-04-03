#include "ktx.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define KTX2_HEADER_SIZE 80

typedef struct index32_t
{
    uint32_t byteOffset;
    uint32_t byteLength;
} index32_t;

typedef struct index64_t
{
    uint32_t byteOffset;
    uint32_t byteLength;
} index64_t;

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
    index32_t data_format_descriptor;
    index32_t key_value_data;
    index64_t supercompression_global_data;
} ktx2_header_t;

typedef enum supercompression_t
{
    SUPERCOMPRESSION_NONE = 0,
    SUPERCOMPRESSION_CRN = 1,
    SUPERCOMPRESSION_ZLIB = 2,
    SUPERCOMPRESSION_ZSTD = 3
} supercompression_t;

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

void ktx2_read_info(void* file, ktx_info_t* ktx_info)
{
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
