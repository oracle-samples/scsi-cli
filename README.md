# scsi-cli

scsi-cli is a tool for listing and manipulating various Small Computer System Interface (SCSI) devices on a system.
It contains basic command to list devices on the system and other commands to
change sysfs values of a file to perform action supported by those sysfs file properties.

Please note, each scsi-cli commands is only supported for block volumes and
transpots such as Fibre channel, iSCSI at the moment. In future other transports
and scsi devices will be added to the tools capabiltiy.

## Getting Started

Requires build tool and make installed on your linux system.

1. Clone the repository

``` sh
git clone https://github.com/oracle-samples/scsi-cli.git
```

2. A simple make comand builds the tool from source

``` sh
make all
```

3. To install this tool issue make command with install option

``` sh
make install
```
This command will install this tool at /usr/local/bin

4. to uninstall this tool issue make command with uninstall option

``` sh
make uninstall
```

## Documentation

see [documentation](doc) for more information on how to use this tool and how to 
contribute.

## Examples

When the tool is build issuing scsi-cli will show all supported command options

``` sh
# ./scsi-cli

    scsi-cli - v0.1

    Usage:
    ------
    scsi-cli <command> <sub-command> [<device>]

    Where:
    ------
    The '<sub-command>' is command specific options supported by the tool
    The '<device>' may be either

      - A Fibre Channel Device

      - An iSCSI Device

      - One of the
        - SCSI generic device (ex: /dev/sdX)
        - SCSI disk device (ex: /dev/sgX)
        - NVMe Block device (ex: /dev/nvmeXnY)
        - NVMe generic Block device (ex: /dev/ngX)
        - Device Mapper stacking device (ex: /dev/dm-X)
        - RAID Array device (ex: /dev/mdXYZ)

    Following commands are implemented:
    ----------------------------------------------------------------
    alias              Create handy alias for disk, adater, controller
    errors             Show errors for a device
    list               List all device on this houst
    online             Change device state to online
    offline            Change device state to offline
    reset              Reset a device
    stats              Show statistics of a device
    show               Show details
    scan               Scan a system for device

    See 'scsi-cli help <command>' for each command
```

Each command supports a list of sub-commands which will be displayed when
you issue the command with no other options.

for example, using command `scsi-cli list` will give option for sub-commands
supportd by the 'list' command.

``` sh
# ./scsi-cli list

  Following Sub-Commands are supported for 'list'
  ----------------------------------------------------------------
  cd-rom             List CD-ROM devices from the host
  tape               List Tape devices from the host
  medium-changer     List Medium Changer devices from the host
  enclosure          List Enclosure devices from the host
  fc_hba             List Fiber Channel HBA instlled
  iscsi              List iSCSI Sessions
  disk               List disks from the host
  controller         List disk controllers from the host
  generic            List generic disk from the host
  multipath          List multipath disk from the host

```

This output is followed by list of all the devices present in the system.

``` sh
Displaying All Disk Controller devices on the system.
----------------------------------------------------------------

BUS ID      	Vendor          	Model           	Revision	Major	Minor	Disk Type       	Disk Name	Device Path
------------	----------------	----------------	--------	-----	-----	----------------	--------	------------------------
[0:0:0:0]	IET             	Controller      	0001    	21   	6    	Storage Array   	sg6     	/sys/class/scsi_generic/sg6
[0:0:0:0]	IET             	Controller      	0001    	21   	4    	Storage Array   	sg4     	/sys/class/scsi_generic/sg4
[0:0:0:0]	IET             	Controller      	0001    	21   	2    	Storage Array   	sg2     	/sys/class/scsi_generic/sg2
[0:0:0:0]	IET             	Controller      	0001    	21   	8    	Storage Array   	sg8     	/sys/class/scsi_generic/sg8

Displaying All block devices on the system.
----------------------------------------------------------------

BUS ID      	Vendor          	Model           	Revision	Major	Minor	Disk Type       	Disk Name	Device Path
------------	----------------	----------------	--------	-----	-----	----------------	--------	------------------------
[6:0:0:1]	ORACLE          	BlockVolume     	1.0     	8    	80   	disk            	sdf     	/sys/block/sdf
[4:0:0:1]	ORACLE          	BlockVolume     	1.0     	8    	48   	disk            	sdd     	/sys/block/sdd
[2:0:1:1]	ORACLE          	BlockVolume     	1.0     	8    	16   	disk            	sdb     	/sys/block/sdb
[5:0:0:1]	ORACLE          	BlockVolume     	1.0     	8    	64   	disk            	sde     	/sys/block/sde
[3:0:0:1]	ORACLE          	BlockVolume     	1.0     	8    	32   	disk            	sdc     	/sys/block/sdc
[2:0:0:1]	ORACLE          	BlockVolume     	1.0     	8    	0    	disk            	sda     	/sys/block/sda

Displaying All nvme-block devices on the system.
----------------------------------------------------------------

BUS ID      	Model                           	Revision    	Major	Minor	Disk Type       	Disk Name	Device Path
------------	--------------------------------	------------	-----	-----	----------------	--------	------------------------

Displaying All Multipath Devices devices on the system.
----------------------------------------------------------------

BUS ID      	Major	Minor	Disk Name	Device Path
------------	-----	-----	--------	------------------------
[0:0:0:0]	252  	1    	dm-1    	/sys/block/dm-1
[0:0:0:0]	252  	0    	dm-0    	/sys/block/dm-0

Displaying All iSCSI devices on the system.
----------------------------------------------------------------

Host Name   	Transport	IP Address      	Port	Connection      	Session         	Target Name
------------	----------	----------------	------	----------------	------------	----------------------------------------------------------------
host5       	tcp     	169.254.2.7     	3260	connection3:0   	session3    	iqn.2015-12.com.oracleiaas:886afa56-8c9f-4501-8a9a-565d0b4b1c3e
host3       	tcp     	169.254.2.5     	3260	connection1:0   	session1    	iqn.2015-12.com.oracleiaas:86161095-2195-477e-bc62-613e4f526301
host6       	tcp     	169.254.2.8     	3260	connection4:0   	session4    	iqn.2015-12.com.oracleiaas:bd6ee62a-f309-4122-8d1b-b7e5eba5dde1
host4       	tcp     	169.254.2.6     	3260	connection2:0   	session2    	iqn.2015-12.com.oracleiaas:2031e68f-73fc-40e4-8ecd-d325c3feb34b

```
Note:	Currently this tool only supports Disk device, Fibre Channel Devices,
	iSCSI devices. Support for other devices and transport will be added
	later in the future release.

## Help

If you are having trouble using scsi-cli tool, please create a Github [issue](issue), and we'll try to help as best as we can.

## Contributing

scsi-cli welcomes contributions from the community. Before submitting a pull request, please [review our contribution guide](./CONTRIBUTING.md)

## Security

Please consult the [security guide](./SECURITY.md) for our responsible security vulnerability disclosure process.

## License

Copyright (c) 2024 Oracle and/or its affiliates.

Released under the Universal Permissive License v1.0 as shown at
<https://oss.oracle.com/licenses/upl/>.

[doc]: https://github.com/oracle-samples/scsi-cli/blob/main/Documentation/development.rst
[issue]: https://github.com/oracle-samples/scsi-cli/issues
