import tensorflow as tf
import tensorflow.keras as keras
import sys

tf.keras.backend.set_learning_phase(0)

class Opt:
    pass
opt = Opt()
opt.postquantize = 0

def INFO(*args):
    print("[INFO] ", *args)

def loadTfModel(model):
    if model.find(".h5") == -1:
        INFO("run saved model load ...")
        model =  tf.saved_model.load(model)
    else:
        INFO("run keras model load ...")
        return keras.models.load_model(model)


def totflite(input_name, name, opt):
    converter = None
    try:
        INFO("load from saved model...")
        model = tf.saved_model.load(input_name)
        concrete_func = model.signatures[
          tf.saved_model.DEFAULT_SERVING_SIGNATURE_DEF_KEY]
        concrete_func.inputs[0].set_shape([1, 512, 512, 3])
        converter = tf.lite.TFLiteConverter.from_concrete_functions([concrete_func])
        converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS, tf.lite.OpsSet.SELECT_TF_OPS]
        converter.experimental_new_converter = True
        converter.optimizations = [tf.lite.Optimize.DEFAULT]
    except:
        INFO("load from keras model...")
        converter = tf.lite.TFLiteConverter.from_keras_model(model)
    #converter = tf.compat.v1.lite.TFLiteConverter.from_keras_model(model)
    #converter.optimizations = [tf.lite.Optimize.DEFAULT]
    if opt.postquantize:
        #converter.post_training_quantize=True
        converter.optimizations = [tf.lite.Optimize.DEFAULT]
        converter.representative_dataset = data_gen(model.input_shape, lambda x: x/255., L=opt.postquantize)
        converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
        #converter.target_spec.supported_types = [tf.int8]
        converter.inference_input_type = tf.int8
        converter.inference_output_type = tf.int8
    tflite_model = converter.convert()
    with tf.io.gfile.GFile(name, 'wb') as f:
        f.write(tflite_model)

input_name = sys.argv[1]
output_name = sys.argv[2]
#model = loadTfModel(input_name)
totflite(input_name, output_name, opt)
