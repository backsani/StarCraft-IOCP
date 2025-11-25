#pragma once
class VisualManager
{
};

/*

// 프로그램 시작 시 1회만 계산
// 반지름에 따른 영역 배열
std::vector<Vec2i> circleMask[radiusMax+1];

for (int r = 1; r <= radiusMax; ++r) {
    for (int dy = -r; dy <= r; ++dy) {
        for (int dx = -r; dx <= r; ++dx) {
            if (dx*dx + dy*dy <= r*r) {
                circleMask[r].push_back({dx, dy});
            }
        }
    }
}

// 기존 계산
void applyVision(FogOfWar& fog, int centerX, int centerY, int radius) {
    int r2 = radius * radius;
    for (int y = centerY - radius; y <= centerY + radius; ++y) {
        if (y < 0 || y >= fog.height) continue;
        for (int x = centerX - radius; x <= centerX + radius; ++x) {
            if (x < 0 || x >= fog.width) continue;

            int dx = x - centerX;
            int dy = y - centerY;
            if (dx * dx + dy * dy <= r2) {
                auto& cell = fog.at(x, y);
                cell = FogState::Visible;     // 지금 보이고
                // 한 번이라도 봤으면 Explored 상태는 유지
                // Explored는 Visible로 덮어써도 문제 없음
            }
        }
    }
}

//반지름 계산이 끝나고 적용만
void applyVision(FogOfWar& fog, int cx, int cy, int radius) {
    for (auto [dx, dy] : circleMask[radius]) {
        int x = cx + dx;
        int y = cy + dy;
        if (x < 0 || x >= fog.width || y < 0 || y >= fog.height)
            continue;
        fog.at(x, y) = FogState::Visible;
    }
}


void updateFogForPlayer(Player& p) {
    FogOfWar& fog = p.fog;

    // 1) Visible → Explored 로 내려주기
    for (auto& cell : fog.tiles) {
        if (cell == FogState::Visible)
            cell = FogState::Explored;
    }

    // 2) 플레이어 유닛들의 시야 적용
    for (Unit* u : p.units) {
        applyVision(fog, u->tileX, u->tileY, u->visionRange);
    }
}

// 언덕, 장애물 시스템 제작 시
struct TileInfo {
    bool blocksMovement;
    bool blocksVision;
};


*/