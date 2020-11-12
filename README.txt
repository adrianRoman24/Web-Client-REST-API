	General description:

    The application consists of the implementation of a client type - server application.
    Communication between the two is done through HTTP routes. The server can receive different
commands and the purpose of the client is to take these commands from the user, to put the entered data
in a server-recognized format and send them to the server.

    Program flow

    Allocate memory for buffer, cookie, token, and a
Json object. Then, the programs enters a loop in which commands are read
from stdin in buffer. From this loop, the program exits when reading the exit command.
An error message is displayed when an invalid command is read, and the program
continues with the next operation. When you enter a valid command, it reopens
connection to the server (because I noticed that this is a way to
ensures that we are connected to the server when we give an order), the function is called
corresponding to that command and then closes the connection to the server.
When it exits the loop, the used memory is de-allocated.

    Description of an order

    All functions used for various commands respect the same skeleton and
the same principle. For example, the register command prompt for the username
and for the password. Once they are read in turn in the buffer, they are added
in a new json object. Then the "pretty" string of this object is obtained and,
using the compute_post_request function in lab 10 with the corresponding route
registering a new account on the server and with the string "pretty" previously created as
body_data, the message to be sent to the server is obtained. Then it is received
the response from the server and it is displayed without any modification. Prior to
reopens the connection to the server at each command happens as this response
be empty, but now it seems to be working properly. This message is also displayed when
the message is a successful one and when it is an error one.
    
    Some features are the following commands:
    At login the retained cookie is retained (this being searched for "connect.sid" in
server response) to prove that we are authenticated when they are given
commands that require this.
    The enter_library command looks for the token in the server response by string
"{\" token \ ": \" ", this is extracted and put in the required form: Authorization: Bearer
<Token>. It is used to prove that we have access to the library
put in the message sent to the server through the compute_get_post function in case of a command
get_book, add_book or get_books.

    Parson json and display response from the server
    
    I used the parson library to build the required json objects as well as
to send them to the server in a "pretty" format. To find and to
process a token or a cookie received from the server I searched manually in the answer
received message after the keywords mentioned above. After entering an order,
the response from the server is displayed unchanged because it contains matching messages
both for success and for error situations. For get_book and get_books commands,
in case of success, the json objects required in the statement are also displayed, also parsed
by using parson library.
