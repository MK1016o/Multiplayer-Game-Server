const socket = new WebSocket('ws://127.0.0.1:8080');

// Listen for WebSocket connection open event
socket.onopen = function() {
    console.log('WebSocket connection established.');
    
    // Enable the buttons once the WebSocket connection is established
    document.getElementById('send-chat').disabled = false;
    document.getElementById('disconnect-btn').disabled = false;
};

// Listen for any errors
socket.onerror = function(error) {
    console.log('WebSocket Error:', error);
};

// Listen for WebSocket connection close event
socket.onclose = function() {
    console.log('WebSocket connection closed.');
    document.getElementById('send-chat').disabled = true;
    document.getElementById('disconnect-btn').disabled = true;
};

// Log received messages for debugging
socket.onmessage = function(event) {
    console.log('Message received:', event.data);
};

// Send a chat message
document.getElementById('send-chat').addEventListener('click', function() {
    const chatInput = document.getElementById('chat-input').value;
    if (socket.readyState === WebSocket.OPEN) {
        socket.send(`CHAT ${chatInput}`);
        document.getElementById('chat-input').value = '';
    } else {
        console.log('WebSocket is not open yet.');
    }
});

// Disconnect from server
document.getElementById('disconnect-btn').addEventListener('click', function() {
    if (socket.readyState === WebSocket.OPEN) {
        socket.send('DISCONNECT');
        socket.close();
        alert('You have been disconnected from the server.');
    } else {
        console.log('WebSocket is not open. Cannot disconnect.');
    }
});
