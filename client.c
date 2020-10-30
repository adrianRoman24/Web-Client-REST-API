#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.c"

void print_pretty(char *response) {
    char *serialized_string = strchr(response, '[');
    if (serialized_string == NULL) {
        return;
    }
    if (serialized_string[3] == 'e') {
        return;
    }
    JSON_Value *root_value = json_parse_string(serialized_string);
    JSON_Array *array = json_value_get_array(root_value);
    int length = json_array_get_count(array);
    if (length == 0) {
        return;
    }
    printf("Parsed response:\n");
    if (length > 1) {
        printf("[");
    }
    for (int i = 0; i < length; i ++) {
        JSON_Value *obj = json_array_get_value(array, i);
        if (i == (length - 1) && length > 1) {
            printf("%s]\n", json_serialize_to_string_pretty(obj));
        } else {
            printf("%s\n", json_serialize_to_string_pretty(obj));
        }
    }
    printf("End of parsed response.\n");
}

void command_register(char *buffer, int buflen, JSON_Value *root_value, JSON_Object *root_object,
            char *host, int sockfd) {
    // get info
    printf("username=");
    fgets(buffer, buflen, stdin);
    buffer[strlen(buffer) - 1] = 0;
    json_object_set_string(root_object, "username", buffer);
    printf("password=");
    fgets(buffer, buflen, stdin);
    buffer[strlen(buffer) - 1] = 0;

    // create json and messae
    json_object_set_string(root_object, "password", buffer);
    char *serialized_string = json_serialize_to_string_pretty(root_value);
    char *message = compute_post_request(host, ROUTE_REGISTER, NULL, serialized_string, NULL);

    // send to server
    send_to_server(sockfd, message);
    // receive from server
    char *response = receive_from_server(sockfd);
    puts(response);

    // free memory
    json_free_serialized_string(serialized_string);
    json_object_clear(root_object);
    free(response);
    free(message);
}

void command_login(char *buffer, int buflen, JSON_Value *root_value, JSON_Object *root_object,
            char *host, char *cookie, int sockfd) {
    // get info
    printf("username=");
    fgets(buffer, buflen, stdin);
    buffer[strlen(buffer) - 1] = 0;
    json_object_set_string(root_object, "username", buffer);
    printf("password=");
    fgets(buffer, buflen, stdin);
    buffer[strlen(buffer) - 1] = 0;

    // create json object and message
    json_object_set_string(root_object, "password", buffer);
    char *serialized_string = json_serialize_to_string_pretty(root_value);
    char *message = compute_post_request(host, ROUTE_LOGIN, NULL, serialized_string, NULL);

    // send to server
    send_to_server(sockfd, message);
    // receive from server
    char *response = receive_from_server(sockfd);
    puts(response);

    // extract cookie
    char *find_cookie = strstr(response, "connect.sid");
    if (find_cookie != NULL) {
        char *end_cookie = strchr(find_cookie, ';');
        end_cookie[0] = 0;
        strcpy(cookie, find_cookie);
    }

    // free memory
    json_free_serialized_string(serialized_string);
    json_object_clear(root_object);
    free(response);
    free(message);
}

void command_add_book(char *buffer, int buflen, JSON_Value *root_value, JSON_Object *root_object,
            char *host, int sockfd, char *token) {
    // get info and create json object
    printf("title=");
    fgets(buffer, BUFLEN, stdin);
    buffer[strlen(buffer) - 1] = 0;
    json_object_set_string(root_object, "title", buffer);
    printf("author=");
    fgets(buffer, BUFLEN, stdin);
    buffer[strlen(buffer) - 1] = 0;
    if (check_if_number(buffer) == 0) {
        printf("Please enter a valid author(not a number)!\n");
        json_object_clear(root_object);
        // exit function
        return;
    }
    json_object_set_string(root_object, "author", buffer);
    printf("genre=");
    fgets(buffer, BUFLEN, stdin);
    buffer[strlen(buffer) - 1] = 0;
    json_object_set_string(root_object, "genre", buffer);
    printf("publisher=");
    fgets(buffer, BUFLEN, stdin);
    buffer[strlen(buffer) - 1] = 0;
    json_object_set_string(root_object, "publisher", buffer);
    printf("page_count=");
    fgets(buffer, BUFLEN, stdin);
    buffer[strlen(buffer) - 1] = 0;
    if (check_if_number(buffer) != 0) {
        printf("Please enter a number for page_count!\n");
        json_object_clear(root_object);
        // exit function
        return;
    }
    if (atoi(buffer) <= 0) {
        printf("The number of pages must be positive!\n");
        json_object_clear(root_object);
        // exit function
        return;
    }
    json_object_set_number(root_object, "page_count", atoi(buffer));

    // get pretty string and message
    char *serialized_string = json_serialize_to_string_pretty(root_value);
    char *message = compute_post_request(host, ROUTE_BOOKS, NULL, serialized_string, token);

    // send to server
    send_to_server(sockfd, message);
    // receive from server
    char *response = receive_from_server(sockfd);
    printf("\n");
    puts(response);

    // free memory
    json_free_serialized_string(serialized_string);
    json_object_clear(root_object);
    free(response);
    free(message); 
}

void command_logout(char *buffer, int buflen, char *host, char *cookie, int sockfd) {
    // create message
    char *message = compute_get_request(host, ROUTE_LOGOUT, NULL, cookie, NULL);
    // send to server
    send_to_server(sockfd, message);
    // receive from server
    char *response = receive_from_server(sockfd);
    puts(response);
    // free memory
    free(response); 
    free(message);  
}

void command_enter_library(char *buffer, int buflen, char *host, char *cookie, int sockfd, char *token) {
    // create message
    char *message = compute_get_request(host, ROUTE_ACCESS, NULL, cookie, NULL);
    // send to server
    send_to_server(sockfd, message);
    // receive from server
    char *response = receive_from_server(sockfd);
    puts(response);
    // extract token
    char *new_token = strstr(response, "{\"token\":\"");
    if (new_token != NULL) {
        new_token[strlen(new_token) - 2] = 0;
        strcpy(token, "Authorization: Bearer ");
        strcat(token, new_token + strlen("{\"token\":\""));
    }

    // free memory
    free(response); 
    free(message);  
}

void command_get_books(char *buffer, int buflen, char *host, char *cookie, int sockfd, char *token) {
    // create message
    char *message = compute_get_request(host, ROUTE_BOOKS, NULL, NULL, token);
    // send to server
    send_to_server(sockfd, message);
    // receive from server
    char *response = receive_from_server(sockfd);
    puts(response);
    // parse json
    print_pretty(response);
    // free memory
    free(response); 
    free(message);  
}

void command_get_book(char *buffer, int buflen, char *host, char *cookie, int sockfd, char *token) {
    // get info
    printf("id=");
    fgets(buffer, BUFLEN, stdin);
    buffer[strlen(buffer) - 1] = 0;
    if (check_if_number(buffer) != 0) {
        printf("Please enter a number for id!\n");
        // exit function
        return;
    }
    char *route_book = (char *)calloc(50, sizeof(char));
    strcpy(route_book, ROUTE_BOOKS);
    strcat(route_book, "/");
    strcat(route_book, buffer);
    // create message
    char *message = compute_get_request(host, route_book, NULL, NULL, token);
    // send to server
    send_to_server(sockfd, message);
    // receive from server
    char *response = receive_from_server(sockfd);
    puts(response);
    // parse json
    print_pretty(response);
    // free memory
    free(response); 
    free(message);  
}

void command_delete_book(char *buffer, int buflen, char *host, char *cookie, int sockfd, char *token) {
    // get info
    printf("id=");
    fgets(buffer, BUFLEN, stdin);
    buffer[strlen(buffer) - 1] = 0;
    if (check_if_number(buffer) != 0) {
        printf("Please enter a number for id!\n");
        // exit function
        return;
    }
    char *route_book = (char *)calloc(50, sizeof(char));
    strcpy(route_book, ROUTE_BOOKS);
    strcat(route_book, "/");
    strcat(route_book, buffer);
    // create message
    char *message = compute_delete_request(host, route_book, NULL, NULL, token);
    // send to server
    send_to_server(sockfd, message);
    // receive from server
    char *response = receive_from_server(sockfd);
    puts(response);
    // free memory
    free(response);
    free(message);
}

int main(int argc, char *argv[])
{
    char* host = "ec2-3-8-116-10.eu-west-2.compute.amazonaws.com";
    char *buffer = (char *)calloc(BUFLEN, sizeof(char));
    char *cookie = (char *)calloc(BUFLEN, sizeof(char));
    char *token = (char *)calloc(BUFLEN, sizeof(char));
    int sockfd;
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    while(1) {
        // read command
        printf("Enter command:");
        fgets(buffer, BUFLEN - 1, stdin);

        // reopen connection with server, check if valid and execute command
        if (strcmp(buffer, "register\n") == 0) {
            sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
            command_register(buffer, BUFLEN, root_value, root_object, host, sockfd);
            close_connection(sockfd);
        } else if (strcmp(buffer, "login\n") == 0) {
            sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
            command_login(buffer, BUFLEN, root_value, root_object, host, cookie, sockfd);
            close_connection(sockfd);
        } else if (strcmp(buffer, "enter_library\n") == 0) {
            sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
            command_enter_library(buffer, BUFLEN, host, cookie, sockfd, token);
            close_connection(sockfd);
        } else if (strcmp(buffer, "get_books\n") == 0) {
            sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
            command_get_books(buffer, BUFLEN, host, cookie, sockfd, token);
            close_connection(sockfd);
        } else if (strcmp(buffer, "get_book\n") == 0) {
            sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
            command_get_book(buffer, BUFLEN, host, cookie, sockfd, token);
            close_connection(sockfd);
        } else if (strcmp(buffer, "add_book\n") == 0) {
            sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
            command_add_book(buffer, BUFLEN, root_value, root_object, host, sockfd, token);
            close_connection(sockfd);
        } else if (strcmp(buffer, "delete_book\n") == 0) {
            sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
            command_delete_book(buffer, BUFLEN, host, cookie, sockfd, token);
            close_connection(sockfd);
        } else if (strcmp(buffer, "logout\n") == 0) {
            sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
            command_logout(buffer, BUFLEN, host, cookie, sockfd);
            close_connection(sockfd);
        } else if (strcmp(buffer, "exit\n") == 0) {
            break;
        } else {
            printf("Please enter a valid command.\n");
        }
    }

    // free memory
    free(token);
    free(cookie);
    free(buffer);
    json_object_free(root_object);
    json_value_free(root_value);

    return 0;
}
