#pragma once
#include "Unit.h"

class ProtossProbe : public Unit
{
protected:
	UnitRef MineTarget;

public:
	enum State
	{
		IDLE,
		MOVE,
		MINING,
	};

	ProtossProbe(RoomRef room, int objectId, Vector3 pos);

	void MiningMineral(UnitRef target);

	State currentStateProbe;
};

