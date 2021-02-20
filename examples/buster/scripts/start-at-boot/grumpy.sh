#!/bin/bash
# You need to build release version of sudo chmod +x /usr/local/bin/htg.sh
# Copy this grumpy.sh file to /usr/local/bin: sudo cp /coderoot/iocom/examples/buster/scripts/start-at-boot/grumpy.sh /usr/local/bin
# Make grumpy.sh is executable sudo chmod +x /usr/local/bin/grumpy.sh
# Copy grumpy.service file: sudo cp /coderoot/iocom/examples/buster/scripts/start-at-boot/grumpy.service /etc/systemd/system/grumpy.service
# Allow read access: sudo chmod 644 /etc/systemd/system/grumpy.service
# Ask status: systemctl status grumpy.service
# Reload services list: sudo systemctl daemon-reload
# Enable grumpy service: sudo systemctl enable grumpy
# Start grumpy service: sudo systemctl start grumpy
# Stop grumpy service: sudo systemctl stop grumpy
# Disable grumpy service: sudo systemctl disable grumpy

echo "grumpy.service: ## Starting ##" | systemd-cat -p info
/coderoot/bin/linux/buster

# do
# TIMESTAMP=$(date '+%Y-%m-%d %H:%M:%S')
# echo "htg.service: timestamp ${TIMESTAMP}" | systemd-cat -p info
# sleep 10
# done
