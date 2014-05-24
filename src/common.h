/* ========================================================================== */
/* File: common.h
 *
 * Project name: CS50 Amazing Project
 * Component name: Avatar
 *
 * This file contains common defines and data structures
 *
 */

/* ========================================================================== */

#ifndef COMMON_H
#define COMMON_H

// ---------------- Prerequisites e.g., Requires "math.h"

// ---------------- Constants
#define CELL_SIZE 11 // side length of cells, 10 for actual cell, 1 for border

// ---------------- Structures/Types

typedef struct Cell {
    int avatarNum;                           // how many avatars
    int traceDir;                            // direction trace points
    int traceOrig;                           // who left trace
} Cell;

typedef struct Grid {
    Cell ***cellArray;                       // multidimensional array 
                                             // of cells
} Grid;

// ---------------- Public Variables

// ---------------- Prototypes/Macros

#endif // COMMON_H
