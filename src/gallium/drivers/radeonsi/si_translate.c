/*
 * Copyright 2010 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Dave Airlie <airlied@redhat.com>
 */

#include "util/u_index_modify.h"
#include "util/u_upload_mgr.h"
#include "si_pipe.h"


void r600_translate_index_buffer(struct si_context *r600,
				 struct pipe_index_buffer *ib,
				 unsigned count)
{
	struct pipe_resource *out_buffer = NULL;
	unsigned out_offset;
	void *ptr;

	switch (ib->index_size) {
	case 1:
		u_upload_alloc(r600->b.uploader, 0, count * 2,
			       &out_offset, &out_buffer, &ptr);

		util_shorten_ubyte_elts_to_userptr(
				&r600->b.b, ib, 0, ib->offset, count, ptr);

		pipe_resource_reference(&ib->buffer, NULL);
		ib->buffer = out_buffer;
		ib->offset = out_offset;
		ib->index_size = 2;
		break;
	}
}