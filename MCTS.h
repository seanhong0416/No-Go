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
#include "board.h"
#include "action.h"
#include "agent.h"

class MCTS : public random_agent{
public:
	MCTS(const std::string& args = "") : random_agent("name=random role=unknown " + args),
		space(board::size_x * board::size_y), who(board::empty) {
		if (name().find_first_of("[]():; ") != std::string::npos)
			throw std::invalid_argument("invalid name: " + name());
		if (role() == "black") who = board::black;
		if (role() == "white") who = board::white;
		if (who == board::empty)
			throw std::invalid_argument("invalid role: " + role());
		for (size_t i = 0; i < space.size(); i++)
			space[i] = action::place(i, who);
	}

	void playOneSequence(Node root){
		std::vector<Node> route;
		route[0] = root;
		int i = 0;
		while(1){
			//check if route[i] is a leaf
			if(route[i].kids.size() == 0){
				//if it is a leaf that hasn't been traverse,
				//expand its kids and then do simulation
				if(route[i].nb == 0){
					route[i]
				}
				else{

				}
			}
		}
	}

	virtual action take_action(const board& state) {
		//initialize
		Node rootNode(state, who);

		//do MCTS
		playOneSequence(rootNode);

		for (const action::place& move : space) {
			board after = state;
			if (move.apply(after) == board::legal)
				return move;
		}
		return action();
	}

private:
	std::vector<action::place> space;
	board::piece_type who;
};

class Node{
public:
    Node(board b_in, board::piece_type who_in):
    nb(0), value(0), b(b_in), who(who_in), space(board::size_x * board::size_y)
    {}

    void new_kids();

	std::vector<Node> kids;
private:
    std::vector<action::place> space;       
    board::piece_type who;                  

	board b;       
    int value;
	int nb;                                 
};