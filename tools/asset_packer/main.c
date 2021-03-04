#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct dds_pixel_format
{
	uint32_t size;
	uint32_t flags;
	uint32_t fourcc;
	uint32_t rgb_bit_count;
	uint32_t bit_mask[4];
} dds_pixel_format;

typedef struct dds_header
{
	uint32_t size;
	uint32_t flags;
	uint32_t height;
	uint32_t width;
	uint32_t pitch_lin_size;
	uint32_t depth;
	uint32_t mip_count;
	uint32_t reserved1[11];
	dds_pixel_format pixel_format;
	uint32_t caps1;
	uint32_t caps2;
	uint32_t caps3;
	uint32_t caps4;
	uint32_t reserved2;
} dds_header;

typedef struct dds_header_dxgi
{
	uint32_t dxgi_format;
	uint32_t dimension;
	uint32_t misc_flags;
	uint32_t array_size;
	uint32_t misc_flags2;
} dds_header_dxgi;

typedef struct texture_info
{
    int                 data_offset;   // start offset of pixel data
    int                 size_bytes;
    ddsktx_format       format;
    unsigned int        flags;         // ddsktx_texture_flags
    int                 width;
    int                 height;
    int                 depth;
    int                 num_layers;
    int                 num_mips;
    int                 bpp;
} texture_info;

typedef struct mem_reader
{
    const uint8_t* buff;
    int            total;
    int            offset;
} mem_reader;

static const char* texconv_args = "-f BC7_UNORM -ft DDS -bc q -y -timing -pmalpha -m 1";

static inline int dds_read(mem_reader* reader, void* buff, int size)
{
	int read_bytes = (reader->offset + size) <= reader->total ? size : (reader->total - reader->offset);
	memcpy(buff, reader->buff + reader->offset, read_bytes);
	reader->offset += read_bytes;
	return read_bytes;
}

static bool parse_dds(texture_info* tc, const void* file_data, int size)
{
    mem_reader r = { (const uint8_t*)file_data, size, sizeof(uint32_t) };
    dds_header header;
    if (read(&r, &header, sizeof(header)) < DDS_HEADER_SIZE ||
        header.size != DDS_HEADER_SIZE)
    {
        fprintf(stderr, "dds: header size does not match");
    }

    uint32_t required_flags = (DDSD_HEIGHT | DDSD_WIDTH);
    if ((header.flags & required_flags) != required_flags) {
        fprintf(stderr, "dds: have invalid flags");
    }

    if (header.pixel_format.size != sizeof(dds_pixel_format)) {
        fprintf(stderr, "dds: pixel format header is invalid");
    }

    uint32_t dxgi_format = 0;
    uint32_t array_size = 1;
    if (DDPF_FOURCC == (header.flags & DDPF_FOURCC) &&
        header.pixel_format.fourcc == DDS_DX10)
    {
        dds_header_dxgi dxgi_header;
        read(&r, &dxgi_header, sizeof(dxgi_header));
        dxgi_format = dxgi_header.dxgi_format;
        array_size = dxgi_header.array_size;
    }

    if ((header.caps1 & DDSCAPS_TEXTURE) == 0) {
        fprintf(stderr, "dds: unsupported caps");
    }

    bool cubemap = (header.caps2 & DDSCAPS2_CUBEMAP) != 0;
    if (cubemap && (header.caps2 & DDSCAPS2_CUBEMAP_ALLSIDES) != DDSCAPS2_CUBEMAP_ALLSIDES) {
        fprintf(stderr, "dds: incomplete cubemap");
    }

    ddsktx_format format = _DDSKTX_FORMAT_COUNT;
    bool has_alpha = (header.pixel_format.flags & DDPF_ALPHA) != 0;
    bool srgb = false;

    if (dxgi_format == 0) {
        if ((header.pixel_format.flags & DDPF_FOURCC) == DDPF_FOURCC) {
            int count = sizeof(k__translate_dds_fourcc) / sizeof(dds_translate_fourcc_format);
            for (int i = 0; i < count; i++) {
                if (k__translate_dds_fourcc[i].dds_format == header.pixel_format.fourcc) {
                    format = k__translate_dds_fourcc[i].format;
                    break;
                }
            }
        }
        else {
            int count = sizeof(k__translate_dds_pixel) / sizeof(dds_translate_pixel_format);
            for (int i = 0; i < count; i++) {
                const dds_translate_pixel_format* f = &k__translate_dds_pixel[i];
                if (f->bit_count == header.pixel_format.rgb_bit_count &&
                    f->flags == header.pixel_format.flags &&
                    f->bit_mask[0] == header.pixel_format.bit_mask[0] &&
                    f->bit_mask[1] == header.pixel_format.bit_mask[1] &&
                    f->bit_mask[2] == header.pixel_format.bit_mask[2] &&
                    f->bit_mask[3] == header.pixel_format.bit_mask[3])
                {
                    format = f->format;
                    break;
                }
            }
        }
    }
    else {
        int count = sizeof(k__translate_dxgi) / sizeof(dds_translate_fourcc_format);
        for (int i = 0; i < count; i++) {
            if (k__translate_dxgi[i].dds_format == dxgi_format) {
                format = k__translate_dxgi[i].format;
                srgb = k__translate_dxgi[i].srgb;
                break;
            }
        }
    }

    if (format == _DDSKTX_FORMAT_COUNT) {
        fprintf(stderr, "dds: unknown format");
    }

    memset(tc, 0x0, sizeof(texture_info));
    tc->data_offset = r.offset;
    tc->size_bytes = r.total - r.offset;
    tc->format = format;
    tc->width = (int)header.width;
    tc->height = (int)header.height;
    tc->depth = max(1, (int)header.depth);
    tc->num_layers = max(1, (int)array_size);
    tc->num_mips = (header.caps1 & DDSCAPS_MIPMAP) ? (int)header.mip_count : 1;
    tc->bpp = k__block_info[format].bpp;
    if (has_alpha || k__formats_info[format].has_alpha)
        tc->flags |= DDSKTX_TEXTURE_FLAG_ALPHA;
    if (cubemap)
        tc->flags |= DDSKTX_TEXTURE_FLAG_CUBEMAP;
    if (srgb)
        tc->flags |= DDSKTX_TEXTURE_FLAG_SRGB;
    tc->flags |= DDSKTX_TEXTURE_FLAG_DDS;

    return true;
}

int main(int argc, char* argv[])
{

	return 0;
}