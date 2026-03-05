#pragma once
#include <cstdint>

// Тип тайла — определяет поведение (проходимость, взаимодействие)
enum class TileType : uint8_t
{
    Empty = 0,   // пустота / вне карты
    Floor,       // проходимый пол
    Wall,        // непроходимая стена
    WallTop,     // верхний край стены (декоративный, непроходимый)
    Pit,         // яма (непроходима)
    Stairs,      // лестница (переход между уровнями)
    Door,        // дверь (интерактивный)
};

// Один тайл карты
struct Tile
{
    TileType type = TileType::Empty;
    uint16_t tileId = 0;      // индекс тайла в тайлсете (row * cols + col)
    bool     walkable = false; // кэшированный флаг проходимости

    // Конструктор по типу — автоматически выставляет walkable
    explicit Tile(TileType t = TileType::Empty, uint16_t id = 0);

    // Утилита: вернуть walkable по типу
    static bool isWalkable(TileType t);
};