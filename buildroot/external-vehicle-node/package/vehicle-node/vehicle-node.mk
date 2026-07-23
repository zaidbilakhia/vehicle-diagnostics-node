################################################################################
#
# vehicle-node
#
################################################################################

VEHICLE_NODE_VERSION = 1.0
VEHICLE_NODE_SITE = $(BR2_EXTERNAL_VEHICLE_NODE_PATH)/../..
VEHICLE_NODE_SITE_METHOD = local
VEHICLE_NODE_SUPPORTS_IN_SOURCE_BUILD = NO

define VEHICLE_NODE_INSTALL_TARGET_EXTRAS
	$(INSTALL) -d -m 0755 $(TARGET_DIR)/etc/vehicle-node
	$(INSTALL) -m 0644 \
		$(BR2_EXTERNAL_VEHICLE_NODE_PATH)/package/vehicle-node/files/vehicle-node.yaml \
		$(TARGET_DIR)/etc/vehicle-node/vehicle-node.yaml
	$(INSTALL) -D -m 0644 \
		$(BR2_EXTERNAL_VEHICLE_NODE_PATH)/package/vehicle-node/files/vehicle-node.service \
		$(TARGET_DIR)/usr/lib/systemd/system/vehicle-node.service
	$(INSTALL) -d -m 0755 $(TARGET_DIR)/var/log/vehicle-node
	$(INSTALL) -d -m 0755 $(TARGET_DIR)/var/lib/vehicle-node
	$(INSTALL) -d -m 0755 \
		$(TARGET_DIR)/etc/systemd/system/multi-user.target.wants
	ln -sf /usr/lib/systemd/system/vehicle-node.service \
		$(TARGET_DIR)/etc/systemd/system/multi-user.target.wants/vehicle-node.service
endef

VEHICLE_NODE_POST_INSTALL_TARGET_HOOKS += VEHICLE_NODE_INSTALL_TARGET_EXTRAS

$(eval $(cmake-package))
