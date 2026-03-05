#include "Game.hpp"
#include "MapGenerator.hpp"
#include "Config.hpp"
#include <iostream>
#include <ctime>

// ---------------------------------------------------------------------------
// Конструктор
// ---------------------------------------------------------------------------
Game::Game()
    : m_window(
        sf::VideoMode::getDesktopMode(),
        "DungeonCrawler") // + sf::Style::None
{
    m_window.setFramerateLimit(FRAMERATE_LIMIT);

    // Реальный размер окна (полный экран)
    auto winSize = m_window.getSize();
    float winW = static_cast<float>(winSize.x);
    float winH = static_cast<float>(winSize.y);

    // Игровая камера
    m_gameView = sf::View(sf::FloatRect({ 0.f, 0.f }, { winW, winH }));

    // UI камера
    m_uiView = sf::View(sf::FloatRect({ 0.f, 0.f }, { winW, winH }));

    // Загрузка тайлсета (11 колонок x 16px = 176px ширина)
    if (!m_tileMap.loadTileset("assets/Tiles/Tileset.png", 16, 19))
        std::cerr << "[Game] Warning: tileset not loaded!\n";
    else
        std::cout << "[Game] Tileset loaded OK\n";


    // Размер карты подбирается под окно с отступом PADDING тайлов по краям
    // Карта вписывается ровно в окно и не вылезает за его пределы
    const int PADDING = 2; // тёмный отступ вокруг карты в тайлах
    m_mapWidth = static_cast<int>(winW / TILE_DRAW_SIZE) - PADDING * 2;
    m_mapHeight = static_cast<int>(winH / TILE_DRAW_SIZE) - PADDING * 2;

    // Seed на основе времени для случайной генерации каждый раз
    unsigned int seed = static_cast<unsigned int>(std::time(nullptr));
    auto mapData = MapGenerator::generate(m_mapWidth, m_mapHeight, seed);
    m_tileMap.loadFromArray(mapData, m_mapWidth, m_mapHeight);
    std::cout << "[Game] Map: " << m_mapWidth << "x" << m_mapHeight
        << " seed=" << seed << "\n";

    // Карта начинается с отступа PADDING тайлов от края окна
    m_mapOffsetX = static_cast<float>(PADDING) * TILE_DRAW_SIZE;
    m_mapOffsetY = static_cast<float>(PADDING) * TILE_DRAW_SIZE;

    // Камера статична — карта вся влезает в окно
    m_gameView.setCenter({ winW / 2.f, winH / 2.f });
}

// ---------------------------------------------------------------------------
// Главный цикл
// ---------------------------------------------------------------------------
void Game::run()
{
    while (m_window.isOpen())
    {
        handleEvents();

        if (!m_paused)
        {
            float dt = m_clock.restart().asSeconds();
            // Ограничиваем dt чтобы не было огромных скачков при лагах
            if (dt > 0.1f) dt = 0.1f;
            update(dt);
        }
        else
        {
            m_clock.restart(); // не накапливать время пока на паузе
        }

        render();
    }
}

// ---------------------------------------------------------------------------
// События
// ---------------------------------------------------------------------------
void Game::handleEvents()
{
    while (const auto event = m_window.pollEvent())
    {
        // Закрытие окна
        if (event->is<sf::Event::Closed>())
        {
            m_window.close();
            return;
        }

        // Ресайз окна — пересчитываем View
        if (const auto* resized = event->getIf<sf::Event::Resized>())
        {
            onResize(resized->size.x, resized->size.y);
        }

        // Потеря фокуса — пауза
        if (event->is<sf::Event::FocusLost>())
        {
            m_paused = true;
        }

        // Возврат фокуса — снимаем паузу
        if (event->is<sf::Event::FocusGained>())
        {
            m_paused = false;
        }

        // Escape — выход
        if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->code == sf::Keyboard::Key::Escape)
            {
                m_window.close();
                return;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Обновление
// ---------------------------------------------------------------------------
void Game::update(float /*dt*/)
{
    // TODO: обновление игрока, врагов, объектов
    // m_player.update(dt, m_tileMap);

    // TODO: камера следует за игроком:
    // m_gameView.setCenter(m_player.getPosition());
}

// ---------------------------------------------------------------------------
// Рендер
// ---------------------------------------------------------------------------
void Game::render()
{
    m_window.clear(sf::Color(20, 18, 35));

    // --- Игровой слой (с камерой) ---
    m_window.setView(m_gameView);

    // Карта рисуется со смещением (отступ от края окна)
    sf::RenderStates mapStates;
    mapStates.transform.translate({ m_mapOffsetX, m_mapOffsetY });
    m_window.draw(m_tileMap, mapStates);

    // TODO: draw entities, objects

    // --- UI слой (без камеры, пиксель в пиксель) ---
    m_window.setView(m_uiView);
    // TODO: draw UI

    m_window.display();
}

// ---------------------------------------------------------------------------
// Ресайз
// ---------------------------------------------------------------------------
void Game::onResize(unsigned int newW, unsigned int newH)
{
    float w = static_cast<float>(newW);
    float h = static_cast<float>(newH);

    m_gameView.setSize({ w, h });
    m_gameView.setCenter({ w / 2.f, h / 2.f });

    m_uiView = sf::View(sf::FloatRect({ 0.f, 0.f }, { w, h }));
}