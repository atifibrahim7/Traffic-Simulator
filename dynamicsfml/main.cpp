#pragma once
#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <ctime>
#include "organism.h"
using namespace std; 
 
//int main()
//{
//    sf::RenderWindow window(sf::VideoMode(415, 415), "Blob Time");
//    World w;
//
//
//    while (window.isOpen())
//    {
//        bool waitForEnter = true;
//
//        // Start the game loop
//        while (window.isOpen()) {
//            // Process events
//            sf::Event event;
//            while (window.pollEvent(event)) {
//                if (event.type == sf::Event::Closed)
//                    window.close();
//
//                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter)
//                {
//                    waitForEnter = false;
//                }
//            }
//
//            if (!waitForEnter)
//            {
//
//                window.clear();
//
//                w.simulate_a_turn();
//                w.display_world(window);
//
//                window.display();
//                //cin.get();
//               // sf::sleep(sf::milliseconds(200));
//
//                waitForEnter = true;
//            }
//        }
//    }
//
//    return 0;
//}



int main()
{
    sf::RenderWindow window(sf::VideoMode(415, 415), "Blob Time");
    World w;


    while (window.isOpen())
    {
        bool waitForEnter = true;

        // Start the game loop
        while (window.isOpen()) {
            // Process events
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();

                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter)
                {
                    waitForEnter = true;
                }
            }

            if (waitForEnter)
            {

                window.clear();

                w.simulate_a_turn();
                w.display_world(window);

                window.display();
                //cin.get();
                sf::sleep(sf::milliseconds(200));

                waitForEnter = true;
            }
        }
    }

    return 0;
}
