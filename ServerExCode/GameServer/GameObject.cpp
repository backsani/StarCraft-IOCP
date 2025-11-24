#include "pch.h"
#include "GameObject.h"
#include "ClientPacketHandler.h"
#include "Room.h"

/*------------------------------------------
****************GameObject******************
------------------------------------------*/

GameObject::GameObject(RoomRef room, int id, float hp, GameObjectCode objectCode, Vector3 position, Vector3 direction, float speed) : room(room), id(id), hp(hp), objectCode(objectCode), position(position), direction(direction), speed(speed)
{
	currentState = GameObjectState::IDLE;
	width = 0.0f;
	height = 0.0f;
	spawnTime = 0;
	damage = 0;
}

SendBufferRef GameObject::Update()
{
	SendBufferRef sendBuffer = nullptr;

	switch (currentState)
	{
	case GameObjectState::IDLE:
		break;
	case GameObjectState::MOVE:
		sendBuffer = move();
		break;
	case GameObjectState::ATTACK:
		attack();
		break;
	case GameObjectState::DEAD:
	{
		JobRef job = MakeShared<Job>(GetId(), Vector3(), Vector3(), GameObjectState::DEAD);
		sendBuffer = dead();
		room->jobQueue.push(job);

		break;
	}
	case GameObjectState::SPAWN:
		sendBuffer = spawn();
		break;
	default:
		break;
	}

	return sendBuffer;
}

SendBufferRef GameObject::move()
{
	SendBufferRef sendBuffer = nullptr;

	position += direction * speed;

	UpdateBounds();

	Protocol::S_MOVE packet;

	packet.set_objectid(GetId());
	packet.set_state((Protocol::GameObjectState)currentState);

	Protocol::Vector3* position = packet.mutable_position();
	Vector3 pos = GetPosition();
	position->set_x(pos.x);
	position->set_y(pos.y);
	position->set_z(pos.z);

	sendBuffer = ClientPacketHandler::MakeSendBuffer(packet);

	return sendBuffer;
}

SendBufferRef GameObject::spawn()
{
	SendBufferRef sendBuffer = nullptr;

	Protocol::S_OBJECT_SPAWN packet;

	Protocol::ObjectData* data = packet.mutable_objectdata();
	data->set_type((Protocol::ObjectType)GetObjectCode());
	data->set_objectid(GetId());

	Protocol::Vector3* position = data->mutable_position();
	Vector3 pos = GetPosition();
	position->set_x(pos.x);
	position->set_y(pos.y);
	position->set_z(pos.z);

	Protocol::Vector3* direction = data->mutable_direction();
	Vector3 dir = GetDirection();
	direction->set_x(dir.x);
	direction->set_y(dir.y);
	direction->set_z(dir.z);

	spawnTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	packet.set_spawntime(spawnTime);

	sendBuffer = ClientPacketHandler::MakeSendBuffer(packet);

	return sendBuffer;
}

SendBufferRef GameObject::dead()
{
	SendBufferRef sendBuffer = nullptr;
	Protocol::S_OBJECT_DEAD packet;

	packet.set_objectid(GetId());
	packet.set_state((Protocol::GameObjectState)GetObjectCode());

	sendBuffer = ClientPacketHandler::MakeSendBuffer(packet);

	return sendBuffer;
}

void GameObject::SetMove(GameObjectState state, Vector3 direction)
{
	this->direction = direction;
	currentState = state;
}

void GameObject::SetAttack(Vector3 targetPos)
{
	currentState = GameObjectState::ATTACK;
}



void GameObject::attack()
{

}

void GameObject::UpdateBounds()
{
	bounds = AABB(
		position.x - width / 2, position.y - height / 2,
		position.x + width / 2, position.y + height / 2
	);
}

void GameObject::TakeDamage(float damage)
{
	hp -= damage;
	Protocol::S_OBJECT_DAMAGE packet;

	packet.set_objectid(GetId());
	packet.set_hp(hp);

	SendBufferRef data = ClientPacketHandler::MakeSendBuffer(packet);

	room->Broadcast(data);
}


/*------------------------------------------
******************Player********************
------------------------------------------*/

Player::Player(RoomRef room, int id, Vector3 position) : GameObject(room, id, 100.f, GameObjectCode::PLAYER, position)
{
	width = 1.0f;
	height = 1.0f;
}

Player::~Player()
{
}



/*------------------------------------------
******************Bullet********************
------------------------------------------*/

Bullet::Bullet(RoomRef room, int id, Vector3 position, Vector3 direction, GameObjectRef shooter) : GameObject(room, id, 1.0f, GameObjectCode::BULLET, position, direction, 0.3f), shooter(shooter)
{
	currentState = GameObjectState::SPAWN;
	lifeTick = 120.0f;
	width = 0.1f;
	height = 0.2f;
	damage = 5.0f;
}

SendBufferRef Bullet::spawn()
{
	currentState = GameObjectState::MOVE;

	return GameObject::spawn();
}

SendBufferRef Bullet::move()
{
	SendBufferRef sendBuffer = GameObject::move();
	if (lifeTick % 20 != 0)
	{
		sendBuffer = nullptr;
	}

	lifeTick -= 1;

	if (lifeTick <= 0)
	{
		SetDead();
	}

	return sendBuffer;
}

Bullet::~Bullet()
{
}
