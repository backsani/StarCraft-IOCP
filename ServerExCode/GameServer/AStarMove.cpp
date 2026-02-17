#include "pch.h"
#include "AStarMove.h"


AStarMove::AStarMove(int32 width, int32 height, const vector<uint16>& map) : mapWidth(width), mapHeight(height), map(map)
{
	
}

bool AStarMove::aStar(int startX, int startY, int endX, int endY, vector<pair<int, int>>& out)
{
	// 다음 경로 탐색을 대기하는 Node들. 우선순위 큐로 가능성이 높은 Node를 위주로 탐색
	priority_queue<shared_ptr<Node>, vector<shared_ptr<Node>>, Compare> openList;
	// 방문한 적이 있는지 검사
	unordered_map<Node, bool, NodeHash> closedList;

	// 현재 노드와 이동하기 전 노드를 map으로 매핑, 
	unordered_map<Node, shared_ptr<Node>, NodeHash> cameFrom;

	shared_ptr<Node> start = MakeShared<Node>(startX, startY);

	// 초기 노드 초기화
	start->g = 0;
	start->h = heuristic(startX, startY, endX, endY);
	start->f = start->g + start->h;

	// 초기값
	openList.push(start);
	cameFrom[*start] = start;

	// 만약 우선순위 큐가 비었다면, 모든 경로를 탐색했지만 목표를 못 찾은 것이기에 false를 반환.
	while (!openList.empty())
	{
		// 우선순위로 f가 가장 낮은 Node 추출
		shared_ptr<Node> current = openList.top();
		openList.pop();

		// 만약 현재 노드가 목표라면
		if (current->x == endX && current->y == endY)
		{
			shared_ptr<Node> node = current;
			// 현재 노드의 전 노드가 nullptr일때까지 전 노드 탐색
			while (node != nullptr)
			{
				// 현재 노드를 경로에 저장
				out.push_back({ node->x, node->y });
				// 현재 노드에 매핑된 노드(전 노드)를 현재 노드로 저장.
				node = cameFrom.find(*node)->second->parent;
			}
			// 경로를 꺼내면서 사용해야하기에 역순으로 저장하는 것이 pop_back에서 성능적으로 좋음.
			return true;
		}

		// 방문한 적이 있다면 이미 먼저 방문한 것이 최적의 경로이기에 확인할 필요 없음.
		if (closedList.find(*current) != closedList.end())
			continue;

		// 현재 노드를 방문처리
		closedList[*current] = true;

		// 현재 노드에서 8방향으로 노드 탐색
		for (int i = 0; i < 8; i++)
		{
			int nx = current->x + dx[i];
			int ny = current->y + dy[i];

			// 이동 불가면 건너뛰기
			if (!canMove(current->x, current->y, nx, ny))
				continue;

			// 해당 방향으로 이동하는데 드는 비용을 지금까지 들었던 비용에 더해서 이동하면 시작점에서 얼마만큼의 비용이 들었는지 계산
			float tentative_g = current->g + moveCost[i];

			Node temp(nx, ny);
			auto it = cameFrom.find(temp);

			// 다음 노드에 이미 방문했거나 이동이 거의 없다면 패스
			bool better = (it == cameFrom.end() || tentative_g < it->second->g - 1e-6f);

			if (better)
			{
				// 다음 노드 정보를 메모리에 생성
				shared_ptr<Node> neighbor = MakeShared<Node>(nx, ny);

				// 다음 노드의 g,h,f, 이전 경로 노드를 저장
				neighbor->g = tentative_g;
				neighbor->h = heuristic(nx, ny, endX, endY);
				neighbor->f = neighbor->g + neighbor->h;
				neighbor->parent = current;

				// 해당 정보 저장
				cameFrom[*neighbor] = neighbor;

				if (closedList.find(*neighbor) != closedList.end())
				{
					closedList.erase(*neighbor);
				}

				// 우선순위 큐에 다음 노드 저장.
				openList.push(neighbor);
			}
		}
	}

	return false;
}

// 다음 벽으로 이동이 가능하지 판별하는 함수
bool AStarMove::canMove(int x1, int y1, int x2, int y2)
{
	// 만약 맵을 벗어나거나 맵의 높이가 0(이동불가)일 경우 불가 판정
	if (x2 < 0 || y2 < 0 || x2 >= mapWidth || y2 >= mapHeight || map[x2 + mapWidth * y2] == 0)
		return false;

	// 현재 위치와 다음 위치의 높이 구하기
	int h1 = map[x1 + mapWidth * y1];
	int h2 = map[x2 + mapWidth * y2];

	// 높이가 2이상이면 이동 불가 판정
	if (abs(h1 - h2) > 1)
		return false;

	int dx = x2 - x1;
	int dy = y2 - y1;

	// 만약 대각선으로 이동할 때 
	if (dx != 0 && dy != 0)
	{
		// 코너 통과가 가능한지
		bool hor_ok = (map[(x1 + dx) + mapWidth * y1] != 0) &&
			(abs(h1 - map[(x1 + dx) + mapWidth * y1]) <= 1);

		bool ver_ok = (map[x1 + mapWidth * (y1 + dy)] != 0) &&
			(abs(h1 - map[x1 + mapWidth * (y1 + dy)]) <= 1);

		// 만약 코너가 둘다 막혀있다면 이동 불가 판정
		if (!hor_ok || !ver_ok)
			return false;
	}

	return true;
}

// 현재 위치에서 목표까지 휴리스틱 구하기, 대각선 계산 포함
float AStarMove::heuristic(int x1, int y1, int x2, int y2)
{
	int dx = abs(x1 - x2);
	int dy = abs(y1 - y2);

	int diag = min(dx, dy);
	int straight = dx + dy - 2 * diag;

	return 1.4f * diag + 1.0f * straight;
}
