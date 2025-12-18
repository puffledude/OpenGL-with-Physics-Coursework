#include "NetworkObject.h"
#include "PhysicsObject.h"
#include "./enet/enet.h"
#include <cmath>
using namespace NCL;
using namespace CSC8503;

NetworkObject::NetworkObject(GameObject& o, int id) : object(o) {
	deltaErrors = 0;
	fullErrors = 0;
	networkID = id;
}

NetworkObject::~NetworkObject() {
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
	Vector3 fullvel = lastFullState.velocity;

	fullpos.x += (float)p.pos[0];
	fullpos.y += (float)p.pos[1];
	fullpos.z += (float)p.pos[2];
	
	fullvel.x += (float)p.vel[0];
	fullvel.y += (float)p.vel[1];
	fullvel.z += (float)p.vel[2];

	fullor.x += ((float)p.orientation[0]) / 127.0f;
	fullor.y += ((float)p.orientation[1]) / 127.0f;
	fullor.z += ((float)p.orientation[2]) / 127.0f;
	fullor.w += ((float)p.orientation[3]) / 127.0f;

	object.GetTransform().SetPosition(fullpos);
	object.GetPhysicsObject()->SetLinearVelocity(fullvel);
	object.GetTransform().SetOrientation(fullor);

	return true;
}

bool NetworkObject::ReadFullPacket(FullPacket &p) {
	if (p.fullState.stateID<= lastFullState.stateID) {
		return false; //we've already seen this or a newer state!
	}

	lastFullState = p.fullState;

	object.GetTransform().SetPosition(lastFullState.position);
	object.GetPhysicsObject()->SetLinearVelocity(lastFullState.velocity);
	object.GetTransform().SetOrientation(lastFullState.orientation);
	stateHistory.push_back(lastFullState);
	return true;
}

static inline int CharClamp(float v) {
	if (v > 127.0f) return 127;
	if (v < -128.0f) return -128;
	return (int)std::lround(v);
}

bool NetworkObject::WriteDeltaPacket(GamePacket**p, int stateID) {
	DeltaPacket* dp = new DeltaPacket();
	NetworkState state;
	if (!GetNetworkState(stateID, state)) { delete dp; return false; }
	// fullID should reference the base state's id that this delta is relative to
	dp->fullID = state.stateID;
	dp->objectID = networkID;

	Vector3 currentPos = object.GetTransform().GetPosition();
	Quaternion currentOr = object.GetTransform().GetOrientation();

	Vector3 posDelta = currentPos - state.position;
	Vector3 velDelta = object.GetPhysicsObject()->GetLinearVelocity() - state.velocity;
	Quaternion orDelta;
	orDelta.x = currentOr.x - state.orientation.x;
	orDelta.y = currentOr.y - state.orientation.y;
	orDelta.z = currentOr.z - state.orientation.z;
	orDelta.w = currentOr.w - state.orientation.w;

	// store deltas in compact char form (same packing as read path expects)
	dp->pos[0] = (char)CharClamp(posDelta.x);
	dp->pos[1] = (char)CharClamp(posDelta.y);
	dp->pos[2] = (char)CharClamp(posDelta.z);

	dp->vel[0] = (char)CharClamp(velDelta.x);
	dp->vel[1] = (char)CharClamp(velDelta.y);
	dp->vel[2] = (char)CharClamp(velDelta.z);

	dp->orientation[0] = (char)CharClamp(orDelta.x * 127.0f);
	dp->orientation[1] = (char)CharClamp(orDelta.y * 127.0f);
	dp->orientation[2] = (char)CharClamp(orDelta.z * 127.0f);
	dp->orientation[3] = (char)CharClamp(orDelta.w * 127.0f);
	*p = dp;
	return true;
	
}

bool NetworkObject::WriteFullPacket(GamePacket**p) {
	FullPacket* fp = new FullPacket();
	fp->objectID = networkID;
	fp->fullState.position = object.GetTransform().GetPosition();
	fp->fullState.velocity = object.GetPhysicsObject()->GetLinearVelocity();
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