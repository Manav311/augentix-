echo ./eaif_server_demo --mtcnn models/facereco/pnet_float_224.tflite models/facereco/rnet_float.tflite models/facereco/onet_float.tflite \
0.5 0.5 0.3 0.79 10 \
--input "$1" \
--imh "$2" \
--imw "$3" \
--zero 127.5 --scale 0.0078125 
