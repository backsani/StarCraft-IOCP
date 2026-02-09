#include "pch.h"
#include "ProtossUnit.h"
#include "GameObject.h"
#include "Unit.h"

ProtossProbe::ProtossProbe(RoomRef room, int objectId, Vector3 pos) : Unit(room, objectId, 20, pos, GameObjectCode::PROBE, 2.3f, Race::PROTOSS, 50.0f, 1.0f, 0.126f)
{
	currentStateProbe = State::IDLE;
}

void ProtossProbe::MiningMineral(UnitRef target)
{
	if (bounds.Collider(target->bounds))
	{
		if (target->GetObjectCode() != GameObjectCode::MINERAL)
			return;

		/*shared_ptr<Mineral> mineral = static_pointer_cast<Mineral>(target);

		if (mineral->TryMining(GetId()))
		{
			SetAttack(target);
			
		}*/
	}
	else
	{
		SetMove(GameObjectState::MOVE, target->GetPosition(), GetPosition());
	}
	
}
