This project implements a simple chat server and client using sockets in C (linux). The server supports multiple clients, and clients can send messages to each other through the server. The server also provides administrative commands to manage clients and shut down the server.


Features
Server Features:
-Supports up to MAX_CHATTERS (10) clients simultaneously.
-Messages sent by one client are broadcast to all other connected clients.

-Admin Commands:
/shutdown: Shuts down the server and disconnects all clients.
/kick <socket_id>: Kicks a specific client by their socket ID.
-Graceful Shutdown:
The server closes all client sockets and exits cleanly when shut down.
-Timeout Handling:
Uses select with a timeout to periodically check for server shutdown and new client connections.

Client Features:
-Connects to the server using the provided host IP and port.
    ./client <HOSTIP> <PORT> <USERNAME>
-Sends messages to the server, which are broadcast to all other clients.
-Receives messages from the server and displays them in the terminal.
-/exit: Disconnects from the server and exits the client program.
-Uses a separate thread to handle sending messages, allowing the client to receive messages simultaneously.