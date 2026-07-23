SUMMARY = "Embedded Linux Vehicle Diagnostics Node"
DESCRIPTION = "Compact C++17 automotive Linux service with SocketCAN/vCAN diagnostics, health checks, logging, JSON status output, and systemd integration."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=25abb823ce29c4d53aa3ed92a3c5f0c0"

SRC_URI = "file://vehicle-node.service \
           file://vehicle-node.yaml"

inherit cmake systemd externalsrc

# The layer is PROJECT_ROOT/yocto/meta-vehicle-node, so two parent traversals
# select the existing C++ source tree for local development builds.
EXTERNALSRC = "${@os.path.abspath(os.path.join(d.getVar('LAYERDIR'), '..', '..'))}"
EXTERNALSRC_BUILD = "${WORKDIR}/build"

# Keep unpacking recipe-owned service/config assets into WORKDIR while the
# application itself is built from EXTERNALSRC.
SRCTREECOVEREDTASKS = "do_patch"
UNPACKDIR = "${WORKDIR}"

SYSTEMD_SERVICE:${PN} = "vehicle-node.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install:append() {
    install -d ${D}${sysconfdir}/vehicle-node
    install -m 0644 ${WORKDIR}/vehicle-node.yaml \
        ${D}${sysconfdir}/vehicle-node/vehicle-node.yaml

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/vehicle-node.service \
        ${D}${systemd_system_unitdir}/vehicle-node.service

    install -d ${D}${localstatedir}/log/vehicle-node
    install -d ${D}${localstatedir}/lib/vehicle-node
}

FILES:${PN} += "${sysconfdir}/vehicle-node/vehicle-node.yaml"
FILES:${PN} += "${systemd_system_unitdir}/vehicle-node.service"
FILES:${PN} += "${localstatedir}/log/vehicle-node"
FILES:${PN} += "${localstatedir}/lib/vehicle-node"

CONFFILES:${PN} += "${sysconfdir}/vehicle-node/vehicle-node.yaml"
RDEPENDS:${PN} += "iproute2"
