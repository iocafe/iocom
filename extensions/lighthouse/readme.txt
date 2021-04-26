lighthouse library - discover services in network using UDP multicasts 
26.4.2021 / pekka

* Server sends UDP multicasts periodically (once per 4 seconds or so). 
* Clients listens for these - server's address is IP from which the multicast was received.

Multicast content
* Port number of the listening TCP/TLS socket and transport.
* IO network names which reply to multicast (first network name to be used, if client doesn't know it's network name). 
* Time stamp
* Random number

