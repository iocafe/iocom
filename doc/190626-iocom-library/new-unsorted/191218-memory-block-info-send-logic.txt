Logic for sending memory block info messages

*** new connection:
- If new connection is pointing "up", add all memory blocks to connection's global transfer list.

*** new memory block by app function call (not trough new received memory block info):
- Add memory block to global transfer list of all connections pointing upwards.

*** memory block info received from connection pointing downwards.
- Create memory block if we do not have one. Resize if needed.
- Set up transfer buffer (for send or receive) if we do not have one.
- Store memory block id into transfer buffer
- Add memory block to global transfer list of all connections pointing upwards.
- Add memory block to connection's downwards transfer list.

*** memory block info received from connection pointing upwards.
- Memory block should exist. If not ignore the info message.
- Set up transfer buffer (for send or receive) if we do not have one.
- Store memory block id into transfer buffer

*** Maintaining transfer list:
- global list for sending memory block info upwards: a connection has current memory block
  pointer (con->sinfo.current_mblk), chain of all memory blocks (mblk->link.next) is used
  to move to next item in list.

*** Special transfer list for cloud specific account data memory blocks:
- current_cloud_mblk, works much the same as current_mblk, but downwards and only for specific memory blocks.

- connections downwnwards transfer list is kept trough transfer buffers, so two heads (separate ones 
  for source and target buffers). Heads are con->sbuf.mbinfo_down and con->tbuf.mbinfo_down .
  links are sbuf->clink.next and tbuf->clink.next.

*** Clarifications
- "connection points upwards flag IOC_CONNECT_UP means that node behind the connection is higher in IO network hierarchy than this one.
- "memory block flag IOC_MBLK_UP means that data in this memory block flows up in IO network hierarchy.

