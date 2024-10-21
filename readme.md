# Real-Time Multiplayer Game Server with Chat Feature

This project is a real-time multiplayer game server that allows multiple users to connect, play simple games, and chat in real-time. The server is implemented in C using WebSocket, and the client-side is developed using HTML, CSS, and JavaScript.

## Features

- **Multiplayer Connection**: Multiple clients can connect to the server.
- **Real-Time Communication**: Low-latency, real-time chat between players.
- **Player Identifiers**: Each player is assigned an identifier like `Player1`, `Player2`, etc.
- **Simple Games**: Includes basic games such as Snake, where players' moves are sent to the server and broadcasted to all clients.
- **Chat System**: A built-in chat system allows players to communicate with each other, with each message labeled by the player's identifier.


## How It Works

### Server Side
The server is written in C using WebSocket. When a client connects, they are assigned an identifier like `Player1`, `Player2`, etc. The server handles:
- Assigning player identifiers
- Receiving messages and moves from players
- Broadcasting messages and game updates to all connected clients

### Client Side
The client is written using HTML, CSS, and JavaScript. It allows players to:
- Send chat messages
- Play simple games
- View other players' chat messages and game moves in real time

The client connects to the WebSocket server and displays a real-time chat panel and game window in a grid layout.

## Setup and Installation

### Prerequisites

- A C compiler (e.g., GCC)
- WebSocket library for C
- A modern browser for running the frontend

### Steps

1. **Compile the Server**:
    Use a C compiler to build the server. If you haven't installed the WebSocket library, ensure you do so before compiling:
    ```bash
    gcc -o server server.c -lpthread
    ```

2. **Run the Server**:
    Start the WebSocket server:
    ```bash
    ./server
    ```

3. **Run the Frontend**:
    Open `index.html` in your browser to connect to the server:
    ```bash
    open index.html
    ```

4. **Multiple Clients**:
    - You can simulate multiple clients by opening the `index.html` file in multiple tabs or browsers.

## How to Play

1. **Connect to the Server**: Open the frontend in a browser.
2. **Chat**: Use the chat panel to send messages. Messages will appear with your player identifier (`Player1`, `Player2`, etc.).
3. **Game**: Play the embedded Snake game by sending your moves to the server.

## Future Improvements

- Add more games (e.g., Tic-Tac-Toe, Battle Game).
- Enhance the chat feature with emojis or private messaging.
- Implement AI opponents for single-player mode.
- Add a leaderboard to track players' scores across multiple sessions.

## Technologies Used

- **Backend**: C, WebSocket
- **Frontend**: HTML, CSS, JavaScript

## Known Issues

- WebSocket connection may terminate unexpectedly if the server isn't configured properly.
- Only two players are supported at the moment (extendable to more).
