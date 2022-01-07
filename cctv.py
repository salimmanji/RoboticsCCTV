import ftplib
from picamera import PiCamera
import time
from time import sleep
from datetime import datetime
import os
import serial
import boto3
import json
import smtplib

# Create a camera object
camera = PiCamera()
# Open serial communication between RPi and Arduino using baud rate 9600.
ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
ser.flush()

counter = 0
completeSignal = 1
# AWS Information
bucket = 'smp-images'

while(1): #infinte loop
    arduinoInput = ser.readline().decode('utf-8', 'replace').rstrip() # Read in arduino input to determine if motion has been detected.
    print("Counter: ")
    print(counter)
    print("Checking messages:")
    print("arduinoInput: " )
    print(arduinoInput)
    if arduinoInput == '5': # Motion has been detected
        print("Motion has been detected")
        sleep(2) # Adjust camera for ISO sensitivity
        filename = "{0:%Y}.{0:%m}.{0:%d}-{0:%H}.{0:%M}.{0:%S}".format(datetime.now()) # Create timestamp
        imagePath = '/home/pi/Desktop/images/' + filename + '.jpg' # Concat timestamp into photo file name.
        videoPath = '/home/pi/Desktop/images/' + filename + '.h264' # Concat time stamp into video file name.
        camera.resolution = (2592,1944) # Set Camera Resolution for still images.
        camera.capture(imagePath)
        uploader = boto3.client('s3') # AWS details, linking to my AWS bucket.
        client = boto3.client('rekognition')
        uploader.upload_file(imagePath, bucket, imagePath)
        response = client.detect_faces(Image={'S3Object':{'Bucket':bucket,'Name':imagePath}},Attributes=['ALL']) # Returns a JSON object with analytic information.
        face_count=len(response['FaceDetails']) # Count number of faces.
        print("NumFaces: " + str(face_count))
        if face_count > 0:
            sleep(2) # Adjust camera for ISO sensitivity (gain). 
            camera.resolution = (1920,1080) # Set Camera Resolution for .h264 video files.
            camera.framerate = 15 
            sleep(2)
            camera.start_recording(videoPath)
            sleep(10) # Record a 10 second clip.
            camera.stop_recording()
            sleep(2)
            # Send email to owner.
            server = smtplib.SMTP('smtp.gmail.com', 587)
            server.starttls()
            server.login('RPI_EMAIL', 'RPI_PASSWORD')
            msg = 'Security Breach!'
            server.sendmail('RPI_EMAIL', 'TO_ADDRESS', msg)
            server.quit()
            # Run bash script to upload image and video files.
            os.system("/home/pi/Desktop/upload")
        else:
            # Run bash script to delete the last image taken (no face detected).
            os.system("/home/pi/Desktop/delete")
        print("Camera functionality complete.")
    for x in range(5): # Spam the Arduino to return to Armed state.
        ser.write(str(completeSignal).encode('ascii') + b'\n')
    print("Restarting.")
    counter+=1
    sleep(2)
    
