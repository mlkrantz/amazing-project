/* ========================================================================== */
/* File: AMStartup.c
 *
 * Author:
 * Date: 05/19/14
 *
 * Input: None.
 * 	
 *
 * Command line options:
 *	
 * 		-n 	nAvatars  	-	the number of Avatars in the maze, minimum 1 and maximum AM_MAX_AVATAR (default = 10)
 *		-d 	Difficulty 	- 	the difficulty level, on a scale of 0 (easy) to AM_MAX_DIFFICULTY (default = 9 (extremely difficult) )
 *		-h 	Hostname	- 	the hostname of the server (i.e. pierce.cs.dartmouth.edu)
 *		--help 			-	for usage information
 *
 * Output:
 *
 * Error Conditions:
 *
 * Special Considerations:
 *
 */
/* ========================================================================== */
// ---------------- Open Issues
// None.

// ---------------- System includes e.g., <stdio.h>
#include <stdio.h>                           
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>  // bcopy()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h>
#include <ctype.h>

// ---------------- Local includes  e.g., "file.h"
#include "amazing.h"

// ---------------- Constant definitions
static const char USAGE[] = "Usage: ./AMStartup -n nAvatars -d Difficulty -h Hostname [--help]";

// ---------------- Macro definitions
#define VERBOSE 0 		// for debugging

// ---------------- Structures/Types
struct option longOpts[] = {
    {"help", no_argument, 0, 'x'},
    {0, 0, 0, 0}
};

// ---------------- Private variables

// ---------------- Private prototypes
int checkArgs(int argc, char givenDifficulty[], char givenNumAvatars[]);
int isNumerical(char inputToCheck[]);
void userHelp();

/* ========================================================================== */

int main(int argc, char *argv[]) {
	int help = 0;
	int numAvatars;		// 1-AM_MAX_AVATAR
	int difficulty;		// 0-AM_MAX_DIFFICULTY
	char *hostname;
	char *givenNumAvatars = NULL;
	char *givenDifficulty = NULL;
	
	// for command line option processing
	int ch;	
	extern char *optarg;	// parameter of an option
	extern int optind;		// current index of argument array
	
	// process options
	while (( ch = getopt_long(argc, argv, "n:d:h:x", longOpts, 0)) != -1) {
		switch(ch){
			case 'n':
				givenNumAvatars = optarg;			
				break;

			case 'd':
				givenDifficulty = optarg;
				break;
			
			case 'h':
				hostname = optarg;
				break;
			case 'x':
				help = 1;
				break;

			// unknown options
			default:
				fprintf(stderr, "%s\n", USAGE);
				exit(EXIT_FAILURE);
				break;
		}
	}

	if (help) {
		userHelp();
		userHelp();
		exit(EXIT_SUCCESS);
	}
	if (!checkArgs(argc, givenDifficulty, givenNumAvatars)) {
		exit(EXIT_FAILURE);
	}

	// convert valid inputs to int
	difficulty = atoi(givenDifficulty);
	numAvatars = atoi(givenNumAvatars);

    // for client-server communication
	int sockfd;							// socket file descriptor
	struct sockaddr_in serverAddr;
	struct hostent *server;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0){
		fprintf(stderr, "Error: Couldn't create socket.\n");
		exit(EXIT_FAILURE);
	}

	// get info about server
	server = gethostbyname(hostname);
	if (server == NULL){
		fprintf(stderr, "Error: Host, %s, is invalid.\n", hostname);
		exit(EXIT_FAILURE);
	}

	// initialize server address
	bzero((char *) &serverAddr, sizeof(serverAddr));
	// memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	// put ip address into the ip address field
	bcopy((char *) server->h_addr, (char *) &serverAddr.sin_addr.s_addr, server->h_length);     
	serverAddr.sin_port = htons(atoi(AM_SERVER_PORT));

    // Connect to the server
	if (connect(sockfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0){
		fprintf(stderr, "Error: Failed to connect to server.\n");
		exit(EXIT_FAILURE);
	}
	printf("Connection established!\n");

    // create message send
	AM_Message *initMsg = calloc(1, sizeof(AM_Message));
	initMsg->type = htonl(AM_INIT);
	initMsg->init.nAvatars = htonl(numAvatars);
	initMsg->init.Difficulty = htonl(difficulty);

	// try to send AM_INIT message to server
	printf("Sending AM_INIT message to server...\n");
	if (send(sockfd, initMsg, sizeof(AM_Message), 0) == -1){
	    fprintf(stderr, "Error: Failed to send message\n");
	    exit(EXIT_FAILURE);
	}

	// create message to receive
	AM_Message serverMessage;
	memset(&serverMessage, 0, sizeof(serverMessage));
	
	// try to receive AM_INIT_OK message
	int recvSize = 0;
	recvSize = recv(sockfd, &serverMessage, sizeof(serverMessage), 0);
	if ( recvSize < 0){
		fprintf(stderr, "Error: Couldn't receive message from server.\n");
		free(initMsg);
		exit(EXIT_FAILURE);
	}
	if(recvSize == 0){
	    fprintf(stderr, "Error: Server connection was closed.\n");
	    free(initMsg);
	    exit(EXIT_FAILURE);
	} 
	
	// now the connection can be closed
	close(sockfd);
	free(initMsg);

	if (IS_AM_ERROR(serverMessage.type)){
		fprintf(stderr, "Error message received from server\n");
	}
	else{
	    printf("AM_INIT_OK message from server successfully received\n");
	}

    // create the log file
    char *userName = getenv("USER");
    char *logFileName = NULL;
    logFileName = (char *)malloc((strlen("Amazing___.log")*sizeof(char) + strlen(userName))*sizeof(char) + sizeof(int)*2 + 1);
    if (!logFileName) {
    	fprintf(stderr, "Error: Couldn't allocate memory for logFileName.\n");
    	exit(EXIT_FAILURE);
    }
    sprintf(logFileName, "Amazing_%s_%d_%d.log", userName, numAvatars, difficulty);
    
    FILE *logFile;
    if ((logFile = fopen(logFileName, "a"))== NULL){
        fprintf(stderr, "Error: Couldn't create log file, %s.\n", logFileName);
        free(logFileName);
        exit(EXIT_FAILURE);
    }
    
    // get time info
    time_t rawCurrTime;
    struct tm *timeinfo;

    time (&rawCurrTime);
    timeinfo = localtime(&rawCurrTime);
    fprintf(logFile, "%s, %d, %s", userName, ntohl(serverMessage.init_ok.MazePort), asctime(timeinfo));

    // make child processes
    pid_t childPID;
    char *childArgs[9];
    for (int i = 1; i <= numAvatars; i++) {
    	childPID = fork();
    	if (childPID >= 0) {
			char avatarID[2];
    		sprintf(avatarID, "%d", i);
    		char IPaddr[15];
    		short convertedIP = ntohs(serverAddr.sin_addr.s_addr)
    		sprintf(IPaddr, "%d", convertedIP);
    		char mazePort[10];
    		short convertedMP = ntohs(serverMessage.init_ok.MazeWidth);
    		sprintf(mazePort, "%d", convertedMP);
    		char mazeWidth[10];
    		short convertedMW = ntohs(serverMessage.init_ok.MazeWidth);
    		sprintf(mazeWidth, "%d", convertedMW);
    		char mazeHeight[10];
    		short convertedMH = ntohs(serverMessage.init_ok.MazeHeight);
    		sprintf(mazeHeight, "%d", convertedMH);
    		fprintf(stderr, "Hello I'm a child and my PID is %d\n", getpid());
    		childArgs[0] = "./avatar";
    		childArgs[1] = avatarID;
    		childArgs[2] = givenNumAvatars;
    		childArgs[3] = givenDifficulty;
    		childArgs[4] = IPaddr;
    		childArgs[5] = mazePort;
    		childArgs[6] = logFileName;
    		childArgs[7] = mazeWidth;
    		childArgs[8] = mazeHeight;
    		childArgs[9] = NULL;

    		if (childPID == 0) {
	    		// inside child process
	    		execve("./avatar", childArgs, NULL);
	    		exit(EXIT_SUCCESS);
    		}
    		else {
    			// inside parent process
    		}
    	}
    	else {
    		fprintf(stderr, "Error: Failed to fork.\n");
    		break;
    	}
    }


    fclose(logFile);
    printf("Log file created as: %s\n", logFileName);
    free(logFileName);
	return 0;
}


/*
 * Check that arguments are valid. If successful return 1, else 0.
 */
int checkArgs(int argc, char givenDifficulty[], char givenNumAvatars[]) {
	if (argc != 7) {
		fprintf(stderr, "%s\n", USAGE);
		return 0;
	}
	if (!givenDifficulty || !givenNumAvatars) {
		fprintf(stderr, "%s\n", USAGE);
		return 0;
	}
	if (!isNumerical(givenDifficulty)) {
		fprintf(stderr, "Error: Difficulty must be an integer from 0 to %d\n", AM_MAX_DIFFICULTY);
		return 0;
	}
	if (!isNumerical(givenNumAvatars)) {
		fprintf(stderr, "Error: Number of avatars must be an integer from 1 to %d\n", AM_MAX_AVATAR);
		return 0;
	}

	// convert to int for comparison
	int difficulty = atoi(givenDifficulty);
	int numAvatars = atoi(givenNumAvatars);


	if (difficulty < 0 || difficulty > AM_MAX_DIFFICULTY) {
		fprintf(stderr, "Error: Difficulty must be an integer from 0 to %d\n", AM_MAX_DIFFICULTY);
		return 0;
	}
	if (numAvatars > AM_MAX_AVATAR) {
		fprintf(stderr, "Error: Number of avatars must be an integer from 1 to %d\n", AM_MAX_AVATAR);
		return 0;
	}
	return 1;
}


/*
 * Checks that a user input is numerical. Returns 1 if numerical, else 0.
 */
int isNumerical(char inputToCheck[]) {
    for (int i = 0; i < strlen(inputToCheck); i++) {
        if (isdigit(inputToCheck[i]) == 0) {
            return 0;
        }
    }
    return 1;
}


/*
 * Prints help information, including version and usage.
 */
void userHelp() {
	printf("The Amazing Project\n");
	printf("Component: AMStartup\n");
	printf("Version 1.0\n");
	printf("%s\n", USAGE);
}

