/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2013 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Chia-I Wu <olv@lunarg.com>
 */

#include "genhw/genhw.h"
#include "intel_winsys.h"

#include "ilo_builder.h"
#include "ilo_builder_render.h"
#include "ilo_render_gen.h"
#include "ilo_render_gen7.h"
#include "ilo_render.h"

/* in U0.4 */
struct sample_position {
   uint8_t x, y;
};

/* \see gen6_get_sample_position() */
static const struct sample_position sample_position_1x[1] = {
   {  8,  8 },
};

static const struct sample_position sample_position_4x[4] = {
   {  6,  2 }, /* distance from the center is sqrt(40) */
   { 14,  6 }, /* distance from the center is sqrt(40) */
   {  2, 10 }, /* distance from the center is sqrt(40) */
   { 10, 14 }, /* distance from the center is sqrt(40) */
};

static const struct sample_position sample_position_8x[8] = {
   {  7,  9 }, /* distance from the center is sqrt(2) */
   {  9, 13 }, /* distance from the center is sqrt(26) */
   { 11,  3 }, /* distance from the center is sqrt(34) */
   { 13, 11 }, /* distance from the center is sqrt(34) */
   {  1,  7 }, /* distance from the center is sqrt(50) */
   {  5,  1 }, /* distance from the center is sqrt(58) */
   { 15,  5 }, /* distance from the center is sqrt(58) */
   {  3, 15 }, /* distance from the center is sqrt(74) */
};

struct ilo_render *
ilo_render_create(struct ilo_builder *builder)
{
   struct ilo_render *render;
   int i;

   render = CALLOC_STRUCT(ilo_render);
   if (!render)
      return NULL;

   render->dev = builder->dev;
   render->builder = builder;

   switch (ilo_dev_gen(render->dev)) {
   case ILO_GEN(6):
      ilo_render_init_gen6(render);
      break;
   case ILO_GEN(7):
   case ILO_GEN(7.5):
      ilo_render_init_gen7(render);
      break;
   default:
      assert(!"unsupported GEN");
      FREE(render);
      return NULL;
      break;
   }

   render->workaround_bo = intel_winsys_alloc_buffer(builder->winsys,
         "PIPE_CONTROL workaround", 4096, false);
   if (!render->workaround_bo) {
      ilo_warn("failed to allocate PIPE_CONTROL workaround bo\n");
      FREE(render);
      return NULL;
   }

   render->packed_sample_position_1x =
      sample_position_1x[0].x << 4 |
      sample_position_1x[0].y;

   /* pack into dwords */
   for (i = 0; i < 4; i++) {
      render->packed_sample_position_4x |=
         sample_position_4x[i].x << (8 * i + 4) |
         sample_position_4x[i].y << (8 * i);

      render->packed_sample_position_8x[0] |=
         sample_position_8x[i].x << (8 * i + 4) |
         sample_position_8x[i].y << (8 * i);

      render->packed_sample_position_8x[1] |=
         sample_position_8x[4 + i].x << (8 * i + 4) |
         sample_position_8x[4 + i].y << (8 * i);
   }

   ilo_render_invalidate_hw(render);
   ilo_render_invalidate_builder(render);

   return render;
}

void
ilo_render_destroy(struct ilo_render *render)
{
   if (render->workaround_bo)
      intel_bo_unreference(render->workaround_bo);

   FREE(render);
}

void
ilo_render_get_sample_position(const struct ilo_render *render,
                               unsigned sample_count,
                               unsigned sample_index,
                               float *x, float *y)
{
   const struct sample_position *pos;

   switch (sample_count) {
   case 1:
      assert(sample_index < Elements(sample_position_1x));
      pos = sample_position_1x;
      break;
   case 4:
      assert(sample_index < Elements(sample_position_4x));
      pos = sample_position_4x;
      break;
   case 8:
      assert(sample_index < Elements(sample_position_8x));
      pos = sample_position_8x;
      break;
   default:
      assert(!"unknown sample count");
      *x = 0.5f;
      *y = 0.5f;
      return;
      break;
   }

   *x = (float) pos[sample_index].x / 16.0f;
   *y = (float) pos[sample_index].y / 16.0f;
}

void
ilo_render_invalidate_hw(struct ilo_render *render)
{
   render->hw_ctx_changed = true;
}

void
ilo_render_invalidate_builder(struct ilo_render *render)
{
   render->batch_bo_changed = true;
   render->state_bo_changed = true;
   render->instruction_bo_changed = true;

   /* Kernel flushes everything.  Shouldn't we set all bits here? */
   render->state.current_pipe_control_dw1 = 0;
}

/**
 * Return the command length of ilo_render_emit_flush().
 */
int
ilo_render_get_flush_len(const struct ilo_render *render)
{
   int len;

   ILO_DEV_ASSERT(render->dev, 6, 7.5);

   len = GEN6_PIPE_CONTROL__SIZE;

   /* plus gen6_wa_pre_pipe_control() */
   if (ilo_dev_gen(render->dev) == ILO_GEN(6))
      len *= 3;

   return len;
}

/**
 * Emit PIPE_CONTROLs to flush all caches.
 */
void
ilo_render_emit_flush(struct ilo_render *render)
{
   const uint32_t dw1 = GEN6_PIPE_CONTROL_INSTRUCTION_CACHE_INVALIDATE |
                        GEN6_PIPE_CONTROL_RENDER_CACHE_FLUSH |
                        GEN6_PIPE_CONTROL_DEPTH_CACHE_FLUSH |
                        GEN6_PIPE_CONTROL_VF_CACHE_INVALIDATE |
                        GEN6_PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE |
                        GEN6_PIPE_CONTROL_CS_STALL;

   ILO_DEV_ASSERT(render->dev, 6, 7.5);

   if (ilo_dev_gen(render->dev) == ILO_GEN(6))
      gen6_wa_pre_pipe_control(render, dw1);

   gen6_PIPE_CONTROL(render->builder, dw1, NULL, 0, false);

   render->state.current_pipe_control_dw1 |= dw1;
   render->state.deferred_pipe_control_dw1 &= ~dw1;
}