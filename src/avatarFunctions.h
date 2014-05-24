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
 * Update the avatar's grid using known positions
 *
 */
void updateGrid(int mazeWidth, int mazeHeight, Cell *grid[mazeWidth][mazeHeight], XYPos **prevXY,
XYPos *newXY, int numAvatars, int avatarID, int *ignoreList);

/*
 * Determine direction in which avatar went (which way
 * trace points)
 *
 */
int getPrevDir(int prevX, int prevY, int newX, int newY);

/*
 * Determine the avatar's next move using knowledge of the
 * grid!
 *
 */
int determineNextMove(int mazeWidth, int mazeHeight, Cell *grid[mazeWidth][mazeHeight], XYPos
**prevXY, XYPos *newXY, int numAvatars, int avatarID, int *ignoreList, int *prevMove, FILE
*log);

/*
 * Free all memory allocated for the avatar and its
 * data structures
 *
 */
void cleanup(int mazeWidth, int mazeHeight, Cell *grid[mazeWidth][mazeHeight], int numAvatars,
XYPos **prevXY);

/*
 * Catalog avatar's movement in log file using turn and direction
 * information
 *
 */
void logMovement(int direction, XYPos *newXY, FILE *log, int avatarID);

#endif // AVATARF_H
