#include "TileMap.hpp"
#include <iostream>
#include <cassert>

// ---------------------------------------------------------------------------
// Маппинг tileId → TileType
// Настрой под свой тайлсет!
// Пример: id 0..N — полы, остальное — стены
// ---------------------------------------------------------------------------
TileType TileMap::tileIdToType(int id)
{
    // id < 0 — явная пустота
    if (id < 0) return TileType::Empty;

    switch (id)
    {
        // С — стена повёрнутая к экрану (фронтальная, непроходимая)
    case 20: case 21: case 22: case 23: case 24:
    case 26: case 28: case 31:
    case 49: case 51:
    case 83: case 87: case 88: case 89:
    case 97: case 98: case 99: case 104:
    case 134: case 136:
    case 157: case 161: case 163: case 165:
    case 172: case 173: case 174:
        return TileType::Wall;

        // О — обычная стена (боковая/задняя, непроходимая)
    case 39: case 43: case 45: case 50:
    case 58: case 62:
    case 77: case 81:
    case 96: case 100: case 107:
    case 139: case 141: case 145:
    case 158: case 160: case 164:
    case 177: case 179: case 183:
        return TileType::WallTop;

        // Т — тёмный пол (проходимый)
    case 40: case 41: case 42:
    case 59: case 60: case 61: case 66:
    case 78: case 79: case 80:
    case 129: case 130: case 131: case 135:
    case 148: case 149: case 150:
    case 167: case 168: case 169:
    case 186: case 187: case 188:
        return TileType::Floor;

        // К — проход между комнатами
    case 102:
        return TileType::Door;

        // П — пусто (нет тайла)
    default:
        return TileType::Empty;
    }
}

// ---------------------------------------------------------------------------

bool TileMap::loadTileset(const std::string& path,
    unsigned int tilesetTileSize,
    unsigned int tilesetCols)
{
    if (!m_tileset.loadFromFile(path))
    {
        std::cerr << "[TileMap] Failed to load tileset: " << path << "\n";
        return false;
    }
    m_tileset.setSmooth(false);
    m_tilesetTileSize = tilesetTileSize;
    m_tilesetCols = tilesetCols;
    return true;
}

// ---------------------------------------------------------------------------

void TileMap::loadFromArray(const std::vector<int>& data, int width, int height)
{
    assert(static_cast<int>(data.size()) == width * height);

    m_width = width;
    m_height = height;

    m_tiles.assign(height, std::vector<Tile>(width));

    for (int row = 0; row < height; ++row)
    {
        for (int col = 0; col < width; ++col)
        {
            int id = data[row * width + col];
            TileType type = tileIdToType(id);
            m_tiles[row][col] = Tile(type, static_cast<uint16_t>(id));
        }
    }

    rebuild();
}

// ---------------------------------------------------------------------------

void TileMap::rebuild()
{
    // Каждый тайл — 4 вершины (quad)
    m_vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    m_vertices.resize(static_cast<std::size_t>(m_width * m_height) * 6);

    const float drawSize = TILE_DRAW_SIZE; // 48px
    const float texSize = static_cast<float>(m_tilesetTileSize); // 16px

    std::size_t idx = 0;
    for (int row = 0; row < m_height; ++row)
    {
        for (int col = 0; col < m_width; ++col)
        {
            const Tile& tile = m_tiles[row][col];

            if (tile.type == TileType::Empty)
            {
                // Пустые тайлы — прозрачные вершины
                for (int i = 0; i < 6; ++i)
                {
                    m_vertices[idx + i].position = { 0.f, 0.f };
                    m_vertices[idx + i].texCoords = { 0.f, 0.f };
                    m_vertices[idx + i].color = sf::Color::Transparent;
                }
                idx += 6;
                continue;
            }

            // Позиция тайла в тайлсете
            int texCol = tile.tileId % m_tilesetCols;
            int texRow = tile.tileId / m_tilesetCols;

            float tx = static_cast<float>(texCol) * texSize;
            float ty = static_cast<float>(texRow) * texSize;

            // Мировые координаты (левый верхний угол)
            float wx = static_cast<float>(col) * drawSize;
            float wy = static_cast<float>(row) * drawSize;

            // Два треугольника на тайл
            // Triangle 1: top-left, top-right, bottom-left
            m_vertices[idx + 0].position = { wx,            wy };
            m_vertices[idx + 0].texCoords = { tx,            ty };
            m_vertices[idx + 1].position = { wx + drawSize, wy };
            m_vertices[idx + 1].texCoords = { tx + texSize,  ty };
            m_vertices[idx + 2].position = { wx,            wy + drawSize };
            m_vertices[idx + 2].texCoords = { tx,            ty + texSize };

            // Triangle 2: top-right, bottom-right, bottom-left
            m_vertices[idx + 3].position = { wx + drawSize, wy };
            m_vertices[idx + 3].texCoords = { tx + texSize,  ty };
            m_vertices[idx + 4].position = { wx + drawSize, wy + drawSize };
            m_vertices[idx + 4].texCoords = { tx + texSize,  ty + texSize };
            m_vertices[idx + 5].position = { wx,            wy + drawSize };
            m_vertices[idx + 5].texCoords = { tx,            ty + texSize };

            idx += 6;
        }
    }
}

// ---------------------------------------------------------------------------

void TileMap::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.texture = &m_tileset;

    // Масштаб: тайлсет 16px → экран 48px
    // Применяем через transform чтобы не трогать текстурные координаты
    // (текстурные координаты уже в пикселях тайлсета)
    // Масштаб заложен в rebuild() через drawSize, поэтому transform = identity
    target.draw(m_vertices, states);
}

// ---------------------------------------------------------------------------

const Tile& TileMap::getTile(int col, int row) const
{
    assert(col >= 0 && col < m_width && row >= 0 && row < m_height);
    return m_tiles[row][col];
}

Tile& TileMap::getTile(int col, int row)
{
    assert(col >= 0 && col < m_width && row >= 0 && row < m_height);
    return m_tiles[row][col];
}

bool TileMap::isWalkable(int col, int row) const
{
    if (col < 0 || col >= m_width || row < 0 || row >= m_height)
        return false;
    return m_tiles[row][col].walkable;
}

sf::Vector2i TileMap::worldToTile(sf::Vector2f worldPos) const
{
    return {
        static_cast<int>(worldPos.x / TILE_DRAW_SIZE),
        static_cast<int>(worldPos.y / TILE_DRAW_SIZE)
    };
}

sf::Vector2f TileMap::tileToWorld(int col, int row) const
{
    return {
        static_cast<float>(col) * TILE_DRAW_SIZE,
        static_cast<float>(row) * TILE_DRAW_SIZE
    };
}