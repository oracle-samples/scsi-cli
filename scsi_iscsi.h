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

#ifndef _SCSI_ISCSI_H
#define _SCSI_ISCSI_H

#include "scsi.h"

struct iscsi_session {
	int	abort_tmo;		/* Abort Timeout */
	int	creator;		/* Creator ID */
	int	data_pdu_in_order;	/* Data PDU */
	int	data_seq_in_order;	/* Data Sequence */
	int	err_level;		/* Err=1,2 */
	int	fast_abort;
	int	first_burst_len;
	char	*ifacename;
	int	immediate_data;
	int	initial_r2t;
	char	*initiatorname;
	int	lu_reset_tmo;
	int	max_burst_len;
	int	max_outstanding_r2t;
	int	recovery_tmo;
	int	target_id;
	char	*targetname;
	char	*target_state;
	char	*state;
	int	tgt_reset_tmo;
	int	tpgt;
	int	scsi_channel;
	int	scsi_bus;
	int	scsi_id;
	int	scsi_lun;
	int	lun_count;
};

struct iscsi_connection {
	char	*address;
	int	data_digest;
	int	exp_statsn;
	int	header_digest;
	int	local_port;
	int	max_recv_dlength;
	int	max_xmit_dlength;
	char	*persistent_address;
	int	persistent_port;
	int	ping_tmo;
	int	port;
	int	recv_tmo;
	char	*state;
};

struct iscsi_host {
	char	*hwaddress;
	char	*ipaddress;
	char	*initiatorname;
	char	*netdev;
};

struct iscsi_dev_info {
	char	*host_name;
	char	*session_name;
	char	*connection_name;
	char	*transport_name;

	/* TargetName, Target address, Target Port */
	char	*target_name;
	char	*conn_address;
	char	*conn_port;

	/* these are convenient path saved for easy access */
	char	*sys_dev_path;
	char	*connection_path;
	char 	*session_path;
	char	*session_disk_path;

	/* number of sessions/connections for this iscsi host */
	int	session_count;
	int	connection_count;

	/* these will be used for 'show' command */
	struct	iscsi_host	*host;
	struct	iscsi_connection *connection;
	struct 	iscsi_session	 *session;
};

int get_iscsi_info(struct iscsi_dev_info *);
int get_iscsi_session_info(struct iscsi_dev_info *);
int get_session_scsi_disks(struct iscsi_dev_info *);
int get_iscsi_connection_info(struct iscsi_dev_info *);
int get_iscsi_disk_hctl(struct iscsi_dev_info *);
int show_iscsi_details(char **, struct scsi_device_list *);
int get_iscsi_error_count(char **, struct iscsi_dev_info *);
int scsi_scan_iscsi_dev(char **, struct iscsi_dev_info *);
int set_iscsi_dev_offline(char **, struct iscsi_dev_info *);
int set_iscsi_dev_online(char **, struct iscsi_dev_info *);
int set_iscsi_alias(char **, struct iscsi_dev_info *);

int get_iscsi_transport(struct iscsi_dev_info *);
int fill_connection_info(struct iscsi_dev_info *);
int fill_session_info(struct iscsi_dev_info *);
int list_iscsi_devs(struct iscsi_dev_info *);
#endif
