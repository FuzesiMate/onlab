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
		
		std::shared_ptr<DataSender<INPUT> > dataSender;
		
		try{
			DataSenderType senderType = res_DataSenderType[parameters.get<std::string>(TYPE)];

			switch (senderType) {
			case DataSenderType::ZEROMQ:
			{
				auto topic = parameters.get<std::string>(TOPIC);
				
				std::vector<std::string> addresses;
				for (auto& address : parameters.get_child(BIND_ADRESSES)) {
					addresses.push_back(address.second.get<std::string>(""));
				}
				dataSender = std::make_shared<ZeroMQDataSender<INPUT> >(topic , addresses ,g);
				break;
			}
			case DataSenderType::MQTT:
				throw std::exception("MQTT data sender is not implemented!");
				break;
			default:
				throw std::exception("Unsupported data sender type!");
				break;
			}
		}
		catch (std::exception& e) {
			throw e;
		}
		
		return dataSender;
	}

	virtual ~DataSenderFactory() = default;
};

#endif