#!/bin/bash
sudo cp /coderoot/iocom/examples/buster/scripts/start-at-boot/grumpy.sh /usr/local/bin
sudo chmod +x /usr/local/bin/grumpy.sh
sudo cp /coderoot/iocom/examples/buster/scripts/start-at-boot/grumpy.service /etc/systemd/system/grumpy.service
sudo chmod 644 /etc/systemd/system/grumpy.service
sudo systemctl daemon-reload
sudo systemctl enable grumpy
sudo systemctl start grumpy

