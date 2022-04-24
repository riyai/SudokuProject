#include <bits/stdc++.h>

#define UNASSIGNED 0

using namespace std;

//Define the size of the grid
const int SQRTDIM = 3;
const int DIM = SQRTDIM * SQRTDIM;

//Create a class Sudoku
//Has functions for all operations
class Sudoku {
private:
  int grid[DIM][DIM];
  int solnGrid[DIM][DIM];
  int guessNum[DIM];
  int gridPos[DIM * DIM];
  bool grid_status;

public:
  Sudoku ();
  Sudoku (string, bool row_major=true);
  void fillEmptyDiagonalBox(int);
  void createSeed();
  void printGrid();
  bool solveGrid();
  void countSoln(int &number);
  void genPuzzle();
  bool verifyGridStatus();
  void getCell(int grow, int gcol);
};

// START: To generate random number
int genRandNum(int maxLimit)
{
  return rand()%maxLimit;
}
// END


// START: These are helper functions for solving grid
//Find unassigned location
bool FindUnassignedLocation(int grid[DIM][DIM], int &row, int &col)
{
    for (row = 0; row < DIM; row++)
    {
        for (col = 0; col < DIM; col++)
        {
            if (grid[row][col] == UNASSIGNED)
                return true;
        }
    }
    return false;
}

//Check if a number valid for a row
bool UsedInRow(int grid[DIM][DIM], int row, int num)
{
    for (int col = 0; col < DIM; col++)
    {
        if (grid[row][col] == num)
            return true;
    }

    return false;
}

//Check if a number valid for a column
bool UsedInCol(int grid[DIM][DIM], int col, int num)
{
    for (int row = 0; row < DIM; row++)
    {
        if (grid[row][col] == num)
            return true;
    }

    return false;
}

//Check if a number valid for a sub grid
bool UsedInBox(int grid[DIM][DIM], int boxStartRow, int boxStartCol, int num)
{
    for (int row = 0; row < SQRTDIM; row++)
    {
        for (int col = 0; col < SQRTDIM; col++)
        {
            if (grid[row+boxStartRow][col+boxStartCol] == num)
                return true;
        }
    }

    return false;
}

//Safe to use a number or not
bool isSafe(int grid[DIM][DIM], int row, int col, int num)
{
    return !UsedInRow(grid, row, num) && !UsedInCol(grid, col, num) && !UsedInBox(grid, row - row%SQRTDIM , col - col%SQRTDIM, num);
}
// END: Helper functions for solving grid


// START: Create seed grid
void Sudoku::fillEmptyDiagonalBox(int idx)
{
  int start = idx*SQRTDIM;
  random_shuffle(this->guessNum, (this->guessNum) + DIM, genRandNum);
  for (int i = 0; i < SQRTDIM; ++i)
  {
    for (int j = 0; j < SQRTDIM; ++j)
    {
      this->grid[start+i][start+j] = guessNum[i*SQRTDIM+j];
    }
  }
}

void Sudoku::createSeed()
{
  /* Fill diagonal boxes to form:
      x | . | .
      . | x | .
      . | . | x
  */
  for (int i = 0; i < SQRTDIM; ++i)
	this->fillEmptyDiagonalBox(i);

  /* Fill the remaining blocks:
      x | x | x
      x | x | x
      x | x | x
  */
  if (!this->solveGrid()) // TODO: not truly random, but still good enough because we generate random diagonals.
	this->createSeed();   // No solution possible so run again

  // Saving the solution grid
  for(int i=0;i<DIM;i++)
  {
    for(int j=0;j<DIM;j++)
    {
      this->solnGrid[i][j] = this->grid[i][j];
    }
  }
}
// END


// START: Intialising
Sudoku::Sudoku()
{

  // Randomly shuffling the array of removing grid positions
  for(int i=0;i<DIM * DIM;i++)
  {
    this->gridPos[i] = i;
  }

  random_shuffle(this->gridPos, (this->gridPos) + DIM * DIM, genRandNum);

  // Randomly shuffling the guessing number array
  for(int i=0;i<DIM;i++)
  {
    this->guessNum[i]=i+1;
  }

  random_shuffle(this->guessNum, (this->guessNum) + DIM, genRandNum);

  // Initialising the grid
  for(int i=0;i<DIM;i++)
  {
    for(int j=0;j<DIM;j++)
    {
      this->grid[i][j]=0;
    }
  }

  grid_status = true;
}
// END

// START: Custom Initialising with grid passed as argument
Sudoku::Sudoku(string grid_str, bool row_major)
{
  if(grid_str.length() != DIM * DIM)
  {
    grid_status=false;
    return;
  }

  // First pass: Check if all cells are valid
  for(int i=0; i<DIM * DIM; ++i)
  {
    int curr_num = grid_str[i]-'0';
    if(!((curr_num == UNASSIGNED) || (curr_num > 0 && curr_num <= DIM)))
    {
      grid_status=false;
      return;
    }

    if(row_major) grid[i/DIM][i%DIM] = curr_num;
    else          grid[i%DIM][i/DIM] = curr_num;
  }

  // Second pass: Check if all columns are valid
  for (int col_num=0; col_num<DIM; ++col_num)
  {
    bool nums[DIM + 1]={false};
    for (int row_num=0; row_num<DIM; ++row_num)
    {
      int curr_num = grid[row_num][col_num];
      if(curr_num!=UNASSIGNED && nums[curr_num]==true)
      {
        grid_status=false;
        return;
      }
      nums[curr_num] = true;
    }
  }

  // Third pass: Check if all rows are valid
  for (int row_num=0; row_num<DIM; ++row_num)
  {
    bool nums[DIM + 1]={false};
    for (int col_num=0; col_num<DIM; ++col_num)
    {
      int curr_num = grid[row_num][col_num];
      if(curr_num!=UNASSIGNED && nums[curr_num]==true)
      {
        grid_status=false;
        return;
      }
      nums[curr_num] = true;
    }
  }

  // Fourth pass: Check if all blocks are valid
  for (int block_num=0; block_num<DIM; ++block_num)
  {
    bool nums[DIM + 1]={false};
    for (int cell_num=0; cell_num<DIM; ++cell_num)
    {
      int curr_num = grid[((int)(block_num/SQRTDIM))*SQRTDIM + (cell_num/SQRTDIM)][((int)(block_num%SQRTDIM))*SQRTDIM + (cell_num%SQRTDIM)];
      if(curr_num!=UNASSIGNED && nums[curr_num]==true)
      {
        grid_status=false;
        return;
      }
      nums[curr_num] = true;
    }
  }

  // Randomly shuffling the guessing number array
  for(int i=0;i<DIM;i++)
  {
    this->guessNum[i]=i+1;
  }

  random_shuffle(this->guessNum, (this->guessNum) + DIM, genRandNum);

  grid_status = true;
}
// END


// START: Verification status of the custom grid passed
bool Sudoku::verifyGridStatus()
{
  return grid_status;
}
// END


// START: Printing the grid
void Sudoku::printGrid()
{
  for (int row = 0; row < DIM; row++){
      for (int col = 0; col < DIM; col++){
         if(col % SQRTDIM == 0 && col != 0)
            cout << " | ";
         cout << grid[row][col] <<" ";
      }
      if(row != 0 && (row + 1) % SQRTDIM == 0){
         cout << endl;
         for(int i = 0; i<DIM; i++)
            cout << "---";
      }
      cout << endl;
   }
}
// END


// START: Modified Sudoku solver
bool Sudoku::solveGrid()
{
    int row, col;

    // If there is no unassigned location, we are done
    if (!FindUnassignedLocation(this->grid, row, col))
       return true; // success!

    // Consider digits 1 to DIM
    for (int num = 0; num < DIM; num++)
    {
        // if looks promising
        if (isSafe(this->grid, row, col, this->guessNum[num]))
        {
            // make tentative assignment
            this->grid[row][col] = this->guessNum[num];

            // return, if success, yay!
            if (solveGrid())
                return true;

            // failure, unmake & try again
            this->grid[row][col] = UNASSIGNED;
        }
    }

    return false; // this triggers backtracking

}
// END


// START: Check if the grid is uniquely solvable
void Sudoku::countSoln(int &number)
{
  int row, col;

  if(!FindUnassignedLocation(this->grid, row, col))
  {
    number++;
    return ;
  }


  for(int i=0;i<DIM && number<2;i++)
  {
      if( isSafe(this->grid, row, col, this->guessNum[i]) )
      {
        this->grid[row][col] = this->guessNum[i];
        countSoln(number);
      }

      this->grid[row][col] = UNASSIGNED;
  }

}
// END


// START: Generate puzzle
void Sudoku::genPuzzle()
{
  for(int i=0;i<DIM * DIM;i++)
  {
    int x = (this->gridPos[i])/DIM;
    int y = (this->gridPos[i])%DIM;
    int temp = this->grid[x][y];
    this->grid[x][y] = UNASSIGNED;

    // If now more than 1 solution , replace the removed cell back.
    int check=0;
    countSoln(check);
    if(check!=1)
    {
      this->grid[x][y] = temp;
    }
  }
}
// END

void Sudoku::getCell(int grow, int gcol)
{
      int grid1[DIM][DIM];
    
      for(int i=0; i<DIM; i++){
          for(int j=0; j<DIM; j++){
              grid1[i][j]=grid[i][j];
          }
      }
      solveGrid();
      grid1[grow][gcol]=grid[grow][gcol];
      for(int i=0; i<DIM; i++){
          for(int j=0; j<DIM; j++){
              grid[i][j]=grid1[i][j];
          }
      }
      printGrid();
}

// START: The main function
int main(int argc, char const *argv[])
{
  // Creating an instance of Sudoku
  Sudoku *puzzle = new Sudoku();

  // Creating a seed for puzzle generation
  puzzle->createSeed();

  // Generating the puzzle
  puzzle->genPuzzle();

  cout << endl << "Solve:" << endl;

  // testing by printing the grid
  puzzle->printGrid();
  
start:
  cout << "1. Display complete solution"<< endl<< "2. View specific cell"<<endl;
    int ch;
    cin >> ch;
    if(ch==1){
        puzzle->solveGrid();
        puzzle->printGrid();
    }
    else if(ch==2){
        char choice='Y';
        do{ 
            int r, c;
            begin:
            cout << endl << "Enter the row and coloumn(0 -"<<DIM-1<< ")" << endl;
            cin >> r >> c;

            if((r>=0&&r<DIM)&&(c>=0&&c<DIM)){
                
                cout << endl << "Solution for the given cell:" << endl;
                puzzle->getCell(r, c);
            }
            else{
                cout << "Given values exceed the size limits, enter valid dimensions" << endl;
                goto begin;
            }
        
            cout << "Continue?(Enter Y for yes/ N for no)";
            cin >> choice;

        }while(choice!='N'&&choice!='n');
      } 
      else{
          cout << "Not a valid choice" << endl;
          goto start;
      }
  
  return 0;
}
// END: The main function
