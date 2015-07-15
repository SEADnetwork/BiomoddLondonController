#pragma once

#include "ofMain.h"
#include "ofxJSON.h"
#include "ofxUI.h"

#define DATA_AMNT 10
#define ACTUATOR_1_MTIME 25
#define ACTUATOR_2_MTIME 25

#define ACTUATOR_1_PIN  3
#define ACTUATOR_2_PIN  4

#define SENSOR_1_PIN    0
#define SENSOR_2_PIN    1
#define SENSOR_3_PIN    2

#define FALSE_DATA      -1

#define ARDUINO         false

struct Sensor {
    Sensor(){
        for (int i(0); i < DATA_AMNT; i++){
            rawData.push_back(0);
            averageData.push_back(0);
        }
    }
    
    vector<float> rawData;
    vector<float> averageData;
};

class ofApp : public ofBaseApp{

	public:
    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    void urlResponse(ofHttpResponse & response);
    void guiEvent(ofxUIEventArgs &e);
    void setGUI();

    void setBackgroundImage();
    string getPostUrl(const unsigned int sensor, const float data);
    void updateSensors();
    void drawBackgroundImage();
    void drawData();
    void initArduino();
    void setupArduino(const int & version);
    void enableActuator(unsigned int nr);
    
    

    Sensor * sensors;
    ofImage img;
    ofxUISuperCanvas *gui;
    ofxUIMovingGraph *sensor1;
    ofxUIMovingGraph *sensor2;
    ofxUIMovingGraph *sensor3;
    ofxUITextArea    *arduinoText;
    ofxUITextArea    *generalText;
    unsigned  int failedAttempts;
    ofArduino arduino;
    int actuator1time;
    int actuator2time;
    ofxUITextArea *actuator1text;
    ofxUITextArea *actuator2text;
    bool simulate;
    
    const string SERVER = "http://biomoddlondon-sead.rhcloud.com";
};
