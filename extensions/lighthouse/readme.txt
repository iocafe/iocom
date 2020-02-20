18.2.2020 / pekka
service discovery using UDP multicasts 
- Server sends UDP multicasts periodically (once per 4 seconds or so). 
- Clients listens for these - server's address is IP from which the multicast was received.

Multicast contains
Port number of the listening TCP/TLS socket and transport.
IO network names which reply to multicast (first network name to be used, if client doesn't know it's network name). 
Time stamp
Random number
.
