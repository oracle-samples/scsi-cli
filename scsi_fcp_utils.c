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
#include "scsi_print.h"

void get_symbolic_name(struct fc_device_info *fc_dev)
{
	FILE		*fp;
	char		*p;
	char		path[64], line[256];

	print_trace_enter();

	if (!fc_dev->host_name) {
		print_info("Please provide valid FC Host Name \n");
		return;
	}

	/* Extract Symbolic Name*/
	snprintf(path, sizeof(path), "%s/symbolic_name", fc_dev->sys_dev_path);

	fp = fopen(path, "r");
	if (fp == NULL) {
		print_info("Can not open %s\n", path);
		return;
	}

	p = fgets(line, sizeof(line), fp);
	if (!p) {
		fclose(fp);
		return;
	}

	remove_newline(p);
	fc_dev->symbolic_name = strdup(p);

	print_debug("Host Name: %s, Symbolic Name %s",
	    fc_dev->host_name, fc_dev->symbolic_name);

	fclose(fp);
}

void get_driver_info(struct fc_device_info *fc_dev_p)
{
	DIR			*dir;
	struct dirent		*d_entry;
	char			*pci_driver_path;

	print_trace_enter();

	pci_driver_path = strdup("/sys/bus/pci/drivers");

	dir = opendir(pci_driver_path);
	if (!dir)
		perror(pci_driver_path);

	for(d_entry = readdir(dir); d_entry; d_entry = readdir(dir)) {
		if (d_entry->d_type != DT_DIR)
			continue;

		print_debug("Driver Name: %s", d_entry->d_name);

		if ((strncmp(d_entry->d_name, "qla2xxx", 7) == 0) ||
		    (strncmp(d_entry->d_name, "lpfc", 4) == 0)) {
			fc_dev_p->driver_name = strdup(d_entry->d_name);
			break;
		}
	}
	closedir(dir);

	print_debug("Host Name: %s, Driver Name: %s",
	    fc_dev_p->host_name, fc_dev_p->driver_name);

	if (pci_driver_path)
		free(pci_driver_path);
}

void get_rport_info(struct fc_rport_info *fc_rprt)
{
	char	temp_path[256];
	char	temp_val[32];
	char 	rport_path[64];

	print_trace_enter();

	snprintf(rport_path, sizeof(rport_path) ,"%s/%s", SYSFS_FC_RPRT_PATH,
	    fc_rprt->rport_name);

	print_debug("Rport Path: %s Rport Name: %s len %ld\n",
	    rport_path, fc_rprt->rport_name, strlen(rport_path));

	/* Extract Node Name */
	memset(temp_path, 0, sizeof(temp_path));
	snprintf(temp_path, sizeof(temp_path), "%s/node_name", rport_path);
	snprintf(temp_val, sizeof(temp_val), open_sysfs_stats_file(temp_path));
	fc_rprt->node_name = strdup(temp_val);

	/*  Extract Port ID */
	memset(temp_path, 0, sizeof(temp_path));
	snprintf(temp_path, sizeof(temp_path), "%s/port_id", rport_path);
	snprintf(temp_val, sizeof(temp_val), open_sysfs_stats_file(temp_path));
	fc_rprt->port_id = strdup(temp_val);

	/* Extract Port Name */
	memset(temp_path, 0, sizeof(temp_path));
	snprintf(temp_path, sizeof(temp_path), "%s/port_name", rport_path);
	snprintf(temp_val, sizeof(temp_val), open_sysfs_stats_file(temp_path));
	fc_rprt->port_name = strdup(temp_val);

	/* Extract Port State */
	memset(temp_path, 0, sizeof(temp_path));
	snprintf(temp_path, sizeof(temp_path), "%s/port_state", rport_path);
	snprintf(temp_val, sizeof(temp_val), open_sysfs_stats_file(temp_path));
	fc_rprt->port_state = strdup(temp_val);

	/* Extract Rport Roles */
	memset(temp_path, 0, sizeof(temp_path));
	snprintf(temp_path, sizeof(temp_path), "%s/roles", rport_path);
	snprintf(temp_val, sizeof(temp_val), open_sysfs_stats_file(temp_path));
	fc_rprt->roles = strdup(temp_val);
}

int list_rport_adapters(struct fc_device_info *fc_dev)
{
	struct fc_rport_info *fc_rprt;
	struct dirent	*entry;
	DIR		*dir;
	char		rport_path[64];

	print_trace_enter();

	if (!fc_dev) {
		print_info("Unable to allocate memory");
		return -EINVAL;
	}

	snprintf(rport_path, sizeof(rport_path), "%s/device",
	    fc_dev->sys_dev_path);

	dir = opendir(rport_path);
	if (unlikely(!dir)) {
		print_err("No Remote Ports found for this adapter");
		fc_dev->no_rports = 0;
		return 0;
	}

	print_trace_enter();

	print_fc_rport_header();

	for_each_dir(entry, dir) {
		fc_rprt = malloc(sizeof(struct fc_rport_info));
		if (strncmp(entry->d_name, "rport", 5) == 0) {
			print_trace_enter();

			fc_rprt->rport_name = strdup(entry->d_name);

			get_rport_info(fc_rprt);
			print_fc_rport_info(fc_rprt);
			continue;
		}
		if (fc_rprt)
			free(fc_rprt);
	}
	closedir(dir);

	return 0;
}

int get_adapter_details(struct fc_device_info *fc_dev_p)
{
	char		path[1024];
	char		temp_val[128];
	char		*end;
	struct dirent	*entry;
	DIR		*dir;
	int		rport_cnt = 0;

	print_trace_enter();

	/*  Extract Port ID */
	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "%s/port_id", fc_dev_p->sys_dev_path);
	memset(temp_val, 0, sizeof(temp_val));
	snprintf(temp_val, sizeof(temp_val), open_sysfs_stats_file(path));
	fc_dev_p->port_id = strdup(temp_val);
	remove_space(fc_dev_p->port_id);

	/* Extract Port Name */
	memset(path, 0, sizeof(path));
	sprintf(path, "%s/port_name", fc_dev_p->sys_dev_path);
	memset(temp_val, 0, sizeof(temp_val));
	sprintf(temp_val, open_sysfs_stats_file(path));
	fc_dev_p->port_name = strdup(temp_val);
	remove_space(fc_dev_p->port_name);

	/* Extract Port State */
	memset(path, 0, sizeof(path));
	sprintf(path, "%s/port_state", fc_dev_p->sys_dev_path);
	memset(temp_val, 0, sizeof(temp_val));
	sprintf(temp_val, open_sysfs_stats_file(path));
	fc_dev_p->port_state = strdup(temp_val);
	remove_space(fc_dev_p->port_state);

	/* Extract Port Type */
	memset(path, 0, sizeof(path));
	sprintf(path, "%s/port_type", fc_dev_p->sys_dev_path);
	memset(temp_val, 0, sizeof(temp_val));
	sprintf(temp_val, open_sysfs_stats_file(path));
	fc_dev_p->port_type = strdup(temp_val);

	/* Extract Node Name */
	memset(path, 0, sizeof(path));
	sprintf(path, "%s/node_name", fc_dev_p->sys_dev_path);
	memset(temp_val, 0, sizeof(temp_val));
	sprintf(temp_val, open_sysfs_stats_file(path));
	fc_dev_p->node_name = strdup(temp_val);
	remove_space(fc_dev_p->node_name);

	/*  /sys/class/fc_host/host10/max_npiv_vports */
	memset(path, 0, sizeof(path));
	sprintf(path, "%s/max_npiv_vports", fc_dev_p->sys_dev_path);
	fc_dev_p->max_npiv_vports =
	    strtoull(open_sysfs_stats_file(path), &end, 0);

	/*  /sys/class/fc_host/host10/npiv_vports_inuse */
	memset(path, 0, sizeof(path));
	sprintf(path, "%s/npiv_vports_inuse", fc_dev_p->sys_dev_path);
	sprintf(temp_val, open_sysfs_stats_file(path));
	fc_dev_p->npiv_vports_inuse = strtoull(temp_val, &end, 0);

	/*  /sys/class/fc_host/host10/fabric_name */
	memset(path, 0, sizeof(path));
	sprintf(path, "%s/fabric_name", fc_dev_p->sys_dev_path);
	memset(temp_val, 0, sizeof(temp_val));
	sprintf(temp_val, open_sysfs_stats_file(path));
	fc_dev_p->fabric_name = strdup(temp_val);

	/* /sys/class/fc_host/host10/device/rport-* */
	sprintf(path, "%s/device", fc_dev_p->sys_dev_path);
	dir = opendir(path);
	if (unlikely(!dir)) {
		print_info("No Remote port for this FC Adapter found");
		fc_dev_p->no_rports = rport_cnt;
		return 0;
	}

	for_each_dir(entry, dir) {
		print_trace_enter();

		if (!strncmp(entry->d_name, "rport", 5)) {
			rport_cnt++;
			continue;
		}
		fc_dev_p->no_rports = rport_cnt;
	}
	closedir(dir);

	return 0;
}

int get_fc_info(struct fc_device_info *fc_dev)
{
	FILE		*fp;
	char		scsi_path[64];
	char		path[2048], sym_name[128];
	char		*end, *p;
	char		line[64];

	print_trace_enter();

	snprintf(scsi_path, sizeof(scsi_path), "%s/device/scsi_host/%s",
	    fc_dev->sys_dev_path, fc_dev->host_name);

	if (strncmp(fc_dev->driver_name, "qla2xxx", 7) == 0) {
		print_trace_enter();

		/* /sys/class/fc_host/host10/device/scsi_host/host10/model_desc */
		memset(sym_name, 0, sizeof(sym_name));
		snprintf(path, sizeof(path), "%s/%s", scsi_path, "model_desc");
		snprintf(sym_name, 64, open_sysfs_stats_file(path));
		fc_dev->model_desc = strdup(sym_name);

		/* /sys/class/fc_host/host10/device/scsi_host/host10/model_name */
		memset(sym_name, 0, sizeof(sym_name));
		snprintf(path, sizeof(path), "%s/%s", scsi_path, "model_name");
		snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
		fc_dev->product_name = strdup(sym_name);

		/* /sys/class/fc_host/host10/device/scsi_host/host10/driver_version */
		memset(sym_name, 0, sizeof(sym_name));
		snprintf(path, sizeof(path), "%s/%s", scsi_path, "driver_version");
		snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
		fc_dev->drv_version = strdup(sym_name);

		/* /sys/class/fc_host/host10/device/scsi_host/host10/fw_version */
		memset(sym_name, 0, sizeof(sym_name));
		snprintf(path, sizeof(path), "%s/%s", scsi_path, "fw_version");
		snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
		fc_dev->fw_version = strdup(sym_name);

		/* /sys/class/fc_host/host10/device/scsi_host/host10/port_speed */
		memset(sym_name, 0, sizeof(sym_name));
		snprintf(path, sizeof(path), "%s/%s", scsi_path, "port_speed");
		snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
		fc_dev->port_speed = strdup(sym_name);

		/* /sys/class/fc_host/host10/device/scsi_host/host10/serial_num */
		memset(sym_name, 0, sizeof(sym_name));
		snprintf(path, sizeof(path), "%s/%s", scsi_path, "serial_num");
		snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
		fc_dev->serial_num = strdup(sym_name); /* 14 */

		/* /sys/class/fc_host/host10/device/scsi_host/host10/optrom_bios_version */

	} else if (strncmp(fc_dev->driver_name, "lpfc", 4) == 0) {
		print_trace_enter();

		/*  /sys/class/fc_host/host10/device/scsi_host/host10/fwrev */
		memset(sym_name, 0, sizeof(sym_name));
		snprintf(path, sizeof(path), "%s/%s", scsi_path, "fwrev");
		snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
		fc_dev->fw_version = strdup(sym_name);

		/* /sys/class/fc_host/host10/device/scsi_host/host10/modeldesc */
		memset(sym_name, 0, sizeof(sym_name));
		snprintf(path, sizeof(path), "%s/%s", scsi_path, "modeldesc");
		snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
		fc_dev->model_desc = strdup(sym_name);

		/* /sys/class/fc_host/host10/device/scsi_host/host10/modelname */
		memset(sym_name, 0, sizeof(sym_name));
		snprintf(path, sizeof(path), "%s/%s", scsi_path, "modelname");
		snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
		fc_dev->product_name = strdup(sym_name);

		memset(sym_name, 0, sizeof(sym_name));
		snprintf(path, sizeof(path), "%s/%s", scsi_path, "serialnum");
		snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
		fc_dev->serial_num = strdup(sym_name);

		/* /sys/class/fc_host/host8/speed */
		memset(sym_name, 0, sizeof(sym_name));
		snprintf(path, sizeof(path), "%s/%s", fc_dev->sys_dev_path, "speed");
		snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
		fc_dev->port_speed = strdup(sym_name);
		remove_space(fc_dev->port_speed);

		memset(sym_name, 0, sizeof(sym_name));
		snprintf(path, sizeof(path), "/sys/module/%s/version", fc_dev->driver_name);
		snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
		fc_dev->drv_version = strdup(sym_name);
	} else {
		print_info("Unknown Adapter");
		return 0;
	}

	/* Reinitialize the buffer */
	memset(sym_name, 0, sizeof(sym_name));
	/* /sys/class/fc_host/host10/device/scsi_host/host10/link_state */
	snprintf(path, sizeof(path), "%s/%s", scsi_path, "link_state");
	fp = fopen(path, "r");
	if (fp == NULL)
		return -EINVAL;

	p = fgets(line, sizeof(line), fp);
	if (!p) {
		fclose(fp);
		return -EINVAL;
	}
	snprintf(sym_name, strlen(p), p);
	fclose(fp);

	fc_dev->link_state = strdup(sym_name);

	/* Extract Supported class*/
	snprintf(path, sizeof(path), "%s/supported_classes", fc_dev->sys_dev_path);
	memset(sym_name, 0, sizeof(sym_name));
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->supported_class = strdup(sym_name);

	/* Extract Supported speed*/
	snprintf(path, sizeof(path), "%s/supported_speeds", fc_dev->sys_dev_path);
	memset(sym_name, 0, sizeof(sym_name));
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->supported_speed = strdup(sym_name);

	/* Extract Dev Loss Timeou */
	snprintf(path, sizeof(path), "%s/dev_loss_tmo", fc_dev->sys_dev_path);
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->dev_loss_tmo = strtol(sym_name, &end, 0);

	/* /sys/class/fc_host/host10/device/scsi_host/host10/active_mode */
	snprintf(path, sizeof(path), "%s/%s", scsi_path, "active_mode");
	memset(sym_name, 0, sizeof(sym_name));
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->active_mode = strdup(sym_name);

	return 0;
}

int list_fc_adapters(struct fc_device_info *fc_dev)
{
	struct dirent 	*entry;
	DIR		*dir;
	int		count = 0;

	print_trace_enter();

	dir = opendir(SYSFS_FC_HOST_PATH);
	if (unlikely(!dir)) {
		print_debug("\n No FCP host configured \n");
		return -ENODEV;
	}

	print_command_label("Fabric");

	print_fc_dev_header();

	for_each_dir(entry, dir) {
		char	dev_path[64];

		print_trace_enter();

		fc_dev = alloc_fc_dev();
		if (!fc_dev)
			return -ENODEV;

		fc_dev->host_name = strdup(entry->d_name);

		snprintf(dev_path, sizeof(dev_path), "%s/%s", SYSFS_FC_HOST_PATH,
		    fc_dev->host_name);

		fc_dev->sys_dev_path = strdup(dev_path);

		/* Get Symbolic Name */
		get_symbolic_name(fc_dev);

		print_debug("Sys Dev Path: %s (len %ld), Symbolic Name %s (Len %ld)\n",
		    fc_dev->sys_dev_path, strlen(fc_dev->sys_dev_path),
		    fc_dev->symbolic_name, strlen(fc_dev->symbolic_name));

		/* Extract Driver Name */
		get_driver_info(fc_dev);

		/* Populate FC adapter info */
		get_fc_info(fc_dev);

		count++;
		print_list_fc_dev(fc_dev);

		put_fc_dev(fc_dev);
	}
	closedir(dir);

	print_debug("\n\n Total Fiber Channel Ports Found %d \n", count);

	return count;
}

int get_fc_dev_stats(char *device_name, struct fc_device_info *fc_dev)
{
	char	scsi_path[256];
	char	path[1024], sym_name[64] = { 0 };
	char	*end;

	print_trace_enter();

	print_debug("Get Stats for %s", device_name);

	fc_dev->host_name = strdup(device_name);

	snprintf(scsi_path, sizeof(scsi_path), "%s/%s/%s", SYSFS_FC_HOST_PATH,
		device_name, "statistics");

	/* /sys/class/fc_host/host0/statistics/lip_count */
	snprintf(path, sizeof(path), "%s/%s", scsi_path, "lip_count");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.lip_count = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "tx_frames");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.tx_frames = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "tx_words");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.tx_words = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "rx_frames");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.rx_frames = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "rx_words");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.rx_words = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "cn_sig_alarm");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.cn_sig_alarm = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "cn_sig_warn");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.cn_sig_warn = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "error_frames");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.error_frames = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "dumped_frames");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.dumped_frames = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "invalid_crc_count");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.invalid_crc_count = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "invalid_tx_word_count");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.invalid_tx_word_count = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "link_failure_count");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.link_failure_count = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "loss_of_signal_count");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.loss_of_signal_count = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "loss_of_sync_count");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.loss_of_sync_count = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "nos_count");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.nos_count = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "fpin_cn");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.fpin_cn = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "fpin_cn_credit_stall");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.fpin_cn_credit_stall = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "fpin_cn_device_specific");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.fpin_cn_device_specific = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "fpin_cn_lost_credit");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.fpin_cn_lost_credit = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "fpin_cn_oversubscription");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.fpin_cn_oversubscription = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "fpin_li");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.fpin_li = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "fpin_li_device_specific");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.fpin_li_device_specific = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "fpin_li_failure_unknown");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.fpin_li_failure_unknown = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "fpin_li_invalid_crc_count");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.fpin_li_invalid_crc_count = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "fpin_li_invalid_tx_word_count");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.fpin_li_invalid_tx_word_count = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "fpin_li_link_failure_count");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.fpin_li_link_failure_count = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "fpin_li_loss_of_signals_count");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.fpin_li_loss_of_signals_count = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "fpin_li_loss_of_sync_count");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.fpin_li_loss_of_sync_count = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "fpin_li_prim_seq_err_count");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.fpin_li_prim_seq_err_count = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "fpin_dn");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.fpin_dn = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "fpin_dn_device_specific");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.fpin_dn_device_specific = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "fpin_dn_timeout");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.fpin_dn_timeout = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "fpin_dn_unable_to_route");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.fpin_dn_unable_to_route = strtol(sym_name, &end, 0);

	snprintf(path, sizeof(path), "%s/%s", scsi_path, "fpin_dn_unknown");
	snprintf(sym_name, sizeof(sym_name), open_sysfs_stats_file(path));
	fc_dev->stats.fpin_dn_unknown = strtol(sym_name, &end, 0);

	return 0;
}

int show_fc_port_details(char *argv[], struct scsi_device_list *sdev)
{
	struct	fc_device_info	*fc_dev = sdev->fc_info;
	char	dev_path[64];

	print_trace_enter();

	fc_dev->host_name = strdup(argv[3]);

	snprintf(dev_path, sizeof(dev_path), "%s/%s", SYSFS_FC_HOST_PATH,
	    fc_dev->host_name);
	fc_dev->sys_dev_path = strdup(dev_path);

	/* Populate Driver Information from Symbolic Name */
	get_symbolic_name(fc_dev);

	print_debug("Sys Dev Path: %s (len %ld), Symbolic Name %s (Len %ld)\n",
	    fc_dev->sys_dev_path, strlen(fc_dev->sys_dev_path),
	    fc_dev->symbolic_name, strlen(fc_dev->symbolic_name));

	/* Extract Driver Name */
	get_driver_info(fc_dev);

	/* Get Adapter related information */
	get_adapter_details(fc_dev);

	/* Get Detail information for the port */
	get_fc_info(fc_dev);

	/* Print all the gathered information */
	print_fc_info(fc_dev);

	/* if there are any Remote Ports available, print those */
	if (fc_dev->no_rports > 0)
		list_rport_adapters(fc_dev);

	return 0;
}

/**
 * Get Eror Count for a Fibre Channel device
 */
int get_fcport_error_count(char **argv, struct fc_device_info *fc_dev)
{
	int	err = 0;
	char	dev_path[256];

	if (strlen(argv[3]) < 4)
		return -EIO;

	fc_dev = alloc_fc_dev();
	if (!fc_dev)
		return -ENODEV;

	fc_dev->host_name = strdup(argv[3]);

	snprintf(dev_path, sizeof(dev_path), "%s/%s", SYSFS_FC_HOST_PATH,
	    fc_dev->host_name);

	fc_dev->sys_dev_path = strdup(dev_path);

	put_fc_dev(fc_dev);

	return err;
}

/**
 * Issue Scan comand to a Fibre Channel device
 */
int scsi_scan_fcport_dev(char **argv, struct fc_device_info *fc_dev)
{
	int	err = 0;
	char	dev_path[256];

	if (strlen(argv[3]) < 4)
		return -EIO;

	fc_dev = alloc_fc_dev();
	if (!fc_dev)
		return -ENODEV;

	fc_dev->host_name = strdup(argv[3]);

	snprintf(dev_path, sizeof(dev_path), "%s/%s", SYSFS_FC_HOST_PATH,
	    fc_dev->host_name);

	fc_dev->sys_dev_path = strdup(dev_path);

	put_fc_dev(fc_dev);

	return err;
}

/**
 * Set Fibre Channel device offline
 */
int set_fcport_offline(char **argv, struct fc_device_info *fc_dev)
{
	int	err = 0;
	char	dev_path[256];

	if (strlen(argv[3]) < 4)
		return -EIO;

	fc_dev = alloc_fc_dev();
	if (!fc_dev)
		return -ENODEV;

	fc_dev->host_name = strdup(argv[3]);

	snprintf(dev_path, sizeof(dev_path), "%s/%s", SYSFS_FC_HOST_PATH,
	    fc_dev->host_name);

	fc_dev->sys_dev_path = strdup(dev_path);

	put_fc_dev(fc_dev);

	return err;
}

/**
 * Set Fibre Channel device online
 */
int set_fcport_online(char **argv, struct fc_device_info *fc_dev)
{
	int	err = 0;
	char	dev_path[256];

	if (strlen(argv[3]) < 4)
		return -EIO;

	fc_dev = alloc_fc_dev();
	if (!fc_dev)
		return -ENODEV;

	fc_dev->host_name = strdup(argv[3]);

	snprintf(dev_path, sizeof(dev_path), "%s/%s", SYSFS_FC_HOST_PATH,
	    fc_dev->host_name);

	fc_dev->sys_dev_path = strdup(dev_path);

	put_fc_dev(fc_dev);

	return err;
}

/**
 * Set Fibre Channel device alias
 */
int set_fcport_alias(char **argv, struct fc_device_info *fc_dev)
{
	int	err = 0;
	char	dev_path[256];

	if (strlen(argv[3]) < 4)
		return -EIO;

	fc_dev = alloc_fc_dev();
	if (!fc_dev)
		return -ENODEV;

	fc_dev->host_name = strdup(argv[3]);

	snprintf(dev_path, sizeof(dev_path), "%s/%s", SYSFS_FC_HOST_PATH,
	    fc_dev->host_name);

	fc_dev->sys_dev_path = strdup(dev_path);

	put_fc_dev(fc_dev);

	return err;
}
