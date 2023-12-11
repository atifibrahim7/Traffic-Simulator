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
#include "city.h"



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