#pragma once
#include <vector>
#include <SFML/System/Vector2.hpp>

struct MapSize { int width; int height; const char* name; };
constexpr MapSize MAP_SMALL = { 24, 18, "Small" };
constexpr MapSize MAP_MEDIUM = { 32, 24, "Medium" };
constexpr MapSize MAP_LARGE = { 48, 36, "Large" };

struct Room
{
    int x, y, w, h;
    int left()   const { return x; }
    int right()  const { return x + w - 1; }
    int top()    const { return y; }
    int bottom() const { return y + h - 1; }
    int centerX()const { return x + w / 2; }
    int centerY()const { return y + h / 2; }
    bool intersects(const Room& other) const;
};

class MapGenerator
{
public:
    static std::vector<int> generate(int width, int height, unsigned int seed = 0);

private:
    static std::vector<Room> placeRooms(int mapW, int mapH, int maxRooms,
        int minSize, int maxSize, unsigned int seed);
    static void fillRoom(std::vector<int>& data, int mapW,
        const Room& r, unsigned int& rng);
    static void digCorridor(std::vector<int>& data, int mapW, int mapH,
        const Room& a, const Room& b, unsigned int& rng,
        std::vector<sf::Vector2i>& hCorridorCells);
    static void buildWalls(std::vector<int>& data, int mapW, int mapH,
        const std::vector<sf::Vector2i>& hCorridorCells);
    static int          randomFrom(const std::vector<int>& ids, unsigned int& rng);
    static unsigned int lcg(unsigned int& state);

    static const std::vector<int> IDS_WALL;    // С — горизонтальная
    static const std::vector<int> IDS_WALLTOP; // О — вертикальная боковая
    static const std::vector<int> IDS_FLOOR;   // Т — пол

    // Специальные тайлы стен
    static constexpr int CORNER_TL = 20;  // верхний-левый угол
    static constexpr int CORNER_TR = 24;  // верхний-правый угол
    static constexpr int T_TOP = 50;  // Т-обр: О сверху, С по бокам
    static constexpr int T_BOT = 88;  // Т-обр: О снизу, С по бокам
    static constexpr int T_LEFT = 158; // Т-обр: О верх+низ, С слева
    static constexpr int T_RIGHT = 160; // Т-обр: О верх+низ, С справа
    static constexpr int T_CROSS = 164; // крест: О верх+низ, С лево+право
    static constexpr int PASSAGE_E = 28;  // верх вертикального прохода
    static constexpr int PASSAGE_N = 104; // низ вертикального прохода

    static constexpr int ID_EMPTY = -1;
};