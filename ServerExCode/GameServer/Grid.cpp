#include "pch.h"
#include "Grid.h"
#include "GameObject.h"
#include <algorithm>


Grid::Grid()
{
    this->bounds = AABB(0, 0, 0, 0);
}

Grid::Grid(const AABB& bounds)
{
    this->bounds = bounds;
}

Grid::~Grid()
{
}

void Grid::Add(GameObjectRef obj)
{
	objects[obj->GetId()] = obj;
}

void Grid::Remove(GameObjectRef obj)
{
	objects.erase(obj->GetId());
}

GridManager::GridManager(float worldWidth, float worldHeight, int cols, int rows) : gridCols(cols), gridRows(rows)
{
	gridWidth = worldWidth / cols;
	gridHeight = worldHeight / rows;

	grids.resize(rows, vector<GridRef>(cols));

    for (int y = 0; y < rows; ++y) 
    {
        for (int x = 0; x < cols; ++x) 
        {
            float minX = x * gridWidth;
            float minY = y * gridHeight;
            float maxX = minX + gridWidth;
            float maxY = minY + gridHeight;

            AABB cellBounds(minX, minY, maxX, maxY);

            grids[y][x] = MakeShared<Grid>(cellBounds);
        }
    }
}

GridManager::~GridManager()
{
}

/// <summary>
/// GameObject를 해당하는 grid 공간에 추가하는 함수
/// </summary>
/// <param name="obj"> GameObject 객체 </param>
void GridManager::AddObject(GameObjectRef obj)
{
    vector<pair<int, int>> cells;
    GetOverlappingCells(obj->bounds, cells);
    for (const auto& grid : cells) 
    {
        grids[grid.second][grid.first]->Add(obj);
    }
}

/// <summary>
/// GameObject를 모든 grid 공간에서 지우는 함수
/// </summary>
/// <param name="obj"> GameObject 객체 </param>
void GridManager::RemoveObject(GameObjectRef obj)
{
    vector<pair<int, int>> cells;
    GetOverlappingCells(obj->bounds, cells);
    for (const auto& grid : cells) 
    {
        grids[grid.second][grid.first]->Remove(obj);    
    }
}

/// <summary>
/// GameObject가 움직였을 때 grid를 갱신하는 함수.
/// </summary>
/// <param name="obj"> GameObject 객체 </param>
/// <param name="bounds"> 움직인 후 AABB영역 </param>
void GridManager::MoveObject(GameObjectRef obj, const AABB& bounds)
{
    RemoveObject(obj);    
    obj->bounds = bounds;
    AddObject(obj);
}

/// <summary>
/// 주어진 AABB 범위가 포함된 모든 grid 공간을 찾아서 반환해주는 함수
/// </summary>
/// <param name="bounds"> 탐색할 범위 </param>
/// <param name="outCells"> 반환할 grid 공간의 x,y 좌표 </param>
void GridManager::GetOverlappingCells(const AABB& bounds, vector<pair<int, int>>& outCells)
{
    int startX = max(0, (int)(bounds.minX / gridWidth));
    int endX = min(gridCols - 1, (int)(bounds.maxX / gridWidth));
    int startY = max(0, (int)(bounds.minY / gridHeight));
    int endY = min(gridRows - 1, (int)(bounds.maxY / gridHeight));

    for (int y = startY; y <= endY; ++y) 
    {
        for (int x = startX; x <= endX; ++x) 
        {
            outCells.emplace_back(x, y);
        }
    }
}

void GridManager::FindAllColliders(vector<pair<GameObjectRef, GameObjectRef>>& collisions)
{
    std::set<std::pair<GameObjectRef, GameObjectRef>> uniquePairs;

    for (int y = 0; y < gridRows; ++y)
    {
        for (int x = 0; x < gridCols; ++x)
        {
            const auto& objs = grids[y][x]->objects;

            for (auto it1 = objs.begin(); it1 != objs.end(); ++it1)
            {
                for (auto it2 = std::next(it1); it2 != objs.end(); ++it2)
                {
                    GameObjectRef a = it1->second;
                    GameObjectRef b = it2->second;

                    if (a->bounds.Collider(b->bounds))
                    {
                        auto codeA = a->GetObjectCode();
                        auto codeB = b->GetObjectCode();

                        if ((codeA == GameObjectCode::BULLET && codeB == GameObjectCode::BULLET) ||
                            (codeA == GameObjectCode::PLAYER && codeB == GameObjectCode::PLAYER))
                            continue;

                        auto pair = minmax(a, b); // 순서 정렬
                        if (uniquePairs.insert(pair).second)
                        {
                            collisions.emplace_back(pair);
                        }
                    }
                }
            }
        }
    }
}

