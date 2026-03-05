#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "Tile.hpp"
#include "Config.hpp"

class TileMap : public sf::Drawable
{
public:
    TileMap() = default;

    // Загрузить тайлсет из файла
    // tilesetTileSize — размер одного тайла в пикселях в файле (16)
    // tilesetCols     — сколько тайлов по горизонтали в тайлсете
    bool loadTileset(const std::string& path,
        unsigned int tilesetTileSize,
        unsigned int tilesetCols);

    // Заполнить карту из плоского массива int (row-major)
    // data[row * width + col] = tileId
    void loadFromArray(const std::vector<int>& data, int width, int height);

    // Получить тайл по тайловым координатам
    const Tile& getTile(int col, int row) const;
    Tile& getTile(int col, int row);

    // Проверка проходимости по тайловым координатам
    bool isWalkable(int col, int row) const;

    // Перевод мировых координат → тайловые
    sf::Vector2i worldToTile(sf::Vector2f worldPos) const;

    // Перевод тайловых координат → мировые (левый верхний угол тайла)
    sf::Vector2f tileToWorld(int col, int row) const;

    int getWidth()  const { return m_width; }
    int getHeight() const { return m_height; }

    // Перестроить VertexArray (вызывать после изменения карты)
    void rebuild();

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    // Маппинг tileId → тип тайла
    // Переопределяй под свой тайлсет
    static TileType tileIdToType(int id);

    sf::Texture              m_tileset;
    unsigned int             m_tilesetTileSize = 16;
    unsigned int             m_tilesetCols = 1;

    std::vector<std::vector<Tile>> m_tiles; // [row][col]
    int                            m_width = 0;
    int                            m_height = 0;

    sf::VertexArray m_vertices;
};