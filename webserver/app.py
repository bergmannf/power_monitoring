#!/usr/bin/env python3
import os
import datetime
import re
from flask import Flask, request
from paddleocr import PaddleOCR

app = Flask(__name__)
paddle = PaddleOCR(use_angle_cls=True, lang="en")
consumption_regex = re.compile(r'\d{6,9}')


def get_power_consumption(monitor: str, ocr_result: list):
    for line in ocr_result:
        for result in line:
            text = line[-1][0]
            matches = consumption_regex.findall(text)
            if matches:
                app.logger.info(f"Consumption found {matches[0]}")
                return matches[0]
    return ""


def ocr(monitor: str, path: str, time: str):
    app.logger.info(f"OCR for {monitor}: {path} at {time}")
    data_file = f"/data/{monitor}/data.csv"
    result = paddle.ocr(path)
    power_consumed = get_power_consumption(monitor, result)
    if not power_consumed:
        return
    with open(data_file, "a") as f:
        line = f"{time}, {power_consumed}"
        f.write(line)


@app.route("/", methods=["GET"])
def index():
    return "Index page"


@app.route("/store/<monitor>", methods=["POST"])
def store_image(monitor):
    now = datetime.datetime.now()
    datestring = now.strftime("%Y-%m-%d_%H:%M:%S")
    base_path = f"/data/{monitor}/"
    os.makedirs(base_path, exist_ok=True)
    path = f"{base_path}{datestring}.jpg"
    app.logger.info(f"Storing picture at {path}")
    with open(path, 'wb') as f:
        f.write(request.get_data())
        ocr(monitor, path, datestring)
    return ""
