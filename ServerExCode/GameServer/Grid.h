#pragma once

class Grid;
using GridRef = shared_ptr<Grid>;

class GridManager;
using GridManagerRef = shared_ptr<GridManager>;

class GameObject;
using GameObjectRef = shared_ptr<GameObject>;

struct AABB
{
	float minX, minY;
	float maxX, maxY;

	AABB() : minX(0), minY(0), maxX(0), maxY(0) {}

	AABB(float minx, float miny, float maxx, float maxy)
	{
		minX = minx;
		minY = miny;
		maxX = maxx;
		maxY = maxy;
	}

	bool Collider(const AABB& other) const
	{
		return !(maxX <= other.minX || minX >= other.maxX ||
			maxY <= other.minY || minY >= other.maxY);
	}

};

class Grid
{
public:
	map<int, GameObjectRef> objects;
	AABB bounds;

	Grid();
	Grid(const AABB& bounds);
	~Grid();

	void Add(GameObjectRef obj);
	void Remove(GameObjectRef obj);
};

class GridManager
{
	float gridWidth;
	float gridHeight;
	int gridCols;
	int gridRows;

	vector<vector<GridRef>> grids;

public:
	GridManager(float worldWidth, float worldHeight, int cols, int rows);
	~GridManager();

	void AddObject(GameObjectRef obj);
	void RemoveObject(GameObjectRef obj);
	void MoveObject(GameObjectRef obj, const AABB& bounds);

	void GetOverlappingCells(const AABB& bounds, std::vector<std::pair<int, int>>& outCells);

	void FindAllColliders(vector<pair<GameObjectRef, GameObjectRef>>& collisions);
};