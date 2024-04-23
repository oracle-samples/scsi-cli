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

#ifndef _SCSI_PRINT_H
#define _SCSI_PRINT_H

#include "scsi.h"
#include "scsi_fcp.h"
#include "scsi_iscsi.h"

void print_list_help();
void print_stats_help();

/* Various helper for printing output */
void print_command_label(char *);
void print_disk_header(void);
void print_disk_info(struct scsi_device_info *);
void print_disk_stats(struct disk_stats *, char *);
void print_mpath_disk_header(void);
void print_mpath_disk_info(struct scsi_device_info *);
void print_nvme_disk_header(void);
void print_nvme_disk_info(struct scsi_device_info *);
void print_generic_disk_header(void);
void print_generic_disk_info(struct scsi_device_info *);

void print_scsi_disk_details(struct scsi_device_info *);
void print_nvme_disk_details(struct scsi_device_info *);
void print_scsi_queue_data(struct disk_queue_data *);

void print_enclosure_header();
void print_enclosure_info(struct scsi_device_info *);

void print_fc_info(struct fc_device_info *);
void print_fc_rport_header(void);
void print_fc_rport_info(struct fc_rport_info *);
void print_fc_dev_header(void);
void print_list_fc_dev(struct fc_device_info *);
void print_fc_port_stats(struct fc_device_info *);

void print_iscsi_dev_header();
void print_iscsi_header(char *, char *);
void print_list_iscsi_dev(struct iscsi_dev_info *);
void print_iscsi_session_info(struct iscsi_session *);
void print_iscsi_connection_info(struct iscsi_connection *);
void print_iscsi_scsi_disk(struct iscsi_dev_info *);
#endif
