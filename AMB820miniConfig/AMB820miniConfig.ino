/*

 Example guide:
 https://www.amebaiot.com/en/amebapro2-arduino-neuralnework-object-detection/

 NN Model Selection
 Select Neural Network(NN) task and models using modelSelect(nntask, objdetmodel, facedetmodel, facerecogmodel).
 Replace with NA_MODEL if they are not necessary for your selected NN Task.

 NN task
 =======
 OBJECT_DETECTION/ FACE_DETECTION/ FACE_RECOGNITION

 Models
 =======
 YOLOv3 model         DEFAULT_YOLOV3TINY   / CUSTOMIZED_YOLOV3TINY
 YOLOv4 model         DEFAULT_YOLOV4TINY   / CUSTOMIZED_YOLOV4TINY
 YOLOv7 model         DEFAULT_YOLOV7TINY   / CUSTOMIZED_YOLOV7TINY
 SCRFD model          DEFAULT_SCRFD        / CUSTOMIZED_SCRFD
 MobileFaceNet model  DEFAULT_MOBILEFACENET/ CUSTOMIZED_MOBILEFACENET
 No model             NA_MODEL
 */

#include "WiFi.h"
#include "StreamIO.h"
#include "VideoStream.h"
#include "RTSP.h"
#include "NNObjectDetection.h"
#include "VideoStreamOverlay.h"
#include "ObjectClassList.h"

#define CHANNEL   0
#define CHANNELNN 3

// Lower resolution for NN processing
#define NNWIDTH  576
#define NNHEIGHT 320

VideoSetting config(VIDEO_D1, 24, VIDEO_JPEG, 0);
VideoSetting configNN(NNWIDTH, NNHEIGHT, 10, VIDEO_RGB, 0);
NNObjectDetection ObjDet;
RTSP rtsp;
StreamIO videoStreamer(1, 1);
StreamIO videoStreamerNN(1, 1);
CameraSetting configCam;

char ssid[] = "";    // your network SSID (name)
char pass[] = "";        // your network password
int status = WL_IDLE_STATUS;

IPAddress ip;
int rtsp_portnum;

void setup()
{
    Serial.begin(115200);

    // attempt to connect to Wifi network:
    while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to WPA SSID: ");
        Serial.println(ssid);
        status = WiFi.begin(ssid, pass);

        // wait 2 seconds for connection:
        delay(2000);
    }
    ip = WiFi.localIP();

    Camera.camInit(configCam);
    
    // Configure camera video channels with video format information
    // Adjust the bitrate based on your WiFi network quality
    config.setBitrate(2 * 1024 * 1024);    // Recommend to use 2Mbps for RTSP streaming to prevent network congestion
    config.setJpegQuality(3);
    Camera.configVideoChannel(CHANNEL, config);
    Camera.configVideoChannel(CHANNELNN, configNN);
    Camera.videoInit();

    // Configure RTSP with corresponding video format information
    rtsp.configVideo(config);
    rtsp.begin();
    rtsp_portnum = rtsp.getPort();

    // Configure object detection with corresponding video format information
    // Select Neural Network(NN) task and models

    ObjDet.configVideo(configNN);
    ObjDet.modelSelect(OBJECT_DETECTION, DEFAULT_YOLOV7TINY, NA_MODEL, NA_MODEL);
    ObjDet.configThreshold(0.7, 0.5);
    ObjDet.begin();

    // Configure StreamIO object to stream data from video channel to RTSP
    videoStreamer.registerInput(Camera.getStream(CHANNEL));
    videoStreamer.registerOutput(rtsp);
    if (videoStreamer.begin() != 0) {
        Serial.println("StreamIO link start failed");
    }

    // Start data stream from video channel
    Camera.channelBegin(CHANNEL);

    // Configure StreamIO object to stream data from RGB video channel to object detection
    videoStreamerNN.registerInput(Camera.getStream(CHANNELNN));
    videoStreamerNN.setStackSize();
    videoStreamerNN.setTaskPriority();
    videoStreamerNN.registerOutput(ObjDet);
    if (videoStreamerNN.begin() != 0) {
        Serial.println("StreamIO link start failed");
    }

    // Start video channel for NN
    Camera.channelBegin(CHANNELNN);

    // Start OSD drawing on RTSP video channel
    OSD.configVideo(CHANNEL, config);
    OSD.begin();
}

void loop()
{
    std::vector<ObjectDetectionResult> results = ObjDet.getResult();

    uint16_t im_h = config.height();
    uint16_t im_w = config.width();

    // Serial.print("Network URL for RTSP Streaming: ");
    // Serial.print("rtsp://");
    // Serial.print(ip);
    // Serial.print(":");
    // Serial.println(rtsp_portnum);
    // Serial.println(" ");

    // printf("Total number of objects detected = %d\r\n", ObjDet.getResultCount());
    OSD.createBitmap(CHANNEL);

    if (ObjDet.getResultCount() > 0) {
        for (int i = 0; i < ObjDet.getResultCount(); i++) {
            int obj_type = results[i].type();
            if (itemList[obj_type].filter) {    // check if item should be ignored

                ObjectDetectionResult item = results[i];
                // Result coordinates are floats ranging from 0.00 to 1.00
                // Multiply with RTSP resolution to get coordinates in pixels
                int xmin = (int)(item.xMin() * im_w);
                int xmax = (int)(item.xMax() * im_w);
                int ymin = (int)(item.yMin() * im_h);
                int ymax = (int)(item.yMax() * im_h);

                // Draw boundary box
                // printf("Item %d %s:\t%d %d %d %d\n\r", i, itemList[obj_type].objectName, xmin, xmax, ymin, ymax);
                OSD.drawRect(CHANNEL, xmin, ymin, xmax, ymax, 3, OSD_COLOR_WHITE);

                // Print identification text
                char text_str[20];
                snprintf(text_str, sizeof(text_str), "%s %d", itemList[obj_type].objectName, item.score());
                OSD.drawText(CHANNEL, xmin, ymin - OSD.getTextHeight(CHANNEL), text_str, OSD_COLOR_CYAN);
            }
        }
    }
    OSD.update(CHANNEL);

    // delay to wait for new results
    delay(100);

    if (Serial.available() > 0) {
        String input = Serial.readString();
        input.trim();

        if (input.startsWith(String("AE="))) {
            String value = input.substring(3);
            int val = value.toInt();
            configCam.setExposureMode(val);
        } else if (input.startsWith(String("AE"))) { // 0 (Manual), 1 (Auto)
            configCam.getExposureMode();
        } else if (input.startsWith(String("JPEGQ="))) { // 1 (worst) to 9 (best)
            String value = input.substring(8);
            int val = value.toInt();
            config.setJpegQuality(val);
        } else if (input.startsWith(String("FPS="))) { // 15 - 30
            String value = input.substring(8);
            int val = value.toInt();
            Camera.setFPS(val);
        } else if (input.startsWith(String("FPS"))) {
            Serial.println(config.fps());
        } else if (input.startsWith(String("DAYNIGHT="))) { // 0 (Day mode), 1 (Night mode)
            String value = input.substring(8);
            int val = value.toInt();
            configCam.setDayNightMode(val);
        } else if (input.startsWith(String("DAYNIGHT"))) {
            configCam.getDayNightMode();
        } else if (input.startsWith(String("EXPTIME="))) { // Valid value must be less than or equal to 33333us
            String value = input.substring(8);
            int val = value.toInt();
            configCam.setExposureTime(val);
        } else if (input.startsWith(String("EXPTIME"))) {
            configCam.getExposureTime();
        } else if (input.startsWith(String("GAIN="))) { // 256 - 32768
            String value = input.substring(5);
            int val = value.toInt();
            configCam.setAEGain(val);
        } else if (input.startsWith(String("GAIN"))) {
            configCam.getAEGain();
        } else if (input.startsWith(String("AWB="))) { // 0 (Manual Temperature), 1 (Auto)
            String value = input.substring(5);
            int val = value.toInt();
            configCam.setAWB(val);
        } else if (input.startsWith(String("AWB"))) {
            configCam.getAWB();
        } else if (input.startsWith(String("WDR="))) { // 0 (Disabled), 1 (Manual)
            String value = input.substring(4);
            int val = value.toInt();
            configCam.setWDR(val);
        } else if (input.startsWith(String("WDR"))) {
            configCam.getWDR();
        } else if (input.startsWith(String("LEVEL="))) { // 50 -100 Preferred Wide Dynamic Range level take effect only if WDR is set to Manual
            String value = input.substring(6);
            int val = value.toInt();
            configCam.setWDRLevel(val);
        } else if (input.startsWith(String("LEVEL"))) {
            configCam.getWDRLevel();
        } else if (input.startsWith(String("PLF="))) { // 0 (Disable), 1 (50Hz), 2: (60Hz), 3 (Auto)
            String value = input.substring(4);
            int val = value.toInt();
            configCam.setPowerLineFreq(val);
        } else if (input.startsWith(String("PLF"))) {
            configCam.getPowerLineFreq();
        } else if (input.startsWith(String("BRIGHTNESS="))) { // -64 to 64
            String value = input.substring(11);
            int val = value.toInt();
            configCam.setBrightness(val);
        } else if (input.startsWith(String("BRIGHTNESS"))) {
            configCam.getBrightness();
        } else if (input.startsWith(String("CONTRAST="))) { // 0 - 100
            String value = input.substring(9);
            int val = value.toInt();
            configCam.setContrast(val);
        } else if (input.startsWith(String("CONTRAST"))) {
            configCam.getContrast();
        } else if (input.startsWith(String("SATURATION="))) { // 0 - 100
            String value = input.substring(11);
            int val = value.toInt();
            configCam.setSaturation(val);
        } else if (input.startsWith(String("SATURATION"))) {
            configCam.getSaturation();
        } else if (input.startsWith(String("SHARPNESS="))) { // 0 - 100
            String value = input.substring(10);
            int val = value.toInt();
            configCam.setSharpness(val);
        } else if (input.startsWith(String("SHARPNESS"))) {
            configCam.getSharpness();
        } else if (input.startsWith(String("LDC="))) { // 0 (Disabled) and 1(Enabled)
            String value = input.substring(4);
            int val = value.toInt();
            configCam.setLDC(val);
        } else if (input.startsWith(String("LDC"))) {
            configCam.getLDC();
        } else if (input.startsWith(String("RESET"))) {
            configCam.reset();
        }
    }
}
