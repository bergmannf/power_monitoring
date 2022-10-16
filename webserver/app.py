#!/usr/bin/env python3
import os
import datetime
from flask import Flask, request
from paddleocr import PaddleOCR

app = Flask(__name__)
ocr = PaddleOCR(use_angle_cls=True, lang="en")

def get_power_consumption(monitor: str, ocr_result: dict):
    pass

def ocr(monitor: str, path: str, time: str):
    data_file = f"/data/{monitor}/data.csv"
    result = ocr.ocr(path)
    power_meter = get_power_meter(monitor, result)
    with open(data_file, "a") as f:
        line = f"{time}, {power_meter}"
        f.write(line)

@app.route("/", methods=["GET"])
def index():
    return "Index page"

@app.route("/store/<monitor>", methods=["POST"])
def store_image(image):
    now = datetime.now()
    datestring = now.strftime("%Y-%m-%d_%H:%M:%S")
    base_path = f"/data/{monitor}/"
    os.makedirs(base_path, exist_ok=True)
    path = f"{base_path}{datestring}.jpg"
    with open(path, 'wb') as f:
        f.write(request.get_data())
        ocr(monitor, path, datestring)
    return ""
