require recipes-core/images/core-image-minimal.bb

SUMMARY = "Minimal image with vehicle-node service"

IMAGE_INSTALL:append = " vehicle-node"

IMAGE_FEATURES += "ssh-server-dropbear"

DISTRO_FEATURES:append = " systemd"
VIRTUAL-RUNTIME_init_manager = "systemd"
VIRTUAL-RUNTIME_initscripts = ""
