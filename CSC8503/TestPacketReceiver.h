#pragma once
#include <string>
#include <iostream>
#include"NetworkBase.h"


class TestPacketReceiver : public PacketReceiver {
public:
	TestPacketReceiver(std::string name) {
		this->name = name;
	}

	void ReceivePacket(int type, GamePacket* payload, int source) {
		if (type == String_Message) {
			StringPacket* realPacket = (StringPacket*)payload;

			std::string message = realPacket->GetStringFromDta();
			std::cout << "Packet Receiver " << name << " received message: " << message << std::endl;
		}
	}

protected:
	std::string name;

};