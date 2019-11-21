notes 21.11.2019/pekka
Frank - server side application demo. 

Freank listens for socket connections from any IO device network. When a first connection from an IO network is established, a new game application is started. The first connection is for player 1. If another connection from the IO network is established, it joins to the game. 

The server side connection received control information, like key presses, from the player application and sends game world state information back to player applications.

