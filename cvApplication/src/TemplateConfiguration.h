/*
 * TemplateConfiguration.h
 *
 *  Created on: 2016. aug. 16.
 *      Author: Máté
 */

#ifndef TEMPLATECONFIGURATION_H_
#define TEMPLATECONFIGURATION_H_

enum MarkerType{
	ARUCO,
	CIRCLE
};

template<typename data , typename identifier>
struct TEMPLATE_CONFIG{
	using dataType = data;
	using identifierType = identifier;
};

#endif /* TEMPLATECONFIGURATION_H_ */
