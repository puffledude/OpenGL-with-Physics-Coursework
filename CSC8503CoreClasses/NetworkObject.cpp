#include "NetworkObject.h"
#include "./enet/enet.h"
using namespace NCL;
using namespace CSC8503;

NetworkObject::NetworkObject(GameObject& o, int id) : object(o)	{
	deltaErrors = 0;
	fullErrors  = 0;
	networkID   = id;
}

NetworkObject::~NetworkObject()	{
}

bool NetworkObject::ReadPacket(GamePacket& p) {
	if (p.type == Delta_State) {
		return ReadDeltaPacket((DeltaPacket&)p);
	}
	if (p.type == Full_State) {
		return ReadFullPacket((FullPacket&)p);
	}
	return false; //this isn't a packet we care about!
}

bool NetworkObject::WritePacket(GamePacket** p, bool deltaFrame, int stateID) {
	if (deltaFrame) {
		if (!WriteDeltaPacket(p, stateID)) {
			return WriteFullPacket(p);
		}
	}
	return WriteFullPacket(p);
}
//Client objects recieve these packets
bool NetworkObject::ReadDeltaPacket(DeltaPacket &p) {
	if(p.fullID != lastFullState.stateID) {
		return false; //we missed a full packet!
	}

	Vector3 fullpos = lastFullState.position;
	Quaternion fullor = lastFullState.orientation;

	fullpos.x += (float)p.pos[0];
	fullpos.y += (float)p.pos[1];
	fullpos.z += (float)p.pos[2];

	fullor.x += ((float)p.orientation[0]) / 127.0f;
	fullor.y += ((float)p.orientation[1]) / 127.0f;
	fullor.z += ((float)p.orientation[2]) / 127.0f;
	fullor.w += ((float)p.orientation[3]) / 127.0f;

	object.GetTransform().SetPosition(fullpos);
	object.GetTransform().SetOrientation(fullor);

	return true;
}

bool NetworkObject::ReadFullPacket(FullPacket &p) {
	if (p.fullState.stateID<= lastFullState.stateID) {
		return false; //we've already seen this or a newer state!
	}

	lastFullState = p.fullState;

	object.GetTransform().SetPosition(lastFullState.position);
	object.GetTransform().SetOrientation(lastFullState.orientation);
	stateHistory.push_back(lastFullState);
	return true;
}

bool NetworkObject::WriteDeltaPacket(GamePacket**p, int stateID) {
	DeltaPacket* dp = new DeltaPacket();
	NetworkState state;
	if (!GetNetworkState(stateID, state)) { return false; }
	dp->fullID = networkID;
	dp->objectID = networkID;

	Vector3 currentPos = object.GetTransform().GetPosition();
	Quaternion currentOr = object.GetTransform().GetOrientation();

	dp->pos[0] = (char)currentPos.x;
	dp->pos[1] = (char)currentPos.y;
	dp->pos[2] = (char)currentPos.z;

	dp->orientation[0] = (char)(currentOr.x*127.0f);
	dp->orientation[1] = (char)(currentOr.y * 127.0f);
	dp->orientation[2] = (char)(currentOr.z * 127.0f);
	dp->orientation[3] = (char)(currentOr.w * 127.0f);
	*p = dp;
	return true;
	
}

bool NetworkObject::WriteFullPacket(GamePacket**p) {
	FullPacket* fp = new FullPacket();
	fp->objectID = networkID;
	fp->fullState.position = object.GetTransform().GetPosition();
	fp->fullState.orientation = object.GetTransform().GetOrientation();
	fp->fullState.stateID = lastFullState.stateID++;
	*p = fp;
	return true;
}

NetworkState& NetworkObject::GetLatestNetworkState() {
	return lastFullState;
}

bool NetworkObject::GetNetworkState(int stateID, NetworkState& state) {

	for (auto i = stateHistory.begin(); i < stateHistory.end(); ++i) {
		if (i->stateID == stateID) {
			state = *i;
			return true;
		}
	}

	return false;
}

void NetworkObject::UpdateStateHistory(int minID) {

	for (auto i = stateHistory.begin(); i != stateHistory.end();) {
		if (i->stateID < minID) {
			i = stateHistory.erase(i);
		}
		else {
			++i;
		}
	}
}