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


// Constructor for node class
Node::Node() {
    parent = NULL;
    f = 0;
    g = 0;
    h = 0;
}

// Non-default constructor
Node::Node(int num_stacks, int num_blocks) {
    parent = NULL;
    f = 0;
    h = 0;
    g = 0;
}

struct runner {
    Node *goal; 
    Node *start; 
    int iterations; 
    int stacks;
    int blocks;
    int queue_size;
};


void print_state(Node *, int, int);
bool compare_states(Node *cur, Node *goal, int num_s, int num_b);

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

// Computes the number of blocks in a stack 
// Used for better printing. 
int blocks_in_stack(Node *state, int s, int num_blocks) {
    int n = 0;

    if (state->v[s].empty()) return 0;

    for (int i = 0; i < num_blocks; i++) {
        if (state->v[s][i] < 65 || state->v[s][i] > 90) {
            return n;
        }
        n++;
    }

    return n;
}

// Prints the blocksworld state
void print_state(Node *state, int num_s, int num_b) {
    for (int i =0 ; i < num_s; i++) {
        std::cout << i << " | ";
        for (int j = 0; j < blocks_in_stack(state, i, num_b); j++) {
            if (!state->v[i].empty())
                std::cout << state->v[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

/* Computes the heuristic value..
*/
double compute_h(Node *cur_state, Node *goal_state, int num_s, int num_b) {
    double h = 0;

    for (int i = 0; i < num_s; i++) {
        for (int j = 0; j < cur_state->v[i].size(); j++) {

            // If a block is in the wrong spot in the goal state
            if (i == 0 && cur_state->v[i][j] != goal_state->v[i][j] ) {
                h += (j+1)*5;
            } 

            // If blocks aren't even in the goal stack
            if (i != 0) {
                h += 3;

                // If a block is above another block with a higher value
                if (cur_state->v[i].size() >= j+2) {
                    if (cur_state->v[i][j] < cur_state->v[i][j+1]) {
                        h += (j+1)*3;
                    }
                }
            }
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

// Perform the A* Search
// Requires the start state, goal state, number of stacks, number of blocks, and the
// runner struct that stores certain information.c
Node * start_search(Node *start, Node *goal, int num_s, int num_b, struct runner &r) {
    cout << "\nStarting Search...." << endl;

    vector <Node*> open, closed;
    int itr = 0;

    // Set the starting values for the first state
    start->parent = NULL;
    start->h = compute_h(start, goal, num_s, num_b);
    start->g = 0;
    start->f = start->h + 0;
    open.push_back(start);

    // If start state is the goal state, end search
    if (compare_states(start, goal, num_s, num_b)) return start;

    while (!open.empty()) {

        r.iterations = itr;

        if (itr >= 5000) return NULL;

        if (open.empty()) {
            cout << "No more states left. Ending search." << endl;
            return NULL; // If no states in frontier, search failed
        }

        // Grab lowest f-value state in the open list
        // Pretty much the pqueue function
        int min_index = 0, min_f = 10000000;
        for (int i = 0; i < open.size(); i++) {
            //cout << open[i]->f << " ";
            if (open[i]->f < min_f) {
                min_f = open[i]->f;
                min_index = i;
            }
        }

        // Grab the lowest node and then remove it from the open list (Frontier)
        Node *current = open[min_index];
        open.erase(open.begin() + min_index);
        closed.push_back(current);

        cout << "ITR: " << itr << " - QUEUE: " << open.size() << " - F: " << current->f <<endl;

        // Check if current state is the goal, return said state if true
        if (compare_states(current, goal, num_s, num_b)) {
            r.queue_size = open.size();
            return current;
        }

        else {
            // Generate successors
            vector <Node*> successors = create_successors(current, num_s, num_b);

            int skip = 0;

            // Iterate through successors
            for (int i = 0; i < successors.size(); i++ ) {

                skip = 0;

                // Compute score for all successors
                successors[i]->parent = current;
                successors[i]->g = current->g + 1;
                successors[i]->h = compute_h(successors[i], goal, num_s, num_b);
                successors[i]->f = successors[i]->g + successors[i]->h;

                // Check if we a successor state is already in the open list, 
                // Update the scores if we found a shorter path to the state
                for (int o = 0; o < open.size(); o++) {
                    if (compare_states(successors[i], open[o], num_s, num_b)) {
                        if (successors[i]->f < open[o]->f) {
                            open[o]->g = successors[i]->g;
                            open[o]->f = successors[i]->f;
                        }
                        skip = 1;
                        break; 
                    }
                }
                
                // If we have already explored a state, we don't need to visit 
                // it again.
                for (int c = 0; c < closed.size(); c++) {
                    if (compare_states(successors[i], closed[c], num_s, num_b)){
                        skip = 1;
                        break;
                    }
                }

                // Only add a new successor if it doesn't need to be skipped.
                if (!skip) {
                    open.push_back(successors[i]);

                }
                
            }
        }
        itr++;
    }
}

// View the path taken from the start to final state
int traceback(Node *state, int num_s, int num_b) {

    vector <Node *> states;
    Node *temp = state;

    while (temp->parent != NULL) {
        states.push_back(temp);
        temp = temp->parent;
    }

    cout << "START: " << endl;
    for (int i = states.size() - 1; i >= 0; i--) {
        print_state(states[i], num_s, num_b);

        cout << endl;
    }

    return states.size();
}

// Validation function for command line arguments 
// when setting the stack/block number
bool validate_args(int argc, char **argv) {

    if (argc != 3) return false;

    if (atoi(argv[1]) < 1) return false;
    if (atoi(argv[2]) < 1) return false;

    return true; 
}

int main(int argc, char **argv) {

    srand(time(NULL)); // seed random
    int pl = 0;

    if (!validate_args(argc, argv)) {
        cout << "Error in command line arguments.\n";
        cout << "USAGE: './a.out 'num-stacks' 'num-blocks'\n";
        cout << "E.G. : '/a.out 3 5'" << endl;
        return 0;
    }

    // Runner struct that contains all necessary information
    struct runner r;
    r.stacks = atoi(argv[1]);
    r.blocks = atoi(argv[2]);
    r.iterations = 0;
    r.queue_size = 0;
    r.goal = new Node(r.stacks, r.blocks);
    r.start = new Node(r.stacks, r.blocks);
    r.goal->v = problem_generator(r.start, r.stacks, r.blocks); // generate start/goal states

    // Perform the A* Search
    Node *final_state = start_search(r.start, r.goal, r.stacks, r.blocks, r);

    // If a final state is found, perform a traceback to produce path
    if(final_state != NULL) pl = traceback(final_state, r.stacks, r.blocks);
    else cout << "SEARCH FAILED. NO GOAL STATE FOUND." << endl;

    cout << "Final number of iterations: " << r.iterations << endl;
    cout << "Final path length: " << pl << endl;
    if (final_state != NULL)
        cout << "Max. queue size: " << r.queue_size << endl;

    return 0;
}
