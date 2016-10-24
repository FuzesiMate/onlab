#ifndef DATASENDERFACTORY_H_
#define DATASENDERFACTORY_H_

#include <tbb/flow_graph.h>
#include <boost/property_tree/ptree.hpp>
#include <exception>
#include "ZeroMQDataSender.h"

#define TYPE			"type"
#define TOPIC			"topic"
#define BROKER_URL		"broker_url"
#define BIND_ADRESSES	"bind_addresses"

enum DataSenderType {
	ZEROMQ,
	MQTT
};

std::map<std::string, DataSenderType> res_DataSenderType = { {"zeromq",DataSenderType::ZEROMQ },{"mqtt", DataSenderType::MQTT} };

class DataSenderFactory {
public:
	DataSenderFactory() = delete;

	static std::shared_ptr<DataSender> createDataSender(boost::property_tree::ptree parameters, tbb::flow::graph& g) {
		auto senderType = res_DataSenderType[parameters.get<std::string>(TYPE)];

		std::shared_ptr<DataSender> sender;

		switch (senderType) {
		case DataSenderType::ZEROMQ:
		{
			auto topic = parameters.get<std::string>(TOPIC);

			auto zeromqSender = std::make_shared<ZeroMQDataSender>(topic , g);

			for (auto& address : parameters.get_child(BIND_ADRESSES)) {

				zeromqSender->bindAddress(address.second.get<std::string>(""));
			}

			for (auto& object : parameters.get_child(OBJECTS)) {
				zeromqSender->addObject(object.second.get<std::string>(""));
			}

			sender = zeromqSender;

			break;
		}
		case DataSenderType::MQTT:
			break;
		default:
			throw std::exception("Unsupported data sender type!");
			break;
		}

		return sender;
	}

	virtual ~DataSenderFactory() = default;
};

#endif