17.3.2021 / pekka
switchbox library is intended for writing a passtrough service to connect to set of IO networks.

Connection netween IO network service in local LAN and switchbox service in cloud
- IO network service on local LAN connects to switchbox service and announces that I have network "pekkanet", etc. 
- This TLS connection is established by an end point specific for the purpose class in IO network service. 

IO device connecting to IO network trough cloud server
- When a device can connect to IO network service trough cloud, it connects first to switchbox service.
- This connection is normal IOCOM or ECOM connection, requested IO network is specified in authentication header. 
- When switchbox server recives a socket connection, it checks if this IO network server with given network name
  is connected. If so, it generates a passtrough to it. 
- Switchbox sends "new connection" message to IO server in local LAN. The IO server needs to create a iocom or exom connection object using switchbox stream class as transport.
- Data from IO device passtrough is forwared "as is" trough switchbox.

UPDATE THIS TEXT AND DOCUMENT, see /coderoot/iocafe-doc/source/extensions/switchbox

NETWORK SELECTION REQUEST MUST BE SEPARATED FROM AUTHENTICATION FRAME TO WORK WITH SWITCHBOX.
- Network selection is sent from only from socket client to server.
- Authentication is sent both ways IOCOM: first from upper level in device hierarchy to lower, then lower to upper. This will not do with switchbox, because network selection in authentication frame is not available to select the IO service before IO service is connected.

Automatic establishment of trust
IF SERVER CERTIFICATE VERIFICATION FAILS IN CLIENT, CLIENT MUST BE ABLE TO ASK FOR SERVER, ETC, CERTIFICATE FROM SERVER. CURRENT IMPLEMENTATION WITH IOCOM USING BRICK TRANSFER TO GET THE CERTIFICATE IS NOT WORKING WITH SWITCBOX. IF SWITCHBOX IS USED, THIS WOULD GET IO NETWORK SERVER'S CERTIFICATE, WHILE WE NEED CLOUD SERVER'S CERTIFICATE.


