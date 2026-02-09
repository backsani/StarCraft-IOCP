#pragma once

struct Node
{
	int x;
	int y;

	float g;
	float h;
	float f;

	shared_ptr<Node> parent;

	Node(int x, int y) : x(x), y(y), g(0), h(0), f(0), parent(nullptr) {}

	bool operator==(const Node& other) const
	{
		return x == other.x && y == other.y;
	}
};

// 사용자 정의 구조체 unordered_map을 사용하기 위한 해시함수
struct NodeHash {
	size_t operator()(const Node& n) const noexcept {
		return hash<int>()(n.x) ^ (hash<int>()(n.y) << 1);
	}
};

// 우선순위 큐의 비교 함수.
// 생성할때 넘겨주는데 이때는 함수 포인터를 사용할 수 없음, 때문에 구조체로 구현해서 구조체 내에 함수를 구현해 넘겨준다.
struct Compare
{
	bool operator()(const shared_ptr<Node> a, const shared_ptr<Node> b)
	{
		if (abs(a->f - b->f) < 1e-6f)
			return a->h > b->h;
		return a->f > b->f;
	}
};

class AStarMove
{
public:
	AStarMove(int32 width, int32 height, const vector<uint16>& map);

	bool aStar(int startX, int startY, int endX, int endY, vector<pair<int, int>>& out);

private:
	int32 mapWidth;
	int32 mapHeight;

	const vector<uint16>& map;
	
	int dx[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };
	int dy[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };

	float moveCost[8] = { 1.4f, 1.0f, 1.4f, 1.0f, 1.0f, 1.4f, 1.0f, 1.4f };

	// 다음 노드로 이동이 가능한지 검사해주는 함수
	bool canMove(int x1, int y1, int x2, int y2);

	// 다음 노드까지의 거리
	float heuristic(int x1, int y1, int x2, int y2);
};

