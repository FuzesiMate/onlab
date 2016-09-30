/*
 * Pattern.h
 *
 *  Created on: 2016. aug. 2.
 *      Author: Máté
 */

#ifndef SRC_PATTERN_H_
#define SRC_PATTERN_H_

#include <iostream>
#include <vector>

class Pattern {
private:
	std::string id;
	std::vector<int> pattern;
public:
	Pattern();
	Pattern(std::string identifier , std::vector<int> values);
	std::string getId();
	std::vector<int> getValues();
	virtual ~Pattern();
};

#endif /* SRC_PATTERN_H_ */
