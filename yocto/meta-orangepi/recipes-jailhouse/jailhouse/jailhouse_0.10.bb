require jailhouse.inc

JAILHOUSE_SRCBRANCH = "master"
JAILHOUSE_SRC ?= "git://github.com/siemens/jailhouse.git;protocol=git"
 
SRC_URI = "${JAILHOUSE_SRC};branch=${JAILHOUSE_SRCBRANCH} \
	  file://one_patch.patch"

SRCREV = "f596aa7355bc2134650544bdf1e13f1f55d3f2fc"

CELLS = ""

COMPATIBLE_MACHINE = "${@bb.utils.contains('MACHINE_FEATURES', 'jailhouse', '${MACHINE}', '(^$)', d)}"
