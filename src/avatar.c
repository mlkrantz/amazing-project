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
				"moves.\nThe hash returned by AM_MAZE_SOLVED is %d.\nLog file complete!", ntohl
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

/*
 * Update the avatar's grid using known positions
 *
 */
void updateGrid(int mazeWidth, int mazeHeight, Cell *grid[mazeWidth][mazeHeight], XYPos **prevXY, 
XYPos *newXY, int numAvatars, int avatarID, int *ignoreList) {

	// Loop through all avatars
	for (int i = 0; i < numAvatars; i++) {
		// Get values
		int prevX = prevXY[i]->x;
		int prevY = prevXY[i]->y;
		int newX = ntohl(newXY[i].x);
		int newY = ntohl(newXY[i].y);
		// If the avatar moved
		if (prevX != newX || prevY != newY) {
			// Adjust counts
			grid[newX][newY]->avatarNum++;
			// If not the first move
			if (prevX != -1 && prevY != -1) {
				grid[prevX][prevY]->avatarNum--;
				// Leave a trace
				grid[prevX][prevY]->traceOrig = i;
				grid[prevX][prevY]->traceDir = getPrevDir(prevX, 
				prevY, newX, newY);
			}
		}
	}
	// Update ignore list
	int currX = ntohl(newXY[avatarID].x);
	int currY = ntohl(newXY[avatarID].y);
	if (grid[currX][currY]->avatarNum > 1) {
		for (int i = 0; i < numAvatars; i++) {
			// Ignore those of higher number
			if (i > avatarID && ntohl(newXY[i].x) == currX && ntohl(newXY
			[i].y) == currY) {
				ignoreList[i] = 1;
			}
		}	
	} 	

}

/*
 * Determine direction in which avatar went (which way
 * trace points)
 *
 */
int getPrevDir(int prevX, int prevY, int newX, int newY) {

	// West
	if (newX < prevX && newY == prevY) {
		return M_WEST;
	}
	// North
	if (newY < prevY && newX == prevX) {
		return M_NORTH;
	}
	// South
	if (newY > prevY && newX == prevX) {
		return M_SOUTH;
	}
	// East
	if (newX > prevX && newY == prevY) {
		return M_EAST;
	}
	
	// No movement
	return M_NULL_MOVE;

}

/*
 * Determine the avatar's next move using knowledge of the
 * grid!
 *
 */
int determineNextMove(int mazeWidth, int mazeHeight, Cell *grid[mazeWidth][mazeHeight], XYPos 
**prevXY, XYPos *newXY, int numAvatars, int avatarID, int *ignoreList, int *prevMove, FILE 
*log) {

	// Get current position
	int currX = ntohl(newXY[avatarID].x);
        int currY = ntohl(newXY[avatarID].y);
	Cell *currCell = grid[currX][currY];

	// Check for others on cell
	if (currCell->avatarNum > 1) {
		for (int i = 0; i < numAvatars; i++) {
			// Only move if lowest ID
			if (ntohl(newXY[i].x) == currX && ntohl(newXY[i].y) == 
			currY && i < avatarID) {
				return M_NULL_MOVE;
			}
		}		
	}

	// Check for traces	
	if (currCell->traceOrig != -1) {
		for (int i = 0; i < numAvatars; i++) {
			// Follow if not on ignore list
			if (currCell->traceOrig == i && !ignoreList[i]) {
				fprintf(log, "Avatar %d followed a trace left by avatar %d\n",
				avatarID, currCell->traceOrig);
				*prevMove = currCell->traceDir;
				return currCell->traceDir;
			}
		}
	}

	// Other cases
	int prevX = prevXY[avatarID]->x;
	int prevY = prevXY[avatarID]->y;

	// First move is always north
	if (prevX == -1 && prevY == -1 && *prevMove == -1) {
		*prevMove = M_NORTH;
		return M_NORTH;
	}

	// Wall follower algorithm
	if (prevX != currX || prevY != currY) {
		int prevDir = getPrevDir(prevX, prevY, currX, currY);
		// Turn left
		switch (prevDir) {
		    case M_NORTH:
			*prevMove = M_WEST;
			return M_WEST;
		    case M_EAST:
			*prevMove = M_NORTH;
			return M_NORTH;
		    case M_SOUTH:
			*prevMove = M_EAST;
			return M_EAST;
		    case M_WEST:
			*prevMove = M_SOUTH;
			return M_SOUTH;
		}
	}
	else {
		// Turn right
		switch (*prevMove) {
		    case M_NORTH:
			*prevMove = M_EAST;
			return M_EAST;
		    case M_EAST:
			*prevMove = M_SOUTH;
			return M_SOUTH;
		    case M_SOUTH:
			*prevMove = M_WEST;
			return M_WEST;
		    case M_WEST:
			*prevMove = M_NORTH;
			return M_NORTH;
		}
	}

	// Shouldn't get here
	return M_NULL_MOVE;

}

void cleanup(int mazeWidth, int mazeHeight, Cell *grid[mazeWidth][mazeHeight], int numAvatars, 
XYPos **prevXY) {

	// Clean up grid
	for (int i = 0; i < mazeWidth; i++) {
                for (int j = 0; j < mazeHeight; j++) {
                        // Free all cells
                        Cell *currCell = grid[i][j];
			free(currCell);
                }
        }
	// Clean up position array
	for (int i = 0; i < numAvatars; i++) {
		XYPos *prevPos = prevXY[i];
		free(prevPos);
	}	

}

/*
 * Catalog avatar's movement in log file using turn and direction
 * information
 *
 */
void logMovement(int direction, XYPos *newXY, FILE *log, int avatarID) {

	// ID and position
	fprintf(log, "Avatar %d, at position (%d, %d), ", avatarID, ntohl
	(newXY[avatarID].x), ntohl(newXY[avatarID].y));

	// Direction of movement
	if (direction != M_NULL_MOVE) {
		char *directionName = NULL;
		switch (direction) {
		    case M_NORTH:
			directionName = "North";
			break;
		    case M_SOUTH:
			directionName = "South";
			break;
		    case M_EAST:
			directionName = "East";
			break;
		    case M_WEST:
			directionName = "West";
			break;
		    default:
			fprintf(log, "made an invalid move!\n\n");
			return;
		}
		fprintf(log, "attempted to move %s\n\n", directionName);
	}
	// No movement
	else {
		fprintf(log, "stood still\n\n");
	}
	
}
