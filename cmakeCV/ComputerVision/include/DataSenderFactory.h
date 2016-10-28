#ifndef DATASENDERFACTORY_H_
#define DATASENDERFACTORY_H_

#include <tbb/flow_graph.h>
#include <boost/property_tree/ptree.hpp>
#include <exception>
#include "ZeroMQDataSender.h"
#include "ZeroMQDataSender.cpp"
#include "DataTypes.h"

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

	template<typename INPUT>
	static std::shared_ptr<DataSender<INPUT> > createDataSender(boost::property_tree::ptree parameters, tbb::flow::graph& g) {
		
		DataSenderType senderType;

		try{
			senderType = res_DataSenderType[parameters.get<std::string>(TYPE)];
		}
		catch (std::exception& e) {
			throw e;
		}
		
		std::shared_ptr<DataSender<INPUT> > sender;

		switch (senderType) {
		case DataSenderType::ZEROMQ:
		{
			try {
				auto topic = parameters.get<std::string>(TOPIC);

				auto zeromqSender = std::make_shared<ZeroMQDataSender<INPUT> >(topic, g);

				for (auto& address : parameters.get_child(BIND_ADRESSES)) {

					zeromqSender->bindAddress(address.second.get<std::string>(""));
				}

				sender = zeromqSender;
			}
			catch (std::exception& e) {
				throw e;
			}

			break;
		}
		case DataSenderType::MQTT:
			throw std::exception("MQTT data sender is not implemented!");
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