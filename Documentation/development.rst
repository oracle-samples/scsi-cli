Development
===========

You can find the source code for scsi-cli
`here <https://linux-git.oraclecorp.com/hmadhani/scsi-cli>`_.
You are welcomed to participare in its development in many ways:

-- Report bugs via issues
-- Submit merge reqeustes with your new features or bug fixes, if you'd like.

This document contains information about how to get a development enviornment
set up, and how to do the main development processes.

Enviornment Setup
-----------------

This repository is provided as-is and would be maintained by Oracle.
In order to contribute, first clone the repository in you local host
using following commands:

1. Clone repository to your local system
 # git clone <repository/path> repository.git

2. Change directory to the newly cloned repository
 # cd repository.git

3. Compile source using make command.
 # make all

Note: User must have make build enviornment installed on the server.

4. Once the source is compiled. look for executable in the directory

# ./scsi-cli

5. There will be various command option presented on the display menu.

6. Use one of the supported command to find out more about the devices present
   on this system.
