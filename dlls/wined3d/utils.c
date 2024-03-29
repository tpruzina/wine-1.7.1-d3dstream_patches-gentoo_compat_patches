/*
 * Utility functions for the WineD3D Library
 *
 * Copyright 2002-2004 Jason Edmeades
 * Copyright 2003-2004 Raphael Junqueira
 * Copyright 2004 Christian Costa
 * Copyright 2005 Oliver Stieber
 * Copyright 2006-2008 Henri Verbeet
 * Copyright 2007-2008 Stefan Dösinger for CodeWeavers
 * Copyright 2009-2010 Henri Verbeet for CodeWeavers
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "config.h"
#include "wine/port.h"

#include "wined3d_private.h"

WINE_DEFAULT_DEBUG_CHANNEL(d3d);

struct wined3d_format_channels
{
    enum wined3d_format_id id;
    DWORD red_size, green_size, blue_size, alpha_size;
    DWORD red_offset, green_offset, blue_offset, alpha_offset;
    UINT bpp;
    BYTE depth_size, stencil_size;
};

static const struct wined3d_format_channels formats[] =
{
    /*                                          size            offset
     *  format id                           r   g   b   a    r   g   b   a    bpp depth stencil */
    {WINED3DFMT_UNKNOWN,                    0,  0,  0,  0,   0,  0,  0,  0,    0,   0,     0},
    /* FourCC formats */
    {WINED3DFMT_UYVY,                       0,  0,  0,  0,   0,  0,  0,  0,    2,   0,     0},
    {WINED3DFMT_YUY2,                       0,  0,  0,  0,   0,  0,  0,  0,    2,   0,     0},
    {WINED3DFMT_YV12,                       0,  0,  0,  0,   0,  0,  0,  0,    1,   0,     0},
    {WINED3DFMT_DXT1,                       0,  0,  0,  0,   0,  0,  0,  0,    1,   0,     0},
    {WINED3DFMT_DXT2,                       0,  0,  0,  0,   0,  0,  0,  0,    1,   0,     0},
    {WINED3DFMT_DXT3,                       0,  0,  0,  0,   0,  0,  0,  0,    1,   0,     0},
    {WINED3DFMT_DXT4,                       0,  0,  0,  0,   0,  0,  0,  0,    1,   0,     0},
    {WINED3DFMT_DXT5,                       0,  0,  0,  0,   0,  0,  0,  0,    1,   0,     0},
    {WINED3DFMT_MULTI2_ARGB8,               0,  0,  0,  0,   0,  0,  0,  0,    1,   0,     0},
    {WINED3DFMT_G8R8_G8B8,                  0,  0,  0,  0,   0,  0,  0,  0,    1,   0,     0},
    {WINED3DFMT_R8G8_B8G8,                  0,  0,  0,  0,   0,  0,  0,  0,    1,   0,     0},
    /* IEEE formats */
    {WINED3DFMT_R32_FLOAT,                 32,  0,  0,  0,   0,  0,  0,  0,    4,   0,     0},
    {WINED3DFMT_R32G32_FLOAT,              32, 32,  0,  0,   0, 32,  0,  0,    8,   0,     0},
    {WINED3DFMT_R32G32B32_FLOAT,           32, 32, 32,  0,   0, 32, 64,  0,   12,   0,     0},
    {WINED3DFMT_R32G32B32A32_FLOAT,        32, 32, 32, 32,   0, 32, 64, 96,   16,   0,     0},
    /* Hmm? */
    {WINED3DFMT_R8G8_SNORM_Cx,              0,  0,  0,  0,   0,  0,  0,  0,    2,   0,     0},
    /* Float */
    {WINED3DFMT_R16_FLOAT,                 16,  0,  0,  0,   0,  0,  0,  0,    2,   0,     0},
    {WINED3DFMT_R16G16_FLOAT,              16, 16,  0,  0,   0, 16,  0,  0,    4,   0,     0},
    {WINED3DFMT_R16G16_SINT,               16, 16,  0,  0,   0, 16,  0,  0,    4,   0,     0},
    {WINED3DFMT_R16G16B16A16_FLOAT,        16, 16, 16, 16,   0, 16, 32, 48,    8,   0,     0},
    {WINED3DFMT_R16G16B16A16_SINT,         16, 16, 16, 16,   0, 16, 32, 48,    8,   0,     0},
    /* Palettized formats */
    {WINED3DFMT_P8_UINT_A8_UNORM,           0,  0,  0,  8,   0,  0,  0,  8,    2,   0,     0},
    {WINED3DFMT_P8_UINT,                    0,  0,  0,  0,   0,  0,  0,  0,    1,   0,     0},
    /* Standard ARGB formats. */
    {WINED3DFMT_B8G8R8_UNORM,               8,  8,  8,  0,  16,  8,  0,  0,    3,   0,     0},
    {WINED3DFMT_B8G8R8A8_UNORM,             8,  8,  8,  8,  16,  8,  0, 24,    4,   0,     0},
    {WINED3DFMT_B8G8R8X8_UNORM,             8,  8,  8,  0,  16,  8,  0,  0,    4,   0,     0},
    {WINED3DFMT_B5G6R5_UNORM,               5,  6,  5,  0,  11,  5,  0,  0,    2,   0,     0},
    {WINED3DFMT_B5G5R5X1_UNORM,             5,  5,  5,  0,  10,  5,  0,  0,    2,   0,     0},
    {WINED3DFMT_B5G5R5A1_UNORM,             5,  5,  5,  1,  10,  5,  0, 15,    2,   0,     0},
    {WINED3DFMT_B4G4R4A4_UNORM,             4,  4,  4,  4,   8,  4,  0, 12,    2,   0,     0},
    {WINED3DFMT_B2G3R3_UNORM,               3,  3,  2,  0,   5,  2,  0,  0,    1,   0,     0},
    {WINED3DFMT_A8_UNORM,                   0,  0,  0,  8,   0,  0,  0,  0,    1,   0,     0},
    {WINED3DFMT_B2G3R3A8_UNORM,             3,  3,  2,  8,   5,  2,  0,  8,    2,   0,     0},
    {WINED3DFMT_B4G4R4X4_UNORM,             4,  4,  4,  0,   8,  4,  0,  0,    2,   0,     0},
    {WINED3DFMT_R10G10B10A2_UNORM,         10, 10, 10,  2,   0, 10, 20, 30,    4,   0,     0},
    {WINED3DFMT_R10G10B10A2_UINT,          10, 10, 10,  2,   0, 10, 20, 30,    4,   0,     0},
    {WINED3DFMT_R10G10B10A2_SNORM,         10, 10, 10,  2,   0, 10, 20, 30,    4,   0,     0},
    {WINED3DFMT_R8G8B8A8_UNORM,             8,  8,  8,  8,   0,  8, 16, 24,    4,   0,     0},
    {WINED3DFMT_R8G8B8A8_UINT,              8,  8,  8,  8,   0,  8, 16, 24,    4,   0,     0},
    {WINED3DFMT_R8G8B8X8_UNORM,             8,  8,  8,  0,   0,  8, 16,  0,    4,   0,     0},
    {WINED3DFMT_R16G16_UNORM,              16, 16,  0,  0,   0, 16,  0,  0,    4,   0,     0},
    {WINED3DFMT_B10G10R10A2_UNORM,         10, 10, 10,  2,  20, 10,  0, 30,    4,   0,     0},
    {WINED3DFMT_R16G16B16A16_UNORM,        16, 16, 16, 16,   0, 16, 32, 48,    8,   0,     0},
    /* Luminance */
    {WINED3DFMT_L8_UNORM,                   0,  0,  0,  0,   0,  0,  0,  0,    1,   0,     0},
    {WINED3DFMT_L8A8_UNORM,                 0,  0,  0,  8,   0,  0,  0,  8,    2,   0,     0},
    {WINED3DFMT_L4A4_UNORM,                 0,  0,  0,  4,   0,  0,  0,  4,    1,   0,     0},
    {WINED3DFMT_L16_UNORM,                  0,  0,  0,  0,   0,  0,  0,  0,    2,   0,     0},
    /* Bump mapping stuff */
    {WINED3DFMT_R8G8_SNORM,                 8,  8,  0,  0,   0,  8,  0,  0,    2,   0,     0},
    {WINED3DFMT_R5G5_SNORM_L6_UNORM,        5,  5,  0,  0,   0,  5,  0,  0,    2,   0,     0},
    {WINED3DFMT_R8G8_SNORM_L8X8_UNORM,      8,  8,  0,  0,   0,  8,  0,  0,    4,   0,     0},
    {WINED3DFMT_R8G8B8A8_SNORM,             8,  8,  8,  8,   0,  8, 16, 24,    4,   0,     0},
    {WINED3DFMT_R16G16_SNORM,              16, 16,  0,  0,   0, 16,  0,  0,    4,   0,     0},
    {WINED3DFMT_R10G11B11_SNORM,           10, 11, 11,  0,   0, 10, 21,  0,    4,   0,     0},
    {WINED3DFMT_R10G10B10_SNORM_A2_UNORM,  10, 10, 10,  2,   0, 10, 20, 30,    4,   0,     0},
    /* Depth stencil formats */
    {WINED3DFMT_D16_LOCKABLE,               0,  0,  0,  0,   0,  0,  0,  0,    2,  16,     0},
    {WINED3DFMT_D32_UNORM,                  0,  0,  0,  0,   0,  0,  0,  0,    4,  32,     0},
    {WINED3DFMT_S1_UINT_D15_UNORM,          0,  0,  0,  0,   0,  0,  0,  0,    2,  15,     1},
    {WINED3DFMT_D24_UNORM_S8_UINT,          0,  0,  0,  0,   0,  0,  0,  0,    4,  24,     8},
    {WINED3DFMT_X8D24_UNORM,                0,  0,  0,  0,   0,  0,  0,  0,    4,  24,     0},
    {WINED3DFMT_S4X4_UINT_D24_UNORM,        0,  0,  0,  0,   0,  0,  0,  0,    4,  24,     4},
    {WINED3DFMT_D16_UNORM,                  0,  0,  0,  0,   0,  0,  0,  0,    2,  16,     0},
    {WINED3DFMT_D32_FLOAT,                  0,  0,  0,  0,   0,  0,  0,  0,    4,  32,     0},
    {WINED3DFMT_S8_UINT_D24_FLOAT,          0,  0,  0,  0,   0,  0,  0,  0,    4,  24,     8},
    {WINED3DFMT_VERTEXDATA,                 0,  0,  0,  0,   0,  0,  0,  0,    0,   0,     0},
    {WINED3DFMT_R16_UINT,                  16,  0,  0,  0,   0,  0,  0,  0,    2,   0,     0},
    {WINED3DFMT_R32_UINT,                  32,  0,  0,  0,   0,  0,  0,  0,    4,   0,     0},
    {WINED3DFMT_R32G32_UINT,               32, 32,  0,  0,   0, 32,  0,  0,    8,   0,     0},
    {WINED3DFMT_R32G32B32_UINT,            32, 32, 32,  0,   0, 32, 64,  0,   12,   0,     0},
    {WINED3DFMT_R32G32B32A32_UINT,         32, 32, 32, 32,   0, 32, 64, 96,   16,   0,     0},
    {WINED3DFMT_R16G16B16A16_SNORM,        16, 16, 16, 16,   0, 16, 32, 48,    8,   0,     0},
    /* Vendor-specific formats */
    {WINED3DFMT_ATI2N,                      0,  0,  0,  0,   0,  0,  0,  0,    1,   0,     0},
    {WINED3DFMT_NVDB,                       0,  0,  0,  0,   0,  0,  0,  0,    0,   0,     0},
    {WINED3DFMT_INST,                       0,  0,  0,  0,   0,  0,  0,  0,    0,   0,     0},
    {WINED3DFMT_INTZ,                       0,  0,  0,  0,   0,  0,  0,  0,    4,  24,     8},
    {WINED3DFMT_RESZ,                       0,  0,  0,  0,   0,  0,  0,  0,    0,   0,     0},
    {WINED3DFMT_NVHU,                       0,  0,  0,  0,   0,  0,  0,  0,    2,   0,     0},
    {WINED3DFMT_NVHS,                       0,  0,  0,  0,   0,  0,  0,  0,    2,   0,     0},
    {WINED3DFMT_NULL,                       8,  8,  8,  8,   0,  8, 16, 24,    4,   0,     0},
    /* Unsure about them, could not find a Windows driver that supports them */
    {WINED3DFMT_R16,                       16,  0,  0,  0,   0,  0,  0,  0,    2,   0,     0},
    {WINED3DFMT_AL16,                       0,  0,  0, 16,   0,  0,  0, 16,    4,   0,     0},
    /* Typeless */
    {WINED3DFMT_R8_TYPELESS,                8,  0,  0,  0,   0,  0,  0,  0,    1,   0,     0},
    {WINED3DFMT_R8G8_TYPELESS,              8,  8,  0,  0,   0,  8,  0,  0,    2,   0,     0},
    {WINED3DFMT_R8G8B8A8_TYPELESS,          8,  8,  8,  8,   0,  8, 16, 24,    4,   0,     0},
    {WINED3DFMT_R16_TYPELESS,              16,  0,  0,  0,   0,  0,  0,  0,    2,   0,     0},
    {WINED3DFMT_R16G16_TYPELESS,           16, 16,  0,  0,   0, 16,  0,  0,    4,   0,     0},
    {WINED3DFMT_R16G16B16A16_TYPELESS,     16, 16, 16, 16,   0, 16, 32, 48,    8,   0,     0},
    {WINED3DFMT_R32_TYPELESS,              32,  0,  0,  0,   0,  0,  0,  0,    4,   0,     0},
    {WINED3DFMT_R32G32_TYPELESS,           32, 32,  0,  0,   0, 32,  0,  0,    8,   0,     0},
    {WINED3DFMT_R32G32B32_TYPELESS,        32, 32, 32,  0,   0, 32, 64,  0,   12,   0,     0},
    {WINED3DFMT_R32G32B32A32_TYPELESS,     32, 32, 32, 32,   0, 32, 64, 96,   16,   0,     0},
};

struct wined3d_format_base_flags
{
    enum wined3d_format_id id;
    DWORD flags;
};

/* The ATI2N format behaves like an uncompressed format in LockRect(), but
 * still needs to use the correct block based calculation for e.g. the
 * resource size. */
static const struct wined3d_format_base_flags format_base_flags[] =
{
    {WINED3DFMT_P8_UINT,            WINED3DFMT_FLAG_GETDC},
    {WINED3DFMT_B8G8R8_UNORM,       WINED3DFMT_FLAG_GETDC},
    {WINED3DFMT_B8G8R8A8_UNORM,     WINED3DFMT_FLAG_GETDC},
    {WINED3DFMT_B8G8R8X8_UNORM,     WINED3DFMT_FLAG_GETDC},
    {WINED3DFMT_B5G6R5_UNORM,       WINED3DFMT_FLAG_GETDC},
    {WINED3DFMT_B5G5R5X1_UNORM,     WINED3DFMT_FLAG_GETDC},
    {WINED3DFMT_B5G5R5A1_UNORM,     WINED3DFMT_FLAG_GETDC},
    {WINED3DFMT_B4G4R4A4_UNORM,     WINED3DFMT_FLAG_GETDC},
    {WINED3DFMT_B4G4R4X4_UNORM,     WINED3DFMT_FLAG_GETDC},
    {WINED3DFMT_R8G8B8A8_UNORM,     WINED3DFMT_FLAG_GETDC},
    {WINED3DFMT_R8G8B8X8_UNORM,     WINED3DFMT_FLAG_GETDC},
    {WINED3DFMT_ATI2N,              WINED3DFMT_FLAG_BROKEN_PITCH},
    {WINED3DFMT_R32_FLOAT,          WINED3DFMT_FLAG_FLOAT},
    {WINED3DFMT_R32G32_FLOAT,       WINED3DFMT_FLAG_FLOAT},
    {WINED3DFMT_R32G32B32_FLOAT,    WINED3DFMT_FLAG_FLOAT},
    {WINED3DFMT_R32G32B32A32_FLOAT, WINED3DFMT_FLAG_FLOAT},
    {WINED3DFMT_R16_FLOAT,          WINED3DFMT_FLAG_FLOAT},
    {WINED3DFMT_R16G16_FLOAT,       WINED3DFMT_FLAG_FLOAT},
    {WINED3DFMT_R16G16B16A16_FLOAT, WINED3DFMT_FLAG_FLOAT},
    {WINED3DFMT_D32_FLOAT,          WINED3DFMT_FLAG_FLOAT},
    {WINED3DFMT_S8_UINT_D24_FLOAT,  WINED3DFMT_FLAG_FLOAT},
};

struct wined3d_format_block_info
{
    enum wined3d_format_id id;
    UINT block_width;
    UINT block_height;
    UINT block_byte_count;
};

static const struct wined3d_format_block_info format_block_info[] =
{
    {WINED3DFMT_DXT1,   4,  4,  8},
    {WINED3DFMT_DXT2,   4,  4,  16},
    {WINED3DFMT_DXT3,   4,  4,  16},
    {WINED3DFMT_DXT4,   4,  4,  16},
    {WINED3DFMT_DXT5,   4,  4,  16},
    {WINED3DFMT_ATI2N,  4,  4,  16},
    {WINED3DFMT_YUY2,   2,  1,  4},
    {WINED3DFMT_UYVY,   2,  1,  4},
};

struct wined3d_format_vertex_info
{
    enum wined3d_format_id id;
    enum wined3d_ffp_emit_idx emit_idx;
    GLint component_count;
    GLenum gl_vtx_type;
    GLint gl_vtx_format;
    GLboolean gl_normalized;
    unsigned int component_size;
};

static const struct wined3d_format_vertex_info format_vertex_info[] =
{
    {WINED3DFMT_R32_FLOAT,          WINED3D_FFP_EMIT_FLOAT1,    1, GL_FLOAT,          1, GL_FALSE, sizeof(float)},
    {WINED3DFMT_R32G32_FLOAT,       WINED3D_FFP_EMIT_FLOAT2,    2, GL_FLOAT,          2, GL_FALSE, sizeof(float)},
    {WINED3DFMT_R32G32B32_FLOAT,    WINED3D_FFP_EMIT_FLOAT3,    3, GL_FLOAT,          3, GL_FALSE, sizeof(float)},
    {WINED3DFMT_R32G32B32A32_FLOAT, WINED3D_FFP_EMIT_FLOAT4,    4, GL_FLOAT,          4, GL_FALSE, sizeof(float)},
    {WINED3DFMT_B8G8R8A8_UNORM,     WINED3D_FFP_EMIT_D3DCOLOR,  4, GL_UNSIGNED_BYTE,  4, GL_TRUE,  sizeof(BYTE)},
    {WINED3DFMT_R8G8B8A8_UINT,      WINED3D_FFP_EMIT_UBYTE4,    4, GL_UNSIGNED_BYTE,  4, GL_FALSE, sizeof(BYTE)},
    {WINED3DFMT_R16G16_SINT,        WINED3D_FFP_EMIT_SHORT2,    2, GL_SHORT,          2, GL_FALSE, sizeof(short int)},
    {WINED3DFMT_R16G16B16A16_SINT,  WINED3D_FFP_EMIT_SHORT4,    4, GL_SHORT,          4, GL_FALSE, sizeof(short int)},
    {WINED3DFMT_R8G8B8A8_UNORM,     WINED3D_FFP_EMIT_UBYTE4N,   4, GL_UNSIGNED_BYTE,  4, GL_TRUE,  sizeof(BYTE)},
    {WINED3DFMT_R16G16_SNORM,       WINED3D_FFP_EMIT_SHORT2N,   2, GL_SHORT,          2, GL_TRUE,  sizeof(short int)},
    {WINED3DFMT_R16G16B16A16_SNORM, WINED3D_FFP_EMIT_SHORT4N,   4, GL_SHORT,          4, GL_TRUE,  sizeof(short int)},
    {WINED3DFMT_R16G16_UNORM,       WINED3D_FFP_EMIT_USHORT2N,  2, GL_UNSIGNED_SHORT, 2, GL_TRUE,  sizeof(short int)},
    {WINED3DFMT_R16G16B16A16_UNORM, WINED3D_FFP_EMIT_USHORT4N,  4, GL_UNSIGNED_SHORT, 4, GL_TRUE,  sizeof(short int)},
    {WINED3DFMT_R10G10B10A2_UINT,   WINED3D_FFP_EMIT_UDEC3,     3, GL_UNSIGNED_SHORT, 3, GL_FALSE, sizeof(short int)},
    {WINED3DFMT_R10G10B10A2_SNORM,  WINED3D_FFP_EMIT_DEC3N,     3, GL_SHORT,          3, GL_TRUE,  sizeof(short int)},
    {WINED3DFMT_R16G16_FLOAT,       WINED3D_FFP_EMIT_FLOAT16_2, 2, GL_FLOAT,          2, GL_FALSE, sizeof(GLhalfNV)},
    {WINED3DFMT_R16G16B16A16_FLOAT, WINED3D_FFP_EMIT_FLOAT16_4, 4, GL_FLOAT,          4, GL_FALSE, sizeof(GLhalfNV)},
    {WINED3DFMT_R32_UINT,           WINED3D_FFP_EMIT_INVALID,   1, GL_UNSIGNED_INT,   1, GL_FALSE, sizeof(UINT)},
    {WINED3DFMT_R32G32_UINT,        WINED3D_FFP_EMIT_INVALID,   2, GL_UNSIGNED_INT,   2, GL_FALSE, sizeof(UINT)},
    {WINED3DFMT_R32G32B32_UINT,     WINED3D_FFP_EMIT_INVALID,   3, GL_UNSIGNED_INT,   3, GL_FALSE, sizeof(UINT)},
    {WINED3DFMT_R32G32B32A32_UINT,  WINED3D_FFP_EMIT_INVALID,   4, GL_UNSIGNED_INT,   4, GL_FALSE, sizeof(UINT)},
};

struct wined3d_format_texture_info
{
    enum wined3d_format_id id;
    GLint gl_internal;
    GLint gl_srgb_internal;
    GLint gl_rt_internal;
    GLint gl_format;
    GLint gl_type;
    unsigned int conv_byte_count;
    unsigned int flags;
    enum wined3d_gl_extension extension;
    void (*convert)(const BYTE *src, BYTE *dst, UINT pitch, UINT width, UINT height);
};

static void convert_l4a4_unorm(const BYTE *src, BYTE *dst, UINT pitch, UINT width, UINT height)
{
    /* WINED3DFMT_L4A4_UNORM exists as an internal gl format, but for some reason there is not
     * format+type combination to load it. Thus convert it to A8L8, then load it
     * with A4L4 internal, but A8L8 format+type
     */
    unsigned int x, y;
    const unsigned char *Source;
    unsigned char *Dest;
    UINT outpitch = pitch * 2;

    for(y = 0; y < height; y++) {
        Source = src + y * pitch;
        Dest = dst + y * outpitch;
        for (x = 0; x < width; x++ ) {
            unsigned char color = (*Source++);
            /* A */ Dest[1] = (color & 0xf0) << 0;
            /* L */ Dest[0] = (color & 0x0f) << 4;
            Dest += 2;
        }
    }
}

static void convert_r5g5_snorm_l6_unorm(const BYTE *src, BYTE *dst, UINT pitch, UINT width, UINT height)
{
    unsigned int x, y;
    const WORD *Source;

    for(y = 0; y < height; y++)
    {
        unsigned short *Dest_s = (unsigned short *) (dst + y * pitch);
        Source = (const WORD *)(src + y * pitch);
        for (x = 0; x < width; x++ )
        {
            short color = (*Source++);
            unsigned char l = ((color >> 10) & 0xfc);
                    short v = ((color >>  5) & 0x3e);
                    short u = ((color      ) & 0x1f);
            short v_conv = v + 16;
            short u_conv = u + 16;

            *Dest_s = ((v_conv << 11) & 0xf800) | ((l << 5) & 0x7e0) | (u_conv & 0x1f);
            Dest_s += 1;
        }
    }
}

static void convert_r5g5_snorm_l6_unorm_nv(const BYTE *src, BYTE *dst, UINT pitch, UINT width, UINT height)
{
    unsigned int x, y;
    const WORD *Source;
    unsigned char *Dest;
    UINT outpitch = (pitch * 3)/2;

    /* This makes the gl surface bigger(24 bit instead of 16), but it works with
     * fixed function and shaders without further conversion once the surface is
     * loaded
     */
    for(y = 0; y < height; y++) {
        Source = (const WORD *)(src + y * pitch);
        Dest = dst + y * outpitch;
        for (x = 0; x < width; x++ ) {
            short color = (*Source++);
            unsigned char l = ((color >> 10) & 0xfc);
                     char v = ((color >>  5) & 0x3e);
                     char u = ((color      ) & 0x1f);

            /* 8 bits destination, 6 bits source, 8th bit is the sign. gl ignores the sign
             * and doubles the positive range. Thus shift left only once, gl does the 2nd
             * shift. GL reads a signed value and converts it into an unsigned value.
             */
            /* M */ Dest[2] = l << 1;

            /* Those are read as signed, but kept signed. Just left-shift 3 times to scale
             * from 5 bit values to 8 bit values.
             */
            /* V */ Dest[1] = v << 3;
            /* U */ Dest[0] = u << 3;
            Dest += 3;
        }
    }
}

static void convert_r8g8_snorm(const BYTE *src, BYTE *dst, UINT pitch, UINT width, UINT height)
{
    unsigned int x, y;
    const short *Source;
    unsigned char *Dest;
    UINT outpitch = (pitch * 3)/2;

    for(y = 0; y < height; y++)
    {
        Source = (const short *)(src + y * pitch);
        Dest = dst + y * outpitch;
        for (x = 0; x < width; x++ )
        {
            const short color = (*Source++);
            /* B */ Dest[0] = 0xff;
            /* G */ Dest[1] = (color >> 8) + 128; /* V */
            /* R */ Dest[2] = (color & 0xff) + 128;      /* U */
            Dest += 3;
        }
    }
}

static void convert_r8g8_snorm_l8x8_unorm(const BYTE *src, BYTE *dst, UINT pitch, UINT width, UINT height)
{
    unsigned int x, y;
    const DWORD *Source;
    unsigned char *Dest;

    /* Doesn't work correctly with the fixed function pipeline, but can work in
     * shaders if the shader is adjusted. (There's no use for this format in gl's
     * standard fixed function pipeline anyway).
     */
    for(y = 0; y < height; y++)
    {
        Source = (const DWORD *)(src + y * pitch);
        Dest = dst + y * pitch;
        for (x = 0; x < width; x++ )
        {
            LONG color = (*Source++);
            /* B */ Dest[0] = ((color >> 16) & 0xff);       /* L */
            /* G */ Dest[1] = ((color >> 8 ) & 0xff) + 128; /* V */
            /* R */ Dest[2] = (color         & 0xff) + 128; /* U */
            Dest += 4;
        }
    }
}

static void convert_r8g8_snorm_l8x8_unorm_nv(const BYTE *src, BYTE *dst, UINT pitch, UINT width, UINT height)
{
    unsigned int x, y;
    const DWORD *Source;
    unsigned char *Dest;

    /* This implementation works with the fixed function pipeline and shaders
     * without further modification after converting the surface.
     */
    for(y = 0; y < height; y++)
    {
        Source = (const DWORD *)(src + y * pitch);
        Dest = dst + y * pitch;
        for (x = 0; x < width; x++ )
        {
            LONG color = (*Source++);
            /* L */ Dest[2] = ((color >> 16) & 0xff);   /* L */
            /* V */ Dest[1] = ((color >> 8 ) & 0xff);   /* V */
            /* U */ Dest[0] = (color         & 0xff);   /* U */
            /* I */ Dest[3] = 255;                      /* X */
            Dest += 4;
        }
    }
}

static void convert_r8g8b8a8_snorm(const BYTE *src, BYTE *dst, UINT pitch, UINT width, UINT height)
{
    unsigned int x, y;
    const DWORD *Source;
    unsigned char *Dest;

    for(y = 0; y < height; y++)
    {
        Source = (const DWORD *)(src + y * pitch);
        Dest = dst + y * pitch;
        for (x = 0; x < width; x++ )
        {
            LONG color = (*Source++);
            /* B */ Dest[0] = ((color >> 16) & 0xff) + 128; /* W */
            /* G */ Dest[1] = ((color >> 8 ) & 0xff) + 128; /* V */
            /* R */ Dest[2] = (color         & 0xff) + 128; /* U */
            /* A */ Dest[3] = ((color >> 24) & 0xff) + 128; /* Q */
            Dest += 4;
        }
    }
}

static void convert_r16g16_snorm(const BYTE *src, BYTE *dst, UINT pitch, UINT width, UINT height)
{
    unsigned int x, y;
    const DWORD *Source;
    unsigned short *Dest;
    UINT outpitch = (pitch * 3)/2;

    for(y = 0; y < height; y++)
    {
        Source = (const DWORD *)(src + y * pitch);
        Dest = (unsigned short *) (dst + y * outpitch);
        for (x = 0; x < width; x++ )
        {
            const DWORD color = (*Source++);
            /* B */ Dest[0] = 0xffff;
            /* G */ Dest[1] = (color >> 16) + 32768; /* V */
            /* R */ Dest[2] = (color & 0xffff) + 32768; /* U */
            Dest += 3;
        }
    }
}

static void convert_r16g16(const BYTE *src, BYTE *dst, UINT pitch, UINT width, UINT height)
{
    unsigned int x, y;
    const WORD *Source;
    WORD *Dest;
    UINT outpitch = (pitch * 3)/2;

    for(y = 0; y < height; y++)
    {
        Source = (const WORD *)(src + y * pitch);
        Dest = (WORD *) (dst + y * outpitch);
        for (x = 0; x < width; x++ )
        {
            WORD green = (*Source++);
            WORD red = (*Source++);
            Dest[0] = green;
            Dest[1] = red;
            /* Strictly speaking not correct for R16G16F, but it doesn't matter because the
             * shader overwrites it anyway
             */
            Dest[2] = 0xffff;
            Dest += 3;
        }
    }
}

static void convert_r32g32_float(const BYTE *src, BYTE *dst, UINT pitch, UINT width, UINT height)
{
    unsigned int x, y;
    const float *Source;
    float *Dest;
    UINT outpitch = (pitch * 3)/2;

    for(y = 0; y < height; y++)
    {
        Source = (const float *)(src + y * pitch);
        Dest = (float *) (dst + y * outpitch);
        for (x = 0; x < width; x++ )
        {
            float green = (*Source++);
            float red = (*Source++);
            Dest[0] = green;
            Dest[1] = red;
            Dest[2] = 1.0f;
            Dest += 3;
        }
    }
}

static void convert_s1_uint_d15_unorm(const BYTE *src, BYTE *dst, UINT pitch, UINT width, UINT height)
{
    unsigned int x, y;
    UINT outpitch = pitch * 2;

    for (y = 0; y < height; ++y)
    {
        const WORD *source = (const WORD *)(src + y * pitch);
        DWORD *dest = (DWORD *)(dst + y * outpitch);

        for (x = 0; x < width; ++x)
        {
            /* The depth data is normalized, so needs to be scaled,
             * the stencil data isn't.  Scale depth data by
             *      (2^24-1)/(2^15-1) ~~ (2^9 + 2^-6). */
            WORD d15 = source[x] >> 1;
            DWORD d24 = (d15 << 9) + (d15 >> 6);
            dest[x] = (d24 << 8) | (source[x] & 0x1);
        }
    }
}

static void convert_s4x4_uint_d24_unorm(const BYTE *src, BYTE *dst, UINT pitch, UINT width, UINT height)
{
    unsigned int x, y;

    for (y = 0; y < height; ++y)
    {
        const DWORD *source = (const DWORD *)(src + y * pitch);
        DWORD *dest = (DWORD *)(dst + y * pitch);

        for (x = 0; x < width; ++x)
        {
            /* Just need to clear out the X4 part. */
            dest[x] = source[x] & ~0xf0;
        }
    }
}

static void convert_s8_uint_d24_float(const BYTE *src, BYTE *dst, UINT pitch, UINT width, UINT height)
{
    unsigned int x, y;
    UINT outpitch = pitch * 2;

    for (y = 0; y < height; ++y)
    {
        const DWORD *source = (const DWORD *)(src + y * pitch);
        float *dest_f = (float *)(dst + y * outpitch);
        DWORD *dest_s = (DWORD *)(dst + y * outpitch);

        for (x = 0; x < width; ++x)
        {
            dest_f[x * 2] = float_24_to_32((source[x] & 0xffffff00) >> 8);
            dest_s[x * 2 + 1] = source[x] & 0xff;
        }
    }
}

/* The following formats explicitly don't have WINED3DFMT_FLAG_TEXTURE set:
 *
 * These are never supported on native.
 *     WINED3DFMT_B8G8R8_UNORM
 *     WINED3DFMT_B2G3R3_UNORM
 *     WINED3DFMT_L4A4_UNORM
 *     WINED3DFMT_S1_UINT_D15_UNORM
 *     WINED3DFMT_S4X4_UINT_D24_UNORM
 *
 * Only some Geforce/Voodoo3/G400 cards offer 8-bit textures in case of ddraw.
 * Since it is not widely available, don't offer it. Further no Windows driver
 * offers WINED3DFMT_P8_UINT_A8_NORM, so don't offer it either.
 *     WINED3DFMT_P8_UINT
 *     WINED3DFMT_P8_UINT_A8_UNORM
 *
 * These formats seem to be similar to the HILO formats in
 * GL_NV_texture_shader. NVHU is said to be GL_UNSIGNED_HILO16,
 * NVHS GL_SIGNED_HILO16. Rumours say that D3D computes a 3rd channel
 * similarly to D3DFMT_CxV8U8 (So NVHS could be called D3DFMT_CxV16U16). ATI
 * refused to support formats which can easily be emulated with pixel shaders,
 * so applications have to deal with not having NVHS and NVHU.
 *     WINED3DFMT_NVHU
 *     WINED3DFMT_NVHS */
static const struct wined3d_format_texture_info format_texture_info[] =
{
    /* format id                        gl_internal                       gl_srgb_internal                      gl_rt_internal
            gl_format                   gl_type                           conv_byte_count
            flags
            extension                   convert */
    /* FourCC formats */
    /* GL_APPLE_ycbcr_422 claims that its '2YUV' format, which is supported via the UNSIGNED_SHORT_8_8_REV_APPLE type
     * is equivalent to 'UYVY' format on Windows, and the 'YUVS' via UNSIGNED_SHORT_8_8_APPLE equates to 'YUY2'. The
     * d3d9 test however shows that the opposite is true. Since the extension is from 2002, it predates the x86 based
     * Macs, so probably the endianness differs. This could be tested as soon as we have a Windows and MacOS on a big
     * endian machine
     */
    {WINED3DFMT_UYVY,                   GL_LUMINANCE_ALPHA,               GL_LUMINANCE_ALPHA,                     0,
            GL_LUMINANCE_ALPHA,         GL_UNSIGNED_BYTE,                 0,
            WINED3DFMT_FLAG_FILTERING,
            WINED3D_GL_EXT_NONE,        NULL},
    {WINED3DFMT_UYVY,                   GL_RGB,                           GL_RGB,                                 0,
            GL_YCBCR_422_APPLE,         GL_UNSIGNED_SHORT_8_8_APPLE,      0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_FILTERING,
            APPLE_YCBCR_422,            NULL},
    {WINED3DFMT_YUY2,                   GL_LUMINANCE_ALPHA,               GL_LUMINANCE_ALPHA,                     0,
            GL_LUMINANCE_ALPHA,         GL_UNSIGNED_BYTE,                 0,
            WINED3DFMT_FLAG_FILTERING,
            WINED3D_GL_EXT_NONE,        NULL},
    {WINED3DFMT_YUY2,                   GL_RGB,                           GL_RGB,                                 0,
            GL_YCBCR_422_APPLE,         GL_UNSIGNED_SHORT_8_8_REV_APPLE,  0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_FILTERING,
            APPLE_YCBCR_422,            NULL},
    {WINED3DFMT_YV12,                   GL_ALPHA,                         GL_ALPHA,                               0,
            GL_ALPHA,                   GL_UNSIGNED_BYTE,                 0,
            WINED3DFMT_FLAG_FILTERING,
            WINED3D_GL_EXT_NONE,        NULL},
    {WINED3DFMT_DXT1,                   GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, 0,
            GL_RGBA,                    GL_UNSIGNED_BYTE,                 0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_SRGB_READ | WINED3DFMT_FLAG_COMPRESSED,
            EXT_TEXTURE_COMPRESSION_S3TC, NULL},
    {WINED3DFMT_DXT2,                   GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, 0,
            GL_RGBA,                    GL_UNSIGNED_BYTE,                 0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_SRGB_READ | WINED3DFMT_FLAG_COMPRESSED,
            EXT_TEXTURE_COMPRESSION_S3TC, NULL},
    {WINED3DFMT_DXT3,                   GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, 0,
            GL_RGBA,                    GL_UNSIGNED_BYTE,                 0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_SRGB_READ | WINED3DFMT_FLAG_COMPRESSED,
            EXT_TEXTURE_COMPRESSION_S3TC, NULL},
    {WINED3DFMT_DXT4,                   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, 0,
            GL_RGBA,                    GL_UNSIGNED_BYTE,                 0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_SRGB_READ | WINED3DFMT_FLAG_COMPRESSED,
            EXT_TEXTURE_COMPRESSION_S3TC, NULL},
    {WINED3DFMT_DXT5,                   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, 0,
            GL_RGBA,                    GL_UNSIGNED_BYTE,                 0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_SRGB_READ | WINED3DFMT_FLAG_COMPRESSED,
            EXT_TEXTURE_COMPRESSION_S3TC, NULL},
    /* IEEE formats */
    {WINED3DFMT_R32_FLOAT,              GL_RGB32F_ARB,                    GL_RGB32F_ARB,                          0,
            GL_RED,                     GL_FLOAT,                         0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_RENDERTARGET | WINED3DFMT_FLAG_VTF,
            ARB_TEXTURE_FLOAT,          NULL},
    {WINED3DFMT_R32_FLOAT,              GL_R32F,                          GL_R32F,                                0,
            GL_RED,                     GL_FLOAT,                         0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_RENDERTARGET | WINED3DFMT_FLAG_VTF,
            ARB_TEXTURE_RG,             NULL},
    {WINED3DFMT_R32G32_FLOAT,           GL_RGB32F_ARB,                    GL_RGB32F_ARB,                          0,
            GL_RGB,                     GL_FLOAT,                         12,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_RENDERTARGET | WINED3DFMT_FLAG_VTF,
            ARB_TEXTURE_FLOAT,          convert_r32g32_float},
    {WINED3DFMT_R32G32_FLOAT,           GL_RG32F,                         GL_RG32F,                               0,
            GL_RG,                      GL_FLOAT,                         0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_RENDERTARGET | WINED3DFMT_FLAG_VTF,
            ARB_TEXTURE_RG,             NULL},
    {WINED3DFMT_R32G32B32A32_FLOAT,     GL_RGBA32F_ARB,                   GL_RGBA32F_ARB,                         0,
            GL_RGBA,                    GL_FLOAT,                         0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_RENDERTARGET | WINED3DFMT_FLAG_VTF,
            ARB_TEXTURE_FLOAT,          NULL},
    /* Float */
    {WINED3DFMT_R16_FLOAT,              GL_RGB16F_ARB,                    GL_RGB16F_ARB,                          0,
            GL_RED,                     GL_HALF_FLOAT_ARB,                0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_RENDERTARGET | WINED3DFMT_FLAG_VTF,
            ARB_TEXTURE_FLOAT,          NULL},
    {WINED3DFMT_R16_FLOAT,              GL_R16F,                          GL_R16F,                                0,
            GL_RED,                     GL_HALF_FLOAT_ARB,                0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_RENDERTARGET | WINED3DFMT_FLAG_VTF,
            ARB_TEXTURE_RG,             NULL},
    {WINED3DFMT_R16G16_FLOAT,           GL_RGB16F_ARB,                    GL_RGB16F_ARB,                          0,
            GL_RGB,                     GL_HALF_FLOAT_ARB,                6,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_RENDERTARGET | WINED3DFMT_FLAG_VTF,
            ARB_TEXTURE_FLOAT,          convert_r16g16},
    {WINED3DFMT_R16G16_FLOAT,           GL_RG16F,                         GL_RG16F,                               0,
            GL_RG,                      GL_HALF_FLOAT_ARB,                0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_RENDERTARGET | WINED3DFMT_FLAG_VTF,
            ARB_TEXTURE_RG,             NULL},
    {WINED3DFMT_R16G16B16A16_FLOAT,     GL_RGBA16F_ARB,                   GL_RGBA16F_ARB,                         0,
            GL_RGBA,                    GL_HALF_FLOAT_ARB,                0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_RENDERTARGET
            | WINED3DFMT_FLAG_VTF,
            ARB_TEXTURE_FLOAT,          NULL},
    /* Palettized formats */
    {WINED3DFMT_P8_UINT,                GL_RGBA,                          GL_RGBA,                                0,
            GL_ALPHA,                   GL_UNSIGNED_BYTE,                 0,
            0,
            ARB_FRAGMENT_PROGRAM,       NULL},
    /* Standard ARGB formats */
    {WINED3DFMT_B8G8R8_UNORM,           GL_RGB8,                          GL_RGB8,                                0,
            GL_BGR,                     GL_UNSIGNED_BYTE,                 0,
            WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING | WINED3DFMT_FLAG_RENDERTARGET,
            WINED3D_GL_EXT_NONE,        NULL},
    {WINED3DFMT_B8G8R8A8_UNORM,         GL_RGBA8,                         GL_SRGB8_ALPHA8_EXT,                    0,
            GL_BGRA,                    GL_UNSIGNED_INT_8_8_8_8_REV,      0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_RENDERTARGET | WINED3DFMT_FLAG_SRGB_READ | WINED3DFMT_FLAG_SRGB_WRITE
            | WINED3DFMT_FLAG_VTF,
            WINED3D_GL_EXT_NONE,        NULL},
    {WINED3DFMT_B8G8R8X8_UNORM,         GL_RGB8,                          GL_SRGB8_EXT,                           0,
            GL_BGRA,                    GL_UNSIGNED_INT_8_8_8_8_REV,      0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_RENDERTARGET | WINED3DFMT_FLAG_SRGB_READ | WINED3DFMT_FLAG_SRGB_WRITE,
            WINED3D_GL_EXT_NONE,        NULL},
    {WINED3DFMT_B5G6R5_UNORM,           GL_RGB5,                          GL_RGB5,                          GL_RGB8,
            GL_RGB,                     GL_UNSIGNED_SHORT_5_6_5,          0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_RENDERTARGET,
            WINED3D_GL_EXT_NONE,        NULL},
    {WINED3DFMT_B5G5R5X1_UNORM,         GL_RGB5,                          GL_RGB5_A1,                             0,
            GL_BGRA,                    GL_UNSIGNED_SHORT_1_5_5_5_REV,    0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING,
            WINED3D_GL_EXT_NONE,        NULL},
    {WINED3DFMT_B5G5R5A1_UNORM,         GL_RGB5_A1,                       GL_RGB5_A1,                             0,
            GL_BGRA,                    GL_UNSIGNED_SHORT_1_5_5_5_REV,    0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING,
            WINED3D_GL_EXT_NONE,        NULL},
    {WINED3DFMT_B4G4R4A4_UNORM,         GL_RGBA4,                         GL_SRGB8_ALPHA8_EXT,                    0,
            GL_BGRA,                    GL_UNSIGNED_SHORT_4_4_4_4_REV,    0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_SRGB_READ,
            WINED3D_GL_EXT_NONE,        NULL},
    {WINED3DFMT_B2G3R3_UNORM,           GL_R3_G3_B2,                      GL_R3_G3_B2,                            0,
            GL_RGB,                     GL_UNSIGNED_BYTE_3_3_2,           0,
            WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING,
            WINED3D_GL_EXT_NONE,        NULL},
    {WINED3DFMT_A8_UNORM,               GL_ALPHA8,                        GL_ALPHA8,                              0,
            GL_ALPHA,                   GL_UNSIGNED_BYTE,                 0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING,
            WINED3D_GL_EXT_NONE,        NULL},
    {WINED3DFMT_B4G4R4X4_UNORM,         GL_RGB4,                          GL_RGB4,                                0,
            GL_BGRA,                    GL_UNSIGNED_SHORT_4_4_4_4_REV,    0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING,
            WINED3D_GL_EXT_NONE,        NULL},
    {WINED3DFMT_R10G10B10A2_UNORM,      GL_RGB10_A2,                      GL_RGB10_A2,                            0,
            GL_RGBA,                    GL_UNSIGNED_INT_2_10_10_10_REV,   0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_RENDERTARGET,
            WINED3D_GL_EXT_NONE,        NULL},
    {WINED3DFMT_R8G8B8A8_UNORM,         GL_RGBA8,                         GL_RGBA8,                               0,
            GL_RGBA,                    GL_UNSIGNED_INT_8_8_8_8_REV,      0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING,
            WINED3D_GL_EXT_NONE,        NULL},
    {WINED3DFMT_R8G8B8X8_UNORM,         GL_RGB8,                          GL_RGB8,                                0,
            GL_RGBA,                    GL_UNSIGNED_INT_8_8_8_8_REV,      0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING,
            WINED3D_GL_EXT_NONE,        NULL},
    {WINED3DFMT_R16G16_UNORM,           GL_RGB16,                         GL_RGB16,                       GL_RGBA16,
            GL_RGB,                     GL_UNSIGNED_SHORT,                6,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING,
            WINED3D_GL_EXT_NONE,        convert_r16g16},
    {WINED3DFMT_R16G16_UNORM,           GL_RG16,                          GL_RG16,                                0,
            GL_RG,                      GL_UNSIGNED_SHORT,                0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_RENDERTARGET,
            ARB_TEXTURE_RG,             NULL},
    {WINED3DFMT_B10G10R10A2_UNORM,      GL_RGB10_A2,                      GL_RGB10_A2,                            0,
            GL_BGRA,                    GL_UNSIGNED_INT_2_10_10_10_REV,   0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_RENDERTARGET,
            WINED3D_GL_EXT_NONE,        NULL},
    {WINED3DFMT_R16G16B16A16_UNORM,     GL_RGBA16,                        GL_RGBA16,                              0,
            GL_RGBA,                    GL_UNSIGNED_SHORT,                0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_RENDERTARGET,
            WINED3D_GL_EXT_NONE,        NULL},
    /* Luminance */
    {WINED3DFMT_L8_UNORM,               GL_LUMINANCE8,                    GL_SLUMINANCE8_EXT,                     0,
            GL_LUMINANCE,               GL_UNSIGNED_BYTE,                 0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_SRGB_READ,
            WINED3D_GL_EXT_NONE,        NULL},
    {WINED3DFMT_L8A8_UNORM,             GL_LUMINANCE8_ALPHA8,             GL_SLUMINANCE8_ALPHA8_EXT,              0,
            GL_LUMINANCE_ALPHA,         GL_UNSIGNED_BYTE,                 0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_SRGB_READ,
            WINED3D_GL_EXT_NONE,        NULL},
    {WINED3DFMT_L4A4_UNORM,             GL_LUMINANCE4_ALPHA4,             GL_LUMINANCE4_ALPHA4,                   0,
            GL_LUMINANCE_ALPHA,         GL_UNSIGNED_BYTE,                 2,
            WINED3DFMT_FLAG_FILTERING,
            WINED3D_GL_EXT_NONE,        convert_l4a4_unorm},
    /* Bump mapping stuff */
    {WINED3DFMT_R8G8_SNORM,             GL_RGB8,                          GL_RGB8,                                0,
            GL_BGR,                     GL_UNSIGNED_BYTE,                 3,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_BUMPMAP,
            WINED3D_GL_EXT_NONE,        convert_r8g8_snorm},
    {WINED3DFMT_R8G8_SNORM,             GL_DSDT8_NV,                      GL_DSDT8_NV,                            0,
            GL_DSDT_NV,                 GL_BYTE,                          0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_BUMPMAP,
            NV_TEXTURE_SHADER,          NULL},
    {WINED3DFMT_R5G5_SNORM_L6_UNORM,    GL_RGB5,                          GL_RGB5,                                0,
            GL_RGB,                     GL_UNSIGNED_SHORT_5_6_5,          2,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_BUMPMAP,
            WINED3D_GL_EXT_NONE,        convert_r5g5_snorm_l6_unorm},
    {WINED3DFMT_R5G5_SNORM_L6_UNORM,    GL_DSDT8_MAG8_NV,                 GL_DSDT8_MAG8_NV,                       0,
            GL_DSDT_MAG_NV,             GL_BYTE,                          3,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_BUMPMAP,
            NV_TEXTURE_SHADER,          convert_r5g5_snorm_l6_unorm_nv},
    {WINED3DFMT_R8G8_SNORM_L8X8_UNORM,  GL_RGB8,                          GL_RGB8,                                0,
            GL_BGRA,                    GL_UNSIGNED_INT_8_8_8_8_REV,      4,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_BUMPMAP,
            WINED3D_GL_EXT_NONE,        convert_r8g8_snorm_l8x8_unorm},
    {WINED3DFMT_R8G8_SNORM_L8X8_UNORM,  GL_DSDT8_MAG8_INTENSITY8_NV,      GL_DSDT8_MAG8_INTENSITY8_NV,            0,
            GL_DSDT_MAG_VIB_NV,         GL_UNSIGNED_INT_8_8_S8_S8_REV_NV, 4,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_BUMPMAP,
            NV_TEXTURE_SHADER,          convert_r8g8_snorm_l8x8_unorm_nv},
    {WINED3DFMT_R8G8B8A8_SNORM,         GL_RGBA8,                         GL_RGBA8,                               0,
            GL_BGRA,                    GL_UNSIGNED_BYTE,                 4,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_BUMPMAP,
            WINED3D_GL_EXT_NONE,        convert_r8g8b8a8_snorm},
    {WINED3DFMT_R8G8B8A8_SNORM,         GL_SIGNED_RGBA8_NV,               GL_SIGNED_RGBA8_NV,                     0,
            GL_RGBA,                    GL_BYTE,                          0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_BUMPMAP,
            NV_TEXTURE_SHADER,          NULL},
    {WINED3DFMT_R16G16_SNORM,           GL_RGB16,                         GL_RGB16,                               0,
            GL_BGR,                     GL_UNSIGNED_SHORT,                6,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_BUMPMAP,
            WINED3D_GL_EXT_NONE,        convert_r16g16_snorm},
    {WINED3DFMT_R16G16_SNORM,           GL_SIGNED_HILO16_NV,              GL_SIGNED_HILO16_NV,                    0,
            GL_HILO_NV,                 GL_SHORT,                         0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_BUMPMAP,
            NV_TEXTURE_SHADER,          NULL},
    /* Depth stencil formats */
    {WINED3DFMT_D16_LOCKABLE,           GL_DEPTH_COMPONENT24_ARB,         GL_DEPTH_COMPONENT24_ARB,               0,
            GL_DEPTH_COMPONENT,         GL_UNSIGNED_SHORT,                0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_DEPTH | WINED3DFMT_FLAG_SHADOW,
            ARB_DEPTH_TEXTURE,          NULL},
    {WINED3DFMT_D32_UNORM,              GL_DEPTH_COMPONENT32_ARB,         GL_DEPTH_COMPONENT32_ARB,               0,
            GL_DEPTH_COMPONENT,         GL_UNSIGNED_INT,                  0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_DEPTH | WINED3DFMT_FLAG_SHADOW,
            ARB_DEPTH_TEXTURE,          NULL},
    {WINED3DFMT_S1_UINT_D15_UNORM,      GL_DEPTH_COMPONENT24_ARB,         GL_DEPTH_COMPONENT24_ARB,               0,
            GL_DEPTH_COMPONENT,         GL_UNSIGNED_SHORT,                0,
            WINED3DFMT_FLAG_DEPTH | WINED3DFMT_FLAG_SHADOW,
            ARB_DEPTH_TEXTURE,          NULL},
    {WINED3DFMT_S1_UINT_D15_UNORM,      GL_DEPTH24_STENCIL8_EXT,          GL_DEPTH24_STENCIL8_EXT,                0,
            GL_DEPTH_STENCIL_EXT,       GL_UNSIGNED_INT_24_8_EXT,         4,
            WINED3DFMT_FLAG_DEPTH | WINED3DFMT_FLAG_STENCIL | WINED3DFMT_FLAG_SHADOW,
            EXT_PACKED_DEPTH_STENCIL,   convert_s1_uint_d15_unorm},
    {WINED3DFMT_S1_UINT_D15_UNORM,      GL_DEPTH24_STENCIL8,              GL_DEPTH24_STENCIL8,                    0,
            GL_DEPTH_STENCIL,           GL_UNSIGNED_INT_24_8,             4,
            WINED3DFMT_FLAG_DEPTH | WINED3DFMT_FLAG_STENCIL | WINED3DFMT_FLAG_SHADOW,
            ARB_FRAMEBUFFER_OBJECT,     convert_s1_uint_d15_unorm},
    {WINED3DFMT_D24_UNORM_S8_UINT,      GL_DEPTH_COMPONENT24_ARB,         GL_DEPTH_COMPONENT24_ARB,               0,
            GL_DEPTH_COMPONENT,         GL_UNSIGNED_INT,                  0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_DEPTH | WINED3DFMT_FLAG_SHADOW,
            ARB_DEPTH_TEXTURE,          NULL},
    {WINED3DFMT_D24_UNORM_S8_UINT,      GL_DEPTH24_STENCIL8_EXT,          GL_DEPTH24_STENCIL8_EXT,                0,
            GL_DEPTH_STENCIL_EXT,       GL_UNSIGNED_INT_24_8_EXT,         0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_DEPTH | WINED3DFMT_FLAG_STENCIL | WINED3DFMT_FLAG_SHADOW,
            EXT_PACKED_DEPTH_STENCIL,   NULL},
    {WINED3DFMT_D24_UNORM_S8_UINT,      GL_DEPTH24_STENCIL8,              GL_DEPTH24_STENCIL8,                    0,
            GL_DEPTH_STENCIL,           GL_UNSIGNED_INT_24_8,             0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_DEPTH | WINED3DFMT_FLAG_STENCIL | WINED3DFMT_FLAG_SHADOW,
            ARB_FRAMEBUFFER_OBJECT,     NULL},
    {WINED3DFMT_X8D24_UNORM,            GL_DEPTH_COMPONENT24_ARB,         GL_DEPTH_COMPONENT24_ARB,               0,
            GL_DEPTH_COMPONENT,         GL_UNSIGNED_INT,                  0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_DEPTH | WINED3DFMT_FLAG_SHADOW,
            ARB_DEPTH_TEXTURE,          NULL},
    {WINED3DFMT_S4X4_UINT_D24_UNORM,    GL_DEPTH_COMPONENT24_ARB,         GL_DEPTH_COMPONENT24_ARB,               0,
            GL_DEPTH_COMPONENT,         GL_UNSIGNED_INT,                  0,
            WINED3DFMT_FLAG_DEPTH | WINED3DFMT_FLAG_SHADOW,
            ARB_DEPTH_TEXTURE,          NULL},
    {WINED3DFMT_S4X4_UINT_D24_UNORM,    GL_DEPTH24_STENCIL8_EXT,          GL_DEPTH24_STENCIL8_EXT,                0,
            GL_DEPTH_STENCIL_EXT,       GL_UNSIGNED_INT_24_8_EXT,         4,
            WINED3DFMT_FLAG_DEPTH | WINED3DFMT_FLAG_STENCIL | WINED3DFMT_FLAG_SHADOW,
            EXT_PACKED_DEPTH_STENCIL,   convert_s4x4_uint_d24_unorm},
    {WINED3DFMT_S4X4_UINT_D24_UNORM,    GL_DEPTH24_STENCIL8,              GL_DEPTH24_STENCIL8,                    0,
            GL_DEPTH_STENCIL,           GL_UNSIGNED_INT_24_8,             4,
            WINED3DFMT_FLAG_DEPTH | WINED3DFMT_FLAG_STENCIL | WINED3DFMT_FLAG_SHADOW,
            ARB_FRAMEBUFFER_OBJECT,     convert_s4x4_uint_d24_unorm},
    {WINED3DFMT_D16_UNORM,              GL_DEPTH_COMPONENT24_ARB,         GL_DEPTH_COMPONENT24_ARB,               0,
            GL_DEPTH_COMPONENT,         GL_UNSIGNED_SHORT,                0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_DEPTH | WINED3DFMT_FLAG_SHADOW,
            ARB_DEPTH_TEXTURE,          NULL},
    {WINED3DFMT_L16_UNORM,              GL_LUMINANCE16,                   GL_LUMINANCE16,                         0,
            GL_LUMINANCE,               GL_UNSIGNED_SHORT,                0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING,
            WINED3D_GL_EXT_NONE,        NULL},
    {WINED3DFMT_D32_FLOAT,              GL_DEPTH_COMPONENT32F,            GL_DEPTH_COMPONENT32F,                  0,
            GL_DEPTH_COMPONENT,         GL_FLOAT,                         0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_DEPTH | WINED3DFMT_FLAG_SHADOW,
            ARB_DEPTH_BUFFER_FLOAT,     NULL},
    {WINED3DFMT_S8_UINT_D24_FLOAT,      GL_DEPTH32F_STENCIL8,             GL_DEPTH32F_STENCIL8,                   0,
            GL_DEPTH_STENCIL,           GL_FLOAT_32_UNSIGNED_INT_24_8_REV, 8,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_DEPTH | WINED3DFMT_FLAG_STENCIL | WINED3DFMT_FLAG_SHADOW,
            ARB_DEPTH_BUFFER_FLOAT,     convert_s8_uint_d24_float},
    /* Vendor-specific formats */
    {WINED3DFMT_ATI2N,                  GL_COMPRESSED_LUMINANCE_ALPHA_3DC_ATI, GL_COMPRESSED_LUMINANCE_ALPHA_3DC_ATI, 0,
            GL_LUMINANCE_ALPHA,         GL_UNSIGNED_BYTE,                 0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_COMPRESSED,
            ATI_TEXTURE_COMPRESSION_3DC, NULL},
    {WINED3DFMT_ATI2N,                  GL_COMPRESSED_RG_RGTC2,           GL_COMPRESSED_RG_RGTC2,                 0,
            GL_LUMINANCE_ALPHA,         GL_UNSIGNED_BYTE,                 0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_COMPRESSED,
            ARB_TEXTURE_COMPRESSION_RGTC, NULL},
    {WINED3DFMT_INTZ,                   GL_DEPTH24_STENCIL8_EXT,          GL_DEPTH24_STENCIL8_EXT,                0,
            GL_DEPTH_STENCIL_EXT,       GL_UNSIGNED_INT_24_8_EXT,         0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_DEPTH | WINED3DFMT_FLAG_STENCIL,
            EXT_PACKED_DEPTH_STENCIL,   NULL},
    {WINED3DFMT_INTZ,                   GL_DEPTH24_STENCIL8,              GL_DEPTH24_STENCIL8,                    0,
            GL_DEPTH_STENCIL,           GL_UNSIGNED_INT_24_8,             0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING | WINED3DFMT_FLAG_FILTERING
            | WINED3DFMT_FLAG_DEPTH | WINED3DFMT_FLAG_STENCIL,
            ARB_FRAMEBUFFER_OBJECT,     NULL},
    {WINED3DFMT_NULL,                   0,                                0,                                      0,
            GL_RGBA,                    GL_UNSIGNED_INT_8_8_8_8_REV,      0,
            WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_RENDERTARGET,
            ARB_FRAMEBUFFER_OBJECT,     NULL},
};

static inline int getFmtIdx(enum wined3d_format_id format_id)
{
    /* First check if the format is at the position of its value.
     * This will catch the argb formats before the loop is entered. */
    if (format_id < (sizeof(formats) / sizeof(*formats))
            && formats[format_id].id == format_id)
    {
        return format_id;
    }
    else
    {
        unsigned int i;

        for (i = 0; i < (sizeof(formats) / sizeof(*formats)); ++i)
        {
            if (formats[i].id == format_id) return i;
        }
    }
    return -1;
}

static BOOL init_format_base_info(struct wined3d_gl_info *gl_info)
{
    UINT format_count = sizeof(formats) / sizeof(*formats);
    UINT i;

    gl_info->formats = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, format_count * sizeof(*gl_info->formats));
    if (!gl_info->formats)
    {
        ERR("Failed to allocate memory.\n");
        return FALSE;
    }

    for (i = 0; i < format_count; ++i)
    {
        struct wined3d_format *format = &gl_info->formats[i];
        format->id = formats[i].id;
        format->red_size = formats[i].red_size;
        format->green_size = formats[i].green_size;
        format->blue_size = formats[i].blue_size;
        format->alpha_size = formats[i].alpha_size;
        format->red_offset = formats[i].red_offset;
        format->green_offset = formats[i].green_offset;
        format->blue_offset = formats[i].blue_offset;
        format->alpha_offset = formats[i].alpha_offset;
        format->byte_count = formats[i].bpp;
        format->depth_size = formats[i].depth_size;
        format->stencil_size = formats[i].stencil_size;
        format->block_width = 1;
        format->block_height = 1;
        format->block_byte_count = formats[i].bpp;
    }

    for (i = 0; i < (sizeof(format_base_flags) / sizeof(*format_base_flags)); ++i)
    {
        int fmt_idx = getFmtIdx(format_base_flags[i].id);

        if (fmt_idx == -1)
        {
            ERR("Format %s (%#x) not found.\n",
                    debug_d3dformat(format_base_flags[i].id), format_base_flags[i].id);
            HeapFree(GetProcessHeap(), 0, gl_info->formats);
            return FALSE;
        }

        gl_info->formats[fmt_idx].flags |= format_base_flags[i].flags;
    }

    return TRUE;
}

static BOOL init_format_block_info(struct wined3d_gl_info *gl_info)
{
    unsigned int i;

    for (i = 0; i < (sizeof(format_block_info) / sizeof(*format_block_info)); ++i)
    {
        struct wined3d_format *format;
        int fmt_idx = getFmtIdx(format_block_info[i].id);

        if (fmt_idx == -1)
        {
            ERR("Format %s (%#x) not found.\n",
                    debug_d3dformat(format_block_info[i].id), format_block_info[i].id);
            return FALSE;
        }

        format = &gl_info->formats[fmt_idx];
        format->block_width = format_block_info[i].block_width;
        format->block_height = format_block_info[i].block_height;
        format->block_byte_count = format_block_info[i].block_byte_count;
        format->flags |= WINED3DFMT_FLAG_BLOCKS;
    }

    return TRUE;
}

/* Context activation is done by the caller. */
static void check_fbo_compat(const struct wined3d_gl_info *gl_info, struct wined3d_format *format)
{
    /* Check if the default internal format is supported as a frame buffer
     * target, otherwise fall back to the render target internal.
     *
     * Try to stick to the standard format if possible, this limits precision differences. */
    GLenum status;
    GLuint tex;

    while (gl_info->gl_ops.gl.p_glGetError());
    gl_info->gl_ops.gl.p_glDisable(GL_BLEND);

    gl_info->gl_ops.gl.p_glGenTextures(1, &tex);
    gl_info->gl_ops.gl.p_glBindTexture(GL_TEXTURE_2D, tex);

    gl_info->gl_ops.gl.p_glTexImage2D(GL_TEXTURE_2D, 0, format->glInternal, 16, 16, 0,
            format->glFormat, format->glType, NULL);
    gl_info->gl_ops.gl.p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl_info->gl_ops.gl.p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    gl_info->fbo_ops.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    status = gl_info->fbo_ops.glCheckFramebufferStatus(GL_FRAMEBUFFER);
    checkGLcall("Framebuffer format check");

    if (status == GL_FRAMEBUFFER_COMPLETE)
    {
        TRACE("Format %s is supported as FBO color attachment.\n", debug_d3dformat(format->id));
        format->flags |= WINED3DFMT_FLAG_FBO_ATTACHABLE;
        format->rtInternal = format->glInternal;
    }
    else
    {
        if (!format->rtInternal)
        {
            if (format->flags & WINED3DFMT_FLAG_RENDERTARGET)
            {
                FIXME("Format %s with rendertarget flag is not supported as FBO color attachment,"
                        " and no fallback specified.\n", debug_d3dformat(format->id));
                format->flags &= ~WINED3DFMT_FLAG_RENDERTARGET;
            }
            else
            {
                TRACE("Format %s is not supported as FBO color attachment.\n", debug_d3dformat(format->id));
            }
            format->rtInternal = format->glInternal;
        }
        else
        {
            TRACE("Format %s is not supported as FBO color attachment, trying rtInternal format as fallback.\n",
                    debug_d3dformat(format->id));

            while (gl_info->gl_ops.gl.p_glGetError());

            gl_info->fbo_ops.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);

            gl_info->gl_ops.gl.p_glTexImage2D(GL_TEXTURE_2D, 0, format->rtInternal, 16, 16, 0,
                    format->glFormat, format->glType, NULL);
            gl_info->gl_ops.gl.p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            gl_info->gl_ops.gl.p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            gl_info->fbo_ops.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

            status = gl_info->fbo_ops.glCheckFramebufferStatus(GL_FRAMEBUFFER);
            checkGLcall("Framebuffer format check");

            if (status == GL_FRAMEBUFFER_COMPLETE)
            {
                TRACE("Format %s rtInternal format is supported as FBO color attachment.\n",
                        debug_d3dformat(format->id));
            }
            else
            {
                FIXME("Format %s rtInternal format is not supported as FBO color attachment.\n",
                        debug_d3dformat(format->id));
                format->flags &= ~WINED3DFMT_FLAG_RENDERTARGET;
            }
        }
    }

    if (status == GL_FRAMEBUFFER_COMPLETE && ((format->flags & WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING)
            || !(gl_info->quirks & WINED3D_QUIRK_LIMITED_TEX_FILTERING))
            && format->id != WINED3DFMT_NULL && format->id != WINED3DFMT_P8_UINT
            && format->glFormat != GL_LUMINANCE && format->glFormat != GL_LUMINANCE_ALPHA
            && (format->red_size || format->alpha_size))
    {
        DWORD readback[16 * 16], color, r_range, a_range;
        BYTE r, a;
        BOOL match = TRUE;
        GLuint rb;

        if (gl_info->supported[ARB_FRAMEBUFFER_OBJECT]
                || gl_info->supported[EXT_PACKED_DEPTH_STENCIL])
        {
            gl_info->fbo_ops.glGenRenderbuffers(1, &rb);
            gl_info->fbo_ops.glBindRenderbuffer(GL_RENDERBUFFER, rb);
            gl_info->fbo_ops.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 16, 16);
            gl_info->fbo_ops.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb);
            gl_info->fbo_ops.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rb);
            checkGLcall("RB attachment");
        }

        gl_info->gl_ops.gl.p_glEnable(GL_BLEND);
        gl_info->gl_ops.gl.p_glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        gl_info->gl_ops.gl.p_glClear(GL_COLOR_BUFFER_BIT);
        if (gl_info->gl_ops.gl.p_glGetError() == GL_INVALID_FRAMEBUFFER_OPERATION)
        {
            while (gl_info->gl_ops.gl.p_glGetError());
            TRACE("Format doesn't support post-pixelshader blending.\n");
            format->flags &= ~WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING;
        }
        else
        {
            gl_info->gl_ops.gl.p_glDisable(GL_BLEND);
            gl_info->gl_ops.gl.p_glViewport(0, 0, 16, 16);
            gl_info->gl_ops.gl.p_glDisable(GL_LIGHTING);
            gl_info->gl_ops.gl.p_glMatrixMode(GL_MODELVIEW);
            gl_info->gl_ops.gl.p_glLoadIdentity();
            gl_info->gl_ops.gl.p_glMatrixMode(GL_PROJECTION);
            gl_info->gl_ops.gl.p_glLoadIdentity();

            gl_info->gl_ops.gl.p_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            /* Draw a full-black quad */
            gl_info->gl_ops.gl.p_glBegin(GL_TRIANGLE_STRIP);
            gl_info->gl_ops.gl.p_glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
            gl_info->gl_ops.gl.p_glVertex3f(-1.0f, -1.0f, 0.0f);
            gl_info->gl_ops.gl.p_glVertex3f(1.0f, -1.0f, 0.0f);
            gl_info->gl_ops.gl.p_glVertex3f(-1.0f, 1.0f, 0.0f);
            gl_info->gl_ops.gl.p_glVertex3f(1.0f, 1.0f, 0.0f);
            gl_info->gl_ops.gl.p_glEnd();

            gl_info->gl_ops.gl.p_glEnable(GL_BLEND);
            /* Draw a half-transparent red quad */
            gl_info->gl_ops.gl.p_glBegin(GL_TRIANGLE_STRIP);
            gl_info->gl_ops.gl.p_glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
            gl_info->gl_ops.gl.p_glVertex3f(-1.0f, -1.0f, 0.0f);
            gl_info->gl_ops.gl.p_glVertex3f(1.0f, -1.0f, 0.0f);
            gl_info->gl_ops.gl.p_glVertex3f(-1.0f, 1.0f, 0.0f);
            gl_info->gl_ops.gl.p_glVertex3f(1.0f, 1.0f, 0.0f);
            gl_info->gl_ops.gl.p_glEnd();

            gl_info->gl_ops.gl.p_glDisable(GL_BLEND);

            /* Rebinding texture to workaround a fglrx bug. */
            gl_info->gl_ops.gl.p_glBindTexture(GL_TEXTURE_2D, tex);
            gl_info->gl_ops.gl.p_glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, readback);
            checkGLcall("Post-pixelshader blending check");

            color = readback[7 * 16 + 7];
            a = color >> 24;
            r = (color & 0x00ff0000) >> 16;

            r_range = format->red_size < 8 ? 1 << (8 - format->red_size) : 1;
            a_range = format->alpha_size < 8 ? 1 << (8 - format->alpha_size) : 1;
            if (format->red_size && (r < 0x7f - r_range || r > 0x7f + r_range))
                match = FALSE;
            else if (format->alpha_size > 1 && (a < 0xbf - a_range || a > 0xbf + a_range))
                match = FALSE;
            if (!match)
            {
                TRACE("Format doesn't support post-pixelshader blending.\n");
                TRACE("Color output: %#x\n", color);
                format->flags &= ~WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING;
            }
            else
            {
                TRACE("Format supports post-pixelshader blending.\n");
                TRACE("Color output: %#x\n", color);
                format->flags |= WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING;
            }
        }

        if (gl_info->supported[ARB_FRAMEBUFFER_OBJECT]
                || gl_info->supported[EXT_PACKED_DEPTH_STENCIL])
        {
            gl_info->fbo_ops.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
            gl_info->fbo_ops.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
            gl_info->fbo_ops.glDeleteRenderbuffers(1, &rb);
            checkGLcall("RB cleanup");
        }
    }

    if (format->glInternal != format->glGammaInternal)
    {
        gl_info->gl_ops.gl.p_glTexImage2D(GL_TEXTURE_2D, 0, format->glGammaInternal, 16, 16, 0,
                format->glFormat, format->glType, NULL);
        gl_info->fbo_ops.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

        status = gl_info->fbo_ops.glCheckFramebufferStatus(GL_FRAMEBUFFER);
        checkGLcall("Framebuffer format check");

        if (status == GL_FRAMEBUFFER_COMPLETE)
        {
            TRACE("Format %s's sRGB format is FBO attachable.\n", debug_d3dformat(format->id));
            format->flags |= WINED3DFMT_FLAG_FBO_ATTACHABLE_SRGB;
        }
        else
        {
            WARN("Format %s's sRGB format is not FBO attachable.\n", debug_d3dformat(format->id));
        }
    }
    else if (status == GL_FRAMEBUFFER_COMPLETE)
        format->flags |= WINED3DFMT_FLAG_FBO_ATTACHABLE_SRGB;

    gl_info->gl_ops.gl.p_glDeleteTextures(1, &tex);
}

static void query_format_flag(struct wined3d_gl_info *gl_info, struct wined3d_format *format,
        GLint internal, GLenum pname, DWORD flag, const char *string)
{
    GLint value;

    gl_info->gl_ops.ext.p_glGetInternalformativ(GL_TEXTURE_2D, internal, pname, 1, &value);
    if (value == GL_FULL_SUPPORT)
    {
        TRACE("Format %s supports %s.\n", debug_d3dformat(format->id), string);
        format->flags |= flag;
    }
    else
    {
        TRACE("Format %s doesn't support %s.\n", debug_d3dformat(format->id), string);
        format->flags &= ~flag;
    }
}

/* Context activation is done by the caller. */
static void init_format_fbo_compat_info(struct wined3d_gl_info *gl_info)
{
    unsigned int i;
    GLuint fbo;

    if (gl_info->supported[ARB_INTERNALFORMAT_QUERY2])
    {
        for (i = 0; i < sizeof(formats) / sizeof(*formats); ++i)
        {
            GLint value;
            struct wined3d_format *format = &gl_info->formats[i];

            if (!format->glInternal)
                continue;
            if (format->flags & (WINED3DFMT_FLAG_DEPTH | WINED3DFMT_FLAG_STENCIL))
                continue;

            gl_info->gl_ops.ext.p_glGetInternalformativ(GL_TEXTURE_2D, format->glInternal,
                    GL_FRAMEBUFFER_RENDERABLE, 1, &value);
            if (value == GL_FULL_SUPPORT)
            {
                TRACE("Format %s is supported as FBO color attachment.\n", debug_d3dformat(format->id));
                format->flags |= WINED3DFMT_FLAG_FBO_ATTACHABLE;
                format->rtInternal = format->glInternal;

                query_format_flag(gl_info, format, format->glInternal, GL_FRAMEBUFFER_BLEND,
                        WINED3DFMT_FLAG_POSTPIXELSHADER_BLENDING, "post-pixelshader blending");
            }
            else
            {
                if (!format->rtInternal)
                {
                    if (format->flags & WINED3DFMT_FLAG_RENDERTARGET)
                    {
                        WARN("Format %s with rendertarget flag is not supported as FBO color attachment"
                                " and no fallback specified.\n", debug_d3dformat(format->id));
                        format->flags &= ~WINED3DFMT_FLAG_RENDERTARGET;
                    }
                    else
                        TRACE("Format %s is not supported as FBO color attachment.\n", debug_d3dformat(format->id));
                    format->rtInternal = format->glInternal;
                }
                else
                {
                    gl_info->gl_ops.ext.p_glGetInternalformativ(GL_TEXTURE_2D, format->rtInternal,
                            GL_FRAMEBUFFER_RENDERABLE, 1, &value);
                    if (value == GL_FULL_SUPPORT)
                    {
                        TRACE("Format %s rtInternal format is supported as FBO color attachment.\n",
                                debug_d3dformat(format->id));
                    }
                    else
                    {
                        WARN("Format %s rtInternal format is not supported as FBO color attachment.\n",
                                debug_d3dformat(format->id));
                        format->flags &= ~WINED3DFMT_FLAG_RENDERTARGET;
                    }
                }
            }

            if (format->glInternal != format->glGammaInternal)
            {
                gl_info->gl_ops.ext.p_glGetInternalformativ(GL_TEXTURE_2D, format->glGammaInternal,
                        GL_FRAMEBUFFER_RENDERABLE, 1, &value);
                if (value == GL_FULL_SUPPORT)
                {
                    TRACE("Format %s's sRGB format is FBO attachable.\n", debug_d3dformat(format->id));
                    format->flags |= WINED3DFMT_FLAG_FBO_ATTACHABLE_SRGB;
                }
                else
                {
                    WARN("Format %s's sRGB format is not FBO attachable.\n", debug_d3dformat(format->id));
                }
            }
            else if (format->flags & WINED3DFMT_FLAG_FBO_ATTACHABLE)
                format->flags |= WINED3DFMT_FLAG_FBO_ATTACHABLE_SRGB;
        }
        return;
    }

    if (wined3d_settings.offscreen_rendering_mode == ORM_FBO)
    {
        gl_info->fbo_ops.glGenFramebuffers(1, &fbo);
        gl_info->fbo_ops.glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        gl_info->gl_ops.gl.p_glDrawBuffer(GL_COLOR_ATTACHMENT0);
        gl_info->gl_ops.gl.p_glReadBuffer(GL_COLOR_ATTACHMENT0);
    }

    for (i = 0; i < sizeof(formats) / sizeof(*formats); ++i)
    {
        struct wined3d_format *format = &gl_info->formats[i];

        if (!format->glInternal) continue;

        if (format->flags & (WINED3DFMT_FLAG_DEPTH | WINED3DFMT_FLAG_STENCIL))
        {
            TRACE("Skipping format %s because it's a depth/stencil format.\n",
                    debug_d3dformat(format->id));
            continue;
        }

        if (format->flags & WINED3DFMT_FLAG_COMPRESSED)
        {
            TRACE("Skipping format %s because it's a compressed format.\n",
                    debug_d3dformat(format->id));
            continue;
        }

        if (wined3d_settings.offscreen_rendering_mode == ORM_FBO)
        {
            TRACE("Checking if format %s is supported as FBO color attachment...\n", debug_d3dformat(format->id));
            check_fbo_compat(gl_info, format);
        }
        else
        {
            format->rtInternal = format->glInternal;
        }
    }

    if (wined3d_settings.offscreen_rendering_mode == ORM_FBO)
        gl_info->fbo_ops.glDeleteFramebuffers(1, &fbo);
}

static BOOL init_format_texture_info(struct wined3d_adapter *adapter, struct wined3d_gl_info *gl_info)
{
    struct fragment_caps fragment_caps;
    struct shader_caps shader_caps;
    BOOL srgb_write;
    unsigned int i;

    adapter->fragment_pipe->get_caps(gl_info, &fragment_caps);
    adapter->shader_backend->shader_get_caps(gl_info, &shader_caps);
    srgb_write = (fragment_caps.wined3d_caps & WINED3D_FRAGMENT_CAP_SRGB_WRITE)
            && (shader_caps.wined3d_caps & WINED3D_SHADER_CAP_SRGB_WRITE);

    for (i = 0; i < sizeof(format_texture_info) / sizeof(*format_texture_info); ++i)
    {
        int fmt_idx = getFmtIdx(format_texture_info[i].id);
        struct wined3d_format *format;

        if (fmt_idx == -1)
        {
            ERR("Format %s (%#x) not found.\n",
                    debug_d3dformat(format_texture_info[i].id), format_texture_info[i].id);
            return FALSE;
        }

        if (!gl_info->supported[format_texture_info[i].extension]) continue;

        format = &gl_info->formats[fmt_idx];

        /* ARB_texture_rg defines floating point formats, but only if
         * ARB_texture_float is also supported. */
        if (!gl_info->supported[ARB_TEXTURE_FLOAT]
                && (format->flags & WINED3DFMT_FLAG_FLOAT))
            continue;

        format->glInternal = format_texture_info[i].gl_internal;
        format->glGammaInternal = format_texture_info[i].gl_srgb_internal;
        format->rtInternal = format_texture_info[i].gl_rt_internal;
        format->glFormat = format_texture_info[i].gl_format;
        format->glType = format_texture_info[i].gl_type;
        format->color_fixup = COLOR_FIXUP_IDENTITY;
        format->flags |= format_texture_info[i].flags;
        format->height_scale.numerator = 1;
        format->height_scale.denominator = 1;

        if (gl_info->supported[ARB_INTERNALFORMAT_QUERY2])
        {
            query_format_flag(gl_info, format, format->glInternal, GL_VERTEX_TEXTURE,
                    WINED3DFMT_FLAG_VTF, "vertex texture usage");
            query_format_flag(gl_info, format, format->glInternal, GL_FILTER,
                    WINED3DFMT_FLAG_FILTERING, "filtering");

            if (format->glGammaInternal != format->glInternal)
            {
                query_format_flag(gl_info, format, format->glGammaInternal, GL_SRGB_READ,
                        WINED3DFMT_FLAG_SRGB_READ, "sRGB read");

                if (srgb_write)
                    query_format_flag(gl_info, format, format->glGammaInternal, GL_SRGB_WRITE,
                            WINED3DFMT_FLAG_SRGB_WRITE, "sRGB write");
                else
                    format->flags &= ~WINED3DFMT_FLAG_SRGB_WRITE;

                if (!(format->flags & (WINED3DFMT_FLAG_SRGB_READ | WINED3DFMT_FLAG_SRGB_WRITE)))
                    format->glGammaInternal = format->glInternal;
                else if (gl_info->supported[EXT_TEXTURE_SRGB_DECODE])
                    format->glInternal = format->glGammaInternal;
            }
        }
        else
        {
            if (!gl_info->limits.vertex_samplers)
                format->flags &= ~WINED3DFMT_FLAG_VTF;

            if (!(gl_info->quirks & WINED3D_QUIRK_LIMITED_TEX_FILTERING))
                format->flags |= WINED3DFMT_FLAG_FILTERING;
            else if (format->id != WINED3DFMT_R32G32B32A32_FLOAT && format->id != WINED3DFMT_R32_FLOAT)
                format->flags &= ~WINED3DFMT_FLAG_VTF;

            if (format->glGammaInternal != format->glInternal)
            {
                /* Filter sRGB capabilities if EXT_texture_sRGB is not supported. */
                if (!gl_info->supported[EXT_TEXTURE_SRGB])
                {
                    format->glGammaInternal = format->glInternal;
                    format->flags &= ~(WINED3DFMT_FLAG_SRGB_READ | WINED3DFMT_FLAG_SRGB_WRITE);
                }
                else if (gl_info->supported[EXT_TEXTURE_SRGB_DECODE])
                {
                    format->glInternal = format->glGammaInternal;
                }
            }

            if ((format->flags & WINED3DFMT_FLAG_SRGB_WRITE) && !srgb_write)
                format->flags &= ~WINED3DFMT_FLAG_SRGB_WRITE;
        }

        /* Texture conversion stuff */
        format->convert = format_texture_info[i].convert;
        format->conv_byte_count = format_texture_info[i].conv_byte_count;
    }

    return TRUE;
}

static BOOL color_match(DWORD c1, DWORD c2, BYTE max_diff)
{
    if (abs((c1 & 0xff) - (c2 & 0xff)) > max_diff) return FALSE;
    c1 >>= 8; c2 >>= 8;
    if (abs((c1 & 0xff) - (c2 & 0xff)) > max_diff) return FALSE;
    c1 >>= 8; c2 >>= 8;
    if (abs((c1 & 0xff) - (c2 & 0xff)) > max_diff) return FALSE;
    c1 >>= 8; c2 >>= 8;
    if (abs((c1 & 0xff) - (c2 & 0xff)) > max_diff) return FALSE;
    return TRUE;
}

/* A context is provided by the caller */
static BOOL check_filter(const struct wined3d_gl_info *gl_info, GLenum internal)
{
    static const DWORD data[] = {0x00000000, 0xffffffff};
    GLuint tex, fbo, buffer;
    DWORD readback[16 * 1];
    BOOL ret = FALSE;

    /* Render a filtered texture and see what happens. This is intended to detect the lack of
     * float16 filtering on ATI X1000 class cards. The drivers disable filtering instead of
     * falling back to software. If this changes in the future this code will get fooled and
     * apps might hit the software path due to incorrectly advertised caps.
     *
     * Its unlikely that this changes however. GL Games like Mass Effect depend on the filter
     * disable fallback, if Apple or ATI ever change the driver behavior they will break more
     * than Wine. The Linux binary <= r500 driver is not maintained any more anyway
     */

    while (gl_info->gl_ops.gl.p_glGetError());

    gl_info->gl_ops.gl.p_glGenTextures(1, &buffer);
    gl_info->gl_ops.gl.p_glBindTexture(GL_TEXTURE_2D, buffer);
    memset(readback, 0x7e, sizeof(readback));
    gl_info->gl_ops.gl.p_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 16, 1, 0,
            GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, readback);
    gl_info->gl_ops.gl.p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl_info->gl_ops.gl.p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl_info->gl_ops.gl.p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl_info->gl_ops.gl.p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl_info->gl_ops.gl.p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    gl_info->gl_ops.gl.p_glGenTextures(1, &tex);
    gl_info->gl_ops.gl.p_glBindTexture(GL_TEXTURE_2D, tex);
    gl_info->gl_ops.gl.p_glTexImage2D(GL_TEXTURE_2D, 0, internal, 2, 1, 0,
            GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, data);
    gl_info->gl_ops.gl.p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl_info->gl_ops.gl.p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl_info->gl_ops.gl.p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl_info->gl_ops.gl.p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl_info->gl_ops.gl.p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl_info->gl_ops.gl.p_glEnable(GL_TEXTURE_2D);

    gl_info->fbo_ops.glGenFramebuffers(1, &fbo);
    gl_info->fbo_ops.glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    gl_info->fbo_ops.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer, 0);
    gl_info->gl_ops.gl.p_glDrawBuffer(GL_COLOR_ATTACHMENT0);

    gl_info->gl_ops.gl.p_glViewport(0, 0, 16, 1);
    gl_info->gl_ops.gl.p_glDisable(GL_LIGHTING);
    gl_info->gl_ops.gl.p_glMatrixMode(GL_MODELVIEW);
    gl_info->gl_ops.gl.p_glLoadIdentity();
    gl_info->gl_ops.gl.p_glMatrixMode(GL_PROJECTION);
    gl_info->gl_ops.gl.p_glLoadIdentity();

    gl_info->gl_ops.gl.p_glClearColor(0, 1, 0, 0);
    gl_info->gl_ops.gl.p_glClear(GL_COLOR_BUFFER_BIT);

    gl_info->gl_ops.gl.p_glBegin(GL_TRIANGLE_STRIP);
    gl_info->gl_ops.gl.p_glTexCoord2f(0.0, 0.0);
    gl_info->gl_ops.gl.p_glVertex2f(-1.0f, -1.0f);
    gl_info->gl_ops.gl.p_glTexCoord2f(1.0, 0.0);
    gl_info->gl_ops.gl.p_glVertex2f(1.0f, -1.0f);
    gl_info->gl_ops.gl.p_glTexCoord2f(0.0, 1.0);
    gl_info->gl_ops.gl.p_glVertex2f(-1.0f, 1.0f);
    gl_info->gl_ops.gl.p_glTexCoord2f(1.0, 1.0);
    gl_info->gl_ops.gl.p_glVertex2f(1.0f, 1.0f);
    gl_info->gl_ops.gl.p_glEnd();

    gl_info->gl_ops.gl.p_glBindTexture(GL_TEXTURE_2D, buffer);
    memset(readback, 0x7f, sizeof(readback));
    gl_info->gl_ops.gl.p_glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, readback);
    if (color_match(readback[6], 0xffffffff, 5) || color_match(readback[6], 0x00000000, 5)
            || color_match(readback[9], 0xffffffff, 5) || color_match(readback[9], 0x00000000, 5))
    {
        TRACE("Read back colors 0x%08x and 0x%08x close to unfiltered color, assuming no filtering\n",
              readback[6], readback[9]);
        ret = FALSE;
    }
    else
    {
        TRACE("Read back colors are 0x%08x and 0x%08x, assuming texture is filtered\n",
              readback[6], readback[9]);
        ret = TRUE;
    }

    gl_info->fbo_ops.glBindFramebuffer(GL_FRAMEBUFFER, 0);
    gl_info->fbo_ops.glDeleteFramebuffers(1, &fbo);
    gl_info->gl_ops.gl.p_glDeleteTextures(1, &tex);
    gl_info->gl_ops.gl.p_glDeleteTextures(1, &buffer);

    if (gl_info->gl_ops.gl.p_glGetError())
    {
        FIXME("Error during filtering test for format %x, returning no filtering\n", internal);
        ret = FALSE;
    }

    return ret;
}

static void init_format_filter_info(struct wined3d_gl_info *gl_info, enum wined3d_pci_vendor vendor)
{
    struct wined3d_format *format;
    unsigned int fmt_idx, i;
    static const enum wined3d_format_id fmts16[] =
    {
        WINED3DFMT_R16_FLOAT,
        WINED3DFMT_R16G16_FLOAT,
        WINED3DFMT_R16G16B16A16_FLOAT,
    };
    BOOL filtered;

    if (gl_info->supported[ARB_INTERNALFORMAT_QUERY2])
        /* This was already handled by init_format_texture_info(). */
        return;

    if(wined3d_settings.offscreen_rendering_mode != ORM_FBO)
    {
        WARN("No FBO support, or no FBO ORM, guessing filter info from GL caps\n");
        if (vendor == HW_VENDOR_NVIDIA && gl_info->supported[ARB_TEXTURE_FLOAT])
        {
            TRACE("Nvidia card with texture_float support: Assuming float16 blending\n");
            filtered = TRUE;
        }
        else if (gl_info->limits.glsl_varyings > 44)
        {
            TRACE("More than 44 GLSL varyings - assuming d3d10 card with float16 blending\n");
            filtered = TRUE;
        }
        else
        {
            TRACE("Assuming no float16 blending\n");
            filtered = FALSE;
        }

        if(filtered)
        {
            for(i = 0; i < (sizeof(fmts16) / sizeof(*fmts16)); i++)
            {
                fmt_idx = getFmtIdx(fmts16[i]);
                gl_info->formats[fmt_idx].flags |= WINED3DFMT_FLAG_FILTERING;
            }
        }
        return;
    }

    for(i = 0; i < (sizeof(fmts16) / sizeof(*fmts16)); i++)
    {
        fmt_idx = getFmtIdx(fmts16[i]);
        format = &gl_info->formats[fmt_idx];
        if (!format->glInternal) continue; /* Not supported by GL */

        filtered = check_filter(gl_info, gl_info->formats[fmt_idx].glInternal);
        if(filtered)
        {
            TRACE("Format %s supports filtering\n", debug_d3dformat(fmts16[i]));
            format->flags |= WINED3DFMT_FLAG_FILTERING;
        }
        else
        {
            TRACE("Format %s does not support filtering\n", debug_d3dformat(fmts16[i]));
        }
    }
}

static void apply_format_fixups(struct wined3d_adapter *adapter, struct wined3d_gl_info *gl_info)
{
    unsigned int i;
    int idx;

    idx = getFmtIdx(WINED3DFMT_R16_FLOAT);
    gl_info->formats[idx].color_fixup = create_color_fixup_desc(
            0, CHANNEL_SOURCE_X, 0, CHANNEL_SOURCE_ONE, 0, CHANNEL_SOURCE_ONE, 0, CHANNEL_SOURCE_W);

    idx = getFmtIdx(WINED3DFMT_R32_FLOAT);
    gl_info->formats[idx].color_fixup = create_color_fixup_desc(
            0, CHANNEL_SOURCE_X, 0, CHANNEL_SOURCE_ONE, 0, CHANNEL_SOURCE_ONE, 0, CHANNEL_SOURCE_W);

    idx = getFmtIdx(WINED3DFMT_R16G16_UNORM);
    gl_info->formats[idx].color_fixup = create_color_fixup_desc(
            0, CHANNEL_SOURCE_X, 0, CHANNEL_SOURCE_Y, 0, CHANNEL_SOURCE_ONE, 0, CHANNEL_SOURCE_W);

    idx = getFmtIdx(WINED3DFMT_R16G16_FLOAT);
    gl_info->formats[idx].color_fixup = create_color_fixup_desc(
            0, CHANNEL_SOURCE_X, 0, CHANNEL_SOURCE_Y, 0, CHANNEL_SOURCE_ONE, 0, CHANNEL_SOURCE_W);

    idx = getFmtIdx(WINED3DFMT_R32G32_FLOAT);
    gl_info->formats[idx].color_fixup = create_color_fixup_desc(
            0, CHANNEL_SOURCE_X, 0, CHANNEL_SOURCE_Y, 0, CHANNEL_SOURCE_ONE, 0, CHANNEL_SOURCE_W);

    /* V8U8 is supported natively by GL_ATI_envmap_bumpmap and GL_NV_texture_shader.
     * V16U16 is only supported by GL_NV_texture_shader. The formats need fixup if
     * their extensions are not available. GL_ATI_envmap_bumpmap is not used because
     * the only driver that implements it(fglrx) has a buggy implementation.
     *
     * V8U8 and V16U16 need a fixup of the undefined blue channel. OpenGL
     * returns 0.0 when sampling from it, DirectX 1.0. So we always have in-shader
     * conversion for this format.
     */
    if (!gl_info->supported[NV_TEXTURE_SHADER])
    {
        idx = getFmtIdx(WINED3DFMT_R8G8_SNORM);
        gl_info->formats[idx].color_fixup = create_color_fixup_desc(
                1, CHANNEL_SOURCE_X, 1, CHANNEL_SOURCE_Y, 0, CHANNEL_SOURCE_ONE, 0, CHANNEL_SOURCE_ONE);
        idx = getFmtIdx(WINED3DFMT_R16G16_SNORM);
        gl_info->formats[idx].color_fixup = create_color_fixup_desc(
                1, CHANNEL_SOURCE_X, 1, CHANNEL_SOURCE_Y, 0, CHANNEL_SOURCE_ONE, 0, CHANNEL_SOURCE_ONE);
    }
    else
    {
        idx = getFmtIdx(WINED3DFMT_R8G8_SNORM);
        gl_info->formats[idx].color_fixup = create_color_fixup_desc(
                0, CHANNEL_SOURCE_X, 0, CHANNEL_SOURCE_Y, 0, CHANNEL_SOURCE_ONE, 0, CHANNEL_SOURCE_ONE);

        idx = getFmtIdx(WINED3DFMT_R16G16_SNORM);
        gl_info->formats[idx].color_fixup = create_color_fixup_desc(
                0, CHANNEL_SOURCE_X, 0, CHANNEL_SOURCE_Y, 0, CHANNEL_SOURCE_ONE, 0, CHANNEL_SOURCE_ONE);
    }

    if (!gl_info->supported[NV_TEXTURE_SHADER])
    {
        /* If GL_NV_texture_shader is not supported, those formats are converted, incompatibly
         * with each other
         */
        idx = getFmtIdx(WINED3DFMT_R5G5_SNORM_L6_UNORM);
        gl_info->formats[idx].color_fixup = create_color_fixup_desc(
                1, CHANNEL_SOURCE_X, 1, CHANNEL_SOURCE_Z, 0, CHANNEL_SOURCE_Y, 0, CHANNEL_SOURCE_ONE);
        idx = getFmtIdx(WINED3DFMT_R8G8_SNORM_L8X8_UNORM);
        gl_info->formats[idx].color_fixup = create_color_fixup_desc(
                1, CHANNEL_SOURCE_X, 1, CHANNEL_SOURCE_Y, 0, CHANNEL_SOURCE_Z, 0, CHANNEL_SOURCE_W);
        idx = getFmtIdx(WINED3DFMT_R8G8B8A8_SNORM);
        gl_info->formats[idx].color_fixup = create_color_fixup_desc(
                1, CHANNEL_SOURCE_X, 1, CHANNEL_SOURCE_Y, 1, CHANNEL_SOURCE_Z, 1, CHANNEL_SOURCE_W);
    }
    else
    {
        /* If GL_NV_texture_shader is supported, WINED3DFMT_L6V5U5 and WINED3DFMT_X8L8V8U8
         * are converted at surface loading time, but they do not need any modification in
         * the shader, thus they are compatible with all WINED3DFMT_UNKNOWN group formats.
         * WINED3DFMT_Q8W8V8U8 doesn't even need load-time conversion
         */
    }

    if (gl_info->supported[EXT_TEXTURE_COMPRESSION_RGTC])
    {
        idx = getFmtIdx(WINED3DFMT_ATI2N);
        gl_info->formats[idx].color_fixup = create_color_fixup_desc(
                0, CHANNEL_SOURCE_Y, 0, CHANNEL_SOURCE_X, 0, CHANNEL_SOURCE_ONE, 0, CHANNEL_SOURCE_ONE);
    }
    else if (gl_info->supported[ATI_TEXTURE_COMPRESSION_3DC])
    {
        idx = getFmtIdx(WINED3DFMT_ATI2N);
        gl_info->formats[idx].color_fixup= create_color_fixup_desc(
                0, CHANNEL_SOURCE_X, 0, CHANNEL_SOURCE_W, 0, CHANNEL_SOURCE_ONE, 0, CHANNEL_SOURCE_ONE);
    }

    if (!gl_info->supported[APPLE_YCBCR_422])
    {
        idx = getFmtIdx(WINED3DFMT_YUY2);
        gl_info->formats[idx].color_fixup = create_complex_fixup_desc(COMPLEX_FIXUP_YUY2);

        idx = getFmtIdx(WINED3DFMT_UYVY);
        gl_info->formats[idx].color_fixup = create_complex_fixup_desc(COMPLEX_FIXUP_UYVY);
    }

    idx = getFmtIdx(WINED3DFMT_YV12);
    gl_info->formats[idx].flags |= WINED3DFMT_FLAG_HEIGHT_SCALE;
    gl_info->formats[idx].height_scale.numerator = 3;
    gl_info->formats[idx].height_scale.denominator = 2;
    gl_info->formats[idx].color_fixup = create_complex_fixup_desc(COMPLEX_FIXUP_YV12);

    if (gl_info->supported[ARB_FRAGMENT_PROGRAM])
    {
        idx = getFmtIdx(WINED3DFMT_P8_UINT);
        gl_info->formats[idx].color_fixup = create_complex_fixup_desc(COMPLEX_FIXUP_P8);
    }

    if (gl_info->supported[ARB_VERTEX_ARRAY_BGRA])
    {
        idx = getFmtIdx(WINED3DFMT_B8G8R8A8_UNORM);
        gl_info->formats[idx].gl_vtx_format = GL_BGRA;
    }

    if (gl_info->supported[ARB_HALF_FLOAT_VERTEX])
    {
        /* Do not change the size of the type, it is CPU side. We have to change the GPU-side information though.
         * It is the job of the vertex buffer code to make sure that the vbos have the right format */
        idx = getFmtIdx(WINED3DFMT_R16G16_FLOAT);
        gl_info->formats[idx].gl_vtx_type = GL_HALF_FLOAT; /* == GL_HALF_FLOAT_NV */

        idx = getFmtIdx(WINED3DFMT_R16G16B16A16_FLOAT);
        gl_info->formats[idx].gl_vtx_type = GL_HALF_FLOAT;
    }

    if (!gl_info->supported[ARB_HALF_FLOAT_PIXEL])
    {
        idx = getFmtIdx(WINED3DFMT_R16_FLOAT);
        gl_info->formats[idx].flags &= ~WINED3DFMT_FLAG_TEXTURE;

        idx = getFmtIdx(WINED3DFMT_R16G16_FLOAT);
        gl_info->formats[idx].flags &= ~WINED3DFMT_FLAG_TEXTURE;

        idx = getFmtIdx(WINED3DFMT_R16G16B16A16_FLOAT);
        gl_info->formats[idx].flags &= ~WINED3DFMT_FLAG_TEXTURE;
    }

    if (gl_info->quirks & WINED3D_QUIRK_BROKEN_RGBA16)
    {
        idx = getFmtIdx(WINED3DFMT_R16G16B16A16_UNORM);
        gl_info->formats[idx].flags &= ~WINED3DFMT_FLAG_TEXTURE;
    }

    /* ATI instancing hack: Although ATI cards do not support Shader Model
     * 3.0, they support instancing. To query if the card supports instancing
     * CheckDeviceFormat() with the special format MAKEFOURCC('I','N','S','T')
     * is used. Should an application check for this, provide a proper return
     * value. We can do instancing with all shader versions, but we need
     * vertex shaders.
     *
     * Additionally applications have to set the D3DRS_POINTSIZE render state
     * to MAKEFOURCC('I','N','S','T') once to enable instancing. Wined3d
     * doesn't need that and just ignores it.
     *
     * With Shader Model 3.0 capable cards Instancing 'just works' in Windows. */
    /* FIXME: This should just check the shader backend caps. */
    if (gl_info->supported[ARB_VERTEX_PROGRAM] || gl_info->supported[ARB_VERTEX_SHADER])
    {
        idx = getFmtIdx(WINED3DFMT_INST);
        gl_info->formats[idx].flags |= WINED3DFMT_FLAG_TEXTURE;
    }

    /* Depth bound test. To query if the card supports it CheckDeviceFormat()
     * with the special format MAKEFOURCC('N','V','D','B') is used. It is
     * enabled by setting D3DRS_ADAPTIVETESS_X render state to
     * MAKEFOURCC('N','V','D','B') and then controlled by setting
     * D3DRS_ADAPTIVETESS_Z (zMin) and D3DRS_ADAPTIVETESS_W (zMax) to test
     * value. */
    if (gl_info->supported[EXT_DEPTH_BOUNDS_TEST])
    {
        idx = getFmtIdx(WINED3DFMT_NVDB);
        gl_info->formats[idx].flags |= WINED3DFMT_FLAG_TEXTURE;
    }

    /* RESZ aka AMD DX9-level hack for multisampled depth buffer resolve. You query for RESZ
     * support by checking for availability of MAKEFOURCC('R','E','S','Z') surfaces with
     * RENDERTARGET usage. */
    if (gl_info->supported[ARB_FRAMEBUFFER_OBJECT])
    {
        idx = getFmtIdx(WINED3DFMT_RESZ);
        gl_info->formats[idx].flags |= WINED3DFMT_FLAG_TEXTURE | WINED3DFMT_FLAG_RENDERTARGET;
    }

    for (i = 0; i < sizeof(formats) / sizeof(*formats); ++i)
    {
        struct wined3d_format *format = &gl_info->formats[i];

        if (!(format->flags & WINED3DFMT_FLAG_TEXTURE))
            continue;

        if (!adapter->shader_backend->shader_color_fixup_supported(format->color_fixup)
                || !adapter->fragment_pipe->color_fixup_supported(format->color_fixup))
            format->flags &= ~WINED3DFMT_FLAG_TEXTURE;
    }
}

static BOOL init_format_vertex_info(struct wined3d_gl_info *gl_info)
{
    unsigned int i;

    for (i = 0; i < (sizeof(format_vertex_info) / sizeof(*format_vertex_info)); ++i)
    {
        struct wined3d_format *format;
        int fmt_idx = getFmtIdx(format_vertex_info[i].id);

        if (fmt_idx == -1)
        {
            ERR("Format %s (%#x) not found.\n",
                    debug_d3dformat(format_vertex_info[i].id), format_vertex_info[i].id);
            return FALSE;
        }

        format = &gl_info->formats[fmt_idx];
        format->emit_idx = format_vertex_info[i].emit_idx;
        format->component_count = format_vertex_info[i].component_count;
        format->gl_vtx_type = format_vertex_info[i].gl_vtx_type;
        format->gl_vtx_format = format_vertex_info[i].gl_vtx_format;
        format->gl_normalized = format_vertex_info[i].gl_normalized;
        format->component_size = format_vertex_info[i].component_size;
    }

    return TRUE;
}

BOOL initPixelFormatsNoGL(struct wined3d_gl_info *gl_info)
{
    if (!init_format_base_info(gl_info)) return FALSE;

    if (!init_format_block_info(gl_info))
    {
        HeapFree(GetProcessHeap(), 0, gl_info->formats);
        gl_info->formats = NULL;
        return FALSE;
    }

    return TRUE;
}

/* Context activation is done by the caller. */
BOOL wined3d_adapter_init_format_info(struct wined3d_adapter *adapter)
{
    struct wined3d_gl_info *gl_info = &adapter->gl_info;

    if (!init_format_base_info(gl_info)) return FALSE;

    if (!init_format_block_info(gl_info)) goto fail;
    if (!init_format_texture_info(adapter, gl_info)) goto fail;
    if (!init_format_vertex_info(gl_info)) goto fail;

    apply_format_fixups(adapter, gl_info);
    init_format_fbo_compat_info(gl_info);
    init_format_filter_info(gl_info, adapter->driver_info.vendor);

    return TRUE;

fail:
    HeapFree(GetProcessHeap(), 0, gl_info->formats);
    gl_info->formats = NULL;
    return FALSE;
}

const struct wined3d_format *wined3d_get_format(const struct wined3d_gl_info *gl_info,
        enum wined3d_format_id format_id)
{
    int idx = getFmtIdx(format_id);

    if (idx == -1)
    {
        FIXME("Can't find format %s (%#x) in the format lookup table\n",
                debug_d3dformat(format_id), format_id);
        /* Get the caller a valid pointer */
        idx = getFmtIdx(WINED3DFMT_UNKNOWN);
    }

    return &gl_info->formats[idx];
}

UINT wined3d_format_calculate_size(const struct wined3d_format *format, UINT alignment,
        UINT width, UINT height, UINT depth)
{
    UINT size;

    if (format->id == WINED3DFMT_UNKNOWN)
    {
        size = 0;
    }
    else if (format->flags & WINED3DFMT_FLAG_BLOCKS)
    {
        UINT row_block_count = (width + format->block_width - 1) / format->block_width;
        UINT row_count = (height + format->block_height - 1) / format->block_height;
        size = row_count * (((row_block_count * format->block_byte_count) + alignment - 1) & ~(alignment - 1));
    }
    else
    {
        size = height * (((width * format->byte_count) + alignment - 1) & ~(alignment - 1));
    }

    if (format->flags & WINED3DFMT_FLAG_HEIGHT_SCALE)
    {
        /* The D3D format requirements make sure that the resulting format is an integer again */
        size *= format->height_scale.numerator;
        size /= format->height_scale.denominator;
    }

    size *= depth;

    return size;
}

/*****************************************************************************
 * Trace formatting of useful values
 */
const char *debug_d3dformat(enum wined3d_format_id format_id)
{
    switch (format_id)
    {
#define FMT_TO_STR(format_id) case format_id: return #format_id
        FMT_TO_STR(WINED3DFMT_UNKNOWN);
        FMT_TO_STR(WINED3DFMT_B8G8R8_UNORM);
        FMT_TO_STR(WINED3DFMT_B5G5R5X1_UNORM);
        FMT_TO_STR(WINED3DFMT_B4G4R4A4_UNORM);
        FMT_TO_STR(WINED3DFMT_B2G3R3_UNORM);
        FMT_TO_STR(WINED3DFMT_B2G3R3A8_UNORM);
        FMT_TO_STR(WINED3DFMT_B4G4R4X4_UNORM);
        FMT_TO_STR(WINED3DFMT_R8G8B8X8_UNORM);
        FMT_TO_STR(WINED3DFMT_B10G10R10A2_UNORM);
        FMT_TO_STR(WINED3DFMT_P8_UINT_A8_UNORM);
        FMT_TO_STR(WINED3DFMT_P8_UINT);
        FMT_TO_STR(WINED3DFMT_L8_UNORM);
        FMT_TO_STR(WINED3DFMT_L8A8_UNORM);
        FMT_TO_STR(WINED3DFMT_L4A4_UNORM);
        FMT_TO_STR(WINED3DFMT_R5G5_SNORM_L6_UNORM);
        FMT_TO_STR(WINED3DFMT_R8G8_SNORM_L8X8_UNORM);
        FMT_TO_STR(WINED3DFMT_R10G11B11_SNORM);
        FMT_TO_STR(WINED3DFMT_R10G10B10_SNORM_A2_UNORM);
        FMT_TO_STR(WINED3DFMT_UYVY);
        FMT_TO_STR(WINED3DFMT_YUY2);
        FMT_TO_STR(WINED3DFMT_YV12);
        FMT_TO_STR(WINED3DFMT_DXT1);
        FMT_TO_STR(WINED3DFMT_DXT2);
        FMT_TO_STR(WINED3DFMT_DXT3);
        FMT_TO_STR(WINED3DFMT_DXT4);
        FMT_TO_STR(WINED3DFMT_DXT5);
        FMT_TO_STR(WINED3DFMT_MULTI2_ARGB8);
        FMT_TO_STR(WINED3DFMT_G8R8_G8B8);
        FMT_TO_STR(WINED3DFMT_R8G8_B8G8);
        FMT_TO_STR(WINED3DFMT_D16_LOCKABLE);
        FMT_TO_STR(WINED3DFMT_D32_UNORM);
        FMT_TO_STR(WINED3DFMT_S1_UINT_D15_UNORM);
        FMT_TO_STR(WINED3DFMT_X8D24_UNORM);
        FMT_TO_STR(WINED3DFMT_S4X4_UINT_D24_UNORM);
        FMT_TO_STR(WINED3DFMT_L16_UNORM);
        FMT_TO_STR(WINED3DFMT_S8_UINT_D24_FLOAT);
        FMT_TO_STR(WINED3DFMT_VERTEXDATA);
        FMT_TO_STR(WINED3DFMT_R8G8_SNORM_Cx);
        FMT_TO_STR(WINED3DFMT_ATI2N);
        FMT_TO_STR(WINED3DFMT_NVDB);
        FMT_TO_STR(WINED3DFMT_NVHU);
        FMT_TO_STR(WINED3DFMT_NVHS);
        FMT_TO_STR(WINED3DFMT_R32G32B32A32_TYPELESS);
        FMT_TO_STR(WINED3DFMT_R32G32B32A32_FLOAT);
        FMT_TO_STR(WINED3DFMT_R32G32B32A32_UINT);
        FMT_TO_STR(WINED3DFMT_R32G32B32A32_SINT);
        FMT_TO_STR(WINED3DFMT_R32G32B32_TYPELESS);
        FMT_TO_STR(WINED3DFMT_R32G32B32_FLOAT);
        FMT_TO_STR(WINED3DFMT_R32G32B32_UINT);
        FMT_TO_STR(WINED3DFMT_R32G32B32_SINT);
        FMT_TO_STR(WINED3DFMT_R16G16B16A16_TYPELESS);
        FMT_TO_STR(WINED3DFMT_R16G16B16A16_FLOAT);
        FMT_TO_STR(WINED3DFMT_R16G16B16A16_UNORM);
        FMT_TO_STR(WINED3DFMT_R16G16B16A16_UINT);
        FMT_TO_STR(WINED3DFMT_R16G16B16A16_SNORM);
        FMT_TO_STR(WINED3DFMT_R16G16B16A16_SINT);
        FMT_TO_STR(WINED3DFMT_R32G32_TYPELESS);
        FMT_TO_STR(WINED3DFMT_R32G32_FLOAT);
        FMT_TO_STR(WINED3DFMT_R32G32_UINT);
        FMT_TO_STR(WINED3DFMT_R32G32_SINT);
        FMT_TO_STR(WINED3DFMT_R32G8X24_TYPELESS);
        FMT_TO_STR(WINED3DFMT_D32_FLOAT_S8X24_UINT);
        FMT_TO_STR(WINED3DFMT_R32_FLOAT_X8X24_TYPELESS);
        FMT_TO_STR(WINED3DFMT_X32_TYPELESS_G8X24_UINT);
        FMT_TO_STR(WINED3DFMT_R10G10B10A2_TYPELESS);
        FMT_TO_STR(WINED3DFMT_R10G10B10A2_UNORM);
        FMT_TO_STR(WINED3DFMT_R10G10B10A2_UINT);
        FMT_TO_STR(WINED3DFMT_R10G10B10A2_SNORM);
        FMT_TO_STR(WINED3DFMT_R11G11B10_FLOAT);
        FMT_TO_STR(WINED3DFMT_R8G8B8A8_TYPELESS);
        FMT_TO_STR(WINED3DFMT_R8G8B8A8_UNORM);
        FMT_TO_STR(WINED3DFMT_R8G8B8A8_UNORM_SRGB);
        FMT_TO_STR(WINED3DFMT_R8G8B8A8_UINT);
        FMT_TO_STR(WINED3DFMT_R8G8B8A8_SNORM);
        FMT_TO_STR(WINED3DFMT_R8G8B8A8_SINT);
        FMT_TO_STR(WINED3DFMT_R16G16_TYPELESS);
        FMT_TO_STR(WINED3DFMT_R16G16_FLOAT);
        FMT_TO_STR(WINED3DFMT_R16G16_UNORM);
        FMT_TO_STR(WINED3DFMT_R16G16_UINT);
        FMT_TO_STR(WINED3DFMT_R16G16_SNORM);
        FMT_TO_STR(WINED3DFMT_R16G16_SINT);
        FMT_TO_STR(WINED3DFMT_R32_TYPELESS);
        FMT_TO_STR(WINED3DFMT_D32_FLOAT);
        FMT_TO_STR(WINED3DFMT_R32_FLOAT);
        FMT_TO_STR(WINED3DFMT_R32_UINT);
        FMT_TO_STR(WINED3DFMT_R32_SINT);
        FMT_TO_STR(WINED3DFMT_R24G8_TYPELESS);
        FMT_TO_STR(WINED3DFMT_D24_UNORM_S8_UINT);
        FMT_TO_STR(WINED3DFMT_R24_UNORM_X8_TYPELESS);
        FMT_TO_STR(WINED3DFMT_X24_TYPELESS_G8_UINT);
        FMT_TO_STR(WINED3DFMT_R8G8_TYPELESS);
        FMT_TO_STR(WINED3DFMT_R8G8_UNORM);
        FMT_TO_STR(WINED3DFMT_R8G8_UINT);
        FMT_TO_STR(WINED3DFMT_R8G8_SNORM);
        FMT_TO_STR(WINED3DFMT_R8G8_SINT);
        FMT_TO_STR(WINED3DFMT_R16_TYPELESS);
        FMT_TO_STR(WINED3DFMT_R16_FLOAT);
        FMT_TO_STR(WINED3DFMT_D16_UNORM);
        FMT_TO_STR(WINED3DFMT_R16_UNORM);
        FMT_TO_STR(WINED3DFMT_R16_UINT);
        FMT_TO_STR(WINED3DFMT_R16_SNORM);
        FMT_TO_STR(WINED3DFMT_R16_SINT);
        FMT_TO_STR(WINED3DFMT_R8_TYPELESS);
        FMT_TO_STR(WINED3DFMT_R8_UNORM);
        FMT_TO_STR(WINED3DFMT_R8_UINT);
        FMT_TO_STR(WINED3DFMT_R8_SNORM);
        FMT_TO_STR(WINED3DFMT_R8_SINT);
        FMT_TO_STR(WINED3DFMT_A8_UNORM);
        FMT_TO_STR(WINED3DFMT_R1_UNORM);
        FMT_TO_STR(WINED3DFMT_R9G9B9E5_SHAREDEXP);
        FMT_TO_STR(WINED3DFMT_R8G8_B8G8_UNORM);
        FMT_TO_STR(WINED3DFMT_G8R8_G8B8_UNORM);
        FMT_TO_STR(WINED3DFMT_BC1_TYPELESS);
        FMT_TO_STR(WINED3DFMT_BC1_UNORM);
        FMT_TO_STR(WINED3DFMT_BC1_UNORM_SRGB);
        FMT_TO_STR(WINED3DFMT_BC2_TYPELESS);
        FMT_TO_STR(WINED3DFMT_BC2_UNORM);
        FMT_TO_STR(WINED3DFMT_BC2_UNORM_SRGB);
        FMT_TO_STR(WINED3DFMT_BC3_TYPELESS);
        FMT_TO_STR(WINED3DFMT_BC3_UNORM);
        FMT_TO_STR(WINED3DFMT_BC3_UNORM_SRGB);
        FMT_TO_STR(WINED3DFMT_BC4_TYPELESS);
        FMT_TO_STR(WINED3DFMT_BC4_UNORM);
        FMT_TO_STR(WINED3DFMT_BC4_SNORM);
        FMT_TO_STR(WINED3DFMT_BC5_TYPELESS);
        FMT_TO_STR(WINED3DFMT_BC5_UNORM);
        FMT_TO_STR(WINED3DFMT_BC5_SNORM);
        FMT_TO_STR(WINED3DFMT_B5G6R5_UNORM);
        FMT_TO_STR(WINED3DFMT_B5G5R5A1_UNORM);
        FMT_TO_STR(WINED3DFMT_B8G8R8A8_UNORM);
        FMT_TO_STR(WINED3DFMT_B8G8R8X8_UNORM);
        FMT_TO_STR(WINED3DFMT_INTZ);
        FMT_TO_STR(WINED3DFMT_RESZ);
        FMT_TO_STR(WINED3DFMT_NULL);
        FMT_TO_STR(WINED3DFMT_R16);
        FMT_TO_STR(WINED3DFMT_AL16);
#undef FMT_TO_STR
        default:
        {
            char fourcc[5];
            fourcc[0] = (char)(format_id);
            fourcc[1] = (char)(format_id >> 8);
            fourcc[2] = (char)(format_id >> 16);
            fourcc[3] = (char)(format_id >> 24);
            fourcc[4] = 0;
            if (isprint(fourcc[0]) && isprint(fourcc[1]) && isprint(fourcc[2]) && isprint(fourcc[3]))
                FIXME("Unrecognized %#x (as fourcc: %s) WINED3DFORMAT!\n", format_id, fourcc);
            else
                FIXME("Unrecognized %#x WINED3DFORMAT!\n", format_id);
        }
        return "unrecognized";
    }
}

const char *debug_d3ddevicetype(enum wined3d_device_type device_type)
{
    switch (device_type)
    {
#define DEVTYPE_TO_STR(dev) case dev: return #dev
        DEVTYPE_TO_STR(WINED3D_DEVICE_TYPE_HAL);
        DEVTYPE_TO_STR(WINED3D_DEVICE_TYPE_REF);
        DEVTYPE_TO_STR(WINED3D_DEVICE_TYPE_SW);
#undef DEVTYPE_TO_STR
        default:
            FIXME("Unrecognized device type %#x.\n", device_type);
            return "unrecognized";
    }
}

const char *debug_d3dusage(DWORD usage)
{
    char buf[333];

    buf[0] = '\0';
#define WINED3DUSAGE_TO_STR(u) if (usage & u) { strcat(buf, " | "#u); usage &= ~u; }
    WINED3DUSAGE_TO_STR(WINED3DUSAGE_RENDERTARGET);
    WINED3DUSAGE_TO_STR(WINED3DUSAGE_DEPTHSTENCIL);
    WINED3DUSAGE_TO_STR(WINED3DUSAGE_WRITEONLY);
    WINED3DUSAGE_TO_STR(WINED3DUSAGE_SOFTWAREPROCESSING);
    WINED3DUSAGE_TO_STR(WINED3DUSAGE_DONOTCLIP);
    WINED3DUSAGE_TO_STR(WINED3DUSAGE_POINTS);
    WINED3DUSAGE_TO_STR(WINED3DUSAGE_RTPATCHES);
    WINED3DUSAGE_TO_STR(WINED3DUSAGE_NPATCHES);
    WINED3DUSAGE_TO_STR(WINED3DUSAGE_DYNAMIC);
    WINED3DUSAGE_TO_STR(WINED3DUSAGE_AUTOGENMIPMAP);
    WINED3DUSAGE_TO_STR(WINED3DUSAGE_DMAP);
    WINED3DUSAGE_TO_STR(WINED3DUSAGE_STATICDECL);
    WINED3DUSAGE_TO_STR(WINED3DUSAGE_OVERLAY);
#undef WINED3DUSAGE_TO_STR
    if (usage) FIXME("Unrecognized usage flag(s) %#x\n", usage);

    return buf[0] ? wine_dbg_sprintf("%s", &buf[3]) : "0";
}

const char *debug_d3dusagequery(DWORD usagequery)
{
    char buf[238];

    buf[0] = '\0';
#define WINED3DUSAGEQUERY_TO_STR(u) if (usagequery & u) { strcat(buf, " | "#u); usagequery &= ~u; }
    WINED3DUSAGEQUERY_TO_STR(WINED3DUSAGE_QUERY_FILTER);
    WINED3DUSAGEQUERY_TO_STR(WINED3DUSAGE_QUERY_LEGACYBUMPMAP);
    WINED3DUSAGEQUERY_TO_STR(WINED3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING);
    WINED3DUSAGEQUERY_TO_STR(WINED3DUSAGE_QUERY_SRGBREAD);
    WINED3DUSAGEQUERY_TO_STR(WINED3DUSAGE_QUERY_SRGBWRITE);
    WINED3DUSAGEQUERY_TO_STR(WINED3DUSAGE_QUERY_VERTEXTEXTURE);
    WINED3DUSAGEQUERY_TO_STR(WINED3DUSAGE_QUERY_WRAPANDMIP);
#undef WINED3DUSAGEQUERY_TO_STR
    if (usagequery) FIXME("Unrecognized usage query flag(s) %#x\n", usagequery);

    return buf[0] ? wine_dbg_sprintf("%s", &buf[3]) : "0";
}

const char *debug_d3ddeclmethod(enum wined3d_decl_method method)
{
    switch (method)
    {
#define WINED3DDECLMETHOD_TO_STR(u) case u: return #u
        WINED3DDECLMETHOD_TO_STR(WINED3D_DECL_METHOD_DEFAULT);
        WINED3DDECLMETHOD_TO_STR(WINED3D_DECL_METHOD_PARTIAL_U);
        WINED3DDECLMETHOD_TO_STR(WINED3D_DECL_METHOD_PARTIAL_V);
        WINED3DDECLMETHOD_TO_STR(WINED3D_DECL_METHOD_CROSS_UV);
        WINED3DDECLMETHOD_TO_STR(WINED3D_DECL_METHOD_UV);
        WINED3DDECLMETHOD_TO_STR(WINED3D_DECL_METHOD_LOOKUP);
        WINED3DDECLMETHOD_TO_STR(WINED3D_DECL_METHOD_LOOKUP_PRESAMPLED);
#undef WINED3DDECLMETHOD_TO_STR
        default:
            FIXME("Unrecognized declaration method %#x.\n", method);
            return "unrecognized";
    }
}

const char *debug_d3ddeclusage(enum wined3d_decl_usage usage)
{
    switch (usage)
    {
#define WINED3DDECLUSAGE_TO_STR(u) case u: return #u
        WINED3DDECLUSAGE_TO_STR(WINED3D_DECL_USAGE_POSITION);
        WINED3DDECLUSAGE_TO_STR(WINED3D_DECL_USAGE_BLEND_WEIGHT);
        WINED3DDECLUSAGE_TO_STR(WINED3D_DECL_USAGE_BLEND_INDICES);
        WINED3DDECLUSAGE_TO_STR(WINED3D_DECL_USAGE_NORMAL);
        WINED3DDECLUSAGE_TO_STR(WINED3D_DECL_USAGE_PSIZE);
        WINED3DDECLUSAGE_TO_STR(WINED3D_DECL_USAGE_TEXCOORD);
        WINED3DDECLUSAGE_TO_STR(WINED3D_DECL_USAGE_TANGENT);
        WINED3DDECLUSAGE_TO_STR(WINED3D_DECL_USAGE_BINORMAL);
        WINED3DDECLUSAGE_TO_STR(WINED3D_DECL_USAGE_TESS_FACTOR);
        WINED3DDECLUSAGE_TO_STR(WINED3D_DECL_USAGE_POSITIONT);
        WINED3DDECLUSAGE_TO_STR(WINED3D_DECL_USAGE_COLOR);
        WINED3DDECLUSAGE_TO_STR(WINED3D_DECL_USAGE_FOG);
        WINED3DDECLUSAGE_TO_STR(WINED3D_DECL_USAGE_DEPTH);
        WINED3DDECLUSAGE_TO_STR(WINED3D_DECL_USAGE_SAMPLE);
#undef WINED3DDECLUSAGE_TO_STR
        default:
            FIXME("Unrecognized %u declaration usage!\n", usage);
            return "unrecognized";
    }
}

const char *debug_d3dresourcetype(enum wined3d_resource_type resource_type)
{
    switch (resource_type)
    {
#define RES_TO_STR(res) case res: return #res
        RES_TO_STR(WINED3D_RTYPE_SURFACE);
        RES_TO_STR(WINED3D_RTYPE_VOLUME);
        RES_TO_STR(WINED3D_RTYPE_TEXTURE);
        RES_TO_STR(WINED3D_RTYPE_VOLUME_TEXTURE);
        RES_TO_STR(WINED3D_RTYPE_CUBE_TEXTURE);
        RES_TO_STR(WINED3D_RTYPE_BUFFER);
#undef  RES_TO_STR
        default:
            FIXME("Unrecognized resource type %#x.\n", resource_type);
            return "unrecognized";
    }
}

const char *debug_d3dprimitivetype(enum wined3d_primitive_type primitive_type)
{
    switch (primitive_type)
    {
#define PRIM_TO_STR(prim) case prim: return #prim
        PRIM_TO_STR(WINED3D_PT_UNDEFINED);
        PRIM_TO_STR(WINED3D_PT_POINTLIST);
        PRIM_TO_STR(WINED3D_PT_LINELIST);
        PRIM_TO_STR(WINED3D_PT_LINESTRIP);
        PRIM_TO_STR(WINED3D_PT_TRIANGLELIST);
        PRIM_TO_STR(WINED3D_PT_TRIANGLESTRIP);
        PRIM_TO_STR(WINED3D_PT_TRIANGLEFAN);
        PRIM_TO_STR(WINED3D_PT_LINELIST_ADJ);
        PRIM_TO_STR(WINED3D_PT_LINESTRIP_ADJ);
        PRIM_TO_STR(WINED3D_PT_TRIANGLELIST_ADJ);
        PRIM_TO_STR(WINED3D_PT_TRIANGLESTRIP_ADJ);
#undef  PRIM_TO_STR
        default:
            FIXME("Unrecognized %u primitive type!\n", primitive_type);
            return "unrecognized";
    }
}

const char *debug_d3drenderstate(enum wined3d_render_state state)
{
    switch (state)
    {
#define D3DSTATE_TO_STR(u) case u: return #u
        D3DSTATE_TO_STR(WINED3D_RS_ANTIALIAS);
        D3DSTATE_TO_STR(WINED3D_RS_TEXTUREPERSPECTIVE);
        D3DSTATE_TO_STR(WINED3D_RS_WRAPU);
        D3DSTATE_TO_STR(WINED3D_RS_WRAPV);
        D3DSTATE_TO_STR(WINED3D_RS_ZENABLE);
        D3DSTATE_TO_STR(WINED3D_RS_FILLMODE);
        D3DSTATE_TO_STR(WINED3D_RS_SHADEMODE);
        D3DSTATE_TO_STR(WINED3D_RS_LINEPATTERN);
        D3DSTATE_TO_STR(WINED3D_RS_MONOENABLE);
        D3DSTATE_TO_STR(WINED3D_RS_ROP2);
        D3DSTATE_TO_STR(WINED3D_RS_PLANEMASK);
        D3DSTATE_TO_STR(WINED3D_RS_ZWRITEENABLE);
        D3DSTATE_TO_STR(WINED3D_RS_ALPHATESTENABLE);
        D3DSTATE_TO_STR(WINED3D_RS_LASTPIXEL);
        D3DSTATE_TO_STR(WINED3D_RS_SRCBLEND);
        D3DSTATE_TO_STR(WINED3D_RS_DESTBLEND);
        D3DSTATE_TO_STR(WINED3D_RS_CULLMODE);
        D3DSTATE_TO_STR(WINED3D_RS_ZFUNC);
        D3DSTATE_TO_STR(WINED3D_RS_ALPHAREF);
        D3DSTATE_TO_STR(WINED3D_RS_ALPHAFUNC);
        D3DSTATE_TO_STR(WINED3D_RS_DITHERENABLE);
        D3DSTATE_TO_STR(WINED3D_RS_ALPHABLENDENABLE);
        D3DSTATE_TO_STR(WINED3D_RS_FOGENABLE);
        D3DSTATE_TO_STR(WINED3D_RS_SPECULARENABLE);
        D3DSTATE_TO_STR(WINED3D_RS_ZVISIBLE);
        D3DSTATE_TO_STR(WINED3D_RS_SUBPIXEL);
        D3DSTATE_TO_STR(WINED3D_RS_SUBPIXELX);
        D3DSTATE_TO_STR(WINED3D_RS_STIPPLEDALPHA);
        D3DSTATE_TO_STR(WINED3D_RS_FOGCOLOR);
        D3DSTATE_TO_STR(WINED3D_RS_FOGTABLEMODE);
        D3DSTATE_TO_STR(WINED3D_RS_FOGSTART);
        D3DSTATE_TO_STR(WINED3D_RS_FOGEND);
        D3DSTATE_TO_STR(WINED3D_RS_FOGDENSITY);
        D3DSTATE_TO_STR(WINED3D_RS_STIPPLEENABLE);
        D3DSTATE_TO_STR(WINED3D_RS_EDGEANTIALIAS);
        D3DSTATE_TO_STR(WINED3D_RS_COLORKEYENABLE);
        D3DSTATE_TO_STR(WINED3D_RS_MIPMAPLODBIAS);
        D3DSTATE_TO_STR(WINED3D_RS_RANGEFOGENABLE);
        D3DSTATE_TO_STR(WINED3D_RS_ANISOTROPY);
        D3DSTATE_TO_STR(WINED3D_RS_FLUSHBATCH);
        D3DSTATE_TO_STR(WINED3D_RS_TRANSLUCENTSORTINDEPENDENT);
        D3DSTATE_TO_STR(WINED3D_RS_STENCILENABLE);
        D3DSTATE_TO_STR(WINED3D_RS_STENCILFAIL);
        D3DSTATE_TO_STR(WINED3D_RS_STENCILZFAIL);
        D3DSTATE_TO_STR(WINED3D_RS_STENCILPASS);
        D3DSTATE_TO_STR(WINED3D_RS_STENCILFUNC);
        D3DSTATE_TO_STR(WINED3D_RS_STENCILREF);
        D3DSTATE_TO_STR(WINED3D_RS_STENCILMASK);
        D3DSTATE_TO_STR(WINED3D_RS_STENCILWRITEMASK);
        D3DSTATE_TO_STR(WINED3D_RS_TEXTUREFACTOR);
        D3DSTATE_TO_STR(WINED3D_RS_WRAP0);
        D3DSTATE_TO_STR(WINED3D_RS_WRAP1);
        D3DSTATE_TO_STR(WINED3D_RS_WRAP2);
        D3DSTATE_TO_STR(WINED3D_RS_WRAP3);
        D3DSTATE_TO_STR(WINED3D_RS_WRAP4);
        D3DSTATE_TO_STR(WINED3D_RS_WRAP5);
        D3DSTATE_TO_STR(WINED3D_RS_WRAP6);
        D3DSTATE_TO_STR(WINED3D_RS_WRAP7);
        D3DSTATE_TO_STR(WINED3D_RS_CLIPPING);
        D3DSTATE_TO_STR(WINED3D_RS_LIGHTING);
        D3DSTATE_TO_STR(WINED3D_RS_EXTENTS);
        D3DSTATE_TO_STR(WINED3D_RS_AMBIENT);
        D3DSTATE_TO_STR(WINED3D_RS_FOGVERTEXMODE);
        D3DSTATE_TO_STR(WINED3D_RS_COLORVERTEX);
        D3DSTATE_TO_STR(WINED3D_RS_LOCALVIEWER);
        D3DSTATE_TO_STR(WINED3D_RS_NORMALIZENORMALS);
        D3DSTATE_TO_STR(WINED3D_RS_COLORKEYBLENDENABLE);
        D3DSTATE_TO_STR(WINED3D_RS_DIFFUSEMATERIALSOURCE);
        D3DSTATE_TO_STR(WINED3D_RS_SPECULARMATERIALSOURCE);
        D3DSTATE_TO_STR(WINED3D_RS_AMBIENTMATERIALSOURCE);
        D3DSTATE_TO_STR(WINED3D_RS_EMISSIVEMATERIALSOURCE);
        D3DSTATE_TO_STR(WINED3D_RS_VERTEXBLEND);
        D3DSTATE_TO_STR(WINED3D_RS_CLIPPLANEENABLE);
        D3DSTATE_TO_STR(WINED3D_RS_SOFTWAREVERTEXPROCESSING);
        D3DSTATE_TO_STR(WINED3D_RS_POINTSIZE);
        D3DSTATE_TO_STR(WINED3D_RS_POINTSIZE_MIN);
        D3DSTATE_TO_STR(WINED3D_RS_POINTSPRITEENABLE);
        D3DSTATE_TO_STR(WINED3D_RS_POINTSCALEENABLE);
        D3DSTATE_TO_STR(WINED3D_RS_POINTSCALE_A);
        D3DSTATE_TO_STR(WINED3D_RS_POINTSCALE_B);
        D3DSTATE_TO_STR(WINED3D_RS_POINTSCALE_C);
        D3DSTATE_TO_STR(WINED3D_RS_MULTISAMPLEANTIALIAS);
        D3DSTATE_TO_STR(WINED3D_RS_MULTISAMPLEMASK);
        D3DSTATE_TO_STR(WINED3D_RS_PATCHEDGESTYLE);
        D3DSTATE_TO_STR(WINED3D_RS_PATCHSEGMENTS);
        D3DSTATE_TO_STR(WINED3D_RS_DEBUGMONITORTOKEN);
        D3DSTATE_TO_STR(WINED3D_RS_POINTSIZE_MAX);
        D3DSTATE_TO_STR(WINED3D_RS_INDEXEDVERTEXBLENDENABLE);
        D3DSTATE_TO_STR(WINED3D_RS_COLORWRITEENABLE);
        D3DSTATE_TO_STR(WINED3D_RS_TWEENFACTOR);
        D3DSTATE_TO_STR(WINED3D_RS_BLENDOP);
        D3DSTATE_TO_STR(WINED3D_RS_POSITIONDEGREE);
        D3DSTATE_TO_STR(WINED3D_RS_NORMALDEGREE);
        D3DSTATE_TO_STR(WINED3D_RS_SCISSORTESTENABLE);
        D3DSTATE_TO_STR(WINED3D_RS_SLOPESCALEDEPTHBIAS);
        D3DSTATE_TO_STR(WINED3D_RS_ANTIALIASEDLINEENABLE);
        D3DSTATE_TO_STR(WINED3D_RS_MINTESSELLATIONLEVEL);
        D3DSTATE_TO_STR(WINED3D_RS_MAXTESSELLATIONLEVEL);
        D3DSTATE_TO_STR(WINED3D_RS_ADAPTIVETESS_X);
        D3DSTATE_TO_STR(WINED3D_RS_ADAPTIVETESS_Y);
        D3DSTATE_TO_STR(WINED3D_RS_ADAPTIVETESS_Z);
        D3DSTATE_TO_STR(WINED3D_RS_ADAPTIVETESS_W);
        D3DSTATE_TO_STR(WINED3D_RS_ENABLEADAPTIVETESSELLATION);
        D3DSTATE_TO_STR(WINED3D_RS_TWOSIDEDSTENCILMODE);
        D3DSTATE_TO_STR(WINED3D_RS_CCW_STENCILFAIL);
        D3DSTATE_TO_STR(WINED3D_RS_CCW_STENCILZFAIL);
        D3DSTATE_TO_STR(WINED3D_RS_CCW_STENCILPASS);
        D3DSTATE_TO_STR(WINED3D_RS_CCW_STENCILFUNC);
        D3DSTATE_TO_STR(WINED3D_RS_COLORWRITEENABLE1);
        D3DSTATE_TO_STR(WINED3D_RS_COLORWRITEENABLE2);
        D3DSTATE_TO_STR(WINED3D_RS_COLORWRITEENABLE3);
        D3DSTATE_TO_STR(WINED3D_RS_BLENDFACTOR);
        D3DSTATE_TO_STR(WINED3D_RS_SRGBWRITEENABLE);
        D3DSTATE_TO_STR(WINED3D_RS_DEPTHBIAS);
        D3DSTATE_TO_STR(WINED3D_RS_WRAP8);
        D3DSTATE_TO_STR(WINED3D_RS_WRAP9);
        D3DSTATE_TO_STR(WINED3D_RS_WRAP10);
        D3DSTATE_TO_STR(WINED3D_RS_WRAP11);
        D3DSTATE_TO_STR(WINED3D_RS_WRAP12);
        D3DSTATE_TO_STR(WINED3D_RS_WRAP13);
        D3DSTATE_TO_STR(WINED3D_RS_WRAP14);
        D3DSTATE_TO_STR(WINED3D_RS_WRAP15);
        D3DSTATE_TO_STR(WINED3D_RS_SEPARATEALPHABLENDENABLE);
        D3DSTATE_TO_STR(WINED3D_RS_SRCBLENDALPHA);
        D3DSTATE_TO_STR(WINED3D_RS_DESTBLENDALPHA);
        D3DSTATE_TO_STR(WINED3D_RS_BLENDOPALPHA);
#undef D3DSTATE_TO_STR
        default:
            FIXME("Unrecognized %u render state!\n", state);
            return "unrecognized";
    }
}

const char *debug_d3dsamplerstate(enum wined3d_sampler_state state)
{
    switch (state)
    {
#define D3DSTATE_TO_STR(u) case u: return #u
        D3DSTATE_TO_STR(WINED3D_SAMP_BORDER_COLOR);
        D3DSTATE_TO_STR(WINED3D_SAMP_ADDRESS_U);
        D3DSTATE_TO_STR(WINED3D_SAMP_ADDRESS_V);
        D3DSTATE_TO_STR(WINED3D_SAMP_ADDRESS_W);
        D3DSTATE_TO_STR(WINED3D_SAMP_MAG_FILTER);
        D3DSTATE_TO_STR(WINED3D_SAMP_MIN_FILTER);
        D3DSTATE_TO_STR(WINED3D_SAMP_MIP_FILTER);
        D3DSTATE_TO_STR(WINED3D_SAMP_MIPMAP_LOD_BIAS);
        D3DSTATE_TO_STR(WINED3D_SAMP_MAX_MIP_LEVEL);
        D3DSTATE_TO_STR(WINED3D_SAMP_MAX_ANISOTROPY);
        D3DSTATE_TO_STR(WINED3D_SAMP_SRGB_TEXTURE);
        D3DSTATE_TO_STR(WINED3D_SAMP_ELEMENT_INDEX);
        D3DSTATE_TO_STR(WINED3D_SAMP_DMAP_OFFSET);
#undef D3DSTATE_TO_STR
        default:
            FIXME("Unrecognized %u sampler state!\n", state);
            return "unrecognized";
    }
}

const char *debug_d3dtexturefiltertype(enum wined3d_texture_filter_type filter_type)
{
    switch (filter_type)
    {
#define D3DTEXTUREFILTERTYPE_TO_STR(u) case u: return #u
        D3DTEXTUREFILTERTYPE_TO_STR(WINED3D_TEXF_NONE);
        D3DTEXTUREFILTERTYPE_TO_STR(WINED3D_TEXF_POINT);
        D3DTEXTUREFILTERTYPE_TO_STR(WINED3D_TEXF_LINEAR);
        D3DTEXTUREFILTERTYPE_TO_STR(WINED3D_TEXF_ANISOTROPIC);
        D3DTEXTUREFILTERTYPE_TO_STR(WINED3D_TEXF_FLAT_CUBIC);
        D3DTEXTUREFILTERTYPE_TO_STR(WINED3D_TEXF_GAUSSIAN_CUBIC);
        D3DTEXTUREFILTERTYPE_TO_STR(WINED3D_TEXF_PYRAMIDAL_QUAD);
        D3DTEXTUREFILTERTYPE_TO_STR(WINED3D_TEXF_GAUSSIAN_QUAD);
#undef D3DTEXTUREFILTERTYPE_TO_STR
        default:
            FIXME("Unrecognied texture filter type 0x%08x.\n", filter_type);
            return "unrecognized";
    }
}

const char *debug_d3dtexturestate(enum wined3d_texture_stage_state state)
{
    switch (state)
    {
#define D3DSTATE_TO_STR(u) case u: return #u
        D3DSTATE_TO_STR(WINED3D_TSS_COLOR_OP);
        D3DSTATE_TO_STR(WINED3D_TSS_COLOR_ARG1);
        D3DSTATE_TO_STR(WINED3D_TSS_COLOR_ARG2);
        D3DSTATE_TO_STR(WINED3D_TSS_ALPHA_OP);
        D3DSTATE_TO_STR(WINED3D_TSS_ALPHA_ARG1);
        D3DSTATE_TO_STR(WINED3D_TSS_ALPHA_ARG2);
        D3DSTATE_TO_STR(WINED3D_TSS_BUMPENV_MAT00);
        D3DSTATE_TO_STR(WINED3D_TSS_BUMPENV_MAT01);
        D3DSTATE_TO_STR(WINED3D_TSS_BUMPENV_MAT10);
        D3DSTATE_TO_STR(WINED3D_TSS_BUMPENV_MAT11);
        D3DSTATE_TO_STR(WINED3D_TSS_TEXCOORD_INDEX);
        D3DSTATE_TO_STR(WINED3D_TSS_BUMPENV_LSCALE);
        D3DSTATE_TO_STR(WINED3D_TSS_BUMPENV_LOFFSET);
        D3DSTATE_TO_STR(WINED3D_TSS_TEXTURE_TRANSFORM_FLAGS);
        D3DSTATE_TO_STR(WINED3D_TSS_COLOR_ARG0);
        D3DSTATE_TO_STR(WINED3D_TSS_ALPHA_ARG0);
        D3DSTATE_TO_STR(WINED3D_TSS_RESULT_ARG);
        D3DSTATE_TO_STR(WINED3D_TSS_CONSTANT);
#undef D3DSTATE_TO_STR
        default:
            FIXME("Unrecognized %u texture state!\n", state);
            return "unrecognized";
    }
}

const char *debug_d3dtop(enum wined3d_texture_op d3dtop)
{
    switch (d3dtop)
    {
#define D3DTOP_TO_STR(u) case u: return #u
        D3DTOP_TO_STR(WINED3D_TOP_DISABLE);
        D3DTOP_TO_STR(WINED3D_TOP_SELECT_ARG1);
        D3DTOP_TO_STR(WINED3D_TOP_SELECT_ARG2);
        D3DTOP_TO_STR(WINED3D_TOP_MODULATE);
        D3DTOP_TO_STR(WINED3D_TOP_MODULATE_2X);
        D3DTOP_TO_STR(WINED3D_TOP_MODULATE_4X);
        D3DTOP_TO_STR(WINED3D_TOP_ADD);
        D3DTOP_TO_STR(WINED3D_TOP_ADD_SIGNED);
        D3DTOP_TO_STR(WINED3D_TOP_ADD_SIGNED_2X);
        D3DTOP_TO_STR(WINED3D_TOP_SUBTRACT);
        D3DTOP_TO_STR(WINED3D_TOP_ADD_SMOOTH);
        D3DTOP_TO_STR(WINED3D_TOP_BLEND_DIFFUSE_ALPHA);
        D3DTOP_TO_STR(WINED3D_TOP_BLEND_TEXTURE_ALPHA);
        D3DTOP_TO_STR(WINED3D_TOP_BLEND_FACTOR_ALPHA);
        D3DTOP_TO_STR(WINED3D_TOP_BLEND_TEXTURE_ALPHA_PM);
        D3DTOP_TO_STR(WINED3D_TOP_BLEND_CURRENT_ALPHA);
        D3DTOP_TO_STR(WINED3D_TOP_PREMODULATE);
        D3DTOP_TO_STR(WINED3D_TOP_MODULATE_ALPHA_ADD_COLOR);
        D3DTOP_TO_STR(WINED3D_TOP_MODULATE_COLOR_ADD_ALPHA);
        D3DTOP_TO_STR(WINED3D_TOP_MODULATE_INVALPHA_ADD_COLOR);
        D3DTOP_TO_STR(WINED3D_TOP_MODULATE_INVCOLOR_ADD_ALPHA);
        D3DTOP_TO_STR(WINED3D_TOP_BUMPENVMAP);
        D3DTOP_TO_STR(WINED3D_TOP_BUMPENVMAP_LUMINANCE);
        D3DTOP_TO_STR(WINED3D_TOP_DOTPRODUCT3);
        D3DTOP_TO_STR(WINED3D_TOP_MULTIPLY_ADD);
        D3DTOP_TO_STR(WINED3D_TOP_LERP);
#undef D3DTOP_TO_STR
        default:
            FIXME("Unrecognized texture op %#x.\n", d3dtop);
            return "unrecognized";
    }
}

const char *debug_d3dtstype(enum wined3d_transform_state tstype)
{
    switch (tstype)
    {
#define TSTYPE_TO_STR(tstype) case tstype: return #tstype
    TSTYPE_TO_STR(WINED3D_TS_VIEW);
    TSTYPE_TO_STR(WINED3D_TS_PROJECTION);
    TSTYPE_TO_STR(WINED3D_TS_TEXTURE0);
    TSTYPE_TO_STR(WINED3D_TS_TEXTURE1);
    TSTYPE_TO_STR(WINED3D_TS_TEXTURE2);
    TSTYPE_TO_STR(WINED3D_TS_TEXTURE3);
    TSTYPE_TO_STR(WINED3D_TS_TEXTURE4);
    TSTYPE_TO_STR(WINED3D_TS_TEXTURE5);
    TSTYPE_TO_STR(WINED3D_TS_TEXTURE6);
    TSTYPE_TO_STR(WINED3D_TS_TEXTURE7);
    TSTYPE_TO_STR(WINED3D_TS_WORLD_MATRIX(0));
#undef TSTYPE_TO_STR
    default:
        if (tstype > 256 && tstype < 512)
        {
            FIXME("WINED3D_TS_WORLD_MATRIX(%u). 1..255 not currently supported.\n", tstype);
            return ("WINED3D_TS_WORLD_MATRIX > 0");
        }
        FIXME("Unrecognized transform state %#x.\n", tstype);
        return "unrecognized";
    }
}

const char *debug_d3dstate(DWORD state)
{
    if (STATE_IS_RENDER(state))
        return wine_dbg_sprintf("STATE_RENDER(%s)", debug_d3drenderstate(state - STATE_RENDER(0)));
    if (STATE_IS_TEXTURESTAGE(state))
    {
        DWORD texture_stage = (state - STATE_TEXTURESTAGE(0, 0)) / (WINED3D_HIGHEST_TEXTURE_STATE + 1);
        DWORD texture_state = state - STATE_TEXTURESTAGE(texture_stage, 0);
        return wine_dbg_sprintf("STATE_TEXTURESTAGE(%#x, %s)",
                texture_stage, debug_d3dtexturestate(texture_state));
    }
    if (STATE_IS_SAMPLER(state))
        return wine_dbg_sprintf("STATE_SAMPLER(%#x)", state - STATE_SAMPLER(0));
    if (STATE_IS_PIXELSHADER(state))
        return "STATE_PIXELSHADER";
    if (STATE_IS_TRANSFORM(state))
        return wine_dbg_sprintf("STATE_TRANSFORM(%s)", debug_d3dtstype(state - STATE_TRANSFORM(0)));
    if (STATE_IS_STREAMSRC(state))
        return "STATE_STREAMSRC";
    if (STATE_IS_INDEXBUFFER(state))
        return "STATE_INDEXBUFFER";
    if (STATE_IS_VDECL(state))
        return "STATE_VDECL";
    if (STATE_IS_VSHADER(state))
        return "STATE_VSHADER";
    if (STATE_IS_GEOMETRY_SHADER(state))
        return "STATE_GEOMETRY_SHADER";
    if (STATE_IS_VIEWPORT(state))
        return "STATE_VIEWPORT";
    if (STATE_IS_LIGHT_TYPE(state))
        return "STATE_LIGHT_TYPE";
    if (STATE_IS_ACTIVELIGHT(state))
        return wine_dbg_sprintf("STATE_ACTIVELIGHT(%#x)", state - STATE_ACTIVELIGHT(0));
    if (STATE_IS_SCISSORRECT(state))
        return "STATE_SCISSORRECT";
    if (STATE_IS_CLIPPLANE(state))
        return wine_dbg_sprintf("STATE_CLIPPLANE(%#x)", state - STATE_CLIPPLANE(0));
    if (STATE_IS_MATERIAL(state))
        return "STATE_MATERIAL";
    if (STATE_IS_FRONTFACE(state))
        return "STATE_FRONTFACE";
    if (STATE_IS_POINTSPRITECOORDORIGIN(state))
        return "STATE_POINTSPRITECOORDORIGIN";
    if (STATE_IS_BASEVERTEXINDEX(state))
        return "STATE_BASEVERTEXINDEX";
    if (STATE_IS_FRAMEBUFFER(state))
        return "STATE_FRAMEBUFFER";
    if (STATE_IS_POINT_SIZE_ENABLE(state))
        return "STATE_POINT_SIZE_ENABLE";

    return wine_dbg_sprintf("UNKNOWN_STATE(%#x)", state);
}

const char *debug_d3dpool(enum wined3d_pool pool)
{
    switch (pool)
    {
#define POOL_TO_STR(p) case p: return #p
        POOL_TO_STR(WINED3D_POOL_DEFAULT);
        POOL_TO_STR(WINED3D_POOL_MANAGED);
        POOL_TO_STR(WINED3D_POOL_SYSTEM_MEM);
        POOL_TO_STR(WINED3D_POOL_SCRATCH);
#undef  POOL_TO_STR
        default:
            FIXME("Unrecognized pool %#x.\n", pool);
            return "unrecognized";
    }
}

const char *debug_fbostatus(GLenum status) {
    switch(status) {
#define FBOSTATUS_TO_STR(u) case u: return #u
        FBOSTATUS_TO_STR(GL_FRAMEBUFFER_COMPLETE);
        FBOSTATUS_TO_STR(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
        FBOSTATUS_TO_STR(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
        FBOSTATUS_TO_STR(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT);
        FBOSTATUS_TO_STR(GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT);
        FBOSTATUS_TO_STR(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER);
        FBOSTATUS_TO_STR(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER);
        FBOSTATUS_TO_STR(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE);
        FBOSTATUS_TO_STR(GL_FRAMEBUFFER_UNSUPPORTED);
        FBOSTATUS_TO_STR(GL_FRAMEBUFFER_UNDEFINED);
#undef FBOSTATUS_TO_STR
        default:
            FIXME("Unrecognied FBO status 0x%08x\n", status);
            return "unrecognized";
    }
}

const char *debug_glerror(GLenum error) {
    switch(error) {
#define GLERROR_TO_STR(u) case u: return #u
        GLERROR_TO_STR(GL_NO_ERROR);
        GLERROR_TO_STR(GL_INVALID_ENUM);
        GLERROR_TO_STR(GL_INVALID_VALUE);
        GLERROR_TO_STR(GL_INVALID_OPERATION);
        GLERROR_TO_STR(GL_STACK_OVERFLOW);
        GLERROR_TO_STR(GL_STACK_UNDERFLOW);
        GLERROR_TO_STR(GL_OUT_OF_MEMORY);
        GLERROR_TO_STR(GL_INVALID_FRAMEBUFFER_OPERATION);
#undef GLERROR_TO_STR
        default:
            FIXME("Unrecognied GL error 0x%08x\n", error);
            return "unrecognized";
    }
}

static const char *debug_fixup_channel_source(enum fixup_channel_source source)
{
    switch(source)
    {
#define WINED3D_TO_STR(x) case x: return #x
        WINED3D_TO_STR(CHANNEL_SOURCE_ZERO);
        WINED3D_TO_STR(CHANNEL_SOURCE_ONE);
        WINED3D_TO_STR(CHANNEL_SOURCE_X);
        WINED3D_TO_STR(CHANNEL_SOURCE_Y);
        WINED3D_TO_STR(CHANNEL_SOURCE_Z);
        WINED3D_TO_STR(CHANNEL_SOURCE_W);
        WINED3D_TO_STR(CHANNEL_SOURCE_COMPLEX0);
        WINED3D_TO_STR(CHANNEL_SOURCE_COMPLEX1);
#undef WINED3D_TO_STR
        default:
            FIXME("Unrecognized fixup_channel_source %#x\n", source);
            return "unrecognized";
    }
}

static const char *debug_complex_fixup(enum complex_fixup fixup)
{
    switch(fixup)
    {
#define WINED3D_TO_STR(x) case x: return #x
        WINED3D_TO_STR(COMPLEX_FIXUP_YUY2);
        WINED3D_TO_STR(COMPLEX_FIXUP_UYVY);
        WINED3D_TO_STR(COMPLEX_FIXUP_YV12);
        WINED3D_TO_STR(COMPLEX_FIXUP_P8);
#undef WINED3D_TO_STR
        default:
            FIXME("Unrecognized complex fixup %#x\n", fixup);
            return "unrecognized";
    }
}

void dump_color_fixup_desc(struct color_fixup_desc fixup)
{
    if (is_complex_fixup(fixup))
    {
        TRACE("\tComplex: %s\n", debug_complex_fixup(get_complex_fixup(fixup)));
        return;
    }

    TRACE("\tX: %s%s\n", debug_fixup_channel_source(fixup.x_source), fixup.x_sign_fixup ? ", SIGN_FIXUP" : "");
    TRACE("\tY: %s%s\n", debug_fixup_channel_source(fixup.y_source), fixup.y_sign_fixup ? ", SIGN_FIXUP" : "");
    TRACE("\tZ: %s%s\n", debug_fixup_channel_source(fixup.z_source), fixup.z_sign_fixup ? ", SIGN_FIXUP" : "");
    TRACE("\tW: %s%s\n", debug_fixup_channel_source(fixup.w_source), fixup.w_sign_fixup ? ", SIGN_FIXUP" : "");
}

const char *debug_surflocation(DWORD flag) {
    char buf[128];

    buf[0] = 0;
    if (flag & SFLAG_INSYSMEM) strcat(buf, " | SFLAG_INSYSMEM");                    /* 17 */
    if (flag & SFLAG_INDRAWABLE) strcat(buf, " | SFLAG_INDRAWABLE");                /* 19 */
    if (flag & SFLAG_INTEXTURE) strcat(buf, " | SFLAG_INTEXTURE");                  /* 18 */
    if (flag & SFLAG_INSRGBTEX) strcat(buf, " | SFLAG_INSRGBTEX");                  /* 18 */
    if (flag & SFLAG_INRB_MULTISAMPLE) strcat(buf, " | SFLAG_INRB_MULTISAMPLE");    /* 25 */
    if (flag & SFLAG_INRB_RESOLVED) strcat(buf, " | SFLAG_INRB_RESOLVED");          /* 22 */
    return wine_dbg_sprintf("%s", buf[0] ? buf + 3 : "0");
}

BOOL is_invalid_op(const struct wined3d_state *state, int stage,
        enum wined3d_texture_op op, DWORD arg1, DWORD arg2, DWORD arg3)
{
    if (op == WINED3D_TOP_DISABLE)
        return FALSE;
    if (state->textures[stage])
        return FALSE;

    if ((arg1 & WINED3DTA_SELECTMASK) == WINED3DTA_TEXTURE
            && op != WINED3D_TOP_SELECT_ARG2)
        return TRUE;
    if ((arg2 & WINED3DTA_SELECTMASK) == WINED3DTA_TEXTURE
            && op != WINED3D_TOP_SELECT_ARG1)
        return TRUE;
    if ((arg3 & WINED3DTA_SELECTMASK) == WINED3DTA_TEXTURE
            && (op == WINED3D_TOP_MULTIPLY_ADD || op == WINED3D_TOP_LERP))
        return TRUE;

    return FALSE;
}

/* Setup this textures matrix according to the texture flags. */
/* Context activation is done by the caller (state handler). */
void set_texture_matrix(const struct wined3d_gl_info *gl_info, const float *smat, DWORD flags,
        BOOL calculatedCoords, BOOL transformed, enum wined3d_format_id vtx_fmt, BOOL ffp_proj_control)
{
    float mat[16];

    gl_info->gl_ops.gl.p_glMatrixMode(GL_TEXTURE);
    checkGLcall("glMatrixMode(GL_TEXTURE)");

    if (flags == WINED3D_TTFF_DISABLE || flags == WINED3D_TTFF_COUNT1 || transformed)
    {
        gl_info->gl_ops.gl.p_glLoadIdentity();
        checkGLcall("glLoadIdentity()");
        return;
    }

    if (flags == (WINED3D_TTFF_COUNT1 | WINED3D_TTFF_PROJECTED))
    {
        ERR("Invalid texture transform flags: WINED3D_TTFF_COUNT1 | WINED3D_TTFF_PROJECTED.\n");
        return;
    }

    memcpy(mat, smat, 16 * sizeof(float));

    if (flags & WINED3D_TTFF_PROJECTED)
    {
        if (!ffp_proj_control)
        {
            switch (flags & ~WINED3D_TTFF_PROJECTED)
            {
                case WINED3D_TTFF_COUNT2:
                    mat[ 3] = mat[ 1];
                    mat[ 7] = mat[ 5];
                    mat[11] = mat[ 9];
                    mat[15] = mat[13];
                    mat[ 1] = mat[ 5] = mat[ 9] = mat[13] = 0.0f;
                    break;
                case WINED3D_TTFF_COUNT3:
                    mat[ 3] = mat[ 2];
                    mat[ 7] = mat[ 6];
                    mat[11] = mat[10];
                    mat[15] = mat[14];
                    mat[ 2] = mat[ 6] = mat[10] = mat[14] = 0.0f;
                    break;
            }
        }
    } else { /* under directx the R/Z coord can be used for translation, under opengl we use the Q coord instead */
        if(!calculatedCoords) {
            switch(vtx_fmt)
            {
                case WINED3DFMT_R32_FLOAT:
                    /* Direct3D passes the default 1.0 in the 2nd coord, while gl passes it in the 4th.
                     * swap 2nd and 4th coord. No need to store the value of mat[12] in mat[4] because
                     * the input value to the transformation will be 0, so the matrix value is irrelevant
                     */
                    mat[12] = mat[4];
                    mat[13] = mat[5];
                    mat[14] = mat[6];
                    mat[15] = mat[7];
                    break;
                case WINED3DFMT_R32G32_FLOAT:
                    /* See above, just 3rd and 4th coord
                    */
                    mat[12] = mat[8];
                    mat[13] = mat[9];
                    mat[14] = mat[10];
                    mat[15] = mat[11];
                    break;
                case WINED3DFMT_R32G32B32_FLOAT: /* Opengl defaults match dx defaults */
                case WINED3DFMT_R32G32B32A32_FLOAT: /* No defaults apply, all app defined */

                /* This is to prevent swapping the matrix lines and put the default 4th coord = 1.0
                 * into a bad place. The division elimination below will apply to make sure the
                 * 1.0 doesn't do anything bad. The caller will set this value if the stride is 0
                 */
                case WINED3DFMT_UNKNOWN: /* No texture coords, 0/0/0/1 defaults are passed */
                    break;
                default:
                    FIXME("Unexpected fixed function texture coord input\n");
            }
        }
        if (!ffp_proj_control)
        {
            switch (flags & ~WINED3D_TTFF_PROJECTED)
            {
                /* case WINED3D_TTFF_COUNT1: Won't ever get here. */
                case WINED3D_TTFF_COUNT2:
                    mat[2] = mat[6] = mat[10] = mat[14] = 0;
                /* OpenGL divides the first 3 vertex coord by the 4th by default,
                * which is essentially the same as D3DTTFF_PROJECTED. Make sure that
                * the 4th coord evaluates to 1.0 to eliminate that.
                *
                * If the fixed function pipeline is used, the 4th value remains unused,
                * so there is no danger in doing this. With vertex shaders we have a
                * problem. Should an app hit that problem, the code here would have to
                * check for pixel shaders, and the shader has to undo the default gl divide.
                *
                * A more serious problem occurs if the app passes 4 coordinates in, and the
                * 4th is != 1.0(opengl default). This would have to be fixed in draw_strided_slow
                * or a replacement shader. */
                default:
                    mat[3] = mat[7] = mat[11] = 0; mat[15] = 1;
            }
        }
    }

    gl_info->gl_ops.gl.p_glLoadMatrixf(mat);
    checkGLcall("glLoadMatrixf(mat)");
}

/* This small helper function is used to convert a bitmask into the number of masked bits */
unsigned int count_bits(unsigned int mask)
{
    unsigned int count;
    for (count = 0; mask; ++count)
    {
        mask &= mask - 1;
    }
    return count;
}

/* Helper function for retrieving color info for ChoosePixelFormat and wglChoosePixelFormatARB.
 * The later function requires individual color components. */
BOOL getColorBits(const struct wined3d_format *format,
        BYTE *redSize, BYTE *greenSize, BYTE *blueSize, BYTE *alphaSize, BYTE *totalSize)
{
    TRACE("format %s.\n", debug_d3dformat(format->id));

    switch (format->id)
    {
        case WINED3DFMT_B10G10R10A2_UNORM:
        case WINED3DFMT_R10G10B10A2_UNORM:
        case WINED3DFMT_B8G8R8X8_UNORM:
        case WINED3DFMT_B8G8R8_UNORM:
        case WINED3DFMT_B8G8R8A8_UNORM:
        case WINED3DFMT_R8G8B8A8_UNORM:
        case WINED3DFMT_B5G5R5X1_UNORM:
        case WINED3DFMT_B5G5R5A1_UNORM:
        case WINED3DFMT_B5G6R5_UNORM:
        case WINED3DFMT_B4G4R4X4_UNORM:
        case WINED3DFMT_B4G4R4A4_UNORM:
        case WINED3DFMT_B2G3R3_UNORM:
        case WINED3DFMT_P8_UINT_A8_UNORM:
        case WINED3DFMT_P8_UINT:
            break;
        default:
            FIXME("Unsupported format %s.\n", debug_d3dformat(format->id));
            return FALSE;
    }

    *redSize = format->red_size;
    *greenSize = format->green_size;
    *blueSize = format->blue_size;
    *alphaSize = format->alpha_size;
    *totalSize = *redSize + *greenSize + *blueSize + *alphaSize;

    TRACE("Returning red: %d, green: %d, blue: %d, alpha: %d, total: %d for format %s.\n",
            *redSize, *greenSize, *blueSize, *alphaSize, *totalSize, debug_d3dformat(format->id));
    return TRUE;
}

/* Helper function for retrieving depth/stencil info for ChoosePixelFormat and wglChoosePixelFormatARB */
BOOL getDepthStencilBits(const struct wined3d_format *format, BYTE *depthSize, BYTE *stencilSize)
{
    TRACE("format %s.\n", debug_d3dformat(format->id));

    switch (format->id)
    {
        case WINED3DFMT_D16_LOCKABLE:
        case WINED3DFMT_D16_UNORM:
        case WINED3DFMT_S1_UINT_D15_UNORM:
        case WINED3DFMT_X8D24_UNORM:
        case WINED3DFMT_S4X4_UINT_D24_UNORM:
        case WINED3DFMT_D24_UNORM_S8_UINT:
        case WINED3DFMT_S8_UINT_D24_FLOAT:
        case WINED3DFMT_D32_UNORM:
        case WINED3DFMT_D32_FLOAT:
        case WINED3DFMT_INTZ:
            break;
        default:
            FIXME("Unsupported depth/stencil format %s.\n", debug_d3dformat(format->id));
            return FALSE;
    }

    *depthSize = format->depth_size;
    *stencilSize = format->stencil_size;

    TRACE("Returning depthSize: %d and stencilSize: %d for format %s.\n",
            *depthSize, *stencilSize, debug_d3dformat(format->id));
    return TRUE;
}

/* Note: It's the caller's responsibility to ensure values can be expressed
 * in the requested format. UNORM formats for example can only express values
 * in the range 0.0f -> 1.0f. */
DWORD wined3d_format_convert_from_float(const struct wined3d_surface *surface, const struct wined3d_color *color)
{
    static const struct
    {
        enum wined3d_format_id format_id;
        float r_mul;
        float g_mul;
        float b_mul;
        float a_mul;
        BYTE r_shift;
        BYTE g_shift;
        BYTE b_shift;
        BYTE a_shift;
    }
    conv[] =
    {
        {WINED3DFMT_B8G8R8A8_UNORM,     255.0f,  255.0f,  255.0f,  255.0f, 16,  8,  0, 24},
        {WINED3DFMT_B8G8R8X8_UNORM,     255.0f,  255.0f,  255.0f,  255.0f, 16,  8,  0, 24},
        {WINED3DFMT_B8G8R8_UNORM,       255.0f,  255.0f,  255.0f,  255.0f, 16,  8,  0, 24},
        {WINED3DFMT_B5G6R5_UNORM,        31.0f,   63.0f,   31.0f,    0.0f, 11,  5,  0,  0},
        {WINED3DFMT_B5G5R5A1_UNORM,      31.0f,   31.0f,   31.0f,    1.0f, 10,  5,  0, 15},
        {WINED3DFMT_B5G5R5X1_UNORM,      31.0f,   31.0f,   31.0f,    1.0f, 10,  5,  0, 15},
        {WINED3DFMT_A8_UNORM,             0.0f,    0.0f,    0.0f,  255.0f,  0,  0,  0,  0},
        {WINED3DFMT_B4G4R4A4_UNORM,      15.0f,   15.0f,   15.0f,   15.0f,  8,  4,  0, 12},
        {WINED3DFMT_B4G4R4X4_UNORM,      15.0f,   15.0f,   15.0f,   15.0f,  8,  4,  0, 12},
        {WINED3DFMT_B2G3R3_UNORM,         7.0f,    7.0f,    3.0f,    0.0f,  5,  2,  0,  0},
        {WINED3DFMT_R8G8B8A8_UNORM,     255.0f,  255.0f,  255.0f,  255.0f,  0,  8, 16, 24},
        {WINED3DFMT_R8G8B8X8_UNORM,     255.0f,  255.0f,  255.0f,  255.0f,  0,  8, 16, 24},
        {WINED3DFMT_B10G10R10A2_UNORM, 1023.0f, 1023.0f, 1023.0f,    3.0f, 20, 10,  0, 30},
        {WINED3DFMT_R10G10B10A2_UNORM, 1023.0f, 1023.0f, 1023.0f,    3.0f,  0, 10, 20, 30},
    };
    const struct wined3d_format *format = surface->resource.format;
    unsigned int i;

    TRACE("Converting color {%.8e %.8e %.8e %.8e} to format %s.\n",
            color->r, color->g, color->b, color->a, debug_d3dformat(format->id));

    for (i = 0; i < sizeof(conv) / sizeof(*conv); ++i)
    {
        DWORD ret;

        if (format->id != conv[i].format_id) continue;

        ret = ((DWORD)((color->r * conv[i].r_mul) + 0.5f)) << conv[i].r_shift;
        ret |= ((DWORD)((color->g * conv[i].g_mul) + 0.5f)) << conv[i].g_shift;
        ret |= ((DWORD)((color->b * conv[i].b_mul) + 0.5f)) << conv[i].b_shift;
        ret |= ((DWORD)((color->a * conv[i].a_mul) + 0.5f)) << conv[i].a_shift;

        TRACE("Returning 0x%08x.\n", ret);

        return ret;
    }

    if (format->id == WINED3DFMT_P8_UINT)
    {
        PALETTEENTRY *e;
        BYTE r, g, b, a;

        if (!surface->palette)
        {
            WARN("Surface doesn't have a palette, returning 0.\n");
            return 0;
        }

        r = (BYTE)((color->r * 255.0f) + 0.5f);
        g = (BYTE)((color->g * 255.0f) + 0.5f);
        b = (BYTE)((color->b * 255.0f) + 0.5f);
        a = (BYTE)((color->a * 255.0f) + 0.5f);

        e = &surface->palette->palents[a];
        if (e->peRed == r && e->peGreen == g && e->peBlue == b)
            return a;

        WARN("Alpha didn't match index, searching full palette.\n");

        for (i = 0; i < 256; ++i)
        {
            e = &surface->palette->palents[i];
            if (e->peRed == r && e->peGreen == g && e->peBlue == b)
                return i;
        }

        FIXME("Unable to convert color to palette index.\n");

        return 0;
    }

    FIXME("Conversion for format %s not implemented.\n", debug_d3dformat(format->id));

    return 0;
}

/* DirectDraw stuff */
enum wined3d_format_id pixelformat_for_depth(DWORD depth)
{
    switch (depth)
    {
        case 8:  return WINED3DFMT_P8_UINT;
        case 15: return WINED3DFMT_B5G5R5X1_UNORM;
        case 16: return WINED3DFMT_B5G6R5_UNORM;
        case 24: return WINED3DFMT_B8G8R8X8_UNORM; /* Robots needs 24bit to be WINED3DFMT_B8G8R8X8_UNORM */
        case 32: return WINED3DFMT_B8G8R8X8_UNORM; /* EVE online and the Fur demo need 32bit AdapterDisplayMode to return WINED3DFMT_B8G8R8X8_UNORM */
        default: return WINED3DFMT_UNKNOWN;
    }
}

void multiply_matrix(struct wined3d_matrix *dest, const struct wined3d_matrix *src1,
        const struct wined3d_matrix *src2)
{
    struct wined3d_matrix temp;

    /* Now do the multiplication 'by hand'.
       I know that all this could be optimised, but this will be done later :-) */
    temp.u.s._11 = (src1->u.s._11 * src2->u.s._11) + (src1->u.s._21 * src2->u.s._12) + (src1->u.s._31 * src2->u.s._13) + (src1->u.s._41 * src2->u.s._14);
    temp.u.s._21 = (src1->u.s._11 * src2->u.s._21) + (src1->u.s._21 * src2->u.s._22) + (src1->u.s._31 * src2->u.s._23) + (src1->u.s._41 * src2->u.s._24);
    temp.u.s._31 = (src1->u.s._11 * src2->u.s._31) + (src1->u.s._21 * src2->u.s._32) + (src1->u.s._31 * src2->u.s._33) + (src1->u.s._41 * src2->u.s._34);
    temp.u.s._41 = (src1->u.s._11 * src2->u.s._41) + (src1->u.s._21 * src2->u.s._42) + (src1->u.s._31 * src2->u.s._43) + (src1->u.s._41 * src2->u.s._44);

    temp.u.s._12 = (src1->u.s._12 * src2->u.s._11) + (src1->u.s._22 * src2->u.s._12) + (src1->u.s._32 * src2->u.s._13) + (src1->u.s._42 * src2->u.s._14);
    temp.u.s._22 = (src1->u.s._12 * src2->u.s._21) + (src1->u.s._22 * src2->u.s._22) + (src1->u.s._32 * src2->u.s._23) + (src1->u.s._42 * src2->u.s._24);
    temp.u.s._32 = (src1->u.s._12 * src2->u.s._31) + (src1->u.s._22 * src2->u.s._32) + (src1->u.s._32 * src2->u.s._33) + (src1->u.s._42 * src2->u.s._34);
    temp.u.s._42 = (src1->u.s._12 * src2->u.s._41) + (src1->u.s._22 * src2->u.s._42) + (src1->u.s._32 * src2->u.s._43) + (src1->u.s._42 * src2->u.s._44);

    temp.u.s._13 = (src1->u.s._13 * src2->u.s._11) + (src1->u.s._23 * src2->u.s._12) + (src1->u.s._33 * src2->u.s._13) + (src1->u.s._43 * src2->u.s._14);
    temp.u.s._23 = (src1->u.s._13 * src2->u.s._21) + (src1->u.s._23 * src2->u.s._22) + (src1->u.s._33 * src2->u.s._23) + (src1->u.s._43 * src2->u.s._24);
    temp.u.s._33 = (src1->u.s._13 * src2->u.s._31) + (src1->u.s._23 * src2->u.s._32) + (src1->u.s._33 * src2->u.s._33) + (src1->u.s._43 * src2->u.s._34);
    temp.u.s._43 = (src1->u.s._13 * src2->u.s._41) + (src1->u.s._23 * src2->u.s._42) + (src1->u.s._33 * src2->u.s._43) + (src1->u.s._43 * src2->u.s._44);

    temp.u.s._14 = (src1->u.s._14 * src2->u.s._11) + (src1->u.s._24 * src2->u.s._12) + (src1->u.s._34 * src2->u.s._13) + (src1->u.s._44 * src2->u.s._14);
    temp.u.s._24 = (src1->u.s._14 * src2->u.s._21) + (src1->u.s._24 * src2->u.s._22) + (src1->u.s._34 * src2->u.s._23) + (src1->u.s._44 * src2->u.s._24);
    temp.u.s._34 = (src1->u.s._14 * src2->u.s._31) + (src1->u.s._24 * src2->u.s._32) + (src1->u.s._34 * src2->u.s._33) + (src1->u.s._44 * src2->u.s._34);
    temp.u.s._44 = (src1->u.s._14 * src2->u.s._41) + (src1->u.s._24 * src2->u.s._42) + (src1->u.s._34 * src2->u.s._43) + (src1->u.s._44 * src2->u.s._44);

    /* And copy the new matrix in the good storage.. */
    memcpy(dest, &temp, 16 * sizeof(float));
}

DWORD get_flexible_vertex_size(DWORD d3dvtVertexType) {
    DWORD size = 0;
    int i;
    int numTextures = (d3dvtVertexType & WINED3DFVF_TEXCOUNT_MASK) >> WINED3DFVF_TEXCOUNT_SHIFT;

    if (d3dvtVertexType & WINED3DFVF_NORMAL) size += 3 * sizeof(float);
    if (d3dvtVertexType & WINED3DFVF_DIFFUSE) size += sizeof(DWORD);
    if (d3dvtVertexType & WINED3DFVF_SPECULAR) size += sizeof(DWORD);
    if (d3dvtVertexType & WINED3DFVF_PSIZE) size += sizeof(DWORD);
    switch (d3dvtVertexType & WINED3DFVF_POSITION_MASK) {
        case WINED3DFVF_XYZ:    size += 3 * sizeof(float); break;
        case WINED3DFVF_XYZRHW: size += 4 * sizeof(float); break;
        case WINED3DFVF_XYZB1:  size += 4 * sizeof(float); break;
        case WINED3DFVF_XYZB2:  size += 5 * sizeof(float); break;
        case WINED3DFVF_XYZB3:  size += 6 * sizeof(float); break;
        case WINED3DFVF_XYZB4:  size += 7 * sizeof(float); break;
        case WINED3DFVF_XYZB5:  size += 8 * sizeof(float); break;
        case WINED3DFVF_XYZW:   size += 4 * sizeof(float); break;
        default: ERR("Unexpected position mask\n");
    }
    for (i = 0; i < numTextures; i++) {
        size += GET_TEXCOORD_SIZE_FROM_FVF(d3dvtVertexType, i) * sizeof(float);
    }

    return size;
}

void gen_ffp_frag_op(const struct wined3d_context *context, const struct wined3d_state *state,
        struct ffp_frag_settings *settings, BOOL ignore_textype)
{
#define ARG1 0x01
#define ARG2 0x02
#define ARG0 0x04
    static const unsigned char args[WINED3D_TOP_LERP + 1] =
    {
        /* undefined                        */  0,
        /* D3DTOP_DISABLE                   */  0,
        /* D3DTOP_SELECTARG1                */  ARG1,
        /* D3DTOP_SELECTARG2                */  ARG2,
        /* D3DTOP_MODULATE                  */  ARG1 | ARG2,
        /* D3DTOP_MODULATE2X                */  ARG1 | ARG2,
        /* D3DTOP_MODULATE4X                */  ARG1 | ARG2,
        /* D3DTOP_ADD                       */  ARG1 | ARG2,
        /* D3DTOP_ADDSIGNED                 */  ARG1 | ARG2,
        /* D3DTOP_ADDSIGNED2X               */  ARG1 | ARG2,
        /* D3DTOP_SUBTRACT                  */  ARG1 | ARG2,
        /* D3DTOP_ADDSMOOTH                 */  ARG1 | ARG2,
        /* D3DTOP_BLENDDIFFUSEALPHA         */  ARG1 | ARG2,
        /* D3DTOP_BLENDTEXTUREALPHA         */  ARG1 | ARG2,
        /* D3DTOP_BLENDFACTORALPHA          */  ARG1 | ARG2,
        /* D3DTOP_BLENDTEXTUREALPHAPM       */  ARG1 | ARG2,
        /* D3DTOP_BLENDCURRENTALPHA         */  ARG1 | ARG2,
        /* D3DTOP_PREMODULATE               */  ARG1 | ARG2,
        /* D3DTOP_MODULATEALPHA_ADDCOLOR    */  ARG1 | ARG2,
        /* D3DTOP_MODULATECOLOR_ADDALPHA    */  ARG1 | ARG2,
        /* D3DTOP_MODULATEINVALPHA_ADDCOLOR */  ARG1 | ARG2,
        /* D3DTOP_MODULATEINVCOLOR_ADDALPHA */  ARG1 | ARG2,
        /* D3DTOP_BUMPENVMAP                */  ARG1 | ARG2,
        /* D3DTOP_BUMPENVMAPLUMINANCE       */  ARG1 | ARG2,
        /* D3DTOP_DOTPRODUCT3               */  ARG1 | ARG2,
        /* D3DTOP_MULTIPLYADD               */  ARG1 | ARG2 | ARG0,
        /* D3DTOP_LERP                      */  ARG1 | ARG2 | ARG0
    };
    unsigned int i;
    DWORD ttff;
    DWORD cop, aop, carg0, carg1, carg2, aarg0, aarg1, aarg2;
    const struct wined3d_surface *rt = state->fb.render_targets[0];
    const struct wined3d_gl_info *gl_info = context->gl_info;
    const struct wined3d_d3d_info *d3d_info = context->d3d_info;

    for (i = 0; i < d3d_info->limits.ffp_blend_stages; ++i)
    {
        const struct wined3d_texture *texture;

        settings->op[i].padding = 0;
        if (state->texture_states[i][WINED3D_TSS_COLOR_OP] == WINED3D_TOP_DISABLE)
        {
            settings->op[i].cop = WINED3D_TOP_DISABLE;
            settings->op[i].aop = WINED3D_TOP_DISABLE;
            settings->op[i].carg0 = settings->op[i].carg1 = settings->op[i].carg2 = ARG_UNUSED;
            settings->op[i].aarg0 = settings->op[i].aarg1 = settings->op[i].aarg2 = ARG_UNUSED;
            settings->op[i].color_fixup = COLOR_FIXUP_IDENTITY;
            settings->op[i].dst = resultreg;
            settings->op[i].tex_type = tex_1d;
            settings->op[i].projected = proj_none;
            i++;
            break;
        }

        if ((texture = state->textures[i]))
        {
            settings->op[i].color_fixup = texture->resource.format->color_fixup;
            if (ignore_textype)
            {
                settings->op[i].tex_type = tex_1d;
            }
            else
            {
                switch (texture->target)
                {
                    case GL_TEXTURE_1D:
                        settings->op[i].tex_type = tex_1d;
                        break;
                    case GL_TEXTURE_2D:
                        settings->op[i].tex_type = tex_2d;
                        break;
                    case GL_TEXTURE_3D:
                        settings->op[i].tex_type = tex_3d;
                        break;
                    case GL_TEXTURE_CUBE_MAP_ARB:
                        settings->op[i].tex_type = tex_cube;
                        break;
                    case GL_TEXTURE_RECTANGLE_ARB:
                        settings->op[i].tex_type = tex_rect;
                        break;
                }
            }
        } else {
            settings->op[i].color_fixup = COLOR_FIXUP_IDENTITY;
            settings->op[i].tex_type = tex_1d;
        }

        cop = state->texture_states[i][WINED3D_TSS_COLOR_OP];
        aop = state->texture_states[i][WINED3D_TSS_ALPHA_OP];

        carg1 = (args[cop] & ARG1) ? state->texture_states[i][WINED3D_TSS_COLOR_ARG1] : ARG_UNUSED;
        carg2 = (args[cop] & ARG2) ? state->texture_states[i][WINED3D_TSS_COLOR_ARG2] : ARG_UNUSED;
        carg0 = (args[cop] & ARG0) ? state->texture_states[i][WINED3D_TSS_COLOR_ARG0] : ARG_UNUSED;

        if (is_invalid_op(state, i, cop, carg1, carg2, carg0))
        {
            carg0 = ARG_UNUSED;
            carg2 = ARG_UNUSED;
            carg1 = WINED3DTA_CURRENT;
            cop = WINED3D_TOP_SELECT_ARG1;
        }

        if (cop == WINED3D_TOP_DOTPRODUCT3)
        {
            /* A dotproduct3 on the colorop overwrites the alphaop operation and replicates
             * the color result to the alpha component of the destination
             */
            aop = cop;
            aarg1 = carg1;
            aarg2 = carg2;
            aarg0 = carg0;
        }
        else
        {
            aarg1 = (args[aop] & ARG1) ? state->texture_states[i][WINED3D_TSS_ALPHA_ARG1] : ARG_UNUSED;
            aarg2 = (args[aop] & ARG2) ? state->texture_states[i][WINED3D_TSS_ALPHA_ARG2] : ARG_UNUSED;
            aarg0 = (args[aop] & ARG0) ? state->texture_states[i][WINED3D_TSS_ALPHA_ARG0] : ARG_UNUSED;
        }

        if (!i && state->textures[0] && state->render_states[WINED3D_RS_COLORKEYENABLE])
        {
            GLenum texture_dimensions;

            texture = state->textures[0];
            texture_dimensions = texture->target;

            if (texture_dimensions == GL_TEXTURE_2D || texture_dimensions == GL_TEXTURE_RECTANGLE_ARB)
            {
                struct wined3d_surface *surf = surface_from_resource(texture->sub_resources[0]);

                if (surf->CKeyFlags & WINEDDSD_CKSRCBLT && !surf->resource.format->alpha_size)
                {
                    if (aop == WINED3D_TOP_DISABLE)
                    {
                       aarg1 = WINED3DTA_TEXTURE;
                       aop = WINED3D_TOP_SELECT_ARG1;
                    }
                    else if (aop == WINED3D_TOP_SELECT_ARG1 && aarg1 != WINED3DTA_TEXTURE)
                    {
                        if (state->render_states[WINED3D_RS_ALPHABLENDENABLE])
                        {
                            aarg2 = WINED3DTA_TEXTURE;
                            aop = WINED3D_TOP_MODULATE;
                        }
                        else aarg1 = WINED3DTA_TEXTURE;
                    }
                    else if (aop == WINED3D_TOP_SELECT_ARG2 && aarg2 != WINED3DTA_TEXTURE)
                    {
                        if (state->render_states[WINED3D_RS_ALPHABLENDENABLE])
                        {
                            aarg1 = WINED3DTA_TEXTURE;
                            aop = WINED3D_TOP_MODULATE;
                        }
                        else aarg2 = WINED3DTA_TEXTURE;
                    }
                }
            }
        }

        if (is_invalid_op(state, i, aop, aarg1, aarg2, aarg0))
        {
               aarg0 = ARG_UNUSED;
               aarg2 = ARG_UNUSED;
               aarg1 = WINED3DTA_CURRENT;
               aop = WINED3D_TOP_SELECT_ARG1;
        }

        if (carg1 == WINED3DTA_TEXTURE || carg2 == WINED3DTA_TEXTURE || carg0 == WINED3DTA_TEXTURE
                || aarg1 == WINED3DTA_TEXTURE || aarg2 == WINED3DTA_TEXTURE || aarg0 == WINED3DTA_TEXTURE)
        {
            ttff = state->texture_states[i][WINED3D_TSS_TEXTURE_TRANSFORM_FLAGS];
            if (ttff == (WINED3D_TTFF_PROJECTED | WINED3D_TTFF_COUNT3))
                settings->op[i].projected = proj_count3;
            else if (ttff & WINED3D_TTFF_PROJECTED)
                settings->op[i].projected = proj_count4;
            else
                settings->op[i].projected = proj_none;
        }
        else
        {
            settings->op[i].projected = proj_none;
        }

        settings->op[i].cop = cop;
        settings->op[i].aop = aop;
        settings->op[i].carg0 = carg0;
        settings->op[i].carg1 = carg1;
        settings->op[i].carg2 = carg2;
        settings->op[i].aarg0 = aarg0;
        settings->op[i].aarg1 = aarg1;
        settings->op[i].aarg2 = aarg2;

        if (state->texture_states[i][WINED3D_TSS_RESULT_ARG] == WINED3DTA_TEMP)
            settings->op[i].dst = tempreg;
        else
            settings->op[i].dst = resultreg;
    }

    /* Clear unsupported stages */
    for(; i < MAX_TEXTURES; i++) {
        memset(&settings->op[i], 0xff, sizeof(settings->op[i]));
    }

    if (!state->render_states[WINED3D_RS_FOGENABLE])
    {
        settings->fog = WINED3D_FFP_PS_FOG_OFF;
    }
    else if (state->render_states[WINED3D_RS_FOGTABLEMODE] == WINED3D_FOG_NONE)
    {
        if (use_vs(state) || state->vertex_declaration->position_transformed)
        {
            settings->fog = WINED3D_FFP_PS_FOG_LINEAR;
        }
        else
        {
            switch (state->render_states[WINED3D_RS_FOGVERTEXMODE])
            {
                case WINED3D_FOG_NONE:
                case WINED3D_FOG_LINEAR:
                    settings->fog = WINED3D_FFP_PS_FOG_LINEAR;
                    break;
                case WINED3D_FOG_EXP:
                    settings->fog = WINED3D_FFP_PS_FOG_EXP;
                    break;
                case WINED3D_FOG_EXP2:
                    settings->fog = WINED3D_FFP_PS_FOG_EXP2;
                    break;
            }
        }
    }
    else
    {
        switch (state->render_states[WINED3D_RS_FOGTABLEMODE])
        {
            case WINED3D_FOG_LINEAR:
                settings->fog = WINED3D_FFP_PS_FOG_LINEAR;
                break;
            case WINED3D_FOG_EXP:
                settings->fog = WINED3D_FFP_PS_FOG_EXP;
                break;
            case WINED3D_FOG_EXP2:
                settings->fog = WINED3D_FFP_PS_FOG_EXP2;
                break;
        }
    }
    if (!gl_info->supported[ARB_FRAMEBUFFER_SRGB]
            && state->render_states[WINED3D_RS_SRGBWRITEENABLE]
            && rt->resource.format->flags & WINED3DFMT_FLAG_SRGB_WRITE)
    {
        settings->sRGB_write = 1;
    } else {
        settings->sRGB_write = 0;
    }
    if (d3d_info->vs_clipping || !use_vs(state) || !state->render_states[WINED3D_RS_CLIPPING]
            || !state->render_states[WINED3D_RS_CLIPPLANEENABLE])
    {
        /* No need to emulate clipplanes if GL supports native vertex shader clipping or if
         * the fixed function vertex pipeline is used(which always supports clipplanes), or
         * if no clipplane is enabled
         */
        settings->emul_clipplanes = 0;
    } else {
        settings->emul_clipplanes = 1;
    }
}

const struct ffp_frag_desc *find_ffp_frag_shader(const struct wine_rb_tree *fragment_shaders,
        const struct ffp_frag_settings *settings)
{
    struct wine_rb_entry *entry = wine_rb_get(fragment_shaders, settings);
    return entry ? WINE_RB_ENTRY_VALUE(entry, struct ffp_frag_desc, entry) : NULL;
}

void add_ffp_frag_shader(struct wine_rb_tree *shaders, struct ffp_frag_desc *desc)
{
    /* Note that the key is the implementation independent part of the ffp_frag_desc structure,
     * whereas desc points to an extended structure with implementation specific parts. */
    if (wine_rb_put(shaders, &desc->settings, &desc->entry) == -1)
    {
        ERR("Failed to insert ffp frag shader.\n");
    }
}

/* Activates the texture dimension according to the bound D3D texture. Does
 * not care for the colorop or correct gl texture unit (when using nvrc).
 * Requires the caller to activate the correct unit. */
/* Context activation is done by the caller (state handler). */
void texture_activate_dimensions(const struct wined3d_texture *texture, const struct wined3d_gl_info *gl_info)
{
    if (texture)
    {
        switch (texture->target)
        {
            case GL_TEXTURE_2D:
                gl_info->gl_ops.gl.p_glDisable(GL_TEXTURE_3D);
                checkGLcall("glDisable(GL_TEXTURE_3D)");
                if (gl_info->supported[ARB_TEXTURE_CUBE_MAP])
                {
                    gl_info->gl_ops.gl.p_glDisable(GL_TEXTURE_CUBE_MAP_ARB);
                    checkGLcall("glDisable(GL_TEXTURE_CUBE_MAP_ARB)");
                }
                if (gl_info->supported[ARB_TEXTURE_RECTANGLE])
                {
                    gl_info->gl_ops.gl.p_glDisable(GL_TEXTURE_RECTANGLE_ARB);
                    checkGLcall("glDisable(GL_TEXTURE_RECTANGLE_ARB)");
                }
                gl_info->gl_ops.gl.p_glEnable(GL_TEXTURE_2D);
                checkGLcall("glEnable(GL_TEXTURE_2D)");
                break;
            case GL_TEXTURE_RECTANGLE_ARB:
                gl_info->gl_ops.gl.p_glDisable(GL_TEXTURE_2D);
                checkGLcall("glDisable(GL_TEXTURE_2D)");
                gl_info->gl_ops.gl.p_glDisable(GL_TEXTURE_3D);
                checkGLcall("glDisable(GL_TEXTURE_3D)");
                if (gl_info->supported[ARB_TEXTURE_CUBE_MAP])
                {
                    gl_info->gl_ops.gl.p_glDisable(GL_TEXTURE_CUBE_MAP_ARB);
                    checkGLcall("glDisable(GL_TEXTURE_CUBE_MAP_ARB)");
                }
                gl_info->gl_ops.gl.p_glEnable(GL_TEXTURE_RECTANGLE_ARB);
                checkGLcall("glEnable(GL_TEXTURE_RECTANGLE_ARB)");
                break;
            case GL_TEXTURE_3D:
                if (gl_info->supported[ARB_TEXTURE_CUBE_MAP])
                {
                    gl_info->gl_ops.gl.p_glDisable(GL_TEXTURE_CUBE_MAP_ARB);
                    checkGLcall("glDisable(GL_TEXTURE_CUBE_MAP_ARB)");
                }
                if (gl_info->supported[ARB_TEXTURE_RECTANGLE])
                {
                    gl_info->gl_ops.gl.p_glDisable(GL_TEXTURE_RECTANGLE_ARB);
                    checkGLcall("glDisable(GL_TEXTURE_RECTANGLE_ARB)");
                }
                gl_info->gl_ops.gl.p_glDisable(GL_TEXTURE_2D);
                checkGLcall("glDisable(GL_TEXTURE_2D)");
                gl_info->gl_ops.gl.p_glEnable(GL_TEXTURE_3D);
                checkGLcall("glEnable(GL_TEXTURE_3D)");
                break;
            case GL_TEXTURE_CUBE_MAP_ARB:
                gl_info->gl_ops.gl.p_glDisable(GL_TEXTURE_2D);
                checkGLcall("glDisable(GL_TEXTURE_2D)");
                gl_info->gl_ops.gl.p_glDisable(GL_TEXTURE_3D);
                checkGLcall("glDisable(GL_TEXTURE_3D)");
                if (gl_info->supported[ARB_TEXTURE_RECTANGLE])
                {
                    gl_info->gl_ops.gl.p_glDisable(GL_TEXTURE_RECTANGLE_ARB);
                    checkGLcall("glDisable(GL_TEXTURE_RECTANGLE_ARB)");
                }
                gl_info->gl_ops.gl.p_glEnable(GL_TEXTURE_CUBE_MAP_ARB);
                checkGLcall("glEnable(GL_TEXTURE_CUBE_MAP_ARB)");
              break;
        }
    }
    else
    {
        gl_info->gl_ops.gl.p_glEnable(GL_TEXTURE_2D);
        checkGLcall("glEnable(GL_TEXTURE_2D)");
        gl_info->gl_ops.gl.p_glDisable(GL_TEXTURE_3D);
        checkGLcall("glDisable(GL_TEXTURE_3D)");
        if (gl_info->supported[ARB_TEXTURE_CUBE_MAP])
        {
            gl_info->gl_ops.gl.p_glDisable(GL_TEXTURE_CUBE_MAP_ARB);
            checkGLcall("glDisable(GL_TEXTURE_CUBE_MAP_ARB)");
        }
        if (gl_info->supported[ARB_TEXTURE_RECTANGLE])
        {
            gl_info->gl_ops.gl.p_glDisable(GL_TEXTURE_RECTANGLE_ARB);
            checkGLcall("glDisable(GL_TEXTURE_RECTANGLE_ARB)");
        }
        /* Binding textures is done by samplers. A dummy texture will be bound */
    }
}

/* Context activation is done by the caller (state handler). */
void sampler_texdim(struct wined3d_context *context, const struct wined3d_state *state, DWORD state_id)
{
    DWORD sampler = state_id - STATE_SAMPLER(0);
    DWORD mapped_stage = context->tex_unit_map[sampler];

    /* No need to enable / disable anything here for unused samplers. The
     * tex_colorop handler takes care. Also no action is needed with pixel
     * shaders, or if tex_colorop will take care of this business. */
    if (mapped_stage == WINED3D_UNMAPPED_STAGE || mapped_stage >= context->gl_info->limits.textures)
        return;
    if (sampler >= state->lowest_disabled_stage)
        return;
    if (isStateDirty(context, STATE_TEXTURESTAGE(sampler, WINED3D_TSS_COLOR_OP)))
        return;

    texture_activate_dimensions(state->textures[sampler], context->gl_info);
}

void *wined3d_rb_alloc(size_t size)
{
    return HeapAlloc(GetProcessHeap(), 0, size);
}

void *wined3d_rb_realloc(void *ptr, size_t size)
{
    return HeapReAlloc(GetProcessHeap(), 0, ptr, size);
}

void wined3d_rb_free(void *ptr)
{
    HeapFree(GetProcessHeap(), 0, ptr);
}

static int ffp_frag_program_key_compare(const void *key, const struct wine_rb_entry *entry)
{
    const struct ffp_frag_settings *ka = key;
    const struct ffp_frag_settings *kb = &WINE_RB_ENTRY_VALUE(entry, const struct ffp_frag_desc, entry)->settings;

    return memcmp(ka, kb, sizeof(*ka));
}

const struct wine_rb_functions wined3d_ffp_frag_program_rb_functions =
{
    wined3d_rb_alloc,
    wined3d_rb_realloc,
    wined3d_rb_free,
    ffp_frag_program_key_compare,
};

void wined3d_ffp_get_vs_settings(const struct wined3d_state *state, const struct wined3d_stream_info *si,
        struct wined3d_ffp_vs_settings *settings)
{
    unsigned int coord_idx, i;

    if (si->position_transformed)
    {
        memset(settings, 0, sizeof(*settings));

        settings->transformed = 1;
        settings->point_size = state->gl_primitive_type == GL_POINTS;
        if (!state->render_states[WINED3D_RS_FOGENABLE])
            settings->fog_mode = WINED3D_FFP_VS_FOG_OFF;
        else if (state->render_states[WINED3D_RS_FOGTABLEMODE] != WINED3D_FOG_NONE)
            settings->fog_mode = WINED3D_FFP_VS_FOG_DEPTH;
        else
            settings->fog_mode = WINED3D_FFP_VS_FOG_FOGCOORD;

        for (i = 0; i < MAX_TEXTURES; ++i)
        {
            coord_idx = state->texture_states[i][WINED3D_TSS_TEXCOORD_INDEX];
            if (coord_idx < MAX_TEXTURES && (si->use_map & (1 << (WINED3D_FFP_TEXCOORD0 + coord_idx))))
                settings->texcoords |= 1 << i;
            settings->texgen[i] = (state->texture_states[i][WINED3D_TSS_TEXCOORD_INDEX] >> WINED3D_FFP_TCI_SHIFT)
                    & WINED3D_FFP_TCI_MASK;
        }
        return;
    }

    settings->transformed = 0;
    settings->clipping = state->render_states[WINED3D_RS_CLIPPING]
            && state->render_states[WINED3D_RS_CLIPPLANEENABLE];
    settings->normal = !!(si->use_map & (1 << WINED3D_FFP_NORMAL));
    settings->normalize = settings->normal && state->render_states[WINED3D_RS_NORMALIZENORMALS];
    settings->lighting = !!state->render_states[WINED3D_RS_LIGHTING];
    settings->localviewer = !!state->render_states[WINED3D_RS_LOCALVIEWER];
    settings->point_size = state->gl_primitive_type == GL_POINTS;

    if (state->render_states[WINED3D_RS_COLORVERTEX] && (si->use_map & (1 << WINED3D_FFP_DIFFUSE)))
    {
        settings->diffuse_source = state->render_states[WINED3D_RS_DIFFUSEMATERIALSOURCE];
        settings->emission_source = state->render_states[WINED3D_RS_EMISSIVEMATERIALSOURCE];
        settings->ambient_source = state->render_states[WINED3D_RS_AMBIENTMATERIALSOURCE];
        settings->specular_source = state->render_states[WINED3D_RS_SPECULARMATERIALSOURCE];
    }
    else
    {
        settings->diffuse_source = WINED3D_MCS_MATERIAL;
        settings->emission_source = WINED3D_MCS_MATERIAL;
        settings->ambient_source = WINED3D_MCS_MATERIAL;
        settings->specular_source = WINED3D_MCS_MATERIAL;
    }

    settings->texcoords = 0;
    for (i = 0; i < MAX_TEXTURES; ++i)
    {
        coord_idx = state->texture_states[i][WINED3D_TSS_TEXCOORD_INDEX];
        if (coord_idx < MAX_TEXTURES && (si->use_map & (1 << (WINED3D_FFP_TEXCOORD0 + coord_idx))))
            settings->texcoords |= 1 << i;
        settings->texgen[i] = (state->texture_states[i][WINED3D_TSS_TEXCOORD_INDEX] >> WINED3D_FFP_TCI_SHIFT)
                & WINED3D_FFP_TCI_MASK;
    }

    settings->light_type = 0;
    for (i = 0; i < MAX_ACTIVE_LIGHTS; ++i)
    {
        if (state->lights[i])
            settings->light_type |= (state->lights[i]->OriginalParms.type
                    & WINED3D_FFP_LIGHT_TYPE_MASK) << WINED3D_FFP_LIGHT_TYPE_SHIFT(i);
    }

    settings->ortho_fog = 0;
    if (!state->render_states[WINED3D_RS_FOGENABLE])
        settings->fog_mode = WINED3D_FFP_VS_FOG_OFF;
    else if (state->render_states[WINED3D_RS_FOGTABLEMODE] != WINED3D_FOG_NONE)
    {
        settings->fog_mode = WINED3D_FFP_VS_FOG_DEPTH;

        if (state->transforms[WINED3D_TS_PROJECTION].u.m[0][3] == 0.0f
                && state->transforms[WINED3D_TS_PROJECTION].u.m[1][3] == 0.0f
                && state->transforms[WINED3D_TS_PROJECTION].u.m[2][3] == 0.0f
                && state->transforms[WINED3D_TS_PROJECTION].u.m[3][3] == 1.0f)
            settings->ortho_fog = 1;
    }
    else if (state->render_states[WINED3D_RS_FOGVERTEXMODE] == WINED3D_FOG_NONE)
        settings->fog_mode = WINED3D_FFP_VS_FOG_FOGCOORD;
    else if (state->render_states[WINED3D_RS_RANGEFOGENABLE])
        settings->fog_mode = WINED3D_FFP_VS_FOG_RANGE;
    else
        settings->fog_mode = WINED3D_FFP_VS_FOG_DEPTH;

    settings->padding = 0;
}

static int wined3d_ffp_vertex_program_key_compare(const void *key, const struct wine_rb_entry *entry)
{
    const struct wined3d_ffp_vs_settings *ka = key;
    const struct wined3d_ffp_vs_settings *kb = &WINE_RB_ENTRY_VALUE(entry,
            const struct wined3d_ffp_vs_desc, entry)->settings;

    return memcmp(ka, kb, sizeof(*ka));
}

const struct wine_rb_functions wined3d_ffp_vertex_program_rb_functions =
{
    wined3d_rb_alloc,
    wined3d_rb_realloc,
    wined3d_rb_free,
    wined3d_ffp_vertex_program_key_compare,
};

UINT wined3d_log2i(UINT32 x)
{
    static const UINT l[] =
    {
        ~0U, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
          4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
          5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
          5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
          6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
          6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
          6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
          6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
          7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
          7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
          7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
          7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
          7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
          7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
          7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
          7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    };
    UINT32 i;

    return (i = x >> 16) ? (x = i >> 8) ? l[x] + 24 : l[i] + 16 : (i = x >> 8) ? l[i] + 8 : l[x];
}

const struct blit_shader *wined3d_select_blitter(const struct wined3d_gl_info *gl_info, enum wined3d_blit_op blit_op,
        const RECT *src_rect, DWORD src_usage, enum wined3d_pool src_pool, const struct wined3d_format *src_format,
        const RECT *dst_rect, DWORD dst_usage, enum wined3d_pool dst_pool, const struct wined3d_format *dst_format)
{
    static const struct blit_shader * const blitters[] =
    {
        &arbfp_blit,
        &ffp_blit,
        &cpu_blit,
    };
    unsigned int i;

    for (i = 0; i < sizeof(blitters) / sizeof(*blitters); ++i)
    {
        if (blitters[i]->blit_supported(gl_info, blit_op,
                src_rect, src_usage, src_pool, src_format,
                dst_rect, dst_usage, dst_pool, dst_format))
            return blitters[i];
    }

    return NULL;
}

void wined3d_get_draw_rect(const struct wined3d_state *state, RECT *rect)
{
    const struct wined3d_viewport *vp = &state->viewport;

    SetRect(rect, vp->x, vp->y, vp->x + vp->width, vp->y + vp->height);

    if (state->render_states[WINED3D_RS_SCISSORTESTENABLE])
        IntersectRect(rect, rect, &state->scissor_rect);
}

const char *wined3d_debug_location(DWORD location)
{
    char buf[200];

    buf[0] = '\0';
#define LOCATION_TO_STR(u) if (location & u) { strcat(buf, " | "#u); location &= ~u; }
    LOCATION_TO_STR(WINED3D_LOCATION_DISCARDED);
    LOCATION_TO_STR(WINED3D_LOCATION_SYSMEM);
    LOCATION_TO_STR(WINED3D_LOCATION_BUFFER);
    LOCATION_TO_STR(WINED3D_LOCATION_TEXTURE_RGB);
    LOCATION_TO_STR(WINED3D_LOCATION_TEXTURE_SRGB);
#undef LOCATION_TO_STR
    if (location) FIXME("Unrecognized location flag(s) %#x.\n", location);

    return buf[0] ? wine_dbg_sprintf("%s", &buf[3]) : "0";
}
