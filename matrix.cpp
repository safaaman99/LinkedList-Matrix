#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

using namespace std;

//node structure for circularly-linked list
struct Node {
    int row, col, value;
    Node* right;
    Node* down;

    Node(int r, int c, int v) : row(r), col(c), value(v), right(nullptr), down(nullptr) {}
};

//SparseMatrix class definition
class SparseMatrix {
private:
    Node* head;  //head node for circular list
    int size;    //matrix size (n x n)

    int getValue(int row, int col);

public:
    SparseMatrix(int n) : size(n) {
        head = new Node(0, 0, 0); //header node for circular list

        //initialize row heads
        Node* curr = head;
        for (int i = 1; i <= n; i++) {
            Node* currHead = new Node(i, 0, 0);         //instantiate head node point it at itself to indicate
            currHead->right = currHead;                  //empty row, then link it with previous node and move on
            curr->down = currHead;
            curr = currHead;
        }
        curr->down = head;                             //link last node to header node to make row header list circular

        //initialize column heads
        curr = head;
        for (int i = 1; i <= n; i++) {
            Node* currHead = new Node(0, i, 0);         //instantiate head node point it at itself to indicate
            currHead->down = currHead;                   //empty column, then link it with previous node and move on
            curr->right = currHead;
            curr = currHead;
        }
        curr->right = head;                            //link last node to header node to make column header list circular
    }

    void insert(int row, int col, int value);
    SparseMatrix add(SparseMatrix& B);
    SparseMatrix multiply(SparseMatrix& B);
    SparseMatrix transpose();
    SparseMatrix scalarMultiply(int scalar);

    void print();
    void writeToFile(const string& filename);

};

//helper function to retrieve value at an index
int SparseMatrix::getValue(int row, int col) {
    Node* curr = head;
    for (int i = 0; i < row; i++) {         //move to proper row linked list to find node
        curr = curr->down;
    }
    curr = curr->right;
    while (curr->col != 0 && curr->col <= col) {  //check if node exists and return it, else return nullptr
        if (curr->col == col){
            return curr->value;
        }
        curr = curr->right;
    }
    return 0;
}

//function to insert node into the sparse matrix
void SparseMatrix::insert(int row, int col, int value) {
    if (value == 0) return;  //skip inserting zero values

    //create new node
    Node* newNode = new Node(row, col, value);

    //insert in circular linked list based on row
    Node* rowCurr = head;
    for (int i = 0; i < row; i++) {         //move to proper row head to insert into row linked list
        rowCurr = rowCurr->down;
    }
    while (rowCurr->right->col != 0 && rowCurr->right->col < col) {
        rowCurr = rowCurr->right;
    }
    newNode->right = rowCurr->right;
    rowCurr->right = newNode;

    //insert in circular linked list based on column
    Node* colCurr = head;
    for (int i = 0; i < col; i++) {         //move to proper col head to insert into col linked list
        colCurr = colCurr->right;
    }
    while (colCurr->down->row != 0 && colCurr->down->row < row) {
        colCurr = colCurr->down;
    }
    newNode->down = colCurr->down;
    colCurr->down = newNode;
}

//matrix addition: A + B
SparseMatrix SparseMatrix::add(SparseMatrix& B) {
    SparseMatrix C(size);
    int a, b;
    for (int i = 1; i <= size; i++) {
        for (int j = 1; j <= size; j++) {
            a = this->getValue(i, j);
            b = B.getValue(i, j);
            C.insert(i, j, a + b);
        }
    }
    return C;
}

//matrix multiplication: A * B
SparseMatrix SparseMatrix::multiply(SparseMatrix& B) {
    SparseMatrix C(size);
    int a, b;
    for (int i = 1; i <= size; i += 1) {
        for (int j = 1; j <= size; j += 1) {
            int dotproduct = 0;
            for (int k = 1; k <= size; k += 1) {
                a = this->getValue(i, k);
                b = B.getValue(k, j);
                dotproduct += a * b;
            }
            C.insert(i, j, dotproduct);
        }
    }

    return C;
}

//matrix transposition
SparseMatrix SparseMatrix::transpose() {
    SparseMatrix transposedMatrix(size);  //create a new matrix for transpose

    //traverse the matrix and insert each element with flipped row/col
    for (Node* colHead = head->right; colHead != head; colHead = colHead->right) {
        for (Node* row = colHead->down; row != colHead; row = row->down) {
            transposedMatrix.insert(row->col, row->row, row->value);  //flip row and col
        }
    }

    return transposedMatrix;
}

//scalar multiplication
SparseMatrix SparseMatrix::scalarMultiply(int scalar) {
    SparseMatrix C(size);

    for (Node* colHead = head->right; colHead != head; colHead = colHead->right) {
        for (Node* row = colHead->down; row != colHead; row = row->down) {
            C.insert(row->row, row->col, row->value * scalar);
        }
    }

    return C;
}

//write matrix to file
void SparseMatrix::writeToFile(const string& filename) {
    ofstream outfile(filename);

    //write matrix to file
    for (int i = 1; i <= size; i++){
        for (int j = 1; j <= size; j++) {
            int value = this->getValue(i,j);
            if (value == 0) {continue;}
            outfile << i << "," << j << "," << value << endl;
        }
    }
    outfile.close();
}   

//read csv file and perform operation specified
void readFromFile(const string& filename) {
    ifstream infile(filename);
    if (!infile.is_open()) {
        cout << "File failed to open" << endl;
        exit(1);
    }

    string line,token;
    char op;
    int n;

    getline(infile, line);          //gets first line
    istringstream iss(line);

    getline(iss,token,',');            //gets operation and size from line;
    op = token[0];
    getline(iss,token,',');
    n = stoi(token);

    getline(infile,line);           //call getline to move forward for later reading
    SparseMatrix A(n);
    SparseMatrix B(n);
    SparseMatrix C(n);

    switch (op) {
        case 'A':       
            getline(infile,line);       //read matrix data to each matrix before performing addition
            while (line != ",,") {
                istringstream index(line);
                int values[3];
                for (int i = 0; i < 3; i++) {
                    getline(index,token,',');
                    values[i] = stoi(token);
                }
                A.insert(values[0],values[1],values[2]);
                getline(infile,line);
            }

            while (getline(infile,line)) {
                istringstream index(line);
                int values[3];
                for (int i = 0; i < 3; i++) {
                    getline(index,token,',');
                    values[i] = stoi(token);
                }
                B.insert(values[0],values[1],values[2]);
            }
            C = A.add(B);
            break;

        case 'M':
            getline(infile,line);       //read matrix data to each matrix before performing multiplication
            while (line != ",,") {
                istringstream index(line);
                int values[3];
                for (int i = 0; i < 3; i++) {
                    getline(index,token,',');
                    values[i] = stoi(token);
                }
                A.insert(values[0],values[1],values[2]);
                getline(infile,line);
            }
       
            while (getline(infile,line)) {
                istringstream index(line);
                int values[3];
                for (int i = 0; i < 3; i++) {
                    getline(index,token,',');
                    values[i] = stoi(token);
                }
                B.insert(values[0],values[1],values[2]);
            }
            C = A.multiply(B);
            break;

        case 'T':            
            while (getline(infile,line)) {      //read matrix data before transposing
                istringstream index(line);
                int values[3];
                for (int i = 0; i < 3; i++) {
                    getline(index,token,',');
                    values[i] = stoi(token);
                }
                A.insert(values[0],values[1],values[2]);
            }
            C = A.transpose();
            break;

        case 'S':
        default:   
            getline(infile,line);    
            while (line != ",,") {      //read matrix data before doing scalar multiplication
                istringstream index(line);
                int values[3];
                for (int i = 0; i < 3; i++) {
                    getline(index,token,',');
                    values[i] = stoi(token);
                }
                A.insert(values[0],values[1],values[2]);
                getline(infile,line);
            }
            getline(infile,line);
            int factor = stoi(line);
            C = A.scalarMultiply(factor);
    }
    infile.close();
    int insertion_pos = filename.find(".csv");
    string outfile = filename;
    outfile.insert(insertion_pos, "_output");
    C.writeToFile(outfile);
}



//main program
int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <input_file>" << endl;
        return 1;
    }
    readFromFile(argv[1]);
    return 0;
}