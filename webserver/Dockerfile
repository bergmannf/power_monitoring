FROM docker.io/library/python:3.9-bullseye

RUN apt update && apt install -y libgl1-mesa-glx && apt clean
RUN pip install paddlepaddle paddleocr flask
RUN mkdir /data && mkdir /app
COPY predownload.py /usr/local/bin/predownload.py
RUN chmod +x /usr/local/bin/predownload.py

RUN /usr/local/bin/predownload.py /root/.paddleocr/whl/det/en/en_PP-OCRv3_det_infer/ https://paddleocr.bj.bcebos.com/PP-OCRv3/english/en_PP-OCRv3_det_infer.tar
RUN /usr/local/bin/predownload.py /root/.paddleocr/whl/rec/en/en_PP-OCRv3_rec_infer/ https://paddleocr.bj.bcebos.com/PP-OCRv3/english/en_PP-OCRv3_rec_infer.tar
RUN /usr/local/bin/predownload.py /root/.paddleocr/whl/cls/ch_ppocr_mobile_v2.0_cls_infer/ https://paddleocr.bj.bcebos.com/dygraph_v2.0/ch/ch_ppocr_mobile_v2.0_cls_infer.tar

COPY . /app
WORKDIR /app

EXPOSE 8000

ENTRYPOINT ["flask", "run"]
CMD ["-h", "0.0.0.0"]
