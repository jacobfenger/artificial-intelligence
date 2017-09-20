/****************************************************
Jacob Fenger - CS 625 PROJECT #1 - 9/18/2017
Resources used:
    - Textbook
    - http://web.mit.edu/eranki/www/tutorials/search/
*********************************************************/

#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <vector>
#include <queue>
#include <unistd.h>
#include <algorithm>

using std::vector;
using std::cout;
using std::endl;


// Used to represent specific states in blockworld
class Node {
    public:
        Node *parent;
        vector <vector<char> > v;
        double f, g, h;
        Node();
        Node(int, int);
};

Node::Node() {
    parent = NULL;
    f = 0;
    g = 0;
    h = 0;
}

Node::Node(int num_stacks, int num_blocks) {
    parent = NULL;
    f = 0;
    h = 0;
    g = 0;
}

void print_state(Node *, int, int);

// Generates the problem by randomly assigning blocks to stacks
// This works by scrambling the final state
// It also returns the goal state
vector <vector<char> > problem_generator(Node *state, int num_s, int num_b) {

    vector <vector<char> > start_board, goal;
    vector<char> row;

    // Initialize the goal state
    for (int i = 0; i < num_b; i++)
        row.push_back(char(i+65));

    start_board.push_back(row);
    for (int i = 1; i < num_s; i++) {
        vector<char> r;
        start_board.push_back(r);
    }

    goal = start_board;

    // Scramble the start_board state to create a starting state
    for (int i = 0; i < 1000; i++) {
        // Generate source and destination for stack swapping
        int src_stack = rand() % num_s;
        int des_stack = rand() % num_s;

        if (!start_board[src_stack].empty()) {
            char val = start_board[src_stack].back();
            start_board[src_stack].pop_back();
            start_board[des_stack].push_back(val);
        }
    }

    // Update start state node
    state->v = start_board;

    return goal;
}

// Prints the blocksworld state
void print_state(Node *state, int num_s, int num_b) {
    for (int i =0 ; i < num_s; i++) {
        std::cout << i << " | ";
        for (int j = 0; j < num_b; j++) {
            if (!state->v[i].empty())
                std::cout << state->v[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

/* Computes the heuristic value.
 * This first approach just adds up the blocks that are out of place.
*/
double compute_h(Node *cur_state, Node *goal_state, int num_s, int num_b) {
    double h = 0;

    for (int i = 0; i < num_s; i++) {
        // Compute blocks in the final stack that aren't in the correct place
        if (i == 0) {
            for (int b = 0; b < cur_state->v[0].size(); b++) {
                // If blocks in final stack are out of place, add to h
                if (cur_state->v[0][b] != goal_state->v[0][b]) {
                    h++;
                }
            }
        }
        // Add up all the blocks in current state that aren't in final stack
        else {
            h += cur_state->v[i].size();
        }
    }
    return h;
}

// Returns true if the cur. state is the same as the goal state
// False otherwise
bool compare_states(Node *cur, Node *goal, int num_s, int num_b) {

    for (int i = 0; i < num_s; i++) {
        if (cur->v[i].size() != goal->v[i].size()) return false;

        for (int j = 0; j < cur->v[i].size(); j++) {

            if (cur->v[i][j] != goal->v[i][j]) return false;
        }
    }

    return true;
}

// Generate all the possible successors
// I THINK THIS IS FULLY WORKING?
vector <Node*> create_successors(Node *current_state, int num_s, int num_b) {
    vector <Node*> s_list;
    vector <vector<char> > stack = current_state->v;
    char block;

    // Loop through all of the stacks
    for (int i = 0; i < num_s; i++) {

        // We can only move a block off of a stack if there is one to move
        if (stack[i].size() > 0) {
            int src = i;

            // Move the block from stack i to the other stacks to generate b - 1
            // childen
            for (int dest = 0; dest < num_s; dest++) {
                vector <vector<char> > c = stack;
                if (src == dest) continue; // ignore the same stack

                block = c[src].back(); // get top of src stack
                c[src].pop_back();

                c[dest].push_back(block);
                Node *n = new Node();
                n->v = c;
                s_list.push_back(n);
            }
        }
    }

    return s_list;
}

bool start_search(Node *start, Node *goal, int num_s, int num_b) {
    cout << "\nStarting Search...." << endl;

    // Initialize pqueue
    //std::priority_queue<Node*> open, closed;

    vector <Node*> open, closed;
    int itr = 0;

    // Set the starting values for the first state
    start->parent = NULL;
    start->h = compute_h(start, goal, num_s, num_b);
    start->g = 0;
    start->f = start->h + 0;
    open.push_back(start);

    if (compare_states(start, goal, num_s, num_b)) return true;

    while (!open.empty()) {

        //cout << "ITERATIONS: " << itr << endl;
        if (open.empty()) return false; // If no states in frontier, search failed



        // Grab lowest node in the pqueue
        int min_index = 0, min_f = 10000000;
        for (int i = 0; i < open.size(); i++) {
            if (open[i]->f < min_f) {
                min_f = open[i]->f;
                min_index = i;
            }
        }
        Node *current = open[min_index];
        open.erase(open.begin() + min_index);
        closed.push_back(current);

        sleep(3);
        cout << "CURRENT STATE: " << endl;
        cout << "G: " << current->g << " H: " << current->h << endl;
        cout << "F value: " << current->f << endl;
        print_state(current, num_s, num_b);

        // Check if current state is the goal, return if true
        if (compare_states(current, goal, num_s, num_b)) {
            cout << "GOAL STATE FOUND: " << endl;
            print_state(current, num_s, num_b);
            return true;
        }
        else {

            // Generate successors
            vector <Node*> successors = create_successors(current, num_s, num_b);
            int skip = 0;
            int num = 0;
            // Iterate through successors
            for (int i = 0; i < successors.size(); i++ ) {

                // Check if we have already been to a successor's state
                for (int o = 0; o < open.size(); o++) {
                    if (compare_states(successors[i], open[o], num_s, num_b)) {
                        skip = 1;
                    }
                }
                for (int c = 0; c < closed.size(); c++) {
                    if (compare_states(successors[i], closed[c], num_s, num_b)){
                        skip = 1;
                    }
                }

                if (skip) continue;

                // Compute score for all successors
                successors[i]->parent = current;
                successors[i]->g = current->g + 1;
                successors[i]->h = compute_h(successors[i], goal, num_s, num_b);
                successors[i]->f = successors[i]->g + successors[i]->h;

                open.push_back(successors[i]);
                num++;

            }
            cout << "NUM OF SUCCESSORS ADDED: " << num << endl;
        }
        itr++;
    }
}

int main(int argc, char **argv) {

    srand(time(NULL)); // seed random

    int stacks = 3, blocks = 3;
    Node *start, *goal;

    goal = new Node(stacks, blocks);

    // Initialize the starting state
    start = new Node(stacks, blocks);
    goal->v = problem_generator(start, stacks, blocks);

    cout << "\nGOAL STATE: " << endl;
    print_state(goal, stacks, blocks);
    cout << endl;

    cout << "STARTING STATE: " << endl;
    print_state(start, stacks, blocks);

    start_search(start, goal, stacks, blocks);

    return 0;
}
