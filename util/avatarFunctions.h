/* ========================================================================== */
/* File: avatarFunctions.h
 *
 * Project name: CS50 Amazing Project
 * Component name: Avatar
 *
 * This file contains functions for the avatar program
 *
 */

/* ========================================================================== */

#ifndef AVATARF_H
#define AVATARF_H

// ---------------- Prerequisites e.g., Requires "math.h"
#include "common.h"

// ---------------- Constants

// ---------------- Structures/Types

// ---------------- Public Variables

// ---------------- Prototypes/Macros

/*
 * updateGrid: update the avatar's grid using known positions
 * @mazeWidth: width of the maze
 * @mazeHeight: height of the maze
 * @grid: avatar's stored maze grid
 * @prevXY: array of previous XY coordinates
 * @newXY: array of new XY coordinates
 * @numAvatars: number of avatars in the maze
 * @avatarID: id of current avatar
 * @ignoreList: list of traces to ignore
 *
 * Does not return anything; rather, upon receiving an appropriate AM_AVATAR_TURN
 * signal, updates the avatar's grid and exits. All values should already be initialized
 * prior to calling updateGrid, and it is assumed that all parameters are valid. It
 * is important to note that prevXY is an array of pointers to XYPos objects, while newXY
 * is an actual array of XYPos objects.
 * Example:
 * 	int mazeWidth, mazeHeight;
 * 	Cell *grid[MazeWidth][MazeHeight];
 * 	XYPos *prevXY[numAvatars];
 * 	XYPos newXY[numAvatars];
 * 	int avatarID;
 * 	int ignoreList[numAvatars]; 
 *
 * 	updateGrid(mazeWidth, mazeHeight, avatarGrid, prevXY, newXY, numAvatars, avatarID,
 * 	ignoreList);
 *
 */
void updateGrid(int mazeWidth, int mazeHeight, Cell *grid[mazeWidth][mazeHeight], XYPos **prevXY,
XYPos *newXY, int numAvatars, int avatarID, int *ignoreList);

/*
 * getPrevDir: determine direction in which avatar went, or in which way
 * trace points
 * @prevX: previous X coordinate
 * @prevY: previous Y coordinate
 * @newX: new X coordinate
 * @newY: new Y coordinate
 *
 * Returns an int representing the direction an avatar moved (M_NORTH, M_SOUTH, etc.),
 * or M_NULL_MOVE if direction cannot be determined/move is invalid. All parameters
 * are assumed to be ints - any values from an XYPos array should be converted
 * to integers using ntohl(). Generally used as a helper function when updating
 * the grid and determining an avatar's next move.
 * Example:
 * 	int prevX, prevY, newX, newY;
 * 	int dir;
 *
 * 	dir = getPrevDir(prevX, prevY, newX, newY);
 * 	if (dir != M_NULL_MOVE) {
 * 		// DO SOMETHING WITH DIR
 * 	}
 * 	else {
 * 		// NO MOVE OR INVALID
 * 	}
 *
 */
int getPrevDir(int prevX, int prevY, int newX, int newY);

/*
 * determineNextMove: determine the avatar's next move using knowledge of the grid
 * @mazeWidth: width of the maze
 * @mazeHeight: height of the maze
 * @grid: avatar's stored maze grid
 * @prevXY: array of previous XY coordinates
 * @newXY: array of new XY coordinates
 * @numAvatars: number of avatars in the maze
 * @avatarID: id of current avatar
 * @ignoreList: list of traces to ignore
 * @prevMove: previous valid move
 * @log: log file to write to
 *
 * Returns the avatar's next move according to our maze-solving algorithm, or
 * M_NULL_MOVE by default/upon error. Makes the same assumptions as updateGrid - that
 * is, all values are initialized, newXY is an array of XYPos objects rather than
 * pointers, etc. (see above for more info). prevMove is generally passed in as &prevmove,
 * because it is modified by the function (it stores the direction of the avatar's last
 * valid move). The function will occasionally write to the log file, indicating that an
 * avatar followed a certain trace.
 * Example:
 * 	int mazeWidth, mazeHeight;
 * 	Cell *grid[MazeWidth][MazeHeight];
 * 	XYPos *prevXY[numAvatars];
 * 	XYPos newXY[numAvatars];
 * 	int avatarID;
 * 	int ignoreList[numAvatars];
 * 	int prevMove, dir;
 * 	FILE *log;
 *
 * 	dir = determineNextMove(mazeWidth, mazeHeight, avatarGrid, prevXY, newXY,
 * 	numAvatars, avatarID, ignoreList, &prevMove, log);
 *	// SEND MOVE MESSAGE WITH DIR
 *
 */
int determineNextMove(int mazeWidth, int mazeHeight, Cell *grid[mazeWidth][mazeHeight], XYPos
**prevXY, XYPos *newXY, int numAvatars, int avatarID, int *ignoreList, int *prevMove, FILE
*log);

/*
 * cleanup: free all memory allocated for the avatar and its data structures
 * @mazeWidth: width of the maze
 * @mazeHeight: height of the maze
 * @grid: avatar's stored maze grid
 * @numAvatars: number of avatars
 * @prevXY: array of previous XY coordinates
 *
 * Frees all memory allocated for avatar data structures (all cells in grid, XYPos objects
 * in prevXY). Does not return any value, but valgrind can verify its success. Again,
 * assumes all data structures have been previously allocated!
 * Example:
 * 	// ALLOCATED GRID, PREVXY
 * 	cleanup(mazeWidth, mazeHeight, grid, numAvatars, prevXY);
 *
 */
void cleanup(int mazeWidth, int mazeHeight, Cell *grid[mazeWidth][mazeHeight], int numAvatars,
XYPos **prevXY);

/*
 * logMovement: catalog avatar's movement in log file using turn and direction
 * information
 * @direction: direction in which avatar moved
 * @newXY: array of new XY coordinates
 * @log: log file to write to
 * @avatarID: ID of current avatar
 * 
 * Records turn number and attempted avatar move in the specified log file (can be
 * stdout). As before, XYPos is an array of XYPos objects rather than an array of
 * pointers to XYPos objects. Successful execution will produce a log file
 * of the desired format for the maze solution.
 * Example:
 * 	int direction;
 * 	XYPos newXY[numAvatars];
 * 	FILE *log;
 * 	int avatarID;
 *
 * 	logMovement(direction, newXY, log, avatarID);
 *	// TURN RECORDED IN LOG
 *
 */
void logMovement(int direction, XYPos *newXY, FILE *log, int avatarID);

#endif // AVATARF_H
