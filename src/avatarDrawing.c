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
#include <gtk/gtk.h>

// ---------------- Local includes  e.g., "file.h"
#include "amazing.h"
#include "common.h"

// ---------------- Constant definitions

// ---------------- Macro definitions

// ---------------- Structures/Types

// ---------------- Private variables
//create pixmap to do actual drawing on
static GdkPixmap *PixMap; //map to do drawning
static GdkFont *fixed_font; //font for drawing text

//initialize gtk window and gtkdrawingarea widget for visualization

static GtkWidget *canvas;
// ---------------- Private prototypes
void updateGrid(int mazeWidth, int mazeHeight, Cell *grid[mazeWidth][mazeHeight],
		XYPos **prevXY, XYPos *newXY, int numAvatars, int avatarID,
		int *ignoreList);
int getPrevDir(int prevX, int prevY, int newX, int newY);
int determineNextMove(int mazeWidth, int mazeHeight, Cell *grid[mazeWidth]
[mazeHeight], XYPos **prevXY, XYPos *newXY, int numAvatars, int avatarID, 
int *ignoreList, int *prevMove);
void cleanup(int mazeWidth, int mazeHeight, Cell *grid[mazeWidth]
[mazeHeight], int numAvatars, XYPos **prevXY);
static gint configure_event(GtkWidget *widget, GdkEventConfigure *event);
static gint expose_event (GtkWidget *widget, GdkEventExpose *event);

/* ========================================================================== */

int main(int argc, char *argv[]) {

  printf("%s %s %s %s %s %s %s %s\n", argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],argv[7],argv[8]);
  
  // Local variables
  int avatarID = atoi(argv[1]);
  int numAvatars = atoi(argv[2]);
  int difficulty = atoi(argv[3]);
  char *ipAddress = argv[4];
  int mazePort = atoi(argv[5]);
  char *logfile = argv[6];
  int mazeWidth = atoi(argv[7]);
  int mazeHeight = atoi(argv[8]);
  char* pro="12582943";
  GdkNativeWindow origWindow = strtoul(pro, NULL, 10);
  	
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
  
  if(avatarID==0) //set avatar 0 as drawing avatar
    {
      fprintf(stderr,"line 1\n");
      gtk_init(&argc,&argv);
      fprintf(stderr,"yo, there's still hope!");
      //initialize drawingArea widget and add it to the original socket
      canvas=gtk_drawing_area_new();
      GtkWidget* plug=gtk_plug_new(origWindow);

      //add canvas to window
      gtk_container_add(GTK_CONTAINER(plug),canvas);

      //connect the window to necessary signals
      //connect configure_event signal to canvas
      g_signal_connect(canvas, "configure-event",G_CALLBACK(configure_event),NULL);
      //connect destroy signal to canvas
      g_signal_connect_swapped(plug, "destroy", G_CALLBACK(gtk_main_quit),G_OBJECT(plug));
      //connect expose event signal to canvas
      g_signal_connect(canvas,"expose-event",G_CALLBACK(expose_event),NULL);
      //resize canvas
      //      gtk_drawing_area_size(canvas, mazeWidth*CELL_SIZE, mazeHeight*CELL_SIZE);

      //load font for draw_text
      fixed_font=gdk_font_load("-adobe-new century schoolbook-bold-i-normal-*-8-*-*-*-p-80-iso8859-1");

      //show everything on screen
      gtk_widget_show_all(plug);
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
    fprintf(stderr, "Error: Couldn't create socket\n");
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
    fprintf(stderr, "Error: Failed to connect to server\n");
    cleanup(mazeWidth, mazeHeight, grid, numAvatars, prevXY);
    exit(EXIT_FAILURE);
  }
  printf("Connection established!\n");

  // Send ready message
  AM_Message readyMsg;
  readyMsg.type = htonl(AM_AVATAR_READY);
  readyMsg.avatar_ready.AvatarId = htonl(avatarID);

  if (send(sockfd, &readyMsg, sizeof(readyMsg), 0) == -1) {
    fprintf(stderr, "Error: Failed to send message\n");
    cleanup(mazeWidth, mazeHeight, grid, numAvatars, prevXY);
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  // Open log file in append mode
  FILE *log = fopen(logfile, "a");
  if (log == NULL) {
    fprintf(stderr, "Error: could not open log file\n");
    cleanup(mazeWidth, mazeHeight, grid, numAvatars, prevXY);
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  // Maze is now running
  while (1) {
		
    // Message to receive
    AM_Message serverMessage;
    memset(&serverMessage, 0, sizeof(serverMessage));

    // Attempt to receive message from server
    int recvSize = 0;
    recvSize = recv(sockfd, &serverMessage, sizeof(serverMessage), 0);
    if (recvSize < 0) {
      fprintf(stderr, "Error: Couldn't receive message from server\n");
      cleanup(mazeWidth, mazeHeight, grid, numAvatars, prevXY);
      close(sockfd);
      fclose(log);
      exit(EXIT_FAILURE);
    }
    if (recvSize == 0) {
      fprintf(stderr, "Error: Server connection was closed\n");
      cleanup(mazeWidth, mazeHeight, grid, numAvatars, prevXY);
      close(sockfd);
      fclose(log);
      exit(EXIT_FAILURE);
    }

    // Check for error message
    if (IS_AM_ERROR(ntohl(serverMessage.type))) {
      fprintf(stderr, "Error message received from server\n");
      cleanup(mazeWidth, mazeHeight, grid, numAvatars, prevXY);
      fclose(log);
      close(sockfd);
      exit(EXIT_FAILURE);
    }

    // Turn message
    if (ntohl(serverMessage.type) == AM_AVATAR_TURN) {
			
      // If current avatar's turn
      if (ntohl(serverMessage.avatar_turn.TurnId) == avatarID) {
				
	// Update grid
	XYPos *newXY = serverMessage.avatar_turn.Pos;
	if(avatarID==0)
	  {
	    updateGrid(mazeWidth, mazeHeight, grid, prevXY, newXY, numAvatars,
		       avatarID, ignoreList);
	  }
	updateGrid(mazeWidth, mazeHeight, grid, prevXY, newXY, numAvatars,
		   avatarID, ignoreList);

	//check for any active signals
	g_main_context_iteration(NULL, FALSE);
	// Determine next move
	int direction = determineNextMove(mazeWidth, mazeHeight, grid,
					  prevXY, newXY, numAvatars, avatarID, ignoreList, &prevMove);
	printf("Avatar %d went in direction %d\n", avatarID, direction);

	// LOG PROGRESS

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
      // LOG SUCCESS
      printf("Success!\n");
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
	if(avatarID==0)
	  {
	    //draw out the initial positions
	    char* ID=calloc(2,sizeof(char));
	    sprintf(ID,"%d",i);
	    //might segfault
	    //erase previous content in the cell
	    gdk_draw_rectangle(PixMap,canvas->style->white_gc,TRUE,newX*CELL_SIZE+1, newY*CELL_SIZE+1,CELL_SIZE-2, CELL_SIZE-2);
	
	    //draw out the new position of avatar
	    gdk_draw_text(PixMap,fixed_font,canvas->style->white_gc,newX*CELL_SIZE+1, newY*CELL_SIZE+1,ID,1);
	    free(ID);

	    //erase previous content in the cell of previous position
	    gdk_draw_rectangle(PixMap,canvas->style->white_gc,TRUE,prevX*CELL_SIZE+1, prevY*CELL_SIZE+1,CELL_SIZE-2, CELL_SIZE-2);
	
	    //draw out the trace
	    //if trace pointing up
	    if(grid[prevX][prevY]->traceDir==1)
	      {
		gdk_draw_text(PixMap,fixed_font,canvas->style->white_gc,prevX*CELL_SIZE+1, prevY*CELL_SIZE+1,"\u2191",1);
	      }

	    //if trace pointing left
	    else if(grid[prevX][prevY]->traceDir==0)
	      {
		gdk_draw_text(PixMap,fixed_font,canvas->style->white_gc,prevX*CELL_SIZE+1, prevY*CELL_SIZE+1,"\u2190",1);
	      }

	    //if trace pointing right
	    else if(grid[prevX][prevY]->traceDir==3)
	      {
		gdk_draw_text(PixMap,fixed_font,canvas->style->white_gc,prevX*CELL_SIZE+1, prevY*CELL_SIZE+1,"\u2192",1);
	      }

	    //if trace pointing down
	    else if(grid[prevX][prevY]->traceDir==2)
	      {
		gdk_draw_text(PixMap,fixed_font,canvas->style->white_gc,prevX*CELL_SIZE+1, prevY*CELL_SIZE+1,"\u2193",1);
	      }

	    //send exposure signal to main function
	    gtk_widget_queue_draw_area(canvas, newX*CELL_SIZE, newY*CELL_SIZE,CELL_SIZE-2, CELL_SIZE-2);
	    gtk_widget_queue_draw_area(canvas, prevX*CELL_SIZE, prevY*CELL_SIZE,CELL_SIZE-2, CELL_SIZE-2);
	  }
      }
      if(avatarID==0)
	{
	  //draw out the initial positions
	  char* ID=calloc(2,sizeof(char));
	  sprintf(ID,"%d",i);
	  //might segfault
	  //draw out initial position of avatars
	  gdk_draw_text(PixMap,fixed_font,canvas->style->white_gc,newX*CELL_SIZE+1, newY*CELL_SIZE+1,ID,1);
	  free(ID);
	  //send exposure signal to main function
	  gtk_widget_queue_draw_area(canvas, newX*CELL_SIZE, newY*CELL_SIZE, CELL_SIZE-2, CELL_SIZE-2);
	}
    }
    //if the avatar didnt move, then draw a wall
    //    gdk_draw_line(PixMap,widget->style->white_gc,(newX+1)*CELL_SIZE+
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
		      **prevXY, XYPos *newXY, int numAvatars, int avatarID, int *ignoreList, int *prevMove) {

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
	printf("Followed trace...\n");
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


//create a new pixmap to do actual drawing, and to be exposed to drawingarea
//for visualization.
static gint configure_event (GtkWidget *widget, GdkEventConfigure *event)
{
  //check to see if it already exist
  //if so, then dereference and remake pixmap
  if (PixMap)
    gdk_pixmap_unref(PixMap);

  //create pixmap
  PixMap = gdk_pixmap_new(widget->window,
                          widget->allocation.width,
                          widget->allocation.height,
                          -1);

  //clear the pixmap initially to white
  gdk_draw_rectangle (PixMap,
                      widget->style->white_gc,
                      TRUE,
                      0, 0,
                      widget->allocation.width,
                      widget->allocation.height);

  return TRUE;
}


/* Redraw the screen from the backing pixmap */
static gint expose_event (GtkWidget *widget, GdkEventExpose *event)
{
  gdk_draw_pixmap(widget->window,
                  widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                  PixMap,
                  event->area.x, event->area.y,
                  event->area.x, event->area.y,
                  event->area.width, event->area.height);

  return FALSE;
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
