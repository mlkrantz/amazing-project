/* ========================================================================== */
/* File: testing.c
 *  
 * Project name: Amazing Project
 * Component name: Testing
 * 
 * Author: Joubertin
 * Date: 05/23/2014
 * 
 * Functionality: Tests basic functionality and boundary cases for
 * avatar and its functions - serves as unit testing for the Amazing
 * Project (specifically the avatar module)
 *
 * Error Conditions: System is out of memory (cannot malloc/calloc)
 *
 * Special Notes: Not meant to be used for anything other than testing
 * purposes (in terms of taking args, etc.)!
 *
 */

/* ========================================================================== */

// ---------------- Open Issues

// ---------------- System includes e.g., <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

// ---------------- Local includes  e.g., "file.h"
#include "amazing.h"
#include "avatarFunctions.h"
#include "common.h"

// ---------------- Constant definitions

// ---------------- Macro definitions

// ---------------- Structures/Types

// ---------------- Private variables

// ---------------- Private prototypes

/* ========================================================================== */

int main(int argc, char* argv[]) {

	// Local variables
	int mazeWidth = 3;
	int mazeHeight = 3;
	int numAvatars = 2;
	XYPos currXY[2];
	int prevMove = -1;
	
	// Avatar positions
	XYPos firstAvatar = { .x = htonl(0), .y = htonl(1) };
	XYPos secondAvatar = { .x = htonl(1), .y = htonl(0) };
	currXY[0] = firstAvatar; 
	currXY[1] = secondAvatar;
	
	// Initialize data structures
	printf("Initializing the grid...\n");
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
	printf("Grid initialized! Width is %d, height is %d\n", mazeWidth,
	mazeHeight);

	// List of traces to ignore
        int ignoreList[numAvatars];
        for (int i = 0; i < numAvatars; i++) {
                // Ignore all
		ignoreList[i] = 1;
	}

        // Previous XY positions
	printf("Initializing prev XY array...\n");
        XYPos *prevXY[numAvatars];
        for (int i = 0; i < numAvatars; i++) {
                XYPos *prevPos = (XYPos*) malloc(sizeof(XYPos));
                prevPos->x = 0;
                prevPos->y = 0;
                prevXY[i] = prevPos;
        }
	printf("Array initialized with %d avatars!\n", numAvatars);

	// Test getPrevDir
	printf("\nTesting getPrevDir function...\nAvatar 0 was on "
	"cell (%d, %d), now on cell (%d, %d)\n", prevXY[0]->x, prevXY[0]->y,
	ntohl(currXY[0].x), ntohl(currXY[0].y));
	int dir = getPrevDir(prevXY[0]->x, prevXY[0]->y, ntohl(currXY[0].x),
	ntohl(currXY[0].y));
	printf("Expected dir to be M_SOUTH (2), is %d\n", dir);
	if (dir == M_SOUTH) {
		printf("Test passed!\n");
	}
	else {
		fprintf(stderr, "Test did not pass! Exiting...\n");
		cleanup(mazeWidth, mazeHeight, grid, numAvatars, prevXY);
		exit(EXIT_FAILURE);
	}

	// Test logMovement
	printf("\nTesting logMovement function...\n");
	logMovement(dir, currXY, stdout, 0);
	printf("Pretending avatar 1 did not move...\n");
	logMovement(M_NULL_MOVE, currXY, stdout, 1);
	printf("Test passed!\n");	

	// Test updateGrid
	printf("\nTesting updateGrid function...\nReinitializing prev XY array...\n");
	for (int i = 0; i < numAvatars; i++) {
                prevXY[i]->x = -1;
                prevXY[i]->y = -1;
        }
	// Update grid
	printf("Attempting to update avatar 1's grid\n\n");
	updateGrid(mazeWidth, mazeHeight, grid, prevXY, currXY, 
	numAvatars, 1, ignoreList);
	for (int i = 0; i < mazeWidth; i++) {
		for (int j = 0; j < mazeWidth; j++) {
			printf("Cell (%d, %d) contains %d avatar(s)", i, j, 
			grid[i][j]->avatarNum);
			if (grid[i][j]->traceOrig != -1) {
				printf(" and a trace left by avatar %d in "
				"direction %d\n", grid[i][j]->traceOrig,
				grid[i][j]->traceDir);
			}
			else {
				printf(" and no trace\n");
			}
		}
	}
	// Test traces
	printf("\nTesting trace functionality of updateGrid..."
	"\nMoving avatar 1 from (1, 0) to (2, 0)...\n");
	prevXY[1]->x = 1; prevXY[1]->y = 0;
	currXY[1].x = htonl(2), currXY[1].y = htonl(0);
	printf("Moving avatar 0 from (0, 1) to (0, 2)...\n"
	"Attempting to update avatar 1's grid\n\n");
	prevXY[0]->x = 0; prevXY[0]->y = 1;
        currXY[0].x = htonl(0), currXY[0].y = htonl(2);
	// Update grid
	updateGrid(mazeWidth, mazeHeight, grid, prevXY, currXY,
        numAvatars, 1, ignoreList);
        for (int i = 0; i < mazeWidth; i++) {
                for (int j = 0; j < mazeWidth; j++) {
                        printf("Cell (%d, %d) contains %d avatar(s)", i, j,
                        grid[i][j]->avatarNum);
                        if (grid[i][j]->traceOrig != -1) {
                                printf(" and a trace left by avatar %d in "
                                "direction %d\n", grid[i][j]->traceOrig,
                                grid[i][j]->traceDir);
                        }
                        else {
                                printf(" and no trace\n");
                        }
                }
        }
	printf("Test passed!\n");

	// Test determineNextMove
	printf("\nTesting determineNextMove function...\n");
	printf("Determining the next move of avatar 0, at position (%d, %d)...\n",
	ntohl(currXY[0].x), ntohl(currXY[0].y));
	prevXY[0]->x = -1; prevXY[0]->y = -1;
	dir = determineNextMove(mazeWidth, mazeHeight, grid, prevXY, currXY,
	numAvatars, 0, ignoreList, &prevMove, stdout);
	// First move should be north
	printf("Expected dir to be M_NORTH (1), is %d\n", dir);
        if (dir != M_NORTH) {
                fprintf(stderr, "Test did not pass! Exiting...\n");
                cleanup(mazeWidth, mazeHeight, grid, numAvatars, prevXY);
                exit(EXIT_FAILURE);
        }
	else {
		printf("Success!\n");
	}
	prevXY[0]->x = 0; prevXY[0]->y = 2;
	currXY[0].x = htonl(0), currXY[0].y = htonl(1);

	// Turn left
	printf("Avatar 0 now at position (%d, %d)\nAttempting to move again,"
	" should turn left...\n", ntohl(currXY[0].x), ntohl(currXY[0].y));
	dir = determineNextMove(mazeWidth, mazeHeight, grid, prevXY, currXY,
        numAvatars, 0, ignoreList, &prevMove, stdout);
        printf("Expected dir to be M_WEST (0), is %d\n", dir);
	if (dir != M_WEST) {
                fprintf(stderr, "Test did not pass! Exiting...\n");
                cleanup(mazeWidth, mazeHeight, grid, numAvatars, prevXY);
                exit(EXIT_FAILURE);
        }
	else {
		printf("Success!\n");
	}

	// Turn right
	printf("Pretending avatar 0 stood still, should turn right...\n");
	currXY[0].x = htonl(0), currXY[0].y = htonl(2);
	dir = determineNextMove(mazeWidth, mazeHeight, grid, prevXY, currXY,
        numAvatars, 0, ignoreList, &prevMove, stdout);
        printf("Expected dir to be M_NORTH (1), is %d\n", dir);
        if (dir != M_NORTH) {
                fprintf(stderr, "Test did not pass! Exiting...\n");
                cleanup(mazeWidth, mazeHeight, grid, numAvatars, prevXY);
                exit(EXIT_FAILURE);
        }
        else {
                printf("Success!\n");
        }
	prevXY[0]->x = 0; prevXY[0]->y = 2;
        currXY[0].x = htonl(0), currXY[0].y = htonl(1);

	// Follow trace
	for (int i = 0; i < numAvatars; i++) {
                // Don't ignore
		ignoreList[i] = 0;
	}
	printf("Having avatar 0 follow its own trace...\n");
	dir = determineNextMove(mazeWidth, mazeHeight, grid, prevXY, currXY,
        numAvatars, 0, ignoreList, &prevMove, stdout);
        printf("Expected dir to be M_SOUTH (2), is %d\n", dir);
        if (dir != M_SOUTH) {
                fprintf(stderr, "Test did not pass! Exiting...\n");
                cleanup(mazeWidth, mazeHeight, grid, numAvatars, prevXY);
                exit(EXIT_FAILURE);
        }
        else {
                printf("Success!\n");
        }
        printf("Test passed!\n");

	// Free memory
	printf("\nCleaning up and freeing memory...\n");
	cleanup(mazeWidth, mazeHeight, grid, numAvatars, prevXY);
	printf("All memory was freed!\n");

}
