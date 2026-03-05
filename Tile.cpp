#include "Tile.hpp"

bool Tile::isWalkable(TileType t)
{
    switch (t)
    {
    case TileType::Floor:
    case TileType::Stairs:
    case TileType::Door:
        return true;
    default:
        return false;
    }
}

Tile::Tile(TileType t, uint16_t id)
    : type(t), tileId(id), walkable(isWalkable(t))
{
}