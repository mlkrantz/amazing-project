/* ========================================================================== */
/* File: avatar.c
 *
 * Author: Joubertin
 * Date: 05/19/14
 *
 * Input: The avatar program is passed information from AMStartup, where
 * it originates as a separate process. Because the program is not meant
 * to be run by people, its start parameters are positional and required.
 * See the design specification for more info!
 * 	
 * Command line options: None
 *
 * Output: Appends information about moves, successes, and failures to
 * the logfile created in AMStartup.
 *
 * Error Conditions: Out of memory, unexpected or error message from 
 * the server
 *
 * Special Considerations: N/A
 *
 */
/* ========================================================================== */
// ---------------- Open Issues

// ---------------- System includes e.g., <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

// ---------------- Local includes  e.g., "file.h"
#include "amazing.h"
#include "avatarFunctions.h"
#include "common.h"

// ---------------- Constant definitions

// ---------------- Macro definitions

// ---------------- Structures/Types

// ---------------- Private variables

// ---------------- Private prototypes
void updateGrid(int mazeWidth, int mazeHeight, Cell *grid[mazeWidth]
[mazeHeight], XYPos **prevXY, XYPos *newXY, int numAvatars, int
avatarID, int *ignoreList);
int getPrevDir(int prevX, int prevY, int newX, int newY);
int determineNextMove(int mazeWidth, int mazeHeight, Cell *grid[mazeWidth]
[mazeHeight], XYPos **prevXY, XYPos *newXY, int numAvatars, int avatarID, 
int *ignoreList, int *prevMove, FILE *log);
void cleanup(int mazeWidth, int mazeHeight, Cell *grid[mazeWidth]
[mazeHeight], int numAvatars, XYPos **prevXY);
void logMovement(int direction, XYPos *newXY, FILE *log, int avatarID);

/* ========================================================================== */

int main(int argc, char *argv[]) {

	printf("%s %s %s %s %s %s %s %s\n", argv[1], argv[2], argv[3], argv[4],
	argv[5], argv[6], argv[7], argv[8]);

	// Local variables
	int avatarID = atoi(argv[1]);
	int numAvatars = atoi(argv[2]);
	char *ipAddress = argv[4];
	int mazePort = atoi(argv[5]);
	char *logfile = argv[6];
	int mazeWidth = atoi(argv[7]);
	int mazeHeight = atoi(argv[8]);
	
	// Store last valid move
	int prevMove = -1;

	// Initialize data structures
	Cell *grid[mazeWidth][mazeHeight];
	for (int i = 0; i < mazeWidth; i++) {
		for (int j = 0; j < mazeHeight; j++) {
			Cell *currCell = (Cell*) malloc(sizeof(Cell));
			currCell->avatarNum = 0;
			currCell->traceDir = -1;
			currCell->traceOrig = -1;
			grid[i][j] = currCell;
		}
	}

	// List of traces to ignore
	int ignoreList[numAvatars];
	for (int i = 0; i < numAvatars; i++) {
		// Ignore own traces
		if (i == avatarID) {
			ignoreList[i] = 1;
		}
		else {
			ignoreList[i] = 0;
		}
	}

	// Previous XY positions
	XYPos *prevXY[numAvatars];
	for (int i = 0; i < numAvatars; i++) {
		XYPos *prevPos = (XYPos*) malloc(sizeof(XYPos));
		prevPos->x = -1;
		prevPos->y = -1;
		prevXY[i] = prevPos;
	}

        // For client-server communication
	int sockfd;
	struct sockaddr_in serverAddr;

	// Connect to server
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
		if (avatarID == 0) {
                	fprintf(stderr, "Error: Couldn't create socket\n");
		}
		cleanup(mazeWidth, mazeHeight, grid, numAvatars, prevXY);
                exit(EXIT_FAILURE);
        }
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	// Put ip address into the ip address field
	bcopy(ipAddress, (char *) &serverAddr.sin_addr.s_addr, strlen(ipAddress));
	serverAddr.sin_port = htons(mazePort);

	// Connect to the server
	if (connect(sockfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
		if (avatarID == 0) {
			fprintf(stderr, "Error: Failed to connect to server\n");
		}
		cleanup(mazeWidth, mazeHeight, grid, numAvatars, prevXY);
		exit(EXIT_FAILURE);
	}
	printf("Connection established!\n");

	// Send ready message
	AM_Message readyMsg;
	readyMsg.type = htonl(AM_AVATAR_READY);
	readyMsg.avatar_ready.AvatarId = htonl(avatarID);

	if (send(sockfd, &readyMsg, sizeof(readyMsg), 0) == -1) {
		if (avatarID == 0) {
			fprintf(stderr, "Error: Failed to send message\n");
		}
		cleanup(mazeWidth, mazeHeight, grid, numAvatars, prevXY);
		close(sockfd);
		exit(EXIT_FAILURE);
        }

	// Open log file in append mode
	FILE *log = fopen(logfile, "a");
	if (log == NULL) {
		if (avatarID == 0) {
			fprintf(stderr, "Error: could not open log file\n");
		}
		cleanup(mazeWidth, mazeHeight, grid, numAvatars, prevXY);
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	// Keep track of the turn number
	if (avatarID == 0) {
		fprintf(log, "\n");
	}
	int turn = -1;

	// Maze is now running
	while (1) {
		
		// Message to receive
		AM_Message serverMessage;
        	memset(&serverMessage, 0, sizeof(serverMessage));

		// Attempt to receive message from server
		int recvSize = 0;
        	recvSize = recv(sockfd, &serverMessage, sizeof(serverMessage), 0);
        	if (recvSize < 0) {
                	if (avatarID == 0) {
				fprintf(stderr, "Error: Couldn't receive message from server\n");
			}
			cleanup(mazeWidth, mazeHeight, grid, numAvatars, prevXY);
			close(sockfd);
			fclose(log);
                	exit(EXIT_FAILURE);
        	}
        	if (recvSize == 0) {
			if (avatarID == 0) {
            			fprintf(stderr, "Error: Server connection was closed\n");
			}
			cleanup(mazeWidth, mazeHeight, grid, numAvatars, prevXY);
			close(sockfd);
			fclose(log);
            		exit(EXIT_FAILURE);
        	}

		// Check for error message
		if (IS_AM_ERROR(ntohl(serverMessage.type))) {
			if (avatarID == 0) {
                		fprintf(stderr, "Error message received from server\n");
			}
			cleanup(mazeWidth, mazeHeight, grid, numAvatars, prevXY);
			fclose(log);
			close(sockfd);
			exit(EXIT_FAILURE);
        	}

		// Turn message
		if (ntohl(serverMessage.type) == AM_AVATAR_TURN) {

			// Increment turn number
			turn++;
			
			// If current avatar's turn
			if (ntohl(serverMessage.avatar_turn.TurnId) == avatarID) {
			
				// Print turn number
				fprintf(log, "***** Turn %d *****\n", turn);
	
				// Update grid
				XYPos *newXY = serverMessage.avatar_turn.Pos;
				updateGrid(mazeWidth, mazeHeight, grid, prevXY, newXY, numAvatars,
				avatarID, ignoreList);

				// Determine next move
				int direction = determineNextMove(mazeWidth, mazeHeight, grid,
				prevXY, newXY, numAvatars, avatarID, ignoreList, &prevMove, 
				log);
				
				// Log progress
				logMovement(direction, newXY, log, avatarID);
				fflush(log);

				// Send move message
				AM_Message moveMsg;
        			moveMsg.type = htonl(AM_AVATAR_MOVE);
       				moveMsg.avatar_move.AvatarId = htonl(avatarID);
				moveMsg.avatar_move.Direction = htonl(direction);

        			if (send(sockfd, &moveMsg, sizeof(moveMsg), 0) == -1) {
                			fprintf(stderr, "Error: Failed to send message\n");
					cleanup(mazeWidth, mazeHeight, grid, numAvatars, prevXY);
					close(sockfd);
					fclose(log);
                			exit(EXIT_FAILURE);
        			}

				// Update arrays
				for (int i = 0; i < numAvatars; i++) {
					prevXY[i]->x = ntohl(newXY[i].x);
					prevXY[i]->y = ntohl(newXY[i].y);
				}
			}
		}

		// Success message
		if (ntohl(serverMessage.type) == AM_MAZE_SOLVED) {
			if (avatarID == 0) {
				fprintf(log, "Success! Solved maze of difficulty %d, with %d avatars, in %d "
				"moves.\nThe hash returned by AM_MAZE_SOLVED is %u.\nLog file complete!", ntohl
				(serverMessage.maze_solved.Difficulty), ntohl(serverMessage.maze_solved.nAvatars), 
				ntohl(serverMessage.maze_solved.nMoves), ntohl(serverMessage.maze_solved.Hash));
			}
			break;
		}

	}

	// Close the connection
	close(sockfd);
	// Clean up
	cleanup(mazeWidth, mazeHeight, grid, numAvatars, prevXY);
	fclose(log);
	exit(EXIT_SUCCESS);

}
