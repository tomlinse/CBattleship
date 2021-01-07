//
// Ed Tomlinson 11/4/19
// CSI 333 - Project 3
// Create log of shots, update board, check if game is over.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
/*
 * Creates a linked list structure to hold information on each shot, the char and int representation of
 * the row, the int representing the column, a char string holding "hit" or "miss," a char string to hold
 * the name of a ship if hit, and a pointer to the next shot in the list.
 */
struct shot {
    char rowLetter;
    int row;
    int column;
    char *status;
    char *shipHit;
    struct shot* next;
};

/*
 * Creates a 10*10 2d array to be used as a game board for battleship.
 */
int** makeBoard(){
    //Counter variables
    int i = 0;
    int j = 0;

    //Rows and columns in the game board
    int rows = 10;
    int col = 10;

    //Initiating a 2d array to be uses as the game board. Dimension Rows * Columns
    int **board = (int**)malloc(rows * sizeof(int *));
    for (i = 0; i < rows ; i++){
        board[i] = (int *) malloc(col * sizeof(int));
    }

    //Fills all slots in game board with 0's
    for (i = 0; i < rows; i++){
        for (j = 0; j < col; j++){
            *(*(board+i)+j) = 0;
        }
    }

    return board;//Return a game board
}

/*
 * Takes a game board and populates it with ships for the game.
 */
int** initialization(int** gameBoard, int* shipSpaces) {

    //Counter variables
    int i = 0;
    int j = 0;
    int k = 0;

    //Seed random number generator using time.
    srand((unsigned int)time(0));

    //Create array holding sizes of ships.
    int ships[5] = {5,4,3,3,2};

    // Places ships on the game board with a random starting placement, and orientation. Cannot
    // be placed where they overlap or hang off the edge.
    for (i = 0; i < 5; i++) {
        int ranCol = (rand() % 10); //random column address
        int ranRow = (rand() % 10); //random row address
        int ranOrient = (rand() % 2); //random orientation 1 == horizontal, 0 == vertical


        for (j = 0; j < ships[i]; j++) {
            if (ranOrient && (ranRow + ships[i] < 10) && gameBoard[ranRow+j][ranCol] != 1) {
                gameBoard[ranRow + j][ranCol] = 1;
                shipSpaces[k]= ranRow+j; //Stores row in shipSpaces, to be used to determine what ship was hit;
                shipSpaces[k+1]= ranCol; //Stores col in shipSpaces, to be used to determine what ship was hit;
                k=k+2;
            } else if (!ranOrient && (ranCol + ships[i] < 10) && gameBoard[ranRow][ranCol+j] != 1) {
                gameBoard[ranRow][ranCol + j] = 1;
                shipSpaces[k]= ranRow; //Stores row in shipSpaces, to be used to determine what ship was hit;
                shipSpaces[k+1]= ranCol+j; //Stores col in shipSpaces, to be used to determine what ship was hit;
                k=k+2;

            } else {
                /*If ships overlap or hang off edge of board: frees the current board and creates a new board
                * to try populating it with ships again.
                */
                free(gameBoard);
                gameBoard = makeBoard();
                i = -1;
                k = 0;
                break;
            }
        }
    }
    //return the populated game board
    return gameBoard;
}

/*
 * Create teardown function. Clears memory of the game board, the displayed board, and the linked list.
 * Will create a log of all shots taken at shotLog.txt.
 */
void teardown(int** gameBoard,int** displayBoard, struct shot** headShot) {

    int fd;//File directory
    char *filename = "shotLog.txt";//File name
    char buf[50];//buffer

    struct shot *readNode = *headShot;//Nodes to be read

    mode_t mode = S_IRWXU | S_IRWXG | S_IROTH;//User and Group Read/Write/Execute, Others Read Only.

    //Opens file directory at file name, Write only, creates if doesn't exist, appends or truncates file,
    // with mode specified.
    fd = open(filename, O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, mode);

    /*
     * Writes all shot nodes to files.
     */
    while(readNode != NULL) {
        //Formatted for a shot hit.
        if (strcmp(readNode->status, "Hit")==0) {
            snprintf(buf, sizeof(buf), "Fired at %c, %d. %s %s.\n", toupper(readNode->rowLetter),
                    readNode->column, readNode->status, readNode->shipHit);
            write(fd, buf, strlen(buf));
            readNode = readNode->next;
        }
        //Formatted for a shot miss.
        else{
            snprintf(buf, sizeof(buf), "Fired at %c, %d. %s.\n", toupper(readNode->rowLetter),
                    readNode->column, readNode->status);
            write(fd, buf, strlen(buf));
            readNode = readNode->next;
        }
    }

    close(fd);//Close File directory.

    struct shot *temp;//Temp Node
    struct shot *current = *headShot;//First node in list

    /*
     * Iterates through linked list and frees the nodes as it goes.
     */
    while(current != NULL)
    {
        temp = current;
        current = current->next;
        free(temp);
    }

    /*
     * Frees the games boards.
     */
    for (int i = 0; i < 10; i++){
        free(gameBoard[i]);
    }
    for (int i = 0; i < 10; i++){
        free(displayBoard[i]);
    }
    return;
}

/*
 * Updates the status of each shot as its take. Whether the shot hit or miss, and if it hit what ship.
 */
void statusUpdate(struct shot** headShot, int** gameBoard, int* shipSpaces){

    int i = 0;//Counter
    char *ship;//To hold name of shot
    struct shot* sht = *headShot;//Shot Node

    //Iterate to end of shot linked list.
    while (sht->next != NULL){
        sht = sht->next;
    }

    /*
     * Iterates through all the spaces ships occupy and finds what ship was hit.
     */
    for(i; i<34;i++) {
        if (shipSpaces[i]==(sht->row) && shipSpaces[i+1] == (sht->column)){
            if(i<10){
                ship = (char*)malloc(9* sizeof(char));
                ship = "- Carrier";
            }
            else if(i<18) {
                ship = (char*)malloc(12* sizeof(char));
                ship = "- Battleship";
            }
            else if(i<24) {
                ship = (char*)malloc(9* sizeof(char));
                ship = "- Cruiser";
            }
            else if(i<30) {
                ship = (char*)malloc(11* sizeof(char));
                ship = "- Submarine";
            }
            else {
                ship = (char*)malloc(11* sizeof(char));
                ship = "- Destroyer";
            }
        }
    }

    /*
     * Updates shot status and, if a hit, the shipHit char* of the shot.
     */
    if(gameBoard[sht->row][sht->column] == 1) {
        sht->status = malloc(3 * sizeof(char));
        sht->status = "Hit";
        sht->shipHit = ship;
    }
    else if(gameBoard[sht->row][sht->column] == 2) {
        sht->status = malloc(4 * sizeof(char));
        sht->status = "Miss";
    }
    else{
        sht->status = malloc( 20 * sizeof(char));
        sht->status = "You already shot here";
    }

    return;
}

/*
 * Function that asks user to input letter and number within bounds, and returns the two as a shot.
 */
void acceptInput(struct shot** headShot) {

    // Create a struct to hold the user inputted data.
    struct shot* sht = (struct shot*) malloc(sizeof(struct shot));

    struct shot *endShot = *headShot;//Node to hold last shot in list
    char lwr; // A char to hold the lower case version of whatever char the user enters.


    // Prompt user to enter a letter for their shot and then scans in as a char.
    //printf("Please enter a row, letters A-J:");
    scanf(" %c", &sht->rowLetter);



    // Converts shot char to its lower case.
    lwr = tolower(sht->rowLetter);

    // Prompt User to enter a number and then scan it in as the shot integer.
    //printf("Please enter a column, numbers 0-9:");
    scanf("%d", &sht -> column);

    // Create two flags, one to check the user entered int and the other to check the user entered char.
    int flagC;
    int flagNum;

    // Check if the char is within A through J. Use lwr to avoid checking upper and lower case chars.
    // Set flagC to 1 if outside bound, 0 if within.
    if (lwr < 'a' || lwr > 'j')
    {
        flagC = 1;
    }
    else {
        flagC = 0;
    }

    // Check if number entered is within range 0 through 9. Set flagNum to 1 if outside bound, 0 if within.
    if (sht -> column < 0 || sht -> column > 9) {
        flagNum = 1;
    }
    else{
        flagNum = 0;
    }

    // If Letter entered is outside bounds then prompt user to enter a new letter and check again.
    for(flagC; flagC == 1;)
    {
        // Prompt user to enter letter in correct range, scan it in, and then convert to lower case.
        printf("You have terrible aim, please enter a letter between A and J: \n");
        scanf(" %c", &sht->rowLetter);
        lwr = tolower(sht->rowLetter);

        // Check if new entry is in correct range.
        if (lwr < 'a' ||  lwr > 'j')
        {
            flagC = 1;
        }
        else {
            flagC = 0;
        }
    }

    // If number entered is outside bounds then prompt user to enter a new number and check again.
    for(flagNum;flagNum == 1;)
    {
        // Prompt user to enter a number within correct range and scan it in.
        printf("You have terrible aim, please enter a number between 0 and 9: \n");
        scanf("%d", &sht->column);

        // Check if new number is within correct range.
        if (sht -> column < 0 || sht -> column > 9)
        {
            flagNum = 1;
        }
        else {
            flagNum = 0;
        }
    }

    //Converts user entered char to integer for shot struct.
    if (lwr == 'a')
        sht->row = 0;
    else if (lwr == 'b')
        sht->row = 1;
    else if (lwr == 'c')
        sht->row = 2;
    else if (lwr == 'd')
        sht->row = 3;
    else if (lwr == 'e')
        sht->row = 4;
    else if (lwr == 'f')
        sht->row = 5;
    else if (lwr == 'g')
        sht->row = 6;
    else if (lwr == 'h')
        sht->row = 7;
    else if (lwr == 'i')
        sht->row = 8;
    else
        sht->row = 9;

    (sht->next) = NULL;//Set the nodes pointer to next in list to NULL

    //If first shot, set it as head of list
    if (*headShot == NULL){
        *headShot = sht;
        return;
    }

    //Iterates through linked list
    while((endShot->next) != NULL){
        endShot = (endShot->next);
    }
    (endShot->next) = sht;//Put current shot at end of list.

    return;
}

/*
 *  Updates the game world. Placing 1 on the display board if the user shot hit a ship on the game board
 *  or a 2 if it missed.
 */
int** update(struct shot** headShot, int** gameBoard, int** displayBoard)
{
    struct shot *inputtedShot = *headShot;//Reference to linked list.

    //Iterate to last shot in linked list
    while((inputtedShot)->next != NULL)
        inputtedShot=(inputtedShot)->next;

    struct shot *sht = inputtedShot; // Takes in parameter shot.
    int rowShot = sht->row; // The row from the user inputted shot.
    int colShot = sht->column; // The column from the user inputted shot.

    char *shotResult; // String to hold hit or miss depending on the shot.
    shotResult = malloc(sizeof(char)*4); // Allocate memory for the string.

    /*
     * If user inputted shot hits a 'ship' assign "Hit" to the string, if its a repeat ship assigns
     * "Already shot here," otherwise assign "Miss."
     */
    if(gameBoard[rowShot][colShot] == 0 || displayBoard[rowShot][colShot] == 2){
        strcpy(shotResult, "Miss");
    }
    else if (displayBoard[rowShot][colShot] == 1)
        strcpy(shotResult, "Already Shot Here");
    else
    {
        strcpy(shotResult,"Hit");
    }


    //Update board with user shot. Places a 2 if shot was a miss, 1 if a hit, or a 3 if it was a repeat shot
    if(strcmp(shotResult, "Miss") == 0) {
        displayBoard[rowShot][colShot] = 2;
    }
    else if (strcmp(shotResult, "Hit") == 0){
        displayBoard[rowShot][colShot] = 1;
    }
    else
        displayBoard[rowShot][colShot] = 3;


    // Returns the updated display board.
    return displayBoard;
}

/*
 * Displays a game board with ships hidden. Updates with 1's and 2's as users take shots,
 * prints the result of each shot to user.
 */
int display(int** displayBoard, struct shot** inputtedShot, int hitCounter)
{
    struct shot *sht = *inputtedShot;//Reference to linked list.
    //Iterates to last shot in list.
    while(sht->next !=NULL)
        sht = sht->next;

    //Updates hit counter if shot was a hit and prints hit to user
    if(displayBoard[sht->row][sht->column] == 1) {
        printf("\nYour shot was a hit \n\n");
        hitCounter++;
    }

    //Tells user if shot was a repeat. Changes the 3 to a 1 on display board.
    else if(displayBoard[sht->row][sht->column] == 3) {
        printf("\nYou already shot here \n\n");
        displayBoard[sht->row][sht->column] = 1;
    }
    //Tells user their shot missed
    else
        printf("\nYour shot was a miss\n\n");

    //print game board
    for (int i = 0; i < 10; i++){
        for(int j = 0; j < 10; j++){
            printf("%d ", displayBoard[i][j]);
        }
        printf("\n");
    }
    printf("\n");

    return hitCounter;//Return updated hit counter
}

/*
 * Main function to run Battleship. Initialize the game, then run it in a loop until a flag is set.
 * Then end the loop and teardown.
 */
int main() {

    int keepRunning = 1; // Flag to keep game running.
    int hitCounter = 0;//How many shots have hit. Used to end the game
    int shipSpaces[34];//Stores the locations of the ships on the board.

    struct shot* userShot = NULL; // Struct to hold the user inputted shot.

    int** gameBoard = makeBoard();//Initializes 2d array for the game board
    int** displayBoard = makeBoard();//Initializes 2d arry for the display board.

    int** activeBoard = initialization(gameBoard, shipSpaces); //Populates a board with ships

    printf("On the game board: 0's are unknown, 1's are hits, and 2's are misses.\n"
           "The board will update after every user shot.\nPlease enter a letter A-J for a row, "
           "followed by a number 0-9 for a column on the game board.");
    /*
     * Main game loop. Uses keepRunning as flag.
     */
    for (keepRunning;keepRunning == 1;) {

        // Asks user to input int and char and creates shot struct.
        acceptInput(&userShot);

        // Updates the display board with out come of users shot.
        displayBoard = update(&userShot, activeBoard, displayBoard);

        //Updates the shots status.
        statusUpdate(&userShot, displayBoard, shipSpaces);

        // Display the result of the shot, and the board to user. Updates hit counter.
        hitCounter = display(displayBoard, &userShot, hitCounter);

        // Prompt the user to change the flag if they want keep playing or quit.
        printf("Enter 0 if you want to quit, or enter your next shot.");
        // Scan in user answer to change keepRunning.
        scanf("%d", &keepRunning);

        /*
         * Ends game when all ships are sunk.
         */
        if(hitCounter == 16) {
            keepRunning = 0;
            hitCounter = display(displayBoard, &userShot,hitCounter);
            printf("You've sank all the ships! You win!\n");
        }
    }

    // teardown the game.
    teardown(activeBoard, displayBoard, &userShot);

    return 0;
}
