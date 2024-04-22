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
#include "scsi_print.h"

int remove_newline(char *s)
{
	char *p = s;

	p[strcspn(p, "\n")] = 0;

	memcpy(s, p, strlen(p));

	return (int)strlen(s);
}

int remove_space(char *s)
{
	int n = 0;
	char *p = s;

	while (*p && isspace((uint8_t)*p)) {
		++p;
		--n;
	}
	memmove(s, p, n + 1);

	return (int)strlen(s);
}

/*
int print_colon_spaced(char *s)
{
	int i = 0, j = 0;
	char *p = s;
	char *ss = s;

	p = p + 2;
	ss = ss + 2;

	printf("%s: %s ===== %s \n", __func__, p, ss);

	printf("%s: %s\n", __func__, p);

	memcpy(s, p, strlen(p));

	return (int)strlen(s);
}
*/

static const char *calculate_size(u64 size)
{
	int	i = 0;
	static	char	result[64];
	char	*bytes_size[] = { "B", "KiB", "MiB", "GiB", "TiB" };
	char	length = sizeof(bytes_size) / sizeof(bytes_size[0]);
	double	dbl_size = size;

	if (size > KB_SIZE) {
		for (i = 0; (size / KB_SIZE) > 0 && i < length - 1;
		    i++, size /= KB_SIZE)
			dbl_size = size / KB_SIZE;
	}

	snprintf(result, sizeof(result), "%d %s", (int)dbl_size, bytes_size[i]);

	return result;
}

void print_command_label(char *label)
{
	print_trace_enter();
	printf("\nDisplaying All %s devices on the system.\n", label);
	printf("%-.64s\n", dash);
}

void print_mpath_disk_header(void)
{
	print_trace_enter();

	printf("\n");
	printf("%-12s\t%-5s\t%-5s\t%-8s\t%-24s\n",
	    "BUS ID", "Major", "Minor", "Disk Name", "Device Path");
	printf("%-.12s\t%-.5s\t%-.5s\t%-.8s\t%-.24s\n",
		dash, dash, dash, dash, dash);
}

void print_mpath_disk_info(struct scsi_device_info *d_info_p)
{
	print_trace_enter();
	printf("[%d:%d:%d:%d]\t%-5d\t%-5d\t%-8s\t%-24s\n",
		d_info_p->host, d_info_p->bus, d_info_p->target, d_info_p->lun,
		d_info_p->major, d_info_p->minor,
		d_info_p->disk_name, d_info_p->disk_path);
}

void print_generic_disk_header(void)
{
	print_trace_enter();

	printf("\n");
	printf("%-12s\t%-5s\t%-5s\t%-16s\t%-8s\t%-24s\n",
	    "BUS ID ", "Major", "Minor", "Disk Type", "Disk Name", "Device Path");
	printf("%-.12s\t%-.5s\t%-.5s\t%-.16s\t%-.8s\t%-.24s\n",
		dash, dash, dash, dash, dash, dash);
}

void print_generic_disk_info(struct scsi_device_info *d_info_p)
{
	print_trace_enter();
	printf("[%d:%d:%d:%d]\t%-5d\t%-5d\t%-16s\t%-8s\t%-24s\n",
		d_info_p->host, d_info_p->bus, d_info_p->target, d_info_p->lun,
		d_info_p->major, d_info_p->minor, d_info_p->disk_type,
		d_info_p->disk_name, d_info_p->disk_path);
}

void print_disk_header(void)
{
	print_trace_enter();

	printf("\n");
	printf("%-12s\t%-16s\t%-16s\t%-8s\t%-5s\t%-5s\t%-16s\t%-8s\t%-24s\n",
	    "BUS ID      ", "Vendor", "Model", "Revision", "Major", "Minor",
	    "Disk Type", "Disk Name", "Device Path");
	printf("%-.12s\t%-.16s\t%-.16s\t%-.8s\t%-.5s\t%-.5s\t%-.16s\t%-.8s\t%-.24s\n",
		dash, dash, dash, dash, dash, dash, dash, dash, dash);
}

void print_disk_info(struct scsi_device_info *d_info_p)
{
	print_trace_enter();
	printf("[%d:%d:%d:%d]\t%-16s\t%-8s\t%-8s\t%-5d\t%-5d\t%-16s\t%-8s\t%-24s\n",
		d_info_p->host, d_info_p->bus, d_info_p->target, d_info_p->lun,
		d_info_p->vendor, d_info_p->model, d_info_p->rev, d_info_p->major,
		d_info_p->minor, d_info_p->disk_type, d_info_p->disk_name,
		d_info_p->disk_path);
}

void print_nvme_disk_header(void)
{
	print_trace_enter();

	printf("\n");
	printf("%-12s\t%-32s\t%-12s\t%-5s\t%-5s\t%-16s\t%-8s\t%-24s\n",
	    "BUS ID      ", "Model ", "Revision", "Major", "Minor", "Disk Type",
	    "Disk Name", "Device Path");
	printf("%-.12s\t%-.32s\t%-.12s\t%-.5s\t%-.5s\t%-.16s\t%-.8s\t%-.24s\n",
		dash, dash, dash, dash, dash, dash, dash, dash);
}

void print_nvme_disk_info(struct scsi_device_info *d_info_p)
{
	print_trace_enter();
	printf("[N:%d:%d:%d]\t%-32s\t%-12s\t%5d\t%5d\t%-16s\t%-8s\t%-24s\n",
		d_info_p->bus, d_info_p->target, d_info_p->lun,
		d_info_p->model, d_info_p->rev, d_info_p->major,
		d_info_p->minor, d_info_p->disk_type, d_info_p->disk_name,
		d_info_p->disk_path);
}

void print_scsi_queue_data(struct disk_queue_data *q_data)
{
	print_trace_enter();
	printf("\n %-.48s\n", dash);
	printf("\t\tQueue Data\t");
	printf("\n %-.48s\n\n", dash);

	printf(" Scheduler: \n  %s\n\n", q_data->scheduler);
	printf(" Physical Block Size	: %s \n",
	    calculate_size(q_data->physical_block_size));
	printf(" Logical Block Size	: %s \n",
	    calculate_size(q_data->logical_block_size));
	printf(" Minimum IO Size	: %s \n",
	    calculate_size(q_data->minimum_io_size));
	printf(" Optimal IO Size	: %s \n",
	    calculate_size(q_data->optimal_io_size));
	printf(" Zoned Append Max Bytes	: %s \n",
	    calculate_size(q_data->zone_append_max_bytes));
	printf(" Zone Write Granularity	: %s \n",
	    calculate_size(q_data->zone_write_granularity));
	printf(" Discard Max Bytes  	: %s \n",
	    calculate_size(q_data->discard_max_bytes));
	printf(" Discard max HW bytes  	: %s \n",
	    calculate_size(q_data->discard_max_hw_bytes));
	printf(" Discard zeroes data  	: %s \n",
	    calculate_size(q_data->discard_zeroes_data));
	printf(" Discard Granularity 	: %s \n\n",
	    calculate_size(q_data->discard_granularity));
	printf(" IO Poll	:  %-8lld \n", q_data->io_poll);
	printf(" IO Poll Delay	:  %-8lld \n", q_data->io_poll_delay);
	printf(" stable_writes	:  %-8lld \n", q_data->stable_writes);
	printf(" wbt_lat_usec	:  %-8lld \n", q_data->wbt_lat_usec);
	printf(" rq_affinity	:  %-8lld \n", q_data->rq_affinity);
	printf(" DAX	  : %-8lld	add_random	 : %-8lld \n",
	    q_data->dax, q_data->add_random);
	printf(" FUA	  : %-8lld 	nomerges 	 : %-8lld \n",
	    q_data->fua, q_data->nomerges);
	printf(" Segments : %-8lld	Segment Size	 : %s \n",
	    q_data->max_segments, calculate_size(q_data->max_segment_size));
	printf(" Zoned	  : %-8lld	nr_zones	 : %-8lld \n",
	    q_data->zoned, q_data->nr_zones);
	printf(" Readahead : %-8lld 	Write Cache	 : %s\n",
	    q_data->read_ahead_kb, q_data->write_cache);
	printf(" WriteSame : %s \tWrite Zeroes  	 : %s \n",
	    calculate_size(q_data->write_same_max_bytes),
	    calculate_size(q_data->write_zeroes_max_bytes));
}

void print_scsi_disk_details(struct scsi_device_info *d_info)
{
	print_trace_enter();
	printf("\n%-.48s\n", dash);
	printf("	Show Details for %s", d_info->disk_name);
	printf("\n%-.48s\n", dash);
	printf("\n");
	printf(" Vendor		:  %s \n", d_info->vendor);
	printf(" Model Name	:  %-16s \n", d_info->model);
	printf(" Size		:  %llu  Sectors, %s \n", d_info->size,
	    calculate_size(d_info->q_data.logical_block_size * d_info->size));
	printf(" Disk Type	:  %s \n", d_info->disk_type);
	printf(" Revision	:  %s \n", d_info->rev);
	printf(" Disk Path	:  %s \n", d_info->disk_path);
	printf(" Max Sectors	:  %llu \n", d_info->max_sectors);
	printf(" Range		:  %d \n", d_info->range);
	printf(" Extent Range	:  %d \n", d_info->ext_range);
	printf(" Capability	:  %#x \n", d_info->capability);
	printf(" Queue Depth	:  %-8d \n", d_info->queue_depth);
	printf(" Queue Type	:  %-8s \n", d_info->queue_type);
	printf(" CDL Enabled	:  %-8d \n", d_info->cdl_enabled);
	printf(" CDL Supported	:  %-8d \n", d_info->cdl_supported);
	printf(" EH Timeout	:  %-8d \n", d_info->eh_timeout);
	printf(" TimeOut	:  %-16d \n", d_info->timeout);
	printf(" DH State	:  %-8s \n", d_info->state);
	printf(" IO Done cnt	:  %#llx \n", d_info->iodone_cnt);
	printf(" IO error cnt	:  %#llx \n", d_info->ioerr_cnt);
	printf(" IO Request cnt	:  %#llx \n", d_info->iorequest_cnt);
	printf(" IO Counter bits:  %lld \n", d_info->iocounterbits);
	printf(" IO Timeout	:  %lld \n", d_info->iotmo_cnt);
	printf(" WWID		:  %-64s \n", d_info->wwid);
	printf(" Alignment Offset :  %#llx \n", d_info->alignment_offset);
	printf(" Discard Alignment:  %#llx \n", d_info->discard_alignment);

	printf("\n Scsi Device Event Notification \n\n");
	printf(" Capacity Change Reported	:  %d \n",
	    d_info->evt_capacity_change_reported);
	printf(" Inquiry Change Reported	:  %d \n",
	    d_info->evt_inquiry_change_reported);
	printf(" LUN Change Reported		:  %d \n",
	    d_info->evt_lun_change_reported);
	printf(" Media Change			:  %d \n",
	    d_info->evt_media_change);
	printf(" Mode Parameter Change Reported :  %d \n",
	    d_info->evt_mode_parameter_change_reported);
	printf(" Soft Threshold Reached 	:  %d \n",
	    d_info->evt_capacity_change_reported);

	print_scsi_queue_data(&d_info->q_data);
}

void print_nvme_disk_details(struct scsi_device_info *d_info)
{
	print_trace_enter();
	printf("\n%-.48s\n", dash);
	printf("	Show Details for %s	", d_info->disk_name);
	printf("\n%-.48s\n", dash);
	printf(" Model Name	:  %s \n", d_info->model);
	printf(" Revision	:  %s \n", d_info->rev);
	printf(" Serial		:  %s \n", d_info->serial);
	printf(" Disk Path	:  %-8s \n", d_info->disk_path);
	printf(" PCI Address	:  %s \n", d_info->pci_address);
	printf(" Transport	:  %s \n", d_info->transport);
	printf(" Size		:  %llu Sectors, %s \n", d_info->size,
	    calculate_size(d_info->q_data.logical_block_size * d_info->size));
	printf(" Range		:  %d \n", d_info->range);
	printf(" Extent Range	:  %d \n", d_info->ext_range);
	printf(" Capability	:  %#x \n", d_info->capability);
	printf(" Queue Depth	:  %d \n", d_info->queue_depth);
	printf(" EH Timeout	:  %d \n", d_info->eh_timeout);
	printf(" State		:  %-8s \n", d_info->state);
	printf(" Controller ID	:  %d	\n", d_info->cntlid);
	printf(" Controller Type:  %s   \n", d_info->cntrltype);
	printf(" KATO		:  %d   \n", d_info->kato);
	printf(" SQ Size	:  %d	\n", d_info->sqsize);
	printf(" UUID		:  %-64s \n", d_info->uuid);
	printf(" NSID		:  %-64s \n", d_info->nsid);
	printf(" WWID		:  %-64s \n", d_info->wwid);
	printf(" NGUID		:  %-64s \n", d_info->nguid);
	printf(" Alignment Offset :  %#llx \n", d_info->alignment_offset);
	printf(" Discard Alignment:  %#llx \n", d_info->discard_alignment);
	printf(" Discovery Controller Type	: %s \n", d_info->dctype);
	printf(" Namespace Utilization		: %llu \n", d_info->nuse);

	print_scsi_queue_data(&d_info->q_data);
}

void print_fc_info(struct fc_device_info *fc_info_p)
{
	print_trace_enter();
	printf("\n%-.48s\n", dash);
	printf("	Show Details for FC %s ", fc_info_p->host_name);
	printf("\n%-.48s\n", dash);
	printf("\n");
	printf(" Adapter Name	  : %s \n", fc_info_p->product_name);
	printf(" Model Desc	  : %s \n", fc_info_p->model_desc);
	printf(" Host Name	  : %s \n", fc_info_p->host_name);
	printf(" Node Name	  : %s \n", fc_info_p->node_name);
	printf(" Port Name	  : %s \n", fc_info_p->port_name);
	printf(" Fabric Name	  : %s \n", fc_info_p->fabric_name);
	printf(" Serial Number	  : %s \n", fc_info_p->serial_num);
	printf(" Port ID	  : %s \n", fc_info_p->port_id);
	printf(" Port State	  : %s \n", fc_info_p->port_state);
	printf(" Port Type 	  : %s \n", fc_info_p->port_type);
	printf(" Driver Attached  : %s \n", fc_info_p->driver_name);
	printf(" Driver Version	  : %s \n", fc_info_p->drv_version);
	printf(" Firmware Version : %s \n", fc_info_p->fw_version);
	printf(" Supported Speed  : %s \n", fc_info_p->supported_speed);
	printf(" Supported Class  : %s \n", fc_info_p->supported_class);
	printf(" Symbolic Name	  : %s \n", fc_info_p->symbolic_name);
	printf(" Dev Loss Timeout : %d \n", fc_info_p->dev_loss_tmo);
	printf(" Link State	  : %s \n", fc_info_p->link_state);
	printf(" Active Mode	  : %s \n", fc_info_p->active_mode);
	printf(" Max NPIV VPorts  : %d \n", fc_info_p->max_npiv_vports);
	printf(" In Use NPIV Port : %d \n", fc_info_p->npiv_vports_inuse);
	printf(" R-Ports Found	  : %d\n", fc_info_p->no_rports);
	printf("\n");
}

void print_fc_dev_header(void)
{
	print_trace_enter();
	printf("\n");
	printf("%-8s\t%-8s\t%-8s\t%-16s\t%-16s\t%-8s\t%-24s\t%-8s\t%-s \n",
	    "Host", "Model", "Speed", "SerialNum", "DriverVer", "DriverName",
	    "FW_Ver", "Role", "State");
	printf("%-.8s\t%-.8s\t%-.8s\t%-.16s\t%-.16s\t%-.8s\t%-.24s\t%-.8s\t%-.24s \n",
		dash, dash, dash, dash, dash, dash, dash, dash, dash);
}

void print_list_fc_dev(struct fc_device_info *fc_info_p)
{
	print_trace_enter();
	printf("%-8s\t%-8s\t%-4sGBit\t%-16s\t%-16s\t%-8s\t%-24s\t%-8s\t%-s \n",
	    fc_info_p->host_name, fc_info_p->product_name, fc_info_p->port_speed,
	    fc_info_p->serial_num, fc_info_p->drv_version, fc_info_p->driver_name,
	    fc_info_p->fw_version, fc_info_p->active_mode, fc_info_p->link_state);
}

void print_disk_stats(struct disk_stats *d_stats_p, char *disk_name)
{
	print_trace_enter();
	printf("\n %-.48s\n", dash);
	printf("	IO Statistics for %s", disk_name);
	printf("\n %-.48s\n", dash);
	printf("\n");
	printf(" Read IO	: %llu \n",
	    (unsigned long long) (d_stats_p->ios[0]));
	printf(" Write IO	: %llu \n",
	    (unsigned long long) d_stats_p->ios[1]);
	printf(" Merged Read 	: %llu \n",
	    (unsigned long long) d_stats_p->merges[0]);
	printf(" Merged Write	: %llu \n",
	    (unsigned long long) d_stats_p->merges[1]);
	printf(" Read Sectors	: %llu \n",
	    (unsigned long long) d_stats_p->sectors[0]);
	printf(" Write Sectors	: %llu \n",
	    (unsigned long long) d_stats_p->sectors[1]);
	printf(" Read IO ticks	: %llu \n",
	    (unsigned long long) d_stats_p->ticks[0]);
	printf(" Write IO ticks	: %llu \n",
	    (unsigned long long) d_stats_p->ticks[1]);
	printf(" Total IO tikcs	: %llu \n",
	    (unsigned long long) d_stats_p->io_ticks);
	printf(" Time spent in IO queue  : %llu \n",
	    (unsigned long long) d_stats_p->time_in_queue);
	printf(" Time spent/IO		 : %llu \n",
	    (unsigned long long) d_stats_p->msec);
	printf("\n");
}

void print_fc_port_stats(struct fc_device_info *fc_dev)
{
	print_trace_enter();

	printf("\n%-.48s\n", dash);
	printf("	FCP Statistics: %s", fc_dev->host_name);
	printf("\n%-.48s\n", dash);
	printf("\n");
	printf(" TX Frames	: %#llx \n", fc_dev->stats.tx_frames);
	printf(" TX Words	: %#llx \n", fc_dev->stats.tx_words);
	printf(" RX Frames	: %#llx \n", fc_dev->stats.rx_frames);
	printf(" RX Words	: %#llx \n", fc_dev->stats.rx_words);
	printf(" LIP Count	: %#llx \n", fc_dev->stats.lip_count);
	printf(" NOS Count	: %#llx \n", fc_dev->stats.nos_count);
	printf(" Error Frames	: %#llx \n", fc_dev->stats.error_frames);
	printf(" Dumped Frames	: %#llx \n", fc_dev->stats.dumped_frames);
	printf(" Invald TX Word	: %#llx \n",
		fc_dev->stats.invalid_tx_word_count);
	printf(" Loss of Sync	: %#llx \n",
		fc_dev->stats.loss_of_sync_count);
	printf(" Loss of Signal	: %#llx \n",
		fc_dev->stats.loss_of_signal_count);
	printf(" Link Failure	: %#llx \n",
		fc_dev->stats.link_failure_count);
	printf(" Invalid CRC	: %#llx \n", fc_dev->stats.invalid_crc_count);

	printf("\n ----- FPIN Diagnostic Statistics ----- \n");
	printf("\n");
	printf(" fpin_dn			: %#llx \n",
		fc_dev->stats.fpin_dn);
	printf(" fpin_dn_device_specific	: %#llx \n",
		fc_dev->stats.fpin_dn_device_specific);
	printf(" fpin_dn_timeout		: %#llx \n",
		fc_dev->stats.fpin_dn_timeout);
	printf(" fpin_dn_unable_to_route	: %#llx \n",
		fc_dev->stats.fpin_dn_unable_to_route);
	printf(" fpin_dn_unknown		: %#llx \n",
		fc_dev->stats.fpin_dn_unknown);

	printf("\n ----- FPIN Link Integrity Statistics ----- \n");
	printf("\n");
	printf(" fpin_li			: %#llx\n",
		fc_dev->stats.fpin_li);
	printf(" fpin_li_device_specific	: %#llx \n",
		fc_dev->stats.fpin_li_device_specific);
	printf(" fpin_li_failure_unknown	: %#llx \n",
		fc_dev->stats.fpin_li_failure_unknown);
	printf(" fpin_li_invalid_crc_count	: %#llx \n",
		fc_dev->stats.fpin_li_invalid_crc_count);
	printf(" fpin_li_invalid_tx_word_count	: %#llx \n",
		fc_dev->stats.fpin_li_invalid_tx_word_count);
	printf(" fpin_li_link_failure_count	: %#llx \n",
		fc_dev->stats.fpin_li_link_failure_count);
	printf(" fpin_li_loss_of_signals_count	: %#llx \n",
		fc_dev->stats.fpin_li_loss_of_signals_count);
	printf(" fpin_li_loss_of_sync_count	: %#llx \n",
		fc_dev->stats.fpin_li_loss_of_sync_count);
	printf(" fpin_li_prim_seq_err_count	: %#llx \n",
		fc_dev->stats.fpin_li_prim_seq_err_count);

	printf("\n ----- FPIN Congestion Statistics ----- \n");
	printf("\n");
	printf(" cn_sig_alarm			: %#llx\n",
		fc_dev->stats.cn_sig_alarm);
	printf(" cn_sig_warn			: %#llx\n",
		fc_dev->stats.cn_sig_warn);
	printf(" fpin_cn			: %#llx\n",
		fc_dev->stats.fpin_cn);
	printf(" fpin_cn_device_specific	: %#llx \n",
		fc_dev->stats.fpin_cn_device_specific);
	printf(" fpin_cn_credit_stall		: %#llx \n",
		fc_dev->stats.fpin_cn_credit_stall);
	printf(" fpin_cn_lost_credits		: %#llx \n",
		fc_dev->stats.fpin_cn_lost_credit);
	printf(" fpin_cn_oversubscription	: %#llx \n",
		fc_dev->stats.fpin_cn_oversubscription);

	printf("\n");
}

void print_fc_rport_header(void)
{
	print_trace_enter();
	printf("\n");
	printf("%-16s %-16s %-32s %-32s %-32s %-16s\n",
		"Rport", "Rport_State", "Roles", "Rport_Name", "Node_Name",
		"Rport_ID");
	printf("%-.16s %-.16s %-.32s %-.32s %-.32s %-.16s\n",
		dash, dash, dash, dash, dash, dash);
}

void print_fc_rport_info(struct fc_rport_info *rprt_p)
{
	print_trace_enter();
	printf("%-16s %-16s %-32s %-32s %-32s %-16s\n",
		rprt_p->rport_name, rprt_p->port_state, rprt_p->roles,
		rprt_p->port_name, rprt_p->node_name, rprt_p->port_id);
}

void print_iscsi_dev_header(void)
{
	/*  Host_Name	Transport	IP_ADD:PORT	Connection	Session		TARGET_NAME	 */
	/*  ----------	-----------	---------------	-----------	-----------	----------------- */
	print_trace_enter();
	printf("\n");
	printf("%-12s\t%-8s\t%-16s\t%-4s\t%-16s\t%-16s\t%-64s \n",
	    "Host Name", "Transport", "IP Address", "Port", "Connection", "Session",
	    "Target Name");
	printf("%-.12s\t%-.10s\t%-.16s\t%-.6s\t%-.16s\t%-.12s\t%-.64s\n",
		dash, dash, dash, dash, dash, dash, dash);
}

void print_list_iscsi_dev(struct iscsi_dev_info *iscsi_info)
{
	print_trace_enter();

	printf("%-12s\t%-8s\t%-16s\t%-4s\t%-16s\t%-12s\t%-64s \n",
	    iscsi_info->host_name, iscsi_info->transport_name,
	    iscsi_info->conn_address, iscsi_info->conn_port,
	    iscsi_info->connection_name, iscsi_info->session_name,
	    iscsi_info->target_name);
}

void print_iscsi_header(char *label, char *name)
{
	printf("%-.64s \n", dash);
	printf(" Displaying %s Information for: %s \n",
	    label, name);
	printf("%-.64s \n", dash);
}

void print_iscsi_session_info(struct iscsi_session *sess)
{
	print_trace_enter();

	printf(" \n");
	printf(" Target Name		:\t%-s \n", sess->targetname);
	printf(" Target State		:\t%-s \n", sess->target_state);
	printf(" Session State		:\t%-s \n", sess->state);
	printf(" Abort Timeout		:\t%d \n", sess->abort_tmo);
	printf(" Creator ID		:\t%-d \n", sess->creator);
	printf(" Data PDU In Order	:\t%d \n", sess->data_pdu_in_order);
	printf(" Data SEQ In Order	:\t%d \n", sess->data_seq_in_order);
	printf(" Error Recovery Level	:\t%d \n", sess->err_level);
	printf(" Fast Abort		:\t%d \n", sess->fast_abort);
	printf(" First Burst Len	:\t%d \n", sess->first_burst_len);
	printf(" ifacename		:\t%s \n", sess->ifacename);
	printf(" immediate Data		:\t%d \n", sess->immediate_data);
	printf(" Initial R2T		:\t%d \n", sess->initial_r2t);
	printf(" Lun Reset Timeout	:\t%d \n", sess->lu_reset_tmo);
	printf(" Max Burst Len		:\t%d \n", sess->max_burst_len);
	printf(" Max Outstanding R2T	:\t%d \n", sess->max_outstanding_r2t);
	printf(" Recovery Timeout	:\t%d \n", sess->recovery_tmo);
	printf("\n");
}

void print_iscsi_connection_info(struct iscsi_connection *conn)
{
	print_trace_enter();

	printf(" \n");
	printf(" State			:\t%s \n", conn->state);
	printf(" Port			:\t%d \n", conn->port);
	printf(" Ping Timeout		:\t%-d \n", conn->ping_tmo);
	printf(" Recv Timeout		:\t%d \n", conn->recv_tmo);
	printf(" Persistent Addr	:\t%si \n", conn->persistent_address);
	printf(" Persistent Port	:\t%d \n", conn->persistent_port);
	printf(" Header Digest		:\t%d \n", conn->header_digest);
	printf(" Data Digest		:\t%d \n", conn->data_digest);
	printf(" Expected StatSN	:\t%d \n", conn->exp_statsn);
	printf(" Max Recv datalen	:\t%d \n", conn->max_recv_dlength);
	printf(" Max Xmit datalen	:\t%d \n", conn->max_xmit_dlength);
	printf(" Address		:\t%s \n", conn->address);
	printf(" \n\n");
}

void print_iscsi_scsi_disk(struct iscsi_dev_info *iscsi_dev)
{
	print_trace_enter();
	printf(" Scsi%02d channel: %02d  Id: %02d Lun: %02d \n",
		iscsi_dev->session->scsi_channel, iscsi_dev->session->scsi_bus,
		iscsi_dev->session->scsi_id, iscsi_dev->session->scsi_lun);
}

void print_enclosure_header(void)
{
	print_trace_enter();

	printf("\n");
	printf("%-12s\t%-16s\t%-16s\t%-16s\t%-16s\n",
	    " BUS ID", "Vendor", "Model", "Device Type", "Device Path");
	printf("%-.12s\t%-.16s\t%-.16s\t%-.16s\t%-.16s\n",
		dash, dash, dash, dash, dash);
}

void print_enclosure_info(struct scsi_device_info *d_info_p)
{
	print_trace_enter();
	printf("[%-s]\t%-16s\t%-16s\t%-16s\t%-16s \n",
		d_info_p->disk_name, d_info_p->vendor, d_info_p->model,
		d_info_p->disk_type, d_info_p->disk_path);
}
