#pragma once
#include <SFML/Graphics.hpp>
#include "TileMap.hpp"
#include "Config.hpp"

class Game
{
public:
    Game();

    // Запуск главного цикла
    void run();

private:
    void handleEvents();
    void update(float dt);
    void render();

    // Пересчёт View при ресайзе окна
    void onResize(unsigned int newW, unsigned int newH);

    sf::RenderWindow m_window;
    sf::View         m_gameView;   // игровая камера (следит за игроком)
    sf::View         m_uiView;     // статичная камера для UI (пиксель в пиксель)

    TileMap          m_tileMap;

    bool             m_paused = false;
    int              m_mapWidth = 0;
    int              m_mapHeight = 0;
    float            m_mapOffsetX = 0.f;
    float            m_mapOffsetY = 0.f;
    sf::Clock        m_clock;
};