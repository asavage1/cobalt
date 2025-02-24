/*
 * Copyright © 2018, VideoLAN and dav1d authors
 * Copyright © 2018, Two Orioles, LLC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DAV1D_SRC_LOOPRESTORATION_H
#define DAV1D_SRC_LOOPRESTORATION_H

#include <stdint.h>
#include <stddef.h>

#include "common/bitdepth.h"

enum LrEdgeFlags {
    LR_HAVE_LEFT = 1 << 0,
    LR_HAVE_RIGHT = 1 << 1,
    LR_HAVE_TOP = 1 << 2,
    LR_HAVE_BOTTOM = 1 << 3,
};

#ifdef BITDEPTH
typedef const pixel (*const_left_pixel_row)[4];
#else
typedef const void *const_left_pixel_row;
#endif

// Although the spec applies restoration filters over 4x4 blocks, the wiener
// filter can be applied to a bigger surface.
//    * w is constrained by the restoration unit size (w <= 256)
//    * h is constrained by the stripe height (h <= 64)
// The filter functions are allowed to do aligned writes past the right
// edge of the buffer, aligned up to the minimum loop restoration unit size
// (which is 32 pixels for subsampled chroma and 64 pixels for luma).
#define decl_wiener_filter_fn(name) \
void (name)(pixel *dst, ptrdiff_t dst_stride, \
            const_left_pixel_row left, \
            const pixel *lpf, ptrdiff_t lpf_stride, \
            int w, int h, const int16_t filter[2][8], \
            enum LrEdgeFlags edges HIGHBD_DECL_SUFFIX)
typedef decl_wiener_filter_fn(*wienerfilter_fn);

#define decl_selfguided_filter_fn(name) \
void (name)(pixel *dst, ptrdiff_t dst_stride, \
            const_left_pixel_row left, \
            const pixel *lpf, ptrdiff_t lpf_stride, \
            int w, int h, int sgr_idx, const int16_t sgr_w[2], \
            const enum LrEdgeFlags edges HIGHBD_DECL_SUFFIX)
typedef decl_selfguided_filter_fn(*selfguided_fn);

typedef struct Dav1dLoopRestorationDSPContext {
    wienerfilter_fn wiener[2]; /* 7-tap, 5-tap */
    selfguided_fn selfguided;
} Dav1dLoopRestorationDSPContext;

bitfn_decls(void dav1d_loop_restoration_dsp_init, Dav1dLoopRestorationDSPContext *c, int bpc);
bitfn_decls(void dav1d_loop_restoration_dsp_init_arm, Dav1dLoopRestorationDSPContext *c, int bpc);
bitfn_decls(void dav1d_loop_restoration_dsp_init_x86, Dav1dLoopRestorationDSPContext *c);
bitfn_decls(void dav1d_loop_restoration_dsp_init_ppc, Dav1dLoopRestorationDSPContext *c);

#endif /* DAV1D_SRC_LOOPRESTORATION_H */
