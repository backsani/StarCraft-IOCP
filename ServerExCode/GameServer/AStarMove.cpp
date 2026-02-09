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

	unordered_map<Node, shared_ptr<Node>, NodeHash> cameFrom;

	shared_ptr<Node> start = MakeShared<Node>(startX, startY);

	start->g = 0;
	start->h = heuristic(startX, startY, endX, endY);
	start->f = start->g + start->h;

	// 초기값
	openList.push(start);
	cameFrom[*start] = start;

	while (!openList.empty())
	{
		shared_ptr<Node> current = openList.top();
		openList.pop();

		if (current->x == endX && current->y == endY)
		{
			shared_ptr<Node> node = current;
			while (node != nullptr)
			{
				out.push_back({ node->x, node->y });
				node = cameFrom.find(*node)->second->parent;
			}
			// 경로를 꺼내면서 사용해야하기에 역순으로 저장하는 것이 pop_back에서 성능적으로 좋음.
			return true;
		}

		if (closedList.find(*current) != closedList.end())
			continue;

		closedList[*current] = true;

		for (int i = 0; i < 8; i++)
		{
			int nx = current->x + dx[i];
			int ny = current->y + dy[i];

			// 이동 불가면 건너뛰기
			if (!canMove(current->x, current->y, nx, ny))
				continue;

			float tentative_g = current->g + moveCost[i];

			Node temp(nx, ny);
			auto it = cameFrom.find(temp);

			bool better = (it == cameFrom.end() || tentative_g < it->second->g - 1e-6f);

			if (better)
			{
				shared_ptr<Node> neighbor = MakeShared<Node>(nx, ny);

				neighbor->g = tentative_g;
				neighbor->h = heuristic(nx, ny, endX, endY);
				neighbor->f = neighbor->g + neighbor->h;
				neighbor->parent = current;

				cameFrom[*neighbor] = neighbor;

				if (closedList.find(*neighbor) != closedList.end())
				{
					closedList.erase(*neighbor);
				}

				openList.push(neighbor);
			}
		}
	}

	return false;
}

bool AStarMove::canMove(int x1, int y1, int x2, int y2)
{
	if (x2 < 0 || y2 < 0 || x2 >= mapWidth || y2 >= mapHeight || map[x2 + mapWidth * y2] == 0)
		return false;

	int h1 = map[x1 + mapWidth * y1];
	int h2 = map[x2 + mapWidth * y2];

	if (abs(h1 - h2) > 1)
		return false;

	int dx = x2 - x1;
	int dy = y2 - y1;

	if (dx != 0 && dy != 0)
	{
		bool hor_ok = (map[(x1 + dx) + mapWidth * y1] != 0) &&
			(abs(h1 - map[(x1 + dx) + mapWidth * y1]) <= 1);

		bool ver_ok = (map[x1 + mapWidth * (y1 + dy)] != 0) &&
			(abs(h1 - map[x1 + mapWidth * (y1 + dy)]) <= 1);

		if (!hor_ok || !ver_ok)
			return false;
	}

	return true;
}

float AStarMove::heuristic(int x1, int y1, int x2, int y2)
{
	int dx = abs(x1 - x2);
	int dy = abs(y1 - y2);

	int diag = min(dx, dy);
	int straight = dx + dy - 2 * diag;

	return 1.4f * diag + 1.0f * straight;
}
