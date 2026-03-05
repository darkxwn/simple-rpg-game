#include "MapGenerator.hpp"
#include <algorithm>

// ---------------------------------------------------------------------------
// Наборы тайлов
// ---------------------------------------------------------------------------
const std::vector<int> MapGenerator::IDS_WALL = { 21,22,23,97,98,99 };
const std::vector<int> MapGenerator::IDS_WALLTOP = { 39,43,58,62,77,81 };
const std::vector<int> MapGenerator::IDS_FLOOR = { 40,41,42,59,60,61,78,79,80,
                                                     129,130,131,148,149,150,
                                                     167,168,169,186,187,188 };

// ---------------------------------------------------------------------------
// LCG
// ---------------------------------------------------------------------------
unsigned int MapGenerator::lcg(unsigned int& state)
{
    state = state * 1664525u + 1013904223u;
    return state;
}

int MapGenerator::randomFrom(const std::vector<int>& ids, unsigned int& rng)
{
    return ids[lcg(rng) % ids.size()];
}

// ---------------------------------------------------------------------------
// Room
// ---------------------------------------------------------------------------
bool Room::intersects(const Room& o) const
{
    // Зазор: комнаты либо перекрываются (общая стена),
    // либо между полами минимум 3 тайла (стена+пусто+стена)
    // gap=1 или gap=2 между полами = две стены вплотную — запрещено
    int gapX = std::max(left(), o.left()) - std::min(right(), o.right()) - 1;
    int gapY = std::max(top(), o.top()) - std::min(bottom(), o.bottom()) - 1;
    // Если перекрытие по обеим осям — пересечение
    if (gapX < 0 && gapY < 0) return true;
    // Если по одной оси зазор 1 или 2 тайла — стены вплотную
    if (gapX >= 0 && gapX <= 2) return true;
    if (gapY >= 0 && gapY <= 2) return true;
    return false;
}

// ---------------------------------------------------------------------------
// Размещение комнат
// ---------------------------------------------------------------------------
std::vector<Room> MapGenerator::placeRooms(int mapW, int mapH,
    int maxRooms,
    int minSize, int maxSize,
    unsigned int seed)
{
    std::vector<Room> rooms;
    unsigned int rng = seed;

    for (int attempt = 0; attempt < 500; ++attempt)
    {
        int w = minSize + lcg(rng) % (maxSize - minSize + 1);
        int h = minSize + lcg(rng) % (maxSize - minSize + 1);

        int maxX = mapW - w - 2;
        int maxY = mapH - h - 2;
        if (maxX < 1 || maxY < 1) continue;

        int x = 1 + lcg(rng) % maxX;
        int y = 1 + lcg(rng) % maxY;

        Room r{ x, y, w, h };
        bool overlaps = false;
        for (const auto& e : rooms)
            if (r.intersects(e)) { overlaps = true; break; }

        if (!overlaps)
        {
            rooms.push_back(r);
            if (static_cast<int>(rooms.size()) >= maxRooms) break;
        }
    }
    return rooms;
}

// ---------------------------------------------------------------------------
// Заполнение комнаты полом
// ---------------------------------------------------------------------------
void MapGenerator::fillRoom(std::vector<int>& data, int mapW,
    const Room& r, unsigned int& rng)
{
    for (int row = r.top(); row <= r.bottom(); ++row)
        for (int col = r.left(); col <= r.right(); ++col)
            data[row * mapW + col] = randomFrom(IDS_FLOOR, rng);
}

// ---------------------------------------------------------------------------
// Г-образный коридор
// ---------------------------------------------------------------------------
void MapGenerator::digCorridor(std::vector<int>& data, int mapW, int mapH,
    const Room& a, const Room& b,
    unsigned int& rng,
    std::vector<sf::Vector2i>& hCorridorCells)
{
    int ax = a.centerX(), ay = a.centerY();
    int bx = b.centerX(), by = b.centerY();
    bool hFirst = (lcg(rng) % 2 == 0);

    auto setFloor = [&](int col, int row, bool isH)
        {
            if (col < 0 || col >= mapW || row < 0 || row >= mapH) return;
            int& cell = data[row * mapW + col];
            if (cell == ID_EMPTY)
                cell = randomFrom(IDS_FLOOR, rng);
            if (isH) hCorridorCells.push_back({ col, row });
        };

    // Ширина прохода 1 или 2 тайла случайно
    int corridorW = 1 + static_cast<int>(lcg(rng) % 2);

    if (hFirst)
    {
        int x0 = std::min(ax, bx), x1 = std::max(ax, bx);
        for (int x = x0; x <= x1; ++x)
            for (int w = 0; w < corridorW; ++w)
                setFloor(x, ay + w, true);
        int y0 = std::min(ay, by), y1 = std::max(ay, by);
        for (int y = y0; y <= y1; ++y)
            for (int w = 0; w < corridorW; ++w)
                setFloor(bx + w, y, false);
    }
    else
    {
        int y0 = std::min(ay, by), y1 = std::max(ay, by);
        for (int y = y0; y <= y1; ++y)
            for (int w = 0; w < corridorW; ++w)
                setFloor(ax + w, y, false);
        int x0 = std::min(ax, bx), x1 = std::max(ax, bx);
        for (int x = x0; x <= x1; ++x)
            for (int w = 0; w < corridorW; ++w)
                setFloor(x, by + w, true);
    }
}

// ---------------------------------------------------------------------------
// Построение стен
//
// Схема комнаты:
//   20 С  С  С  24    верхний-левый=20, верхний-правый=24, верх=С
//   О  Т  Т  Т  О     бока=О
//   О  Т  Т  Т  О
//   С  С  С  С  С     низ=С
//
// Т-образные пересечения:
//   50  — сверху О, по бокам С, снизу пол/пусто   (⊥ — тупик сверху)
//   88  — снизу О/С, по бокам С, сверху пол/пусто (⊥ — тупик снизу)
//   158 — сверху О, снизу О/С, слева С, справа пол (⊣)
//   160 — сверху О, снизу О/С, справа С, слева пол (⊢)
//   164 — сверху О, снизу О/С, слева С, справа С   (крест ╋)
//
// Проход (вертикальный, в О стене):
//   28  — Е верхняя часть прохода
//   104 — Н нижняя часть прохода
// ---------------------------------------------------------------------------
void MapGenerator::buildWalls(std::vector<int>& data, int mapW, int mapH,
    const std::vector<sf::Vector2i>& hCorridorCells)
{
    // --- вспомогательные лямбды ---

    auto isFloor = [&](int col, int row) -> bool
        {
            if (col < 0 || col >= mapW || row < 0 || row >= mapH) return false;
            int id = data[row * mapW + col];
            for (int fid : IDS_FLOOR) if (id == fid) return true;
            return false;
        };

    // Является ли клетка горизонтальной стеной С (уже расставленной или будет)
    // Используем для определения соседей при Т-образных
    auto isWall = [&](int col, int row) -> bool
        {
            if (col < 0 || col >= mapW || row < 0 || row >= mapH) return false;
            int id = data[row * mapW + col];
            for (int wid : IDS_WALL) if (id == wid) return true;
            // Специальные С-тайлы
            for (int wid : {20, 24, 50, 88, 97, 98, 99, 134, 136, 172, 173, 174})
                if (id == wid) return true;
            return false;
        };

    auto isWallTop = [&](int col, int row) -> bool
        {
            if (col < 0 || col >= mapW || row < 0 || row >= mapH) return false;
            int id = data[row * mapW + col];
            for (int wid : IDS_WALLTOP) if (id == wid) return true;
            for (int wid : {39, 43, 45, 58, 62, 77, 81, 96, 100, 139, 141, 145, 158, 160, 164})
                if (id == wid) return true;
            return false;
        };

    auto isEmpty = [&](int col, int row) -> bool
        {
            if (col < 0 || col >= mapW || row < 0 || row >= mapH) return true;
            return data[row * mapW + col] == ID_EMPTY;
        };

    auto isHCorridor = [&](int col, int row) -> bool
        {
            for (const auto& c : hCorridorCells)
                if (c.x == col && c.y == row) return true;
            return false;
        };

    // --- собираем пустые клетки рядом с полом ---
    std::vector<std::pair<int, int>> wallCells;
    for (int row = 0; row < mapH; ++row)
        for (int col = 0; col < mapW; ++col)
        {
            if (data[row * mapW + col] != ID_EMPTY) continue;
            bool near = false;
            for (int dy = -1; dy <= 1 && !near; ++dy)
                for (int dx = -1; dx <= 1 && !near; ++dx)
                    if (!(dx == 0 && dy == 0) && isFloor(col + dx, row + dy))
                        near = true;
            if (near) wallCells.push_back({ col, row });
        }

    // --- первый проход: расставляем базовые С и О ---
    for (auto [col, row] : wallCells)
    {
        bool fA = isFloor(col, row - 1); // пол сверху
        bool fB = isFloor(col, row + 1); // пол снизу
        bool fL = isFloor(col - 1, row); // пол слева
        bool fR = isFloor(col + 1, row); // пол справа

        unsigned int lr = static_cast<unsigned int>(row * 7919u + col * 6271u);

        if (fA || fB)
        {
            // Горизонтальная стена С
            data[row * mapW + col] = randomFrom(IDS_WALL, lr);
        }
        else if (fL || fR)
        {
            // Боковая стена О — проверяем проход
            bool corridorBelow = isHCorridor(col, row + 1);
            bool corridorAbove = isHCorridor(col, row - 1);

            if (corridorBelow)
                data[row * mapW + col] = 28;   // Е — верх прохода
            else if (corridorAbove)
                data[row * mapW + col] = 104;  // Н — низ прохода
            else
                data[row * mapW + col] = randomFrom(IDS_WALLTOP, lr);
        }
        else
        {
            // Только диагональные соседи
            // Проверяем диагонали: если пол только по диагонали снизу — это нижний угол → С
            bool fBL = isFloor(col - 1, row + 1);
            bool fBR = isFloor(col + 1, row + 1);
            bool fTL = isFloor(col - 1, row - 1);
            bool fTR = isFloor(col + 1, row - 1);

            bool onlyBottomDiag = (fBL || fBR) && !fTL && !fTR;
            bool onlyTopDiag = (fTL || fTR) && !fBL && !fBR;

            if (onlyBottomDiag)
            {
                // Нижний угол — 96 (левый) или 100 (правый)
                bool isLeftCorner = fBR && !fBL; // пол справа-снизу → левый угол
                bool isRightCorner = fBL && !fBR; // пол слева-снизу → правый угол
                if (isLeftCorner)       data[row * mapW + col] = 96;
                else if (isRightCorner) data[row * mapW + col] = 100;
                else                    data[row * mapW + col] = randomFrom(IDS_WALL, lr);
            }
            else if (onlyTopDiag)
                // Верхний внешний угол — С
                data[row * mapW + col] = randomFrom(IDS_WALL, lr);
            else
                data[row * mapW + col] = randomFrom(IDS_WALLTOP, lr);
        }
    }

    // --- второй проход: углы и Т-образные ---
    for (auto [col, row] : wallCells)
    {
        bool fA = isFloor(col, row - 1);
        bool fB = isFloor(col, row + 1);
        bool fL = isFloor(col - 1, row);
        bool fR = isFloor(col + 1, row);

        bool wA = isWallTop(col, row - 1);  // О сверху
        bool wB = isWallTop(col, row + 1);  // О снизу
        bool wL = isWall(col - 1, row);     // С слева
        bool wR = isWall(col + 1, row);     // С справа

        bool eA = isEmpty(col, row - 1);
        bool eB = isEmpty(col, row + 1);
        bool eL = isEmpty(col - 1, row);
        bool eR = isEmpty(col + 1, row);

        // --- УГЛЫ 20/24 ---
        // 20: снизу О, справа С, сверху пусто, слева пусто, снизу НЕ пусто
        // Нижние углы — это просто С (первый проход уже поставил С через fA)
        if (wB && wR && eA && eL && !fB && !fL && !eB)
        {
            data[row * mapW + col] = 20;
            continue;
        }
        // 24: снизу О, слева С, сверху пусто, справа пусто, снизу НЕ пусто
        if (wB && wL && eA && eR && !fB && !fR && !eB)
        {
            data[row * mapW + col] = 24;
            continue;
        }

        // --- Т-ОБРАЗНЫЕ ---
        // Только на О-стенах (пол только сбоку, не сверху/снизу)
        bool isVertWall = !fA && !fB && (fL || fR);
        if (!isVertWall) continue;

        // Т-образные ставятся только если стена НЕ крайняя:
        // крайняя = с одной из сторон (сверху или снизу) пусто
        bool openAbove = eA; // пусто сверху — крайняя верхняя позиция
        bool openBelow = eB; // пусто снизу — крайняя нижняя позиция

        // 164 — крест: О сверху И снизу (не крайняя), С слева И справа
        if (wA && wB && wL && wR && !openAbove && !openBelow)
        {
            data[row * mapW + col] = 164;
            continue;
        }

        // 158 — ⊣: О сверху И снизу (не крайняя), С слева, пол справа
        if (wA && wB && wL && !wR && !openAbove && !openBelow)
        {
            data[row * mapW + col] = 158;
            continue;
        }

        // 160 — ⊢: О сверху И снизу (не крайняя), С справа, пол слева
        if (wA && wB && wR && !wL && !openAbove && !openBelow)
        {
            data[row * mapW + col] = 160;
            continue;
        }

        // 50 — тупик снизу: О сверху, С по бокам, снизу НЕ О (конец стены)
        // Только если снизу пол или пусто — не крайняя стена сверху
        if (wA && !wB && wL && wR && !openAbove)
        {
            data[row * mapW + col] = 50;
            continue;
        }

        // 88 — тупик сверху: О снизу, С по бокам, сверху НЕ О (начало стены)
        // Только если сверху пол или пусто — не крайняя стена снизу
        if (!wA && wB && wL && wR && !openBelow)
        {
            data[row * mapW + col] = 88;
            continue;
        }
    }
}

// ---------------------------------------------------------------------------
// Главная функция
// ---------------------------------------------------------------------------
std::vector<int> MapGenerator::generate(int width, int height, unsigned int seed)
{
    if (seed == 0) seed = 0xDEADBEEFu;

    unsigned int rng = seed;
    for (int i = 0; i < 32; ++i) lcg(rng);

    std::vector<int> data(width * height, ID_EMPTY);

    int numRooms = 3 + static_cast<int>(lcg(rng) % 3); // 3–5
    const int minRoom = 4;
    const int maxRoom = 9;

    auto rooms = placeRooms(width, height, numRooms, minRoom, maxRoom, rng);
    if (static_cast<int>(rooms.size()) < 2)
        rooms = placeRooms(width, height, numRooms, 3, 6, lcg(rng));

    for (auto& r : rooms)
        fillRoom(data, width, r, rng);

    std::vector<sf::Vector2i> hCorridorCells;
    for (std::size_t i = 1; i < rooms.size(); ++i)
        digCorridor(data, width, height, rooms[i - 1], rooms[i], rng, hCorridorCells);

    buildWalls(data, width, height, hCorridorCells);

    return data;
}