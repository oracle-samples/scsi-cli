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
#include "scsi_iscsi.h"
#include "scsi_print.h"

/*
 * Fill out Transport information for the iscsi host
 */
int get_iscsi_transport(struct iscsi_dev_info *iscsi_info)
{
	struct dirent	*entry;
	DIR		*dir;
	char		path[1024] = { 0 };

	print_trace_enter();

	sprintf(path, "/sys/class/iscsi_transport");

	dir = opendir(path);
	if (unlikely(!dir)) {
		print_debug("\n No iSCSI Transport configured, Check connection \n");
		return -EINVAL;
	}

	for_each_dir(entry, dir) {
		if (!strncmp(entry->d_name, "tcp", 3)) {
			iscsi_info->transport_name = strdup(entry->d_name);
			break;
		}
	}
	closedir(dir);

	return 0;
}

int list_iscsi_devs(struct iscsi_dev_info *iscsi_info)
{
	struct dirent	*entry;
	DIR		*dir;
	int		count = 0;

	print_trace_enter();

	count = get_device_count(SYSFS_ISCSI_HOST_PATH);
	if (count < 0)
		return -ENODEV;

	print_debug("%s: Count %d ", SYSFS_ISCSI_HOST_PATH, count);

	dir = opendir(SYSFS_ISCSI_HOST_PATH);
	if (unlikely(!dir)) {
		print_info("\n No iSCSI host configured \n");
		return -EINVAL;
	}

	print_command_label("iSCSI");

	print_iscsi_dev_header();

	for_each_dir(entry, dir) {
		iscsi_info = alloc_iscsi_dev();
		if (!iscsi_info)
			return -EINVAL;

		iscsi_info->host_name = strdup(entry->d_name);
		iscsi_info->sys_dev_path = strdup(SYSFS_ISCSI_HOST_PATH);

		print_debug("Path: %s Name: %s \n",
		    iscsi_info->session_path, iscsi_info->session_name);

		/*  fill in transport information */
		get_iscsi_transport(iscsi_info);

		fill_session_info(iscsi_info);

		/* Fill connection/address/port information */
		fill_connection_info (iscsi_info);

		print_list_iscsi_dev(iscsi_info);
		put_iscsi_dev(iscsi_info);
	}
	closedir(dir);

	return count;
}

/*
 * Fill out Transport information for the iscsi host
 */
int fill_connection_info(struct iscsi_dev_info *iscsi_info)
{
	DIR		*dir;
	struct dirent	*entry;
	char		session_path[1024] = { 0 };
	char		connection_path[2048] = { 0 };
	char		str[256] = { 0 };

	print_trace_enter();

	print_debug("Host: %s Path: %s \n", iscsi_info->host_name,
	    iscsi_info->sys_dev_path);

	print_debug("Path: %s Session: %s \n", iscsi_info->session_path,
	    iscsi_info->session_name);

	/* Extract Connection Information */
	sprintf(session_path, "/sys/class/iscsi_host/%s/device/%s",
	    iscsi_info->host_name, iscsi_info->session_name);

	dir = opendir(session_path);
	if (unlikely(!dir)) {
		print_info(" No iSCSI Connections found for %s \n",
			iscsi_info->session_name);
		return 0;
	}

	print_debug("Path: %s Session: %s \n", session_path,
	    iscsi_info->session_name);

	for_each_dir(entry, dir) {
		print_trace_enter();
		if (!strncmp(entry->d_name, "connection", 10)) {
			iscsi_info->connection_name = strdup(entry->d_name);
			iscsi_info->connection_count++;
			sprintf(connection_path, "%s/%s", session_path,
			    iscsi_info->connection_name);
			iscsi_info->connection_path = strdup(connection_path);
			print_debug("Path: %s connection %s \n",
				iscsi_info->connection_path,
				iscsi_info->session_name);
			continue;
		}
	}
	closedir(dir);

	/* /sys/class/iscsi_session/session1/targetname */
	memset(session_path, 0, sizeof(session_path));
	sprintf(session_path, "/sys/class/iscsi_session/%s/%s",
	    iscsi_info->session_name, "targetname");
	sprintf(str, open_sysfs_stats_file(session_path));
	iscsi_info->target_name = strdup(str);

	print_debug("Path: %s Target Name:  %s \n",
	    iscsi_info->connection_path, iscsi_info->target_name);

	/* /sys/class/iscsi_connection/connection1:0/address */
	memset(session_path, 0, sizeof(session_path));
	sprintf(connection_path, "/sys/class/iscsi_connection/%s/%s",
	    iscsi_info->connection_name, "address");
	memset(str, 0, sizeof(str));
	sprintf(str, open_sysfs_stats_file(connection_path));
	iscsi_info->conn_address = strdup(str);

	print_debug("Path: %s IP Add: %s \n",
	    iscsi_info->connection_path, iscsi_info->conn_address);

	/* /sys/class/iscsi_connection/connection1:0/port */
	memset(session_path, 0, sizeof(session_path));
	sprintf(connection_path, "/sys/class/iscsi_connection/%s/%s",
	    iscsi_info->connection_name, "port");
	memset(str, 0, sizeof(str));
	sprintf(str, open_sysfs_stats_file(connection_path));
	iscsi_info->conn_port = strdup(str);

	print_debug("Path: %s Conn Port : %s \n",
	    iscsi_info->connection_path, iscsi_info->conn_port);

	return 0;
}

/*
 * Take input structure with host_name and fill up
 * Session related information
 */
int fill_session_info(struct iscsi_dev_info *iscsi_info)
{
	DIR		*dir;
	struct dirent	*entry;
	char	host_path[512] = { 0};
	char	iscsi_dev_path[1024] = { 0 };

	print_trace_enter();

	sprintf(host_path, "%s/%s/device", iscsi_info->sys_dev_path,
		iscsi_info->host_name);

	dir = opendir(host_path);
	if (unlikely(!dir)) {
		print_info(" No iSCSI Sessions found for %s \n",
			iscsi_info->host_name);
		return 0;
	}

	print_debug(" %s %s \n", host_path, iscsi_info->host_name);

	for_each_dir(entry, dir) {
		print_trace_enter();
		if (!strncmp(entry->d_name, "session", 7)) {
			iscsi_info->session_name = strdup(entry->d_name);
			print_debug("(%s) %s: %s \n", __func__, host_path,
			    iscsi_info->session_name);
			sprintf(iscsi_dev_path, "%s/%s", host_path,
				iscsi_info->session_name);
			iscsi_info->session_path = strdup(iscsi_dev_path);
			iscsi_info->session_count++;
			print_debug("(%s) %s: %s \n", __func__,
			    iscsi_info->session_path, iscsi_info->session_name);
			break;
		}
	}
	closedir(dir);

	print_debug(" Session Path %s, Session Name %s ",
		iscsi_info->session_path, iscsi_info->session_name);

	return 0;
}

int show_iscsi_details(char **argv, struct scsi_device_list *s_dev)
{
	struct	iscsi_dev_info		*iscsi_info = s_dev->iscsi_info;
	struct	iscsi_session		*sess;
	struct	iscsi_connection	*conn;
	char	host_path[1024] = { 0};
	int	err = 0;

	print_trace_enter();

	/* Allocate memory for session info */
	sess = malloc(sizeof(struct iscsi_session));
	if (!sess) {
		err = -ENODEV;
		goto sess_err_out;
	}
	iscsi_info->session = sess;

	/* Allocate memory for connection info */
	conn = malloc(sizeof(struct iscsi_connection));
	if (!conn) {
		err = -ENODEV;
		goto conn_err_out;
	}
	iscsi_info->connection = conn;

	/* copy host name from the input arg */
	iscsi_info->host_name = strdup(argv[3]);
	iscsi_info->sys_dev_path = strdup(SYSFS_ISCSI_HOST_PATH);

	print_debug("Show details for %s: %s",
	    argv[2], iscsi_info->host_name);

	snprintf(host_path, sizeof(host_path), "%s/%s/device", iscsi_info->sys_dev_path,
		iscsi_info->host_name);

	/* Fill Session Path and Session Name */
	fill_session_info(iscsi_info);

	/* Get Session Information */
	get_iscsi_session_info(iscsi_info);

	/* Fill Connection Path and Name */
	fill_connection_info(iscsi_info);

	/* Get Connection Information */
	get_iscsi_connection_info(iscsi_info);

conn_err_out:
	if (conn)
		free(conn);
sess_err_out:
	if (sess)
		free(sess);

	return err;
}

int get_iscsi_disk_hctl(struct iscsi_dev_info *iscsi_dev)
{
	struct dirent	*dent;
	DIR		*dir;
	int		count;
	char		disk_path[2048];
	char		disk_name[64];
	char		disk_state[64];

	print_trace_enter();

	/* Open Session Directory location */
	dir = opendir(iscsi_dev->session_disk_path);
	if (!dir)
		return -ENODEV;

	print_iscsi_header("LUN", iscsi_dev->host_name);

	for (dent = readdir(dir); dent; dent = readdir(dir)) {
		char iscsi_disk_path[1024];
		char iscsi_disk_type[32];
		char *end;
		int disk_type;

		print_debug("%s: %s", iscsi_dev->session_disk_path,
			  dent->d_name);

		if (dent->d_type != DT_DIR)
			continue;

		print_trace_enter();

		count = sscanf(dent->d_name, "%d:%d:%d:%d",
			&iscsi_dev->session->scsi_channel,
			&iscsi_dev->session->scsi_bus,
			&iscsi_dev->session->scsi_id,
			&iscsi_dev->session->scsi_lun);

		print_debug("%s: %s count %d", iscsi_dev->session_disk_path,
			dent->d_name, count);

		snprintf(iscsi_disk_path, sizeof(iscsi_disk_path), "%s/%s/%s",
		    iscsi_dev->session_disk_path, dent->d_name, "type");
		snprintf(iscsi_disk_type, sizeof(iscsi_disk_type),
		    open_sysfs_stats_file(iscsi_disk_path));
		disk_type = strtoull(iscsi_disk_type, &end, 0);

		print_debug("%s: %s = %s ( %s )\n", __func__, iscsi_disk_path,
		    iscsi_disk_type, dev_type_to_dev_name(disk_type));

		if (disk_type != STORAGE_ARRAY_CNTROLLER) {
			iscsi_dev->session->lun_count++;

			print_debug("Scsi%d Channel %d ID %d Lun: %d ",
			    iscsi_dev->session->scsi_channel,
			    iscsi_dev->session->scsi_bus,
			    iscsi_dev->session->scsi_id,
			    iscsi_dev->session->scsi_lun);

			if (count == 4) {
				/* Get Disk Name */
				snprintf(disk_path, sizeof(disk_path),
				    "/sys/class/scsi_disk/%s/device/block",
				    dent->d_name);
				snprintf(disk_name, sizeof(disk_name),
				    get_device_entry(disk_path));
				print_debug(" disk Path %s, disk name %s\n",
				    disk_path, disk_name);

				/* Get Disk State */
				memset(disk_path, 0, sizeof(disk_path));
				snprintf(disk_path, sizeof(disk_path),
				    "/sys/class/scsi_disk/%s/device/state",
				    dent->d_name);
				snprintf(disk_state, sizeof(disk_state),
				    open_sysfs_stats_file(disk_path));
				print_debug(" Disk Name: %s, Disk State: %s \n",
				    disk_name, disk_state);

				print_iscsi_scsi_disk(iscsi_dev);
				printf(" \tAttached Scsi Disk: %s\tState: %s \n",
				    disk_name, disk_state);
				printf("\n");
			}
		}
	}
	closedir(dir);

	if (!dent)
		return -ENODEV;

	return 0;
}

int get_session_scsi_disks(struct iscsi_dev_info *iscsi_dev)
{
	DIR		*dir;
	struct dirent   *entry;
	char		disk_attached_path[4096] = { 0 };

	print_trace_enter();

	dir = opendir(iscsi_dev->session_path);
	if (unlikely(!dir))
		return -ENODEV;

	print_debug("Open Session path %s \n", iscsi_dev->session_path);

	/* Extract Number of Disks Attached to this session */
	for_each_dir(entry, dir) {
		print_debug("Compare entry: %s \n", entry->d_name);
		if (strncmp(entry->d_name, "target", 6) == 0) {
			print_debug("Get Disks attached at %s \n",
				entry->d_name);
			snprintf(disk_attached_path, sizeof(disk_attached_path), "%s/%s",
				iscsi_dev->session_path, entry->d_name);
			iscsi_dev->session_disk_path =
				strdup(disk_attached_path);
			get_iscsi_disk_hctl(iscsi_dev);
			continue;
		}
	}
	closedir(dir);

	return 0;
}

int get_iscsi_session_info(struct iscsi_dev_info *iscsi_dev)
{
	struct dirent   *entry;
	DIR		*dir;
	char	disk_attached_path[4096] = { 0 };
	char	session_path[1024] = { 0 };
	char	session_str[2048] = { 0 };
	char	str[256] = { 0 };
	char	*end;
	struct	iscsi_session *sess = iscsi_dev->session;

	print_trace_enter();

	snprintf(session_path, sizeof(session_path), "%s/%s",
	    SYSFS_ISCSI_SESS_PATH, iscsi_dev->session_name);

	/* /sys/class/iscsi_session/session1/initiatorname */
	snprintf(session_str, sizeof(session_str), "%s/%s", session_path,
	    "initiatorname");
	snprintf(str, sizeof(str), open_sysfs_stats_file(session_str));
	sess->initiatorname = strdup(str);

	/* /sys/class/iscsi_session/session1/targetname */
	memset(str, 0, sizeof(str));
	snprintf(session_str, sizeof(session_str), "%s/%s", session_path,
	    "targetname");
	snprintf(str, sizeof(str), open_sysfs_stats_file(session_str));
	sess->targetname = strdup(str);

	/* sys/class/iscsi_session/session1/port */
	memset(session_str, 0, sizeof(session_str));
	snprintf(session_str, sizeof(session_str), "%s/%s", session_path,
	    "target_id");
	sess->target_id = strtoull(open_sysfs_stats_file(session_str), &end, 0);

	/* /sys/class/iscsi_session/session1/state */
	memset(session_str, 0, sizeof(session_str));
	snprintf(session_str, sizeof(session_str), "%s/%s", session_path,
	    "state");
	memset(str, 0, sizeof(str));
	snprintf(str, sizeof(str), open_sysfs_stats_file(session_str));
	sess->state = strdup(str);

	/* /sys/class/iscsi_session/session1/target_state */
	memset(session_str, 0, sizeof(session_str));
	snprintf(session_str, sizeof(session_str), "%s/%s", session_path,
	    "target_state");
	memset(str, 0, sizeof(str));
	snprintf(str, sizeof(str), open_sysfs_stats_file(session_str));
	sess->target_state = strdup(str);

	/* /sys/class/iscsi_session/session1/abort_tmo */
	memset(session_str, 0, sizeof(session_str));
	snprintf(session_str, sizeof(session_str), "%s/%s", session_path,
	    "abort_tmo");
	sess->abort_tmo =
	    strtoull(open_sysfs_stats_file(session_str), &end, 0);

	/* /sys/class/iscsi_session/session1/creator */
	memset(session_str, 0, sizeof(session_str));
	snprintf(session_str, sizeof(session_str), "%s/%s", session_path,
	    "creator");
	sess->creator = strtoull(open_sysfs_stats_file(session_str), &end, 0);

	/* /sys/class/iscsi_session/session1/data_pdu_in_order */
	memset(session_str, 0, sizeof(session_str));
	snprintf(session_str, sizeof(session_str), "%s/%s", session_path,
	    "data_pdu_in_order");
	sess->data_pdu_in_order =
	       strtoull(open_sysfs_stats_file(session_str), &end, 0);

	/* /sys/class/iscsi_session/session1/data_seq_in_order */
	memset(session_str, 0, sizeof(session_str));
	snprintf(session_str, sizeof(session_str), "%s/%s", session_path,
	    "data_seq_in_order");
	sess->data_seq_in_order =
	       strtoull(open_sysfs_stats_file(session_str), &end, 0);

	/* /sys/class/iscsi_session/session1/erl */
	memset(session_str, 0, sizeof(session_str));
	snprintf(session_str, sizeof(session_str), "%s/%s", session_path,
	    "erl");
	sess->err_level =
	    strtoull(open_sysfs_stats_file(session_str), &end, 0);

	/* /sys/class/iscsi_session/session1/fast_abort */
	memset(session_str, 0, sizeof(session_str));
	snprintf(session_str, sizeof(session_str), "%s/%s", session_path,
	    "fast_abort");
	sess->fast_abort =
	    strtoull(open_sysfs_stats_file(session_str), &end, 0);

	/* /sys/class/iscsi_session/session1/first_burst_len */
	memset(session_str, 0, sizeof(session_str));
	snprintf(session_str, sizeof(session_str), "%s/%s", session_path,
	    "first_burst_len");
	sess->first_burst_len =
	    strtoull(open_sysfs_stats_file(session_str), &end, 0);

	/* /sys/class/iscsi_session/session1/targetname */
	memset(session_str, 0, sizeof(session_str));
	memset(str, 0, sizeof(str));
	snprintf(session_str, sizeof(session_str), "%s/%s", session_path,
	    "ifacename");
	snprintf(str, sizeof(str), open_sysfs_stats_file(session_str));
	sess->ifacename = strdup(str);

	/* /sys/class/iscsi_session/session1/immediate_data */
	memset(session_str, 0, sizeof(session_str));
	snprintf(session_str, sizeof(session_str), "%s/%s", session_path,
	    "immediate_data");
	sess->immediate_data =
	    strtoull(open_sysfs_stats_file(session_str), &end, 0);

	/* /sys/class/iscsi_session/session1/initial_r2t */
	memset(session_str, 0, sizeof(session_str));
	snprintf(session_str, sizeof(session_str), "%s/%s", session_path,
	    "initial_r2t");
	sess->initial_r2t =
	    strtoull(open_sysfs_stats_file(session_str), &end, 0);

	/* /sys/class/iscsi_session/session1/lun_reset_tmo */
	memset(session_str, 0, sizeof(session_str));
	snprintf(session_str, sizeof(session_str), "%s/%s", session_path,
	    "lu_reset_tmo");
	sess->lu_reset_tmo =
	    strtoull(open_sysfs_stats_file(session_str), &end, 0);

	/* /sys/class/iscsi_session/session1/max_burst_len */
	memset(session_str, 0, sizeof(session_str));
	snprintf(session_str, sizeof(session_str), "%s/%s", session_path,
	    "max_burst_len");
	sess->max_burst_len =
	    strtoull(open_sysfs_stats_file(session_str), &end, 0);

	/* /sys/class/iscsi_session/session1/max_outstanding_r2t */
	memset(session_str, 0, sizeof(session_str));
	snprintf(session_str, sizeof(session_str), "%s/%s", session_path,
	    "max_outstanding_r2t");
	sess->max_outstanding_r2t =
	    strtoull(open_sysfs_stats_file(session_str), &end, 0);

	/* /sys/class/iscsi_session/session1/recovery_tmo */
	memset(session_str, 0, sizeof(session_str));
	snprintf(session_str, sizeof(session_str), "%s/%s", session_path,
	    "recovery_tmo");
	sess->recovery_tmo =
	    strtoull(open_sysfs_stats_file(session_str), &end, 0);

	dir = opendir(iscsi_dev->session_path);
	if (unlikely(!dir))
		return -ENODEV;

	print_debug("Open Session path %s \n", iscsi_dev->session_path);

	print_iscsi_header("Session", iscsi_dev->session_name);

	print_iscsi_session_info(iscsi_dev->session);

	/* Extract Number of Disks Attached to this session */
	for_each_dir(entry, dir) {
		print_debug("Compare entry %s \n", entry->d_name);
		if (strncmp(entry->d_name, "target", 6) == 0) {
			print_trace_enter();
			print_debug("Get Disks attached at %s \n",
				entry->d_name);
			snprintf(disk_attached_path,  sizeof(disk_attached_path),"%s/%s",
				iscsi_dev->session_path,
				entry->d_name);
			iscsi_dev->session_disk_path =
				strdup(disk_attached_path);
			get_session_scsi_disks(iscsi_dev);
			break;
		}
	}

	print_debug(" Session str %s , InitiatorName %s targetname %s",
		session_str, sess->initiatorname, sess->targetname);

	return 0;
}

int get_iscsi_connection_info(struct iscsi_dev_info *iscsi_dev)
{
	char	conn_path[1024] = { 0 };
	char	conn_str[2048] = { 0 };
	char	str[256] = { 0 };
	char	*end;
	struct	iscsi_connection *conn = iscsi_dev->connection;

	print_trace_enter();

	sprintf(conn_path, "%s/%s", SYSFS_ISCSI_CONN_PATH,
		iscsi_dev->connection_name);

	/* /sys/class/iscsi_connection/connection1:0/address */
	sprintf(conn_str, "%s/%s", conn_path, "address");
	sprintf(str, open_sysfs_stats_file(conn_str));
	conn->address = strdup(str);

	/* /sys/class/iscsi_connection/connection1:0/max_recv_dlength */
	memset(conn_str, 0, sizeof(conn_str));
	sprintf(conn_str, "%s/%s", conn_path, "max_recv_dlength");
	conn->max_recv_dlength =
	       strtoull(open_sysfs_stats_file(conn_str), &end, 0);

	/* /sys/class/iscsi_connection/connection1:0/max_xmit_dlength */
	memset(conn_str, 0, sizeof(conn_str));
	sprintf(conn_str, "%s/%s", conn_path, "max_xmit_dlength");
	conn->max_xmit_dlength =
	       strtoull(open_sysfs_stats_file(conn_str), &end, 0);

	/* /sys/class/iscsi_connection/connection1:0/data_digest */
	memset(conn_str, 0, sizeof(conn_str));
	sprintf(conn_str, "%s/%s", conn_path, "data_digest");
	conn->data_digest = strtoull(open_sysfs_stats_file(conn_str), &end, 0);

	/* /sys/class/iscsi_connection/connection1:0/exp_statsn */
	memset(conn_str, 0, sizeof(conn_str));
	sprintf(conn_str, "%s/%s", conn_path, "exp_statsn");
	conn->exp_statsn = strtoull(open_sysfs_stats_file(conn_str), &end, 0);

	/* /sys/class/iscsi_connection/connection1:0/header_digest */
	memset(conn_str, 0, sizeof(conn_str));
	sprintf(conn_str, "%s/%s", conn_path, "header_digest");
	conn->header_digest = strtoull(open_sysfs_stats_file(conn_str), &end, 0);

	/* /sys/class/iscsi_connection/connection1:0/persistent_address */
	sprintf(conn_str, "%s/%s", conn_path, "persistent_address");
	sprintf(str, open_sysfs_stats_file(conn_str));
	conn->persistent_address = strdup(str);

	/* /sys/class/iscsi_connection/connection1:0/persistent_port */
	memset(conn_str, 0, sizeof(conn_str));
	sprintf(conn_str, "%s/%s", conn_path, "persistent_port");
	conn->persistent_port = strtoull(open_sysfs_stats_file(conn_str), &end, 0);

	/* /sys/class/iscsi_connection/connection1:0/ping_tmo */
	memset(conn_str, 0, sizeof(conn_str));
	sprintf(conn_str, "%s/%s", conn_path, "ping_tmo");
	conn->ping_tmo = strtoull(open_sysfs_stats_file(conn_str), &end, 0);

	/* /sys/class/iscsi_connection/connection1:0/port */
	memset(conn_str, 0, sizeof(conn_str));
	sprintf(conn_str, "%s/%s", conn_path, "port");
	conn->port = strtoull(open_sysfs_stats_file(conn_str), &end, 0);

	/* /sys/class/iscsi_connection/connection1:0/recv_tmo */
	memset(conn_str, 0, sizeof(conn_str));
	sprintf(conn_str, "%s/%s", conn_path, "recv_tmo");
	conn->recv_tmo = strtoull(open_sysfs_stats_file(conn_str), &end, 0);

	/* /sys/class/iscsi_connection/connection1:0/state */
	sprintf(conn_str, "%s/%s", conn_path, "state");
	sprintf(str, open_sysfs_stats_file(conn_str));
	conn->state = strdup(str);

	print_iscsi_header("Connection", iscsi_dev->connection_name);

	print_iscsi_connection_info(iscsi_dev->connection);

	print_debug("State: %s persistenit_addr: %s \n",
		conn->state, conn->persistent_address);

	return 0;
}

int get_iscsi_info(struct iscsi_dev_info *iscsi_dev)
{
	DIR			*dir;
	struct	dirent		*entry;
	struct	iscsi_host		*host;
	struct  iscsi_session 		*session;
	struct  iscsi_connection	*connection;
	char	iscsi_sysfs_host_path[512] = { 0 };
	char	iscsi_dev_path[1024] = { 0 };
	char	session_path[2048] = { 0 };
	char	str[64] = { 0 };
	int err = 0;

	print_trace_enter();

	sprintf(iscsi_sysfs_host_path, "%s/%s",
	    SYSFS_ISCSI_HOST_PATH, iscsi_dev->host_name);

	print_debug("Path - %s, Host Name: %s \n", iscsi_sysfs_host_path,
		iscsi_dev->host_name);

	/* Allocate memory for host information */
	host = malloc(sizeof(struct iscsi_host));
	if (!host) {
		err = -EINVAL;
		goto err_out;
	}
	iscsi_dev->host = host;

	/* Allocate memory for Session information */
	session = malloc(sizeof(struct iscsi_session));
	if (!session) {
		err = -ENODEV;
		goto sess_err_out;
	}
	iscsi_dev->session = session;

	/* Allocate memory for Connection information */
	connection = malloc(sizeof(struct  iscsi_connection));
	if (!connection) {
		err = -ENODEV;
		goto conn_err_out;
	}
	iscsi_dev->connection = connection;

	sprintf(iscsi_dev_path, "%s/%s", iscsi_sysfs_host_path, "hwaddress");
	sprintf(str, open_sysfs_stats_file(iscsi_dev_path));
	host->hwaddress = strdup(str);

	sprintf(iscsi_dev_path, "%s/%s", iscsi_sysfs_host_path, "ipaddress");
	sprintf(str, open_sysfs_stats_file(iscsi_dev_path));
	host->ipaddress = strdup(str);

	sprintf(iscsi_dev_path, "%s/%s", iscsi_sysfs_host_path, "netdev");
	sprintf(str, open_sysfs_stats_file(iscsi_dev_path));
	host->netdev = strdup(str);

	/* Extract Session Information */
	sprintf(iscsi_dev_path, "%s/device", iscsi_sysfs_host_path);
	dir = opendir(iscsi_dev_path);
	if (unlikely(!dir)) {
		print_info(" No iSCSI Sessions found for %s \n",
			iscsi_dev->host_name);
		err = -ENODEV;
		goto conn_err_out;
	}

	print_debug("Session_Path %s \n", iscsi_dev_path);

	for_each_dir(entry, dir) {
		print_trace_enter();

		if (strncmp(entry->d_name, "session", 7) == 0) {
			iscsi_dev->session_name = strdup(entry->d_name);
			sprintf(session_path, "%s/%s", iscsi_dev_path,
				iscsi_dev->session_name);
			iscsi_dev->session_path = strdup(session_path);
			iscsi_dev->session_count++;
			print_debug("Session Name: %s Path %s \n",
				iscsi_dev->session_name, iscsi_dev->session_path);

			get_iscsi_session_info(iscsi_dev);
			continue;
		}
	}
	closedir(dir);

	/* Extract Connection Information */
	sprintf(iscsi_dev_path, "%s/device/%s", iscsi_sysfs_host_path,
		iscsi_dev->session_name);

	dir = opendir(iscsi_dev_path);
	if (unlikely(!dir)) {
		print_info(" No iSCSI Connections found for %s \n",
			iscsi_dev->session_name);
		err = -ENODEV;
		goto conn_err_out;
	}

	for_each_dir(entry, dir) {
		print_trace_enter();
		if (!strncmp(entry->d_name, "connection", 10)) {
			iscsi_dev->connection_name = strdup(entry->d_name);
			iscsi_dev->connection_count++;
			get_iscsi_connection_info(iscsi_dev);
			continue;
		}
	}
	closedir(dir);

	print_debug("Path - %s, Host Name: %s, Session: %s Session count :%d Connection: %s Connection Count: %d \n",
		iscsi_sysfs_host_path, iscsi_dev->host_name,
		iscsi_dev->session_name, iscsi_dev->session_count,
		iscsi_dev->connection_name, iscsi_dev->connection_count);

conn_err_out:
	if (connection)
		free(connection);
sess_err_out:
	if (session)
		free(session);
err_out:
	if (host)
		free(host);

	return err;
}

/**
 * Get Eror Count for a iSCSI device
 */
int get_iscsi_error_count(char **argv, struct iscsi_dev_info *iscsi_dev)
{
	int	err = 0;
	char	iscsi_sysfs_host_path[256];

	if (strlen(argv[3]) < 4)
		return -EIO;

	iscsi_dev = alloc_iscsi_dev();
	if (!iscsi_dev)
		return -ENODEV;

	iscsi_dev->host_name = strdup(argv[3]);

	sprintf(iscsi_sysfs_host_path, "%s/%s",
	    SYSFS_ISCSI_HOST_PATH, iscsi_dev->host_name);

	print_debug("Path - %s, Host Name: %s \n", iscsi_sysfs_host_path,
		iscsi_dev->host_name);

	iscsi_dev->sys_dev_path = strdup(iscsi_sysfs_host_path);

	put_iscsi_dev(iscsi_dev);

	return err;
}

/**
 * Issue scan command for an iSCSI device
 */
int scsi_scan_iscsi_dev(char **argv, struct iscsi_dev_info *iscsi_dev)
{
	int	err = 0;
	char	iscsi_sysfs_host_path[256];

	if (strlen(argv[3]) < 4)
		return -EIO;

	iscsi_dev = alloc_iscsi_dev();
	if (!iscsi_dev)
		return -ENODEV;

	iscsi_dev->host_name = strdup(argv[3]);

	sprintf(iscsi_sysfs_host_path, "%s/%s",
	    SYSFS_ISCSI_HOST_PATH, iscsi_dev->host_name);

	print_debug("Path - %s, Host Name: %s \n", iscsi_sysfs_host_path,
		iscsi_dev->host_name);

	iscsi_dev->sys_dev_path = strdup(iscsi_sysfs_host_path);

	put_iscsi_dev(iscsi_dev);

	return err;
}

/**
 * set iSCSI device offline
 */
int set_iscsi_dev_offline(char **argv, struct iscsi_dev_info *iscsi_dev)
{
	int	err = 0;
	char	iscsi_sysfs_host_path[256];

	if (strlen(argv[3]) < 4)
		return -EIO;

	iscsi_dev = alloc_iscsi_dev();
	if (!iscsi_dev)
		return -ENODEV;

	iscsi_dev->host_name = strdup(argv[3]);

	sprintf(iscsi_sysfs_host_path, "%s/%s",
	    SYSFS_ISCSI_HOST_PATH, iscsi_dev->host_name);

	print_debug("Path - %s, Host Name: %s \n", iscsi_sysfs_host_path,
		iscsi_dev->host_name);

	iscsi_dev->sys_dev_path = strdup(iscsi_sysfs_host_path);

	put_iscsi_dev(iscsi_dev);

	return err;
}

/**
 * set iSCSI device online
 */
int set_iscsi_dev_online(char **argv, struct iscsi_dev_info *iscsi_dev)
{
	int	err = 0;
	char	iscsi_sysfs_host_path[256];

	if (strlen(argv[3]) < 4)
		return -EIO;

	iscsi_dev = alloc_iscsi_dev();
	if (!iscsi_dev)
		return -ENODEV;

	iscsi_dev->host_name = strdup(argv[3]);

	sprintf(iscsi_sysfs_host_path, "%s/%s",
	    SYSFS_ISCSI_HOST_PATH, iscsi_dev->host_name);

	print_debug("Path - %s, Host Name: %s \n", iscsi_sysfs_host_path,
		iscsi_dev->host_name);

	iscsi_dev->sys_dev_path = strdup(iscsi_sysfs_host_path);

	put_iscsi_dev(iscsi_dev);

	return err;
}

/**
 * set iSCSI device alias to favorite name
 */
int set_iscsi_alias(char **argv, struct iscsi_dev_info *iscsi_dev)
{
	int	err = 0;
	char	iscsi_sysfs_host_path[256];

	if (strlen(argv[3]) < 4)
		return -EIO;

	iscsi_dev = alloc_iscsi_dev();
	if (!iscsi_dev)
		return -ENODEV;

	iscsi_dev->host_name = strdup(argv[3]);

	sprintf(iscsi_sysfs_host_path, "%s/%s",
	    SYSFS_ISCSI_HOST_PATH, iscsi_dev->host_name);

	print_debug("Path - %s, Host Name: %s \n", iscsi_sysfs_host_path,
		iscsi_dev->host_name);

	iscsi_dev->sys_dev_path = strdup(iscsi_sysfs_host_path);

	put_iscsi_dev(iscsi_dev);

	return err;
}
