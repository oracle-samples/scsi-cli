// SPDX-License-Identifier: UPL-1.0
/*
 * Copyright (c) 2024, Oracle and/or its affiliates.
 *
 * The Universal Permissive License (UPL), Version 1.0
 *
 * Subject to the condition set forth below, permission is hereby granted to any
 * person obtaining a copy of this software, associated documentation and/or data
 * (collectively the "Software"), free of charge and under any and all copyright
 * rights in the Software, and any and all patent rights owned or freely
 * licensable by each licensor hereunder covering either (i) the unmodified
 * Software as contributed to or provided by such licensor, or (ii) the Larger
 * Works (as defined below), to deal in both
 *
 * (a) the Software, and
 * (b) any piece of software and/or hardware listed in the
 * lrgrwrks.txt file if one is included with the Software (each a "Larger
 * Work" to which the Software is contributed by such licensors),
 *
 * without restriction, including without limitation the rights to copy, create
 * derivative works of, display, perform, and distribute the Software and make,
 * use, sell, offer for sale, import, export, have made, and have sold the
 * Software and the Larger Work(s), and to sublicense the foregoing rights on
 * either these or other terms.
 *
 * This license is subject to the following condition:
 * The above copyright notice and either this complete permission notice or at
 * a minimum a reference to the UPL must be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "scsi.h"
#include "scsi_fcp.h"
#include "scsi_iscsi.h"
#include "utils.h"

struct scsi_device_info *alloc_scsi_dev(void)
{
	struct scsi_device_info *s_info;

	print_trace_enter();
	s_info = malloc(sizeof(struct scsi_device_info));
	if (!s_info)
		return NULL;

	memset(s_info, 0, sizeof(*s_info));

	print_debug("Allocated %ld bytes for scsi_device_info", sizeof(*s_info));

	return s_info;
}

struct fc_device_info *alloc_fc_dev(void)
{
	struct fc_device_info *fc_dev;

	print_trace_enter();
	fc_dev = malloc(sizeof(struct fc_device_info));
	if (!fc_dev)
		return NULL;

	memset(fc_dev, 0, sizeof(*fc_dev));

	print_debug("Allocated %ld bytes for fc_device_info", sizeof(*fc_dev));

	return fc_dev;
}

struct iscsi_dev_info *alloc_iscsi_dev()
{
	struct iscsi_dev_info	*iscsi_dev;

	print_trace_enter();

	iscsi_dev = malloc(sizeof(struct iscsi_dev_info));
	if (!iscsi_dev)
		return NULL;

	memset(iscsi_dev, 0, sizeof(*iscsi_dev));

	print_debug("Allocated %ld bytes for iscsi_dev_info", sizeof(*iscsi_dev));

	return iscsi_dev;
}

void put_fc_dev(struct fc_device_info *fc_dev)
{
	if (fc_dev)
		free(fc_dev);
}

void put_scsi_dev(struct scsi_device_info *scsi_dev)
{
	if (scsi_dev)
		free(scsi_dev);
}

void put_iscsi_dev(struct iscsi_dev_info *iscsi_dev)
{
	if (iscsi_dev)
		free(iscsi_dev);
}
