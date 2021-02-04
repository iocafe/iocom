3.2.2021 / pekka
The dynamicio directory contains dynamic data extensions for base iocom. These extensions allow controller to configure itself by signal/pin configuration from the IO device.

Implementation of dynamic IO network presentation can depend on use, for example if we compile iocom with 
the eobjects library, we want to use eobjects data abstraction. If we run with QT, we may want to use QT 
objects, etc. Simple default implementation of dynamic IO network objects implemented here is used by default,
unless an alternate implementation not provided.

