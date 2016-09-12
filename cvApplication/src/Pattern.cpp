/*
 * Pattern.cpp
 *
 *  Created on: 2016. aug. 2.
 *      Author: Máté
 */

#include "Pattern.h"

Pattern::Pattern() {
	// TODO Auto-generated constructor stub

}

Pattern::Pattern(std::string identifier , std::vector<int> values){
	id = identifier;
	pattern = values;
}

std::string Pattern::getId(){
	return id;
}

std::vector<int> Pattern::getValues(){
	return pattern;
}

Pattern::~Pattern() {
	// TODO Auto-generated destructor stub
}

