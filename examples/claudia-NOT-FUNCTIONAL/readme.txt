notes 16.1.2020/pekka
Claudia - routing connections trough cloud server. 

Home network application example: A top level controller (frank, etc) is running in computer at house (home network). 
IO devices within the house network connect directly to frank. But the user  has also devices outside home,
which need to be connected as a part of the house home network. This could be Android phone user interface
used to control the home network, or some other device. Here Cloudia comes in. Frank at pekka's house creates
a socket to claudia in cloud server. Then data from remote application can be routed trough the server to
house network.

One instance of claudia can support multiple users. If there is also home network of other user, we do not want to set
up own cloud server instance. So same claudia process serves multiple IO networks, which could be here named pekkanet
and markkunet. Claudia makes sure that Markku's and Pekka's stuff is kept completely separated.

