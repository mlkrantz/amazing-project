/* ========================================================================== */
/* File: avatarFunctions.c
 *  
 * Project name: Amazing Project
 * Component name: Avatar
 * 
 * Author: Joubertin
 * Date: 05/23/2014
 * 
 * Functionality: Contains the functions used by the avatar program as
 * part of the amazing project. See avatarFunctions.h for more 
 * information on specific functions and their uses
 *
 * Error Conditions: Specific to each function
 *
 * Special Notes: Only meant to be used for avatar!
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
