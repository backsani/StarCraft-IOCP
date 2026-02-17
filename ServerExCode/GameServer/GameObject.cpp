#include "pch.h"
#include <cmath>
#include "GameObject.h"
#include "ClientPacketHandler.h"
#include "Room.h"
#include "Unit.h"
#include "ProtossUnit.h"
#include "ProtossBuilding.h"
#include "AStarMove.h"

/*------------------------------------------
****************GameObject******************
------------------------------------------*/

GameObject::GameObject(RoomRef room, int id, float hp, GameObjectCode objectCode, Vector3 position, Vector3 direction, float speed, Race race) : room(room), id(id), hp(hp), objectCode(objectCode), position(position), direction(direction), speed(speed), race(race)
{
	this->position = position;
	moveTarget = position;
	currentState = GameObjectState::IDLE;
	width = 0.0f;
	height = 0.0f;
	spawnTime = 0;
	damage = 0;
	ownerId = -1;
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

void GameObject::attack()
{
}

SendBufferRef GameObject::move()
{
	SendBufferRef sendBuffer = nullptr;

	// 다음 경로가 없다면 목표 도착 완료
	if (path.size() == 0)
	{
		// 현재 상태를 IDLE로 바꾸고 nullptr 반환(nullptr은 패킷을 보내지 않음)
		currentState = GameObjectState::IDLE;
		return sendBuffer;
	}
	// 가장 뒤에 있는 원소(다음 위치) 추출
	// 앞에서 빼지 않는 이유는 vector는 자료구조 특성상 앞에서 뺀다면 재배치 때문에 성능이 좋지 않음.
	pair<int, int> next = path.back();
	
	Vector3 nextPosition((float)next.first, (float)next.second, 0);

	cout << position.x << ", " << position.y << " : " << nextPosition.x << ", " << nextPosition.y << endl;

	// 목표 위치의 방향 구하기
	Vector3 toTarget = nextPosition - position;
	float distanceSq = toTarget.x * toTarget.x + toTarget.y * toTarget.y;

	// 이를 제곱하고 루트를 씌어서 정규화
	float dist = sqrt(distanceSq);

	// GameObject의 속도에 게임 프레임(임시)를 곱한 값이 목표의 정규화 방향보다 길다면 목표를 넘어서 움직인다는 것이기에 현재 위치를 목표로 설정하고 경로에 도착하였으므로 경로 제거.
	if (speed * 0.1f >= dist)
	{
		position = nextPosition;
		path.pop_back();
	}
	
	// 목표 위치로 정규화를 나누어 다음 위치까지 1프레임 후 얼마만큼 이동하는지 계산
	float nx = toTarget.x / dist;
	float ny = toTarget.y / dist;

	// 현재 위치를 이동한 수치만큼 증감.
	position.x += nx * speed * 0.1f;
	position.y += ny * speed * 0.1f;

	// 이동이 완료했으니 총돌 범위를 현재 위치로 업데이트
	UpdateBounds();

	// 오브젝트의 이동 패킷 생성
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

	data->set_playerid(ownerId);

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

	data->set_hp(hp);

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

void GameObject::SetMove(GameObjectState state, Vector3 position, Vector3 target)
{
	// 기존의 저장된 경로 초기화
	path.clear();

	// 패킷에게 받은 위치를 현재 위치로 설정
	this->position = position;

	// 타겟을 보는 방향 구하기
	Vector3 direction = position - target;
	// 방향의 제곱 길이
	float lengthSq = direction.x * direction.x + direction.y * direction.y + direction.z * direction.z;

	// 상태가 IDLE 이거나 목표까지의 거리가 너무 작다면
	if (state == GameObjectState::IDLE || lengthSq <= 0.000001f)
	{
		// 상태를 IDLE로 유지하고 목표를 현재 위치로 설정
		currentState = GameObjectState::IDLE;
		this->direction = { 0,0,0 };
		moveTarget = position;
		return;
	}

	// aStar 객체를 생성(매번 생성하는 것이 아닌 room이나 roomManager에게 한번만 생성시키는 방향으로 개선 예정)
	shared_ptr<AStarMove> aStar = MakeShared<AStarMove>(room->mapSectionData->mapWidth, room->mapSectionData->mapHeight, room->mapSectionData->height);

	// aStar 알고리즘 실행, 이후 경로를 저장
	if (!aStar->aStar((int)position.x, (int)position.y, (int)target.x, (int)target.y, path))
	{
		currentState = GameObjectState::IDLE;
		this->direction = { 0,0,0 };
		moveTarget = position;

		cout << "aStar errer" << endl;

		return;
	}

	// 마지막 경로(현재 위치) 제거
	path.pop_back();

	// 작동 디버깅용 
	for (pair<int, int> p : path)
	{
		cout << p.first << ", " << p.second << endl;
	}

	// 현재 상태를 Move로 지정
	currentState = state;
}

void GameObject::SetAttack(Vector3 targetPos)
{
	currentState = GameObjectState::ATTACK;
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

float Clamp(float v, float minV, float maxV)
{
	if (v < minV) return minV;
	if (v > maxV) return maxV;
	return v;
}
