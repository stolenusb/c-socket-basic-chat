<h1>Chat Server and Client</h1>

<p>
    This project implements a simple chat server and client using sockets in C (Linux). The server supports multiple clients, and clients can send messages to each other through the server. The server also provides administrative commands to manage clients and shut down the server.
</p>

<h2>Features</h2>

<h3>Server Features:</h3>
<ul>
    <li>Supports up to <code>MAX_CHATTERS</code> (10) clients simultaneously.</li>
    <li>Messages sent by one client are broadcast to all other connected clients.</li>
</ul>

<h3>Admin Commands:</h3>
<ul>
    <li><code>/shutdown</code>: Shuts down the server and disconnects all clients.</li>
    <li><code>/kick &lt;socket_id&gt;</code>: Kicks a specific client by their socket ID.</li>
    <li>The server closes all client sockets and exits cleanly when shut down.</li>
    <li>Uses <code>select</code> with a timeout to periodically check for server shutdown and new client connections.</li>
</ul>

<h3>Client Features:</h3>
<ul>
    <li>Connects to the server using the provided host IP and port.<br>
        Example: <code>./client &lt;HOSTIP&gt; &lt;PORT&gt; &lt;USERNAME&gt;</code>
    </li>
    <li>Sends messages to the server, which are broadcast to all other clients.</li>
    <li>Receives messages from the server and displays them in the terminal.</li>
    <li><code>/exit</code>: Disconnects from the server and exits the client program.</li>
    <li>Uses a separate thread to handle sending messages, allowing the client to receive messages simultaneously.</li>
</ul>
