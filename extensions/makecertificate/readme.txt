makecertificate library - make root and server certificates from C code
26.4.2021 / pekka

Functions to create root and server certificates, certificate requests and related keys.
Goal is to automate basic secutiry setup so that "pairing" security is easy. This secures and locks connection and identification once initial "pairing"
for communication has been done, so it cannot be tampered afterwards. It is on user responsibility that two devices to be connected are actually ones they claim to be.

Thus automated security setup doesn't secure identification initial connection, is uses self signed certificate. If client needs to be able to verify 
that server is who it claims it is, it is necessary to use manually configured root certificate, and use it to sign server certificate.

This library depends on eosal library. There is no iocom dependency, even the library is intended
to be used with iocom.

