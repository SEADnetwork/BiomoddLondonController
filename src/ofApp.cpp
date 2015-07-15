#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    sensors = new Sensor[3];
    simulate = false;
    setBackgroundImage();
    setGUI();
#if ARDUINO
    initArduino();
#endif
    failedAttempts = 0;
    
    // listen for EInitialized notification. this indicates that
    // the arduino is ready to receive commands and it is safe to
    // call setupArduino()
    initArduino();
    ofAddListener(arduino.EInitialized, this, &ofApp::setupArduino);
}

//--------------------------------------------------------------
void ofApp::update(){
#if ARDUINO
    arduino.update();
#endif
    updateSensors();
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackgroundGradient(ofColor::cyan, ofColor::purple);
    drawBackgroundImage();
    drawData();
}

void ofApp::drawData(){
    ofPushMatrix();
    ofTranslate(ofGetWidth()/4, 0);
    for (int i(0); i < 3 ; i++) {
        ofTranslate(0, ofGetHeight()/4);
        ofDrawBitmapString("sensor"+ofToString(i), -100, -50);
        ofPoint prevpoint = ofPoint(0, 0);
        for (int x(0); x < sensors[i].averageData.size(); x++) {
            ofPoint pt;
            pt.x = x * 60;
            pt.y = ofMap(sensors[i].averageData[x], 0, 1, -20, 20);
            ofDrawCircle(pt, 10);
            ofLine(pt, prevpoint);
            prevpoint = pt;
        }
    }
    ofPopMatrix();
}

//--------------------------------------------------------------

void ofApp::setBackgroundImage(){
    img.load("logo.png");
    img.resize(ofGetHeight()/img.getHeight()*img.getWidth(), ofGetHeight());
}

void ofApp::drawBackgroundImage(){
    ofPushStyle();
    ofPushMatrix();
    ofSetColor(255,100);
    ofTranslate((ofGetWidth()-img.getWidth())/2, (ofGetHeight()-img.getHeight())/2);
    img.draw(0,0);
    ofPopMatrix();
    ofPopStyle();
}


//--------------------------------------------------------------
string ofApp::getPostUrl(const unsigned int sensor, const float data){
    string url = SERVER+"/sensor/postSensor?d=";
    
    url+=ofToString(data);
    url+="&s=";
    url+=ofToString(sensor);
    return url;
}

float getSimulatedData(){
    return ofRandom(10);
}

void ofApp::updateSensors(){
    const unsigned int frequency = 100;
    int sensor = (int)ofRandom(100)%3;
    
    // send data
    if ((int)ofGetFrameNum()%frequency==0) {
        
        float data;
    
#if ARDUINO
#else
        simulate = true;
#endif
        
        if (simulate) {
            arduinoText->setTextString("simulate arduino connection");
            data = getSimulatedData();
        } else {
            arduinoText->setTextString("arduino connected");
            data = arduino.getAnalog(sensor);
            if (data==FALSE_DATA) {
                data = getSimulatedData();
            }
        }
        
        ofxJSONElement result;
        bool parsing = result.open(getPostUrl(sensor, data));
        
        if (parsing) {
            generalText->setTextString("succesfully sended data");
        } else {
            cout << "not parsed: error" << endl;
        }
    }
    
    //receive data
    if ((int)ofGetFrameNum()%frequency==floor(frequency/2)) {
        const string url = SERVER+"/sensor/getSensors";
        
        ofHttpResponse resp = ofLoadURL(url);
        generalText->setTextString("data received");
        
        ofxJSONElement json;
        bool parsingSuccessful = json.parse(resp.data);
        
        if (parsingSuccessful){
            for (int i(0); i < 3; i++) {
                sensors[i].rawData.clear();
                sensors[i].averageData.clear();
                
                for (Json::ArrayIndex rd = 0; rd < json[i]["raw"].size(); rd++) {
                    sensors[i].rawData.push_back(ofToFloat(json[i]["raw"][rd].asString()));
                    sensors[i].averageData.push_back(ofToFloat(json[i]["averageData"][rd].asString()));
                }
            }
            sensor1->setBuffer(sensors[0].averageData);
            sensor2->setBuffer(sensors[1].averageData);
            sensor3->setBuffer(sensors[2].averageData);
        } else {
            ofLogNotice("ofApp::setup")  << "Failed to parse JSON" << endl;
        }
    }
    
    //update actuators
    // actuator 1
    //can be improved by erasing duplication
    if (actuator1time > 0) {
        int remainingTime = actuator1time-ofGetElapsedTimef();
        if (remainingTime <= 0) {
            actuator1time = 0;
            arduino.sendDigital(3, ARD_LOW);
            actuator1text->setTextString("actuator 1 disabled");
        } else {
            actuator1text->setTextString("actuator 1: "+ofToString(remainingTime)+" seconds remaining");
        }
    }
    
    if (actuator2time > 0) {
        int remainingTime = actuator2time-ofGetElapsedTimef();
        if (remainingTime <= 0) {
            actuator2time = 0;
            arduino.sendDigital(4, ARD_LOW);
            actuator1text->setTextString("actuator 2 disabled");
        } else {
            actuator1text->setTextString("actuator 2: "+ofToString(remainingTime)+" seconds remaining");
        }
    }
}


void ofApp::initArduino(){
    ofSerial serial;
    auto devicelist = serial.getDeviceList();
    
    for(auto it(devicelist.begin()); it != devicelist.end(); it++){
        cout << (*it).getDeviceID() << "--" << (*it).getDeviceName() << "--" << (*it).getDevicePath() << endl;
    }
    
    arduino.connect("/dev/cu.usbserial-A9007Lns", 57600);
}

void ofApp::setupArduino(const int & version){
    if (!arduino.isArduinoReady()){
        ofLogError("arduino connection failure");
        arduinoText->setTextString("arduino initial connection failure");
    }
    
    //set digital pins
    arduino.sendDigitalPinMode(ACTUATOR_1_PIN, ARD_OUTPUT);
    arduino.sendDigitalPinMode(ACTUATOR_2_PIN, ARD_OUTPUT);
    
    //set analog pins
    arduino.sendAnalogPinReporting(SENSOR_1_PIN, ARD_ANALOG);
    arduino.sendAnalogPinReporting(SENSOR_2_PIN, ARD_ANALOG);
    arduino.sendAnalogPinReporting(SENSOR_3_PIN, ARD_ANALOG);
}

void ofApp::enableActuator(unsigned int nr){

    
    float inittimme = ofGetElapsedTimef();
    if (nr==1) {
        arduino.sendDigital(ACTUATOR_1_PIN, ARD_HIGH);
        actuator1time = inittimme+ACTUATOR_1_MTIME;
    } else {
        arduino.sendDigital(ACTUATOR_2_PIN, ARD_HIGH);
        actuator2time = inittimme+ACTUATOR_2_MTIME;
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch (key) {
        case 'h':
            gui->toggleMinified();
            break;
            
        default:
            break;
    }

}

void ofApp::setGUI(){
    
    gui = new ofxUISuperCanvas("BIOMODD LONDON");
    gui->addSpacer();
    gui->addLabel("Press 'h' to Hide GUIs", OFX_UI_FONT_SMALL);
    gui->addSpacer();
    gui->addLabel("SERVER");
    gui->addLabel("SENSORS");
    gui->addLabel("");
    gui->addToggle("simulate", simulate);
    gui->addLabel("");
    sensor1 = gui->addMovingGraph("SENSOR 1", sensors[0].averageData, DATA_AMNT, 0.0, 1.0);
    gui->addTextArea("s1", "SENSOR1");
    sensor2 = gui->addMovingGraph("SENSOR 2", sensors[1].averageData, DATA_AMNT, 0.0, 1.0);
    gui->addTextArea("s1", "SENSOR2");
    sensor3 = gui->addMovingGraph("SENSOR 3", sensors[2].averageData, DATA_AMNT, 0.0, 1.0);
    gui->addTextArea("s1", "SENSOR3");
    
    gui->addLabel("");
    gui->addLabelButton("enable actuator 1", false);
    actuator1text = gui->addTextArea("", "actuator 1 disabled");
    
    gui->addLabel("");
    gui->addLabelButton("enable actuator 2", false);
    actuator2text = gui->addTextArea("", "actuator 2 disabled");
    
    gui->addSpacer();
    gui->addLabel("");
    gui->addLabel("GAME");
    
    gui->addLabel("");
    gui->addLabel("ARDUINO");
    arduinoText = gui->addTextArea("ArduinoText", "arduino running");
    
    gui->addLabel("");
    gui->addSpacer();
    generalText = gui->addTextArea("generalTextt", "general information display will occur here");
    
    
    gui->autoSizeToFitWidgets();
    ofAddListener(gui->newGUIEvent,this,&ofApp::guiEvent);
}

void ofApp::guiEvent(ofxUIEventArgs &e){
    string name = e.getName();
    int kind = e.getKind();
    
    if (name=="simulate") {
        simulate = ((ofxUIToggle *)e.widget)->getValue();
    } else if (name == "enable actuator 1"){
        enableActuator(1);
    } else if (name == "enable actuator 2"){
        enableActuator(2);
    }
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
