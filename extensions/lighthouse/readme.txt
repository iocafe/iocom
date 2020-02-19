18.2.2020 / pekka
service discovery using UDP multicasts 
- Server multicasts UDP message periodically
- Clients listens for these

Multicast contains
IO network name/service IP address/port/protocol. 0 address means sender of multicast. Protocol = service protocol iocom+tls, iocom+socket, eobjects+tls
Time stamp
Random number
Network name to use if unspecified.
