The server is structured in two main components:
1. **HTTP Layer** (`http-server.c`):  
   - Opens TCP socket on given port  
   - Accepts client connections and reads raw HTTP requests  
   - Forwards request buffer to a handler function  
   - Sends back responses and closes connections  

2. **Chat Logic** (`chat-server.c`):  
   - Maintains global `Server` struct containing all chat messages  
   - Supports operations:
     - Add new message with timestamp and auto-incrementing ID
     - Edit existing message
     - Add reactions (with dynamic resizing of reaction arrays)
     - Serialize chats to text for HTTP response

## Data Structures
- **Chat**: stores ID, username, message, timestamp, and dynamic array of reactions  
- **Reaction**: stores username and message  
- **Server**: resizable array of chats  

## Memory Management
- Allocates new storage for usernames/messages with `calloc`  
- Reallocates arrays when capacity is exceeded  
- Provides `reset()` function to free all memory and reinitialize server state  

## Request Handling
- Uses `sscanf` to parse query parameters from request path  
- Supports URL decoding of `%xx` sequences  
- For invalid inputs, returns `400 Bad Request` or `404 Not Found`  
