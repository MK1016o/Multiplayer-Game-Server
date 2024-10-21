#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <pthread.h>
#include <stdint.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define WEBSOCKET_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define MAX_CLIENTS 10

typedef struct {
    int socket;
    int id; // Unique identifier for the client
} Client;

Client *clients[MAX_CLIENTS];
int client_count = 0;

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Function to generate WebSocket Accept Key
void generate_accept_key(const char *client_key, char *accept_key) {
    char concat[BUFFER_SIZE];
    snprintf(concat, sizeof(concat), "%s%s", client_key, WEBSOCKET_GUID);

    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1((unsigned char *)concat, strlen(concat), hash);

    // Base64 encode the SHA1 hash
    static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i, j;
    for (i = 0, j = 0; i < SHA_DIGEST_LENGTH;) {
        uint32_t octet_a = i < SHA_DIGEST_LENGTH ? (unsigned char)hash[i++] : 0;
        uint32_t octet_b = i < SHA_DIGEST_LENGTH ? (unsigned char)hash[i++] : 0;
        uint32_t octet_c = i < SHA_DIGEST_LENGTH ? (unsigned char)hash[i++] : 0;

        uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;

        accept_key[j++] = base64_table[(triple >> 18) & 0x3F];
        accept_key[j++] = base64_table[(triple >> 12) & 0x3F];
        accept_key[j++] = base64_table[(triple >> 6) & 0x3F];
        accept_key[j++] = base64_table[triple & 0x3F];
    }

    for (i = 0; i < (3 - SHA_DIGEST_LENGTH % 3) % 3; i++)
        accept_key[j - 1 - i] = '=';
    accept_key[j] = '\0';
}

// Function to broadcast message to all connected clients
void broadcast_message(const char *message, int sender_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i] && clients[i]->socket != sender_socket) {
            send(clients[i]->socket, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Function to decode WebSocket frame
void decode_frame(const char *frame, char *message) {
    // The first byte contains the FIN and Opcode (first 4 bits are the opcode)
    // The second byte contains the masking key (first bit indicates if it's masked)
    int payload_length = (unsigned char)frame[1] & 0x7F; // Mask out the first bit
    int mask_offset = (payload_length == 126) ? 4 : (payload_length == 127) ? 10 : 2;

    // If the frame is masked, get the mask key
    unsigned char mask[4] = {0};
    if ((unsigned char)frame[1] & 0x80) {
        memcpy(mask, &frame[mask_offset], 4);
    }

    // Decode the message using the mask
    for (int i = 0; i < payload_length; i++) {
        message[i] = frame[mask_offset + 4 + i] ^ mask[i % 4];
    }
    message[payload_length] = '\0'; // Null-terminate the string
}

// Function to handle each client connection
void *handle_client(void *arg) {
    Client *client = (Client *)arg;
    char buffer[BUFFER_SIZE];
    
    // Wait for the handshake
    ssize_t bytes_received = recv(client->socket, buffer, sizeof(buffer), 0);
    
    if (bytes_received > 0) {
        printf("Received handshake: %s\n", buffer);
        
        // Perform WebSocket handshake
        if (strstr(buffer, "Upgrade: websocket") != NULL) {
            // Extract the Sec-WebSocket-Key from the client's request
            char client_key[BUFFER_SIZE] = {0};
            sscanf(strstr(buffer, "Sec-WebSocket-Key: ") + 19, "%s", client_key);

            // Generate the Sec-WebSocket-Accept key
            char accept_key[BUFFER_SIZE] = {0};
            generate_accept_key(client_key, accept_key);

            // Prepare WebSocket handshake response
            char response[BUFFER_SIZE] = {0};
            snprintf(response, sizeof(response),
                "HTTP/1.1 101 Switching Protocols\r\n"
                "Upgrade: websocket\r\n"
                "Connection: Upgrade\r\n"
                "Sec-WebSocket-Accept: %s\r\n\r\n", accept_key);

            // Send the handshake response to the client
            send(client->socket, response, strlen(response), 0);
            printf("Handshake completed. WebSocket connection established.\n");
        }
    }

    // Loop to handle incoming messages
    while (1) {
        bytes_received = recv(client->socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            printf("Client %d disconnected.\n", client->id);
            break; // Exit the loop on disconnect
        }

        char message[BUFFER_SIZE]; // Buffer to store decoded message
        decode_frame(buffer, message); // Decode the incoming WebSocket frame
        printf("Received message: %s\n", message);
        
        // Broadcast the chat message to all clients
        broadcast_message(message, client->socket); // Broadcast to other clients
    }

    // Clean up after client disconnection
    close(client->socket);
    free(client);
    pthread_exit(NULL);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;

    // Create the server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Could not create socket");
        exit(EXIT_FAILURE);
    }

    // Configure the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the address and port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    // Accept incoming client connections
    addr_size = sizeof(client_addr);
    while ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size)) > 0) {
        printf("Client connected.\n");

        pthread_mutex_lock(&clients_mutex);
        if (client_count < MAX_CLIENTS) {
            Client *new_client = malloc(sizeof(Client));
            new_client->socket = client_socket;
            new_client->id = client_count++;

            clients[client_count] = new_client;

            pthread_t tid;
            pthread_create(&tid, NULL, handle_client, (void *)new_client);
        } else {
            printf("Max clients reached. Client rejected.\n");
            close(client_socket);
        }
        pthread_mutex_unlock(&clients_mutex);
    }

    // Close the server socket
    close(server_socket);
    return 0;
}
