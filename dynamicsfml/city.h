// City.h
#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include "tinyxml2.h"
#include <algorithm>
#include <SFML/Graphics.hpp>


float mapWidth = 1920;
float mapHeight = 1080;

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
class Road {
public:
    int id;
    int length;
    pair<int, int> position; // Add position attribute
    Lane lanes[2];
    vector<Car*> carsOnRoad;
};

class Intersection {
public:
    vector<Road*> connectedRoads;
    pair<int, int> position; // Add position attribute

    void calculatePosition() {
        int x = 0;
        int y = 0;

        Road* roa = connectedRoads[0];
        Road* roa2 = connectedRoads[1];
        x = (roa->position.first == roa2->position.first) ? roa->position.first : (roa->position.first + roa->length);
        y = (roa->position.second == roa2->position.second) ? roa->position.second : (roa->position.second);

        position = make_pair(x, y);
    }

};


class Car {
public:
    int size;
    pair<int, int> origin;
    pair<int,int> destination;
    pair<double, double> position;
    Road* currentRoad;
    string currentLane;
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
        }
        else {
            cerr << "Unable to open file for writing." << endl;
        }
    }

};

class City {
public:
    vector<Building> buildings;
    vector<Road> roads;
    vector<Intersection> intersections;

    void reAdjustBuildings()
    {
        for (Road& road : roads)
        {
            for (Building& building : buildings)
            {
                if (building.position.first == road.position.first)
                {
                    if (road.lanes->direction == "east" || road.lanes->direction == "west")
                        building.position.second += 20;
                    else
                        building.position.first += 20;
                }
                if (building.position.second == road.position.second)
                {
                    if (road.lanes->direction == "east" || road.lanes->direction == "west")
                        building.position.second += 20;
                    else
                        building.position.first += 20;
                }
            }
        }
    }
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
        }
        else {
            cerr << "Could not load XML file." << endl;
        }
        city.reAdjustBuildings();
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
            intersection.calculatePosition();
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


void drawHistogram(const std::vector<int>& carCounts) 
{
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
    bool flag = 1;
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
            yAxisValueText.setPosition(i * barWidth + 2.5 , windowHeight - barHeight -70);
            yAxisValueText.setFillColor(sf::Color::Black);
           window.draw(yAxisValueText);

            // Display y-axis label
            sf::Text yAxisLabelText("Car\nCount", font, 6);
            yAxisLabelText.setPosition((i * barWidth + 2.5) + barWidth / 2, windowHeight - barHeight -70);
            yAxisLabelText.setFillColor(sf::Color::Black);
            window.draw(yAxisLabelText);
            
        }

        // Drawing text labels on the x-axis
        for (int i = 0; i < 24; ++i) {
            sf::Text text(std::to_string(i), font, 12);
            text.setPosition(i * barWidth +10, windowHeight - 50);
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
    inputBox.setPosition(220,50);

    std::string roadId;

    while (window.isOpen()) {

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode < 128) {
                    if (event.text.unicode == '\b' && !roadId.empty()) {
                        roadId.pop_back();
                    }
                    else if (event.text.unicode == '\r') {
                        window.close();
                    }
                    else {
                        roadId += static_cast<char>(event.text.unicode);
                    }
                }
            }
        }

        text.setString("Enter Road ID: " + roadId);

        window.clear(sf::Color::Cyan);
        window.draw(inputBox);
        window.draw(text);
       
        window.display();
    }

    return roadId;
}
Road findClosestRoad(vector<Road> roads, pair<int, int> position)
{
	Road closestRoad;
	int distance = INT_MAX;
    int temp;
	for (Road road : roads)
	{
        temp = min(abs(road.position.first - position.first), abs(road.position.second - position.second));
        if (temp < distance)
        {
			distance = temp;
			closestRoad = road;
        }
	}
	return closestRoad;

}
void generateCars(City& city)
{
    srand(time(NULL));
    pair<int, int> origin = city.buildings[rand() % city.buildings.size()].position;
    pair<int, int> destination = city.buildings[rand() % city.buildings.size()].position;
    while (origin == destination)
        destination = city.buildings[rand() % city.buildings.size()].position;

    Car* car = new Car();
    car->origin= origin;
    car->destination = destination;
    Road* r1 = new Road(findClosestRoad(city.roads,origin));
    car->currentRoad = r1;
    Lane& lane = car->currentRoad->lanes[rand() % 2];
    bool es= 0;
    car->currentLane = lane.direction;
    if (lane.direction == "east" || lane.direction == "west")
    {
        if (destination.second < origin.second)
        {
            es = 0;
            car->position.first = origin.first;
            car->position.second = car->currentRoad->position.second + 8;
        }
        else
        {
            es = 1;
            car->position.first = car->origin.first;
            car->position.second = car->currentRoad->position.second;
        }
       
	}
    else
    {
        if (destination.first < origin.first)
        {
            car->position.first = origin.first + 8;
            car->position.second = car->currentRoad->position.second;
            es = 0;
        }
        else
        {
            car->position.first = origin.first;
            car->position.second = car->currentRoad->position.second;
            es = 1;
        }
    }
    if (car->currentRoad->carsOnRoad.size() <= 5)
    {
        bool flag = 0;
        for (Car* c1 : car->currentRoad->carsOnRoad)
        {
			if (c1->position == car->position)
			{
				flag = 1;
				break;
			}
        }
        if (!flag)
        {
            for (Road& road : city.roads)
            {
				if (road.id == car->currentRoad->id)
				{
					road.carsOnRoad.push_back(car);
                    if (es)
                        road.lanes[0].carsInLane.push_back(car);
                    else
                        road.lanes[1].carsInLane.push_back(car);
                    car->currentLane= road.lanes[!es].direction;
					break;
				}
            }
        }
    }
}
void moveCars(City& city)
{
    for (Road& road : city.roads)
    {
        for (Lane& lane : road.lanes)
        {
            for (Car* car : lane.carsInLane)
            {
                if (lane.direction == "east")
                    car->position.first += 1.5;
                else if(lane.direction == "west")
                    car->position.first -= 1.5;
                else if (lane.direction == "south")
                    car->position.second += 1.5;
                else
                    car->position.second -= 1.5;
            }
        }
    }
}
void deleteCars(City& city)
{
    for (Road& road : city.roads)
    {
        for (Car* car : road.carsOnRoad)
        {
            if (car->position.first < 0 || car->position.first > mapWidth || car->position.second < 0 || car->position.second > mapHeight) {
                // Remove the car from the current road
                road.carsOnRoad.erase(std::remove(road.carsOnRoad.begin(), road.carsOnRoad.end(), car), road.carsOnRoad.end());

                // Free the memory allocated for the car
                delete car;

                // Set the car pointer to nullptr to avoid using a deleted pointer
                car = nullptr;
            }
        }
    }
}
void checkInterSections(City& city)
{
    for (Road& road : city.roads)
    {
        for (Car* car : road.carsOnRoad)
        {
            for (Intersection& inters : city.intersections)
            {
                if (car != 0)
                {
                    if (inters.position.first-5 <= car->position.first && inters.position.first + 14 >= car->position.first && inters.position.second-15 <= car->position.second && inters.position.second + 20 >= car->position.second)
                    {
                        for (Road* r : inters.connectedRoads)
                        {
                            if (r->id != car->currentRoad->id)
                            {
                                Car* c1 = new Car(*car);
                                r->carsOnRoad.push_back(c1);
                                road.carsOnRoad.erase(std::remove(road.carsOnRoad.begin(), road.carsOnRoad.end(), car), road.carsOnRoad.end());
                                if (r->lanes[0].direction == "east" || r->lanes[1].direction == "west")
                                {
                                    if (car->destination.first < car->position.first)// sending west
                                    {
                                        r->lanes[1].carsInLane.push_back(c1);
                                        if (car->currentLane == "south")
                                        {
                                            c1->position.first = inters.position.first - 30;
                                            c1->position.second = inters.position.second + 8;
                                        }
                                        else
                                        {
                                            c1->position.first = inters.position.first - 30;
                                            c1->position.second = inters.position.second + 8;
                                        }
                                        c1->currentRoad = r;
                                        c1->currentLane = r->lanes[1].direction;
                                    }
                                    else// sending east
                                    {
                                        r->lanes[0].carsInLane.push_back(c1);

                                        if (car->currentLane == "south")
                                        {
                                            c1->position.first = inters.position.first + 30;
                                            c1->position.second = inters.position.second;
                                        }
                                        else
                                        {
                                            c1->position.first = inters.position.first + 30;
                                            c1->position.second = inters.position.second;
                                        }
                                        c1->currentRoad = r;
                                        c1->currentLane = r->lanes[0].direction;

                                    }
                                    
                                }
                                else
                                {
                                    if (car->destination.second > car->position.second) //sending south
                                    {
										r->lanes[0].carsInLane.push_back(c1);              
                                        if (car->currentLane == "east")                    
                                        {                                                  
                                            c1->position.first = inters.position.first + 8;
                                            c1->position.second = inters.position.second + 30;
										}                                                  
										else                                               
										{                                                  
                                            c1->position.first = inters.position.first + 8;
                                            c1->position.second = inters.position.second + 30;
                                        }                                                  
                                        c1->currentLane = r->lanes[0].direction;           
									}
									else// sending North
									{
										r->lanes[1].carsInLane.push_back(c1);
                                        if (car->currentLane == "east")
                                        {
                                            c1->position.first = inters.position.first;
											c1->position.second = inters.position.second -30;
										}
                                        else
                                        {
                                            c1->position.first = inters.position.first;
                                            c1->position.second = inters.position.second - 30;
                                        }
                                        c1->currentLane = r->lanes[1].direction;
                                    }
                                }
								c1->currentRoad = r;
                                road.lanes[1].carsInLane.erase(std::remove(road.lanes[1].carsInLane.begin(), road.lanes[1].carsInLane.end(), car), road.lanes[1].carsInLane.end());
                                road.lanes[0].carsInLane.erase(std::remove(road.lanes[0].carsInLane.begin(), road.lanes[0].carsInLane.end(), car), road.lanes[0].carsInLane.end());
                                //delete car;
                            }
                        }
                    }
                }
            }
        }
    }
}
void drawCity(City& city) {

    // Increase the window size to fit the map of 1000x1000
    const int windowWidth = 1200;
    const int windowHeight = 1200;

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "City Visualization");
    sf::View view = window.getDefaultView(); // Get the default view

    // Define the map area dimensions and position to be 1000x1000
    float mapAreaX = (windowWidth - mapWidth) / 2;
    float mapAreaY = (windowHeight - mapHeight) / 2;

    // Create the map border
    sf::RectangleShape border(sf::Vector2f(mapWidth, mapHeight));
    border.setPosition(mapAreaX, mapAreaY);
    border.setFillColor(sf::Color::Green); // Set the map area to green
    border.setOutlineThickness(-5); // Negative thickness to draw inside the shape
    border.setOutlineColor(sf::Color::Black); // Set border color to b

    sf::Clock clock1;
    sf::Clock clock2;


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

        for (const Road& road : city.roads) {
            for (const Lane& lane : road.lanes) {
                sf::RectangleShape line;
                if (lane.direction == "east" || lane.direction == "west") {
                    // Ensure the road does not exceed map width
                    int actualLength = std::min(road.length, static_cast<int>(mapWidth - road.position.first));
                    line = sf::RectangleShape(sf::Vector2f(actualLength, 8)); // 8 pixels thick lane
                    line.setPosition(road.position.first, road.position.second + (lane.direction == "east" ? 0 : 8));
                }
                else {
                    // Ensure the road does not exceed map height
                    int actualLength = std::min(road.length, static_cast<int>(mapHeight - road.position.second));
                    line = sf::RectangleShape(sf::Vector2f(8, actualLength)); // 8 pixels thick lane
                    line.setPosition(road.position.first + (lane.direction == "north" ? 0 : 8), road.position.second);
                }
                line.setFillColor(sf::Color(128, 128, 128)); // Set lane color to gray
                line.setOutlineThickness(1); // Set border thickness
                line.setOutlineColor(sf::Color::White); // Set border color to white
                window.draw(line);
            }
        }
        // Draw buildings
        for (const Building& building : city.buildings) {
            sf::RectangleShape rectangle(sf::Vector2f(25, 25));
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
        //DRAW INTERSECTIONS

        for (const Intersection& inters : city.intersections) {

            sf::RectangleShape box;
            box = sf::RectangleShape(sf::Vector2f(20, 20)); // 8 pixels thick lane
            box.setPosition(inters.position.first, inters.position.second);
            box.setFillColor(sf::Color(128, 128, 128)); // Set lane color to gray
            box.setOutlineThickness(1); // Set border thickness
            box.setOutlineColor(sf::Color::White); // Set border color to white
            window.draw(box);

        }


        // Draw cars
        float time1 = clock1.getElapsedTime().asSeconds();
        float time2 = clock2.getElapsedTime().asSeconds();
        if (time2 >= 3)
        {
            generateCars(city);
            clock2.restart();
        }
        if (time1 >= 0.01)
        {
            moveCars(city);
            clock1.restart();
        }
        checkInterSections(city);
     //   deleteCars(city);


        for (const Road& road : city.roads) {
            for (const Car* car : road.carsOnRoad) {
                if (car) {
                    sf::CircleShape circle(4);
                    circle.setPosition(car->position.first, car->position.second);
                    circle.setFillColor(sf::Color::Yellow);
                    window.draw(circle);
                }
            }
        }
        window.display();
    }
}