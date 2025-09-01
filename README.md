# Chat Server in C

A lightweight chat server implemented in C with a simple HTTP interface for posting, editing, and reacting to chat messages.  
Built as a systems programming project to explore sockets, HTTP request parsing, memory management, and server design.

## Features
- TCP server handling multiple HTTP requests sequentially
- Endpoints supported:
  - `GET /chats` → returns all chats with reactions
  - `GET /post?user=<user>&message=<msg>` → add a new chat message
  - `GET /react?user=<user>&message=<msg>&id=<id>` → add a reaction to a message
  - `GET /edit?id=<id>&message=<msg>` → edit an existing message
  - `GET /reset` → clear all chats
- Messages timestamped and assigned unique IDs
- Supports dynamic memory growth for chat storage and reactions

## Build
bash
gcc -Wall -Wextra -O2 chat-server.c http-server.c -o chat-server
