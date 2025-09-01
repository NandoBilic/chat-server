#include <stdio.h>
#include <string.h>
#include "http-server.h"
#include <stdlib.h>
#include <stdint.h>

char const HTTP_404_NOT[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n";
char const HTTP_200_OK[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";// ; charset=utf-8\r\n\r\n
const char HTTP_400[] = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nBad Request";

uint8_t chat_id = 0;

struct Reaction {
	char *username;
	char *message;
};
typedef struct Reaction Reaction;

struct Chat {
	//id counts number of chats and is printed on each line wih chat
	uint32_t id;
	char *user;
	char *message;
	char timeStamp[22];
	uint32_t num_reactions;
	uint32_t capacity;
	Reaction *reactions;
};
typedef struct Chat Chat;

struct Server{
	Chat *allchats;
	uint32_t size;
	uint32_t capacity;
};
typedef struct Server Server;

//Global variable for whole chat server
Server status;

void add_server(Chat add_chat);


void add_chat(char* username, char* message) {
	chat_id += 1;
	time_t now = time(NULL);
	struct tm *local_time = localtime(&now);
	char buffer[22];
	strftime(buffer,sizeof(buffer), "%Y-%m-%d %H:%M:%S",local_time);
	
	//Trying to make c a Chat* (need to use -> on all fields now)
	Chat c;
	c.id = chat_id;
	c.user = calloc(strlen(username) + 1,sizeof(char));
	c.message  = calloc(strlen(message) + 1, sizeof(char));
	
	//alocation check
	if( c.user == NULL || c.message == NULL) {
		printf("Error allocating in add_chat func");
	}
	strncpy(c.user,username,strlen(username) + 1);
	strncpy(c.message,message,strlen(message) + 1);
	strncpy(c.timeStamp,buffer,strlen(buffer) + 1);
	
	c.num_reactions = 0;
	c.capacity = 5;
	Reaction *reactions = calloc(c.capacity,sizeof(Reaction));
	c.reactions = reactions;
	
	add_server(c);
}

uint8_t add_reaction(char* username,char* message, int id){
	//uint32_t int_id = atoi(id);
	
	/*if(int_id > chat_id) {
		return 0;
	}*/
	//changed id from char* to int. If doesn't work switch back.
	uint32_t int_id = id;
	Reaction r;
	r.username = calloc(strlen(username) + 1,sizeof(char));
	r.message = calloc(strlen(message) + 1,sizeof(char));
	strncpy(r.username,username,strlen(username) + 1);
	strncpy(r.message,message,strlen(message) + 1);
	
	Chat *c = &(status.allchats[int_id - 1]);
	
	if(c->num_reactions >= c->capacity) {
		c->capacity = c->capacity * 2;
		c->reactions = realloc(c->reactions,c->capacity * sizeof(Reaction));
	}

	c->reactions[c->num_reactions] = r;
	c->num_reactions += 1;
	
	return 1;
}

void edit_chat(char *message, int id) {
	Chat *c = &(status.allchats[id - 1]);
	free(c->message);
	c->message = malloc(strlen(message) + 1);
	strcpy(c->message,message);
}
	

void handle_edit(char *path , int client_sock);
void handle_response(char *request, int client_sock);
void handle_404(int client_sock,char *path);
void handle_400(int client_sock,char *path);
void reset();
void respond_with_chats(int client);
void handle_post(char* post,int client);
void test_handle_post();
void handle_reaction(char *path, int client);
void url_decode(char *message, char* dest);
uint8_t hex_to_byte(char c);

Server make_server(){
	Chat *initial_contents = calloc(5,sizeof(Chat));
	Server s = {initial_contents,0,5};
	return s;
}

void add_server(Chat add_chat) {
	if(status.size >= status.capacity) {
		status.capacity = (status.capacity) * 2;
		status.allchats = realloc(status.allchats,status.capacity * sizeof(Chat));
	}

	status.allchats[status.size] = add_chat;
	status.size += 1;
}

void test_add_chat() {
	reset();
	status = make_server();
    add_chat("Alice", "Hello World");
    assert(status.size == 1); // Check that one chat was added
    assert(status.allchats[0].id == 1); // Check chat ID
    assert(strcmp(status.allchats[0].user, "Alice") == 0); // Check username
    assert(strcmp(status.allchats[0].message, "Hello World") == 0); // Check message
	edit_chat("NEW MESSAGE",1);
	//printf("%s\n",status.allchats[0].message)
	assert(strcmp(status.allchats[0].message, "NEW MESSAGE") == 0);
    add_chat("Bob", "Goodbye World");
    assert(status.size == 2); // Check that a second chat was added
    assert(status.allchats[1].id == 2); // Check chat ID for second chat
    assert(strcmp(status.allchats[1].user, "Bob") == 0); // Check username for second chat
    assert(strcmp(status.allchats[1].message, "Goodbye World") == 0); // Check message for second chat
}

void test_add_reaction() {
	
	reset();
	status= make_server();
	
	add_chat("Charlie", "React to this!");
    assert(status.size == 1); // Ensure the chat was added
    assert(status.allchats[0].num_reactions == 0); // Check no reactions initially

    uint8_t result = add_reaction("Dave", "Nice!", 1);
    assert(result == 1); // Check that the reaction was successfully added
    assert(status.allchats[0].num_reactions == 1); // Check that reaction count increased
    assert(strcmp(status.allchats[0].reactions[0].username, "Dave") == 0); // Check username of reaction
    assert(strcmp(status.allchats[0].reactions[0].message, "Nice!") == 0); // Check message of reaction

    // Test adding reaction to a non-existing chat
    result = add_reaction("Eve", "Doesn't exist", 2);
    //assert(result == 0); // Check that it returns -1 for non-existing chat
}

void reset(){
	int i;
	int j;
	for(i = 0; i < chat_id; i++) {
		uint8_t num_reacts = status.allchats[i].num_reactions;
		for(j = 0; j < num_reacts; j++) {
			free(status.allchats[i].reactions[j].username);
			free(status.allchats[i].reactions[j].message);
		}
		free(status.allchats[i].reactions);
		free(status.allchats[i].user);
		free(status.allchats[i].message);
	}
	free(status.allchats);
	chat_id = 0;
	status = make_server();
}

void test_handle_post(){
	reset();
	handle_post("/post?user=Nando&message=Testing",1);
	

}	

uint8_t testing = 0;
int main(int argc, char** argv) {
	int port = 0;
	if(argc >= 2) {
		port = atoi(argv[1]);
	}

	if (testing == 1){
		test_add_chat();
		test_add_reaction();
		
		reset();
		exit(0);
	}
	else{
		status = make_server();
		start_server(&handle_response,port);
	}
}

void handle_response(char *request, int client_sock){
	char path[300];
	char to_modify[300];
	printf("\nSERVER LOG: Got request: \"%s\"\n", request);

	if (sscanf(request, "GET %299s", path) != 1) {
		printf("Invalid request line\n");
		return;
	}

	strncpy(to_modify,path,strlen(path));
	char *token = strtok(to_modify,"?");
	


	if(strcmp("/chats",path) == 0) {
		respond_with_chats(client_sock);
	}
	else if (strcmp("/reset",path) == 0){
		reset();
		write(client_sock,HTTP_200_OK,strlen(HTTP_200_OK));
		char str[] = "\r\nRESET\r\n";
		//write(client_sock,str,strlen(str));
	}
	else if(strcmp("/post",token) == 0){
		handle_post(path,client_sock);
	}
	else if(strcmp("/react",token) == 0){
		handle_reaction(path,client_sock);
	}
	else if(strcmp("/edit",token) == 0){
		handle_edit(path,client_sock);
	}
	else{
		handle_404(client_sock,path);
	}
}


void respond_with_chats(int client){
	
	write(client,HTTP_200_OK,strlen(HTTP_200_OK));
	char chat[BUFFER_SIZE];
	char reaction[BUFFER_SIZE];

	int i;
	int j;
	for(i = 0; i < chat_id; i++) {
		Chat c = status.allchats[i];
		snprintf(chat, BUFFER_SIZE, "[#%d %s]  %s: %s\r\n",c.id,c.timeStamp,c.user,c.message);
		write(client,chat,strlen(chat));
		for(j = 0; j < c.num_reactions; j++) {
			snprintf(reaction,BUFFER_SIZE,"       (%s)  %s\r\n",c.reactions[j].username,c.reactions[j].message);
			write(client,reaction,strlen(reaction));
		}
	}
}

void handle_post(char* path, int client) {
	char user[16];
	char message[400];
	char c_message[400];
	if(sscanf(path,"/post?user=%15[^&]&message=%399[^\r\n]",user,message) != 2){
		handle_400(client,path);
		return;
	}
	if(strlen(message) >= 256){
		handle_400(client,path);
		return;
	}
	if(100000 <= chat_id){
		handle_404(client,path);
	}
	
	url_decode(message,c_message);
	add_chat(user,c_message);
	respond_with_chats(client);
	
}	

void handle_reaction(char *path,int client){
	char user[16];
	char message[16];
	char c_message[16];
	uint32_t id ;//what size do i make this. Could make int but would have to change add_response func.
	if(sscanf(path,"/react?user=%15[^&]&message=%15[^&]&id=%d",user,message,&id) != 3){
		handle_400(client,path);
		return;
	}
	if(id > chat_id || id < 1){
		handle_400(client,path);
		return;
	}
	if(status.allchats[id-1].num_reactions >= 100) {
		handle_400(client,path);
		return;
	}
	
	url_decode(message,c_message);
	add_reaction(user,c_message,id);
	respond_with_chats(client);
}

void handle_edit(char *path, int client_sock) {
	char message[400];
	char c_message[400];
	uint32_t id;
	if(sscanf(path,"/edit?id=%d&message=%399[^\r\n]",&id,message) != 2) {
		handle_400(client_sock,path);
		return;
	}

	if(strlen(message) >= 256){
		handle_400(client_sock,path);
		return;
	}
	if(id > status.size) {
		handle_400(client_sock,path);
		return;
	}
	url_decode(message,c_message);
	edit_chat(c_message,id);
	respond_with_chats(client_sock);

}

void url_decode(char* original, char* decoded){
	char *p_orig = original;
	char *p_dest = decoded;
	while(*p_orig) {
		if (*p_orig == '%'){
			char c1 = *(p_orig + 1);
			char c2 = *(p_orig +2);
			uint8_t i1 = hex_to_byte(c1);
			uint8_t i2 = hex_to_byte(c2);
			uint8_t value = (i1 << 4) | i2;
			*p_dest = (unsigned char) value;
			p_orig += 2;
		}
		else{
			*p_dest = *p_orig;
		}
		p_orig++;
		p_dest++;
	}
	*p_dest = 0;
}

uint8_t hex_to_byte(char c) {
	if ('0' <= c && c <= '9') return c -'0';
	if ('a' <= c && c <= 'f') return c - 'a' + 10;
	if ('A' <= c && c <= 'F') return c - 'A' + 10;
}

void handle_404(int client_sock,char *path) {
	printf("SERVER LOG: Got request for unrecognized path \"%s\"\r\n",path);

	char response[BUFFER_SIZE];
	snprintf(response, BUFFER_SIZE, "Error 404:\r\nUnrecognized path \"%s\"\r\n",path);
	write(client_sock,HTTP_404_NOT,strlen(HTTP_404_NOT));
	write(client_sock,response, strlen(response));
}

void handle_400(int client_sock,char* path) {
	printf("SERVER LOG: Overloaded request");
	write(client_sock,HTTP_400,strlen(HTTP_400));
	write(client_sock,"\r\nOverflow\r\n",8);
}
