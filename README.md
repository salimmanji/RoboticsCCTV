# RoboticsCCTV
CCTV system built with RPI and Arduino.
A microwave/doppler sensor detects motion, tripping the RPi to expose an image.
The image is sent to AWS Rekognition to detect faces. If a face is found, a video clip
is taken and uploaded via FTP to my NAS. The local files are deleted.
