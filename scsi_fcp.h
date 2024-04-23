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

#ifndef SCSI_FCP_H
#define SCSI_FCP_H

#include "scsi.h"

#define SCSI_VENDOR_LEN		16
#define	SCSI_PROD_LEN		16
#define PCI_BUS_LEN		16

enum fc_driver_module {
	qla2xxx = 1,
	lpfc
};

struct fc_host_statistics {
        u64 seconds_since_last_reset;

        /* TX/RX data */
        u64 tx_frames;
        u64 tx_words;
        u64 rx_frames;
        u64 rx_words;
        u64 invalid_tx_word_count;

        u64 lip_count;
        u64 nos_count;
        u64 error_frames;
        u64 dumped_frames;
        u64 link_failure_count;
        u64 loss_of_sync_count;
        u64 loss_of_signal_count;
        u64 prim_seq_protocol_err_count;
        u64 invalid_crc_count;

        /* fc4 statistics  (only FCP supported currently) */
        u64 fcp_input_requests;
        u64 fcp_output_requests;
        u64 fcp_control_requests;
        u64 fcp_input_megabytes;
        u64 fcp_output_megabytes;
        u64 fcp_packet_alloc_failures;  /* fcp packet allocation failures */
        u64 fcp_packet_aborts;          /* fcp packet aborted */
        u64 fcp_frame_alloc_failures;   /* fcp frame allocation failures */

        /* fc exches statistics */
        u64 fc_no_free_exch;            /* no free exch memory */
        u64 fc_no_free_exch_xid;        /* no free exch id */
        u64 fc_xid_not_found;           /* exch not found for a response */
        u64 fc_xid_busy;                /* exch exist for new a request */
        u64 fc_seq_not_found;           /* seq is not found for exchange */
        u64 fc_non_bls_resp;            /* a non BLS response frame with
                                           a sequence responder in new exch */
        /* Host Congestion Signals */
        u64 cn_sig_warn;
        u64 cn_sig_alarm;

	/* Congetion */
	u64 fpin_cn;
	u64 fpin_cn_credit_stall;
	u64 fpin_cn_device_specific;
	u64 fpin_cn_lost_credit;
	u64 fpin_cn_oversubscription;

	/* Link Integrity Event Stats */
	u64 fpin_li;
	u64 fpin_li_device_specific;
	u64 fpin_li_failure_unknown;
	u64 fpin_li_invalid_crc_count;
	u64 fpin_li_invalid_tx_word_count;
	u64 fpin_li_link_failure_count;
	u64 fpin_li_loss_of_signals_count;
	u64 fpin_li_loss_of_sync_count;
	u64 fpin_li_prim_seq_err_count;

	/* Diagnostic */
	u64 fpin_dn;
	u64 fpin_dn_device_specific;
	u64 fpin_dn_timeout;
	u64 fpin_dn_unable_to_route;
	u64 fpin_dn_unknown;
};

struct fc_rport_info {
	char	*rport_path;
	char	*rport_name;
	char	*node_name;
	char	*port_name;
	char	*port_id;
	char	*roles;
	char	*maxframe_size;
	char	*supported_classes;
	int	dev_loss_tmo;
	char	*port_state; /* Linkedown, Linkup */
};

struct fc_device_info {
	char	*sys_dev_path;	/* Complete path ex: /sys/class/fc_host/host10 */
	char	pci_bus_id[16];		/*Bus ID. For ex: 00:b4:1 */
	char	*host_name;		/* Host name listed in the system. for ex host0..host10 */
	char	*model_desc;
	char	*product_name;		/* Product name */
	char	*port_state;		/* Linkedown, Linkup */
	char	*port_speed;		/* 32/16/8 */
	char	*port_type;		/* NPort */
	char	*port_id;		/* ff:ff:ff */
	char	*port_name;		/* 20:00:00:00:00:00:00:00 */
	char	*node_name;		/* 20:00:00:00:00:00:00:00 */
	char	*driver_name;		/* Emulex/QLogic */
	char	*fw_version;		/* Firmware version */
	char	*drv_version;		/* Driver Version */
	char    *supported_class;
	char    *supported_speed;
	char	*link_state;
	char	*active_mode;
	char	*serial_num;
	char	*fabric_name;		/* 20:00:00:00:00:00:00:00 */
	char	*symbolic_name;
	int	dev_loss_tmo;		/* typically 30s */
	int	max_npiv_vports;
	int	npiv_vports_inuse;
	int	no_rports;		/*  Rport Count */

	struct list_head		rport_list;
	struct fc_host_statistics	stats;
	struct fc_rport_info		*remote_port;
	struct fc_device_info		*next;
};

void get_symbolic_name(struct fc_device_info *);
int show_fc_port_details(char **, struct scsi_device_list *);
int validate_fc_path(char *);
int list_fc_adapters(struct fc_device_info *);
int show_fc_adapter_details(int, char **, struct fc_device_info *);
int get_adapter_details(struct fc_device_info *);
void get_driver_info(struct fc_device_info *);
int fc_hba_reset(int, char **);
int get_fc_dev_stats(char *, struct fc_device_info *);
int get_fc_info(struct fc_device_info *);
int get_fcport_error_count(char **, struct fc_device_info *);
int scsi_scan_fcport_dev(char **, struct fc_device_info *);
int set_fcport_offline(char **, struct fc_device_info *);
int set_fcport_online(char **, struct fc_device_info *);
int set_fcport_alias(char **, struct fc_device_info *);

/* For Rport Information */
int list_rport_adapters(struct fc_device_info *);
void get_rport_info(struct fc_rport_info *);
#endif
