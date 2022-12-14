/*
Agent with Monte Carlo Tree Search
*/

#pragma once
#include <atomic>
#include <omp.h>
#include <chrono>
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
#include <memory>

class Node_RAVE{
public:
    Node_RAVE(board b_in, board::piece_type who_in, action::place action_in)
    {
		who = who_in;
		b = b_in;
		action_taken = action_in;
		RAVE_value = 0;
		RAVE_nb = 0;
		value = 0;
		nb = 0;
		kids = NULL;
	}

	void new_kids();

	std::shared_ptr< std::vector< std::shared_ptr<Node_RAVE> > > kids;

    board::piece_type who;                  
	board b;
	action::place action_taken;

    volatile std::atomic<int> value;
	volatile std::atomic<int> nb;
	volatile std::atomic<int> RAVE_value;
	volatile std::atomic<int> RAVE_nb;                  
};

class MC_RAVE : public random_agent{
public:
	MC_RAVE(const std::string& args = "") : random_agent("name=random role=unknown " + args),
		space(board::size_x * board::size_y), who(board::empty) {

		printf("MC_RAVE player initialized\n");
		iteration_num = 1000;
		equivalence_parameter = 1000;
		total_time_limit = 39000;
		total_time_spent = 0;
		open_ratio = 2;
		open_step = 25;
		mid_ratio = 1.5;
		mid_step = 10;

		if (meta.find("T") != meta.end())
			iteration_num = int(meta["T"]);
		if (meta.find("k") != meta.end())
			equivalence_parameter = int(meta["k"]);
		if (meta.find("total_time_limit") != meta.end())
			total_time_limit = time_t(meta["total_time_limit"]);
		if (name().find_first_of("[]():; ") != std::string::npos)
			throw std::invalid_argument("invalid name: " + name());
		if (role() == "black") who = board::black;
		if (role() == "white") who = board::white;
		if (who == board::empty)
			throw std::invalid_argument("invalid role: " + role());
		for (size_t i = 0; i < space.size(); i++)
			space[i] = action::place(i, who);
		
	}

	std::shared_ptr<Node_RAVE> selection(std::shared_ptr<Node_RAVE> parent);

	int random_simulation(std::shared_ptr<Node_RAVE> start, board::piece_type who_start);

	void update_value(std::vector<std::shared_ptr<Node_RAVE> >& route, int v);

	void open_episode(const std::string& flag = "") {
		total_time_spent = 0;
	}

	void playOneSequence(std::shared_ptr<Node_RAVE> root){
		int i = 0;
		int simulation_value;
		std::vector<std::shared_ptr<Node_RAVE>> route;
		route.clear();
		route.push_back(root);
		while(1){
			//printf("route : %d\n",i);
			//printf("kids size:%d\n",route[i]->kids.size());
			//check if route[i] is a leaf
			if(route[i]->kids->size() == 0){
				//if it is a leaf that hasn't been traverse,
				//expand its kids and then do simulation
				if(route[i]->nb == 0){
					printf("it is a leaf that hasn't been traversed\n");
					route[i]->new_kids();
					printf("number of kids after new_kids : %d\n", (int)route[i]->kids->size());
					//simulate
					simulation_value = random_simulation(route[i], route[i]->who);
					//update value and nb of the last node
					route[i]->nb = route[i]->nb + 1;
					route[i]->value = simulation_value;
					route[i]->RAVE_nb = route[i]->RAVE_nb + 1;
					route[i]->RAVE_value = route[i]->RAVE_value + simulation_value;
					printf("sim_val = %d, nb = %d, value = %d\n", simulation_value, route[i]->nb.load(), route[i]->value.load());
				}
				//else it is the end of the game
				else{
					printf("it is a leaf that is the end of game\n");
					//simulate
					simulation_value = random_simulation(route[i], route[i]->who);	
					//update value and nb of the last node
					route[i]->nb = route[i]->nb + 1;
					route[i]->value = route[i]->value + simulation_value;
					route[i]->RAVE_nb = route[i]->RAVE_nb + 1;
					route[i]->RAVE_value = route[i]->RAVE_value + simulation_value;
				}
				//update the value of each node in the route
				update_value(route, -simulation_value);
				break;
			}
			//not a leaf, continue selection
			printf("select next kid\n");
			std::shared_ptr<Node_RAVE> next_kid = selection(route[i]);
			route.push_back(next_kid);
			i++;
		}
	}

	void print_tree(std::shared_ptr<Node_RAVE> root, std::shared_ptr<Node_RAVE> parent){
		int kids_len = root->kids->size();
		printf("value : %d, nb : %d, parent value : %d, parent nb : %d\n",root->value.load(),root->nb.load(),parent->value.load(),parent->nb.load());
		for(int i=0;i<kids_len;i++){
			print_tree(root->kids->operator[](i), root);
		}
	}

	/*
	void delete_tree(std::shared_ptr<Node_RAVE> root){
		int kids_len = root->kids->size();
		for(int i=0;i<kids_len;i++){
			delete_tree(root->kids->operator[](i));
		}
		delete(root);
	}
	*/

	virtual action take_action(const board& state) {
		//initialization
		//action in rootNode doesn't mean anything
		int iteration_count = 0;
		std::shared_ptr<Node_RAVE> rootNode = std::make_shared<Node_RAVE>(state, who, action());
		rootNode->new_kids();
		int rootNode_kids_len = (int)rootNode->kids->size();
		//initialize time
		time_t round_start_time = get_current_time();
		time_t round_time_limit = 2*(total_time_limit - total_time_spent)/rootNode_kids_len;
		if(rootNode_kids_len >= open_step)
			round_time_limit *= open_ratio;
		if(rootNode_kids_len >= mid_step)
			round_time_limit *= mid_ratio;
		printf("root kids number = %d\n",rootNode_kids_len);
		printf("round_time_limit = %ld\n",round_time_limit);
		printf("total_time_spent = %ld\n",total_time_spent);

		#pragma omp parallel
		{
		
		//do MCTS
		while(1){
			//printf("iteration : %d\n", i);
			printf("playOneSequence\n");
			playOneSequence(rootNode);
			//check if time runs out
			//printf("time spent = %ld\n", get_current_time() - round_start_time);
			if((get_current_time() - round_start_time) > round_time_limit){
				printf("iteration_count = %d\n",iteration_count);
				break;
			}
			iteration_count++;
		}

		}	//end of parallelization

		//print_tree(rootNode, rootNode);
		//std::cout << state << std::endl;
		action::place best_action;
		int best_visit_count = -1;
		if(rootNode_kids_len != 0){
			for(int i=0;i<rootNode_kids_len;i++){
				//printf("kid %d visited %d\n",i,rootNode->kids[i]->nb);
				if(rootNode->kids->operator[](i)->nb > best_visit_count){
					best_visit_count = rootNode->kids->operator[](i)->nb;
					best_action = rootNode->kids->operator[](i)->action_taken;
				}
			}
			//std::cout << "best action chosen = " << best_action << std::endl;
			total_time_spent += get_current_time() - round_start_time;
			//printf("total_time_spent = %ld\n",total_time_spent);
			return best_action;
		}
		else{
			std::cout << "MC_RAVE no moves left" << std::endl;
			return action();
		}
	}

	time_t get_current_time() {
		auto now = std::chrono::system_clock::now().time_since_epoch();
		return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
	}

private:
	std::vector<action::place> space;
	board::piece_type who;
	int iteration_num;
	int equivalence_parameter;
	time_t total_time_limit;
	time_t total_time_spent;
	float open_ratio;
	float mid_ratio;
	int open_step;
	int mid_step;
};