#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include "tinyxml2.h"
#include <algorithm>
#include <SFML/Graphics.hpp>
#include <vector>
#include <iomanip>
#include <sstream>
#include <string>
#include <exception>

using namespace tinyxml2;
using namespace std;

class Road;
class Car;

class Building {
public:
    enum class Type { RESIDENTIAL, INDUSTRIAL, COMMERCIAL };
    Type type;
    pair<int, int> position;
};

class Lane {
public:
    std::string direction;
    vector<Car*> carsInLane;
};

class Intersection {
public:
    vector<Road*> connectedRoads;
    pair<int, int> position; // Add position attribute
    void calculatePosition() {
		int x = 0;
		int y = 0;
        for (Road* road : connectedRoads) {
			x += road->position.first;
			y += road->position.second;
		}
		position = make_pair(x / connectedRoads.size(), y / connectedRoads.size());
	}
};

class Road {
public:
    int id;
    int length;
    pair<int, int> position; // Add position attribute
    Lane lanes[2];
    vector<Car*> carsOnRoad;
};

class Car {
public:
    int size;
    pair<int, int> position;
    Road* currentRoad;
};

class Passenger {
public:
    Building* home;
    Building* work;
    Building* commercial;
};

class TrafficAnalysis {
private:
    
public:
    Road* analysisRoad;
    unordered_map<Road*, vector<int>> carCounts;

    void analyzeRoad(Road* road) {
        if (carCounts.find(road) == carCounts.end()) {
            carCounts[road] = vector<int>(24, 0);
        }
        carCounts[road][currentHour()]++;
    }

    void setRoadForAnalysis(Road* road) {
        analysisRoad = road;
    }    
    
    int currentHour() {
        static int hour = 0;
        return hour++ % 24;
    }

    void exportCSV(const string& filename) {
        ofstream file(filename);
        if (file.is_open()) {
            Road* road = analysisRoad;
            if (road != nullptr && carCounts.find(road) != carCounts.end()) {
                file << "Road ID " << road->id << ",Hour,Car Count\n";
                for (int hour = 0; hour < carCounts[road].size(); hour++) {
                    file << road->id << "," << hour << "," << carCounts[road][hour] << "\n";
                }
            }
            file.close();
        } else {
            cerr << "Unable to open file for writing." << endl;
        }
    }

};

class City {
public:
    vector<Building> buildings;
    vector<Road> roads;
    vector<Intersection> intersections;
};

class CityLoader {
public:
    City loadFromXML(const string& filepath) {
        City city;
        XMLDocument doc;
        if (doc.LoadFile(filepath.c_str()) == XML_SUCCESS) {
            XMLElement* pRoot = doc.RootElement();
            parseBuildings(pRoot, city);
            parseRoads(pRoot, city);
            parseIntersections(pRoot, city);
        } else {
            cerr << "Could not load XML file." << endl;
        }
        return city;
    }

private:
    void parseBuildings(XMLElement* pRoot, City& city) {
        XMLElement* pBuildings = pRoot->FirstChildElement("buildings");
        for (XMLElement* pBuilding = pBuildings->FirstChildElement("building"); pBuilding != nullptr; pBuilding = pBuilding->NextSiblingElement("building")) {
            Building building;
            building.type = parseBuildingType(pBuilding->Attribute("type"));
            building.position = make_pair(pBuilding->IntAttribute("positionX"), pBuilding->IntAttribute("positionY"));
            city.buildings.push_back(building);
        }
    }

    Building::Type parseBuildingType(const char* type) {
        if (strcmp(type, "residential") == 0) return Building::Type::RESIDENTIAL;
        if (strcmp(type, "industrial") == 0) return Building::Type::INDUSTRIAL;
        if (strcmp(type, "commercial") == 0) return Building::Type::COMMERCIAL;
        throw std::runtime_error("Unknown building type");
    }

    void parseIntersections(XMLElement* pRoot, City& city) {
        XMLElement* pIntersections = pRoot->FirstChildElement("intersections");
        for (XMLElement* pIntersection = pIntersections->FirstChildElement("intersection"); pIntersection != nullptr; pIntersection = pIntersection->NextSiblingElement("intersection")) {
            Intersection intersection;
            for (XMLElement* pRoadRef = pIntersection->FirstChildElement("roadRef"); pRoadRef != nullptr; pRoadRef = pRoadRef->NextSiblingElement("roadRef")) {
                int roadId = pRoadRef->IntAttribute("id");
                // Find the road by ID and add it to the intersection
                auto it = find_if(city.roads.begin(), city.roads.end(), [roadId](const Road& road) { return road.id == roadId; });
                if (it != city.roads.end()) {
                    intersection.connectedRoads.push_back(&(*it));
                }
            }
            city.intersections.push_back(intersection);
        }
    }

    void parseRoads(XMLElement* pRoot, City& city) {
        XMLElement* pRoads = pRoot->FirstChildElement("roads");
        for (XMLElement* pRoad = pRoads->FirstChildElement("road"); pRoad != nullptr; pRoad = pRoad->NextSiblingElement("road")) {
            Road road;
            road.id = pRoad->IntAttribute("id");
            road.length = pRoad->IntAttribute("length");
            road.position = make_pair(pRoad->IntAttribute("positionX"), pRoad->IntAttribute("positionY"));

            // Correctly handle Lane array
            XMLElement* pLanes = pRoad->FirstChildElement("lanes");
            int laneIndex = 0;
            for (XMLElement* pLane = pLanes->FirstChildElement("lane"); pLane != nullptr; pLane = pLane->NextSiblingElement("lane")) {
                Lane lane;
                lane.direction = pLane->Attribute("direction");
                road.lanes[laneIndex++] = lane; // Correctly assign lane to array
                if (laneIndex >= 2) break; // Safety check
            }

            city.roads.push_back(road);
        }
    }
};

class Simulation {
public:
    City city;
    TrafficAnalysis trafficAnalysis;

    Simulation(const string& cityFile) : city(CityLoader().loadFromXML(cityFile)) {}

    void run() {
        for (int hour = 0; hour < 24; hour++) {
            for (Road& road : city.roads) {
                trafficAnalysis.analyzeRoad(&road);
            }
        }
    }
};


void drawHistogram(const std::vector<int>& carCounts) {
    const int windowWidth = 800;
    const int windowHeight = 600;
    const int barWidth = windowWidth / 24;
    const float maxBarHeight = windowHeight - 100; 

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Traffic Histogram");
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) { 
        cerr << "Failed to load font from arial.ttf" << endl;
        return;
    }

    int maxCount = *max_element(carCounts.begin(), carCounts.end());
    if (maxCount == 0) {
        cerr << "Max car count is zero, nothing to draw." << endl;
        return;
    }

    // Start the event loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::White); 

        // Drawing bars in orange
        for (size_t i = 0; i < carCounts.size(); ++i) {
            float barHeight = (static_cast<float>(carCounts[i]) / maxCount) * maxBarHeight;
            sf::RectangleShape bar(sf::Vector2f(barWidth - 5, barHeight));
            bar.setPosition(i * barWidth, windowHeight - barHeight - 50); // Adjust position
            bar.setFillColor(sf::Color(255, 165, 0)); // Orange color
            window.draw(bar);

            // Display y-axis values
            sf::Text yAxisValueText(std::to_string(carCounts[i]), font, 12);
            yAxisValueText.setPosition(i * barWidth + 2.5, windowHeight - barHeight - 5);
            yAxisValueText.setFillColor(sf::Color::Black);
            window.draw(yAxisValueText);

            // Display y-axis label
            sf::Text yAxisLabelText("Car Count", font, 12);
            yAxisLabelText.setPosition((i * barWidth + 2.5) + barWidth / 2, windowHeight - barHeight - 25);
            yAxisLabelText.setFillColor(sf::Color::Black);
            window.draw(yAxisLabelText);
        }

        // Drawing text labels on the x-axis
        for (int i = 0; i < 24; ++i) {
            sf::Text text(std::to_string(i), font, 12);
            text.setPosition(i * barWidth, windowHeight - 50);
            text.setFillColor(sf::Color::Black);
            window.draw(text);
        }

        window.display();
    }
}

std::string getRoadIdFromUser() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Enter Road ID");
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Failed to load font from arial.ttf" << std::endl;
        return "";
    }

    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::Black);
    text.setPosition(50, 50);

    sf::RectangleShape inputBox;
    inputBox.setSize(sf::Vector2f(200, 50));
    inputBox.setFillColor(sf::Color::White);
    inputBox.setPosition(50, 100);

    std::string roadId;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode < 128) {
                    if (event.text.unicode == '\b' && !roadId.empty()) { 
                        roadId.pop_back();
                    } else if (event.text.unicode == '\r') { 
                        window.close();
                    } else {
                        roadId += static_cast<char>(event.text.unicode);
                    }
                }
            }
        }

        text.setString("Enter Road ID: " + roadId);

        window.clear(sf::Color::Cyan); 
        window.draw(text);
        window.draw(inputBox);
        window.display();
    }

    return roadId;
}


void drawCity(const City& city) {
    // Increase the window size to fit the map of 1000x1000
    const int windowWidth = 1200;
    const int windowHeight = 1200;

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "City Visualization");
    sf::View view = window.getDefaultView(); // Get the default view

    // Define the map area dimensions and position to be 1000x1000
    float mapWidth = 1920;
    float mapHeight = 1080;
    float mapAreaX = (windowWidth - mapWidth) / 2;
    float mapAreaY = (windowHeight - mapHeight) / 2;

    // Create the map border
    sf::RectangleShape border(sf::Vector2f(mapWidth, mapHeight));
    border.setPosition(mapAreaX, mapAreaY);
    border.setFillColor(sf::Color::Green); // Set the map area to green
    border.setOutlineThickness(-5); // Negative thickness to draw inside the shape
    border.setOutlineColor(sf::Color::Black); // Set border color to b

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::MouseWheelScrolled) { // Handle mouse wheel scrolled event
                if (event.mouseWheelScroll.delta > 0)
                    view.zoom(0.9f); // Zoom in
                else if (event.mouseWheelScroll.delta < 0)
                    view.zoom(1.1f); // Zoom out
            }
            else if (event.type == sf::Event::KeyPressed) { // Handle key pressed event
                if (event.key.code == sf::Keyboard::Up)
                    view.move(0, -50); // Scroll up
                else if (event.key.code == sf::Keyboard::Down)
                    view.move(0, 50); // Scroll down
                else if (event.key.code == sf::Keyboard::Left)
                    view.move(-50, 0); // Scroll left
                else if (event.key.code == sf::Keyboard::Right)
                    view.move(50, 0); // Scroll right
            }
        }
        window.clear(sf::Color::White); // Clear with white color for the background

        // Draw the map border
        window.draw(border);

        // Set the view back to the moved and zoomed view for drawing the city elements
        window.setView(view);

        // Draw buildings
        for (const Building& building : city.buildings) {
            sf::RectangleShape rectangle(sf::Vector2f(10, 10));
            rectangle.setPosition(building.position.first, building.position.second);
            switch (building.type) {
                case Building::Type::RESIDENTIAL:
                    rectangle.setFillColor(sf::Color::Blue); // Set residential building color to blue
                    break;
                case Building::Type::INDUSTRIAL:
                    rectangle.setFillColor(sf::Color::Red); // Set industrial building color to red
                    break;
                case Building::Type::COMMERCIAL:
                    rectangle.setFillColor(sf::Color::Yellow); // Set commercial building color to yellow
                    break;
            }
            window.draw(rectangle);
        }


        // Draw roads
        for (const Road& road : city.roads) {
            for (const Lane& lane : road.lanes) {
                sf::RectangleShape line;
                if (lane.direction == "east" || lane.direction == "west") {
                    // Ensure the road does not exceed map width
                    int actualLength = std::min(road.length, static_cast<int>(mapWidth - road.position.first));
                    line = sf::RectangleShape(sf::Vector2f(actualLength, 8)); // 8 pixels thick lane
                    line.setPosition(road.position.first, road.position.second + (lane.direction == "east" ? 0 : 12));
                } else {
                    // Ensure the road does not exceed map height
                    int actualLength = std::min(road.length, static_cast<int>(mapHeight - road.position.second));
                    line = sf::RectangleShape(sf::Vector2f(8, actualLength)); // 8 pixels thick lane
                    line.setPosition(road.position.first + (lane.direction == "north" ? 0 : 12), road.position.second);
                }
                line.setFillColor(sf::Color(128, 128, 128)); // Set lane color to gray
                line.setOutlineThickness(1); // Set border thickness
                line.setOutlineColor(sf::Color::White); // Set border color to white
                window.draw(line);
            }
        }
        // Draw cars
        for (const Road& road : city.roads) {
            for (const Car* car : road.carsOnRoad) {
                if (car) {
                    sf::CircleShape circle(2);
                    circle.setPosition(car->position.first, car->position.second);
                    circle.setFillColor(sf::Color::Yellow);
                    window.draw(circle);
                }
            }
        }

        window.display();
    }
}


int main() {
    try {
        std::string cityFilePath = "city.xml";
        Simulation simulation(cityFilePath);

        while (true) {
            std::cout << "Enter 1 to visualize the simulation, 2 for analytics, or 0 to exit: ";
            int choice;
            std::cin >> choice;

            if (choice == 1) {
                // Visualize the simulation
                drawCity(simulation.city);
            } else if (choice == 2) {
                // Ask for the road ID
                std::string roadIdStr = getRoadIdFromUser();
                int roadId = std::stoi(roadIdStr);

                // Set the entered road ID for analysis
                auto it = std::find_if(simulation.city.roads.begin(), simulation.city.roads.end(), [roadId](const Road& r) { return r.id == roadId; });
                if (it != simulation.city.roads.end()) {
                    simulation.trafficAnalysis.setRoadForAnalysis(&(*it));
                } else {
                    std::cerr << "No road found with the entered ID." << std::endl;
                    continue;
                }

                // Run the simulation
                simulation.run();

                // Export traffic data to CSV
                simulation.trafficAnalysis.exportCSV("traffic_data.csv");

                // After simulation, draw the histogram if there is data for the analyzed road
                if (!simulation.trafficAnalysis.carCounts.empty()) {
                    Road* analyzedRoad = simulation.trafficAnalysis.analysisRoad;
                    if (analyzedRoad && simulation.trafficAnalysis.carCounts.find(analyzedRoad) != simulation.trafficAnalysis.carCounts.end()) {
                        drawHistogram(simulation.trafficAnalysis.carCounts[analyzedRoad]);
                    }
                }
            } else if (choice == 0) {
                break;
            } else {
                std::cerr << "Invalid choice. Please enter 1, 2, or 0." << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }

    return 0;
}