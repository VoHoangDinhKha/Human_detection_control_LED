import cv2
import cvlib as cv
from cvlib.object_detection import draw_bbox
import numpy as np
import urllib.request
import websocket

url = 'http://192.168.1.19/640x480.jpg'

boxes=[]
#Thêm 2 hình chữ nhật bên trái
for i in range(2):
    x1 = 0
    y1 = i * 240
    x2 = 320
    y2 = (i + 1) * 240
    boxes.append(((x1, y1), (x2, y2)))

# Thêm 2 hình chữ nhật bên phải
for i in range(2):
    x1 = 320
    y1 = i * 240
    x2 = 640
    y2 = (i + 1) * 240
    boxes.append(((x1, y1), (x2, y2)))
# boxes = [((0, 0), (512, 384)), ((0, 384), (512, 768)), ((512, 0), (1024, 384)), ((512, 384), (1024, 768))]
person_boxes = [0, 0, 0, 0]

def send_position(position):
    ws = websocket.WebSocket()
    ws.connect("ws://192.168.1.19:81")

    # Send value to the ESP32
    ws.send(str(position))

    ws.close()

def update_person_boxes(persons):
    # Reset person_boxes to all zeros
    person_boxes = [0, 0, 0, 0]

    for person in persons:
        x, y, w, h = person

        for i, box in enumerate(boxes):
            if (x >= box[0][0] or (x + w) >= box[0][0]) and \
            (y >= box[0][1] or (y + h) >= box[0][1]) and \
            (x <= box[1][0] or (x + w) <= box[1][0]) and \
            (y <= box[1][1] or (y + h) <= box[1][1]):
                person_boxes[i] = 1

    # Convert person_boxes to a binary string
    binary_string = ''.join(str(x) for x in person_boxes)

    # Send the binary string to ESP32
    send_position(int(binary_string, 2))

def run():
    while True:
        img_resp = urllib.request.urlopen(url)
        imgnp = np.array(bytearray(img_resp.read()), dtype=np.uint8)
        img = cv2.imdecode(imgnp, -1)

        # Detect common objects in the image
        bbox, label, conf = cv.detect_common_objects(img)

        # Create a new list containing only "person" label
        persons = []
        for i, lab in enumerate(label):
            if lab == "person":
                persons.append(bbox[i])

        # Draw bounding boxes around persons only
        img = draw_bbox(img, persons, ["person"] * len(persons), [1.0] * len(persons))
     
        # Draw the 8 boxes
        for box in boxes:
            cv2.rectangle(img, box[0], box[1], (0, 0, 255), 2)
    
        # Update person_boxes based on detected persons
        update_person_boxes(persons)

        cv2.imshow('detection', img)
        key = cv2.waitKey(500)
        if key == ord('q'):
            break

    cv2.destroyAllWindows()

if __name__ == '__main__':
    print("started")
    run()
