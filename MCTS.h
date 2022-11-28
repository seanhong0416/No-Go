/*
Agent with Monte Carlo Tree Search
*/

#pragma once
#include <string>
#include <random>
#include <sstream>
#include <map>
#include <type_traits>
#include <algorithm>
#include <fstream>
#include <limits>
#include "board.h"
#include "action.h"
#include "agent.h"

class Node{
public:
    Node(board b_in, board::piece_type who_in, action::place action_in)
    {
		who = who_in;
		b = b_in;
		action_taken = action_in;
		value = 0;
		nb = 0;
	}

	void new_kids();

	std::vector<Node *> kids;      
    board::piece_type who;                  

	board b;
	action::place action_taken;
    int value;
	int nb;                                 
};

class MCTS : public random_agent{
public:
	MCTS(const std::string& args = "") : random_agent("name=random role=unknown " + args),
		space(board::size_x * board::size_y), who(board::empty) {
		printf("MCTS player initialized\n");
		if (name().find_first_of("[]():; ") != std::string::npos)
			throw std::invalid_argument("invalid name: " + name());
		if (role() == "black") who = board::black;
		if (role() == "white") who = board::white;
		if (who == board::empty)
			throw std::invalid_argument("invalid role: " + role());
		for (size_t i = 0; i < space.size(); i++)
			space[i] = action::place(i, who);
	}

	Node * selection(Node * parent);

	int random_simulation(Node * start, board::piece_type who_start);

	void update_value(std::vector<Node *>& route, float v);

	void playOneSequence(Node* root){
		int i = 0;
		int simulation_value;
		std::vector<Node *> route;
		route.clear();
		route.push_back(root);
		while(1){
			//printf("route : %d\n",i);
			//check if route[i] is a leaf
			if(route[i]->kids.size() == 0){
				//if it is a leaf that hasn't been traverse,
				//expand its kids and then do simulation
				if(route[i]->nb == 0){
					//printf("it is a leaf that hasn't been traversed\n");
					route[i]->new_kids();
					//printf("number of kids after new_kids : %d\n", (int)route[i]->kids.size());
					//simulate
					simulation_value = random_simulation(route[i], route[i]->who);
					//update value and nb of the last node
					route[i]->nb += 1;
					route[i]->value = simulation_value;
					//printf("sim_val = %d, nb = %d, value = %d\n", simulation_value, route[i]->nb, route[i]->value);
				}
				//else it is the end of the game
				else{
					//printf("it is a leaf that is the end of game\n");
					//simulate
					simulation_value = random_simulation(route[i], route[i]->who);
					//update value and nb of the last node
					route[i]->nb += 1;
					route[i]->value += simulation_value;
				}
				//update the value of each node in the route
				update_value(route, -simulation_value);
				break;
			}
			//not a leaf, continue selection
			//printf("select next kid\n");
			Node * next_kid = selection(route[i]);
			route.push_back(next_kid);
			i++;
		}
	}

	void print_tree(Node * root, Node * parent){
		int kids_len = root->kids.size();
		//printf("value : %d, nb : %d, parent value : %d, parent nb : %d\n",root->value,root->nb,parent->value,parent->nb);
		for(int i=0;i<kids_len;i++){
			print_tree(root->kids[i], root);
		}
	}

	virtual action take_action(const board& state) {
		//initialize
		//action in rootNode doesn't mean anything
		Node * rootNode = new Node(state, who, action());
		//do MCTS
		for(int i=0;i<100;i++){
			//printf("iteration : %d\n", i);
			//printf("playOneSequence\n");
			playOneSequence(rootNode);
		}
		print_tree(rootNode, rootNode);
		//std::cout << state << std::endl;
		action::place best_action;
		int best_visit_count = -1;
		int rootNode_kids_len = (int)rootNode->kids.size();
		if(rootNode_kids_len != 0){
			for(int i=0;i<rootNode_kids_len;i++){
				//printf("kid %d visited %d\n",i,rootNode->kids[i]->nb);
				if(rootNode->kids[i]->nb > best_visit_count){
					best_visit_count = rootNode->kids[i]->nb;
					best_action = rootNode->kids[i]->action_taken;
				}
			}
			//std::cout << "best action chosen = " << best_action << std::endl;
			return best_action;
		}
		else{
			std::cout << "no moves left" << std::endl;
			return action();
		}
	}

private:
	std::vector<action::place> space;
	board::piece_type who;
};