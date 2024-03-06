/*
 * ProcessFlightData.c
 *
 *  Created on: Mar 4, 2024
 *      Author: sethm
 */

// When uploading to the payload bay micro, change this to true and the other INO to false

#include "bufferCopy.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <string>
#include <algorithm>
#include <ctime>
#include <random>

using namespace std;


enum Mode {
  BELOW_5K,
  WATCHING,
  PAST_APOGEE
};

string grabNextField(string& csvLine) {
	size_t len;
	string field = "";
	while (csvLine[0] == '"') {
		csvLine.erase(0, 2);
		len = csvLine.find('"');
		field += csvLine.substr(0, len);
		csvLine.erase(0, len + 1);
	}
	len = csvLine.find(',');
	field += csvLine.substr(0, len);
	csvLine.erase(0, len + 1);
	return field;
}

int main() {
	srand(time(NULL));
	std::default_random_engine rng(rand());
	const float STDEV = 1.4432f;
	std::normal_distribution<float> gauss(0, STDEV);

	for (int i = 0; i < 10; ++i) {
		//get input file to some data structure
		ifstream data("C:\\Users\\sethm\\OneDrive\\Desktop\\School\\Current_Classes\\Senior_Design\\Technical\\TestingStuff\\newTest.csv");

		float currTime;
		float currAlt;

		Buffer bufData;
		Mode mode = BELOW_5K;

		string line;
		while(getline(data,line)){
			string timeStr = grabNextField(line);
			currTime = strtof(timeStr.c_str(), NULL);
			string altStr = grabNextField(line);
			altStr.erase(std::remove<string::iterator, char>(altStr.begin(), altStr.end(), ','), altStr.end());
			currAlt = strtof(altStr.c_str(), NULL);

			currAlt += gauss(rng);

			switch (mode) {
			case BELOW_5K:
				if (currAlt >= 9000.0) { //right now below 30.0
					mode = WATCHING;
				}
				break;
			case WATCHING:
	//			cout << "Adding point " << currAlt << " at time: " << currTime << endl;
				bufData.addPoint(currAlt);
				if (bufData.isDecreasing()) { //If we notice our altitude is decreasing, we've reached apogee
					cout << "We hit apogee at time " << currTime << endl;
					mode = PAST_APOGEE; //Put us in 'PAST_APOGEE' mode
				}
				break;
			case PAST_APOGEE:
				// nothing to do
				break;
			}

		}
	}

	return 0;
}




