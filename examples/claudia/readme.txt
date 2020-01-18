notes 16.1.2020/pekka
Claudia - cloud server application for routing connections to controller at site. 

A top level controller (claudia, etc) is running in computer at local network (pekka's home network, etc). IO devices within Pekka's home network connect directly to claudia. But Pekka has also devices outside home, which need to be connected as part of Pekka's home network. Like Android phone (ispy) used to control the home network, and car music device. Here Cloudia comes in. Frank at Pekka's house, the i-spy in andoid phone and music device in car all connect to claudia running on cloud server. Claudia passes information between these. 

Then there is Markku's home network. We do not want to set up own cloud server application for Markku, but Markku and Pekka share same claudia. So same claudia process server multiple IO networks, which could be here named pekkanet and markkunet. Security in claudia makes sure that Markku's and Pekka's stuff is kept completely separated.
