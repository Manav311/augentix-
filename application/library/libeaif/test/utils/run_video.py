import os
import sys
import subprocess as sp
import re
from multiprocessing import Pool
import logging
from typing import List, Iterator, Tuple

EAIF_PARAM = 'eaif.param.hd'

EXTRACT_SEI = "utils/extract_sei%s.sh %s %s"
PARSE_SEI = "python utils/parse_obj_list.py %s %s "
UPDATE_CONF = f"eaif_param_file=config/{EAIF_PARAM}\n" + \
              "eaif_objList_file=%s\n" + \
              "video_input_file=%s\n" + \
              "video_fps=%.4f\n" + \
              "resoln=%dx%d\n" + \
              "num_frames=%d\n"

CONFIG_FILE = "config/eaif_test.ini"
RUN_EAIF_TEST = "FR_MIN_FACE=80 INF_CAP_PREFIX=%s ./eaif_demo.elf config/eaif_test.ini 1 > %s"
MV_LOG = "mv %s %s"
DRAW_VIDEO = f"python utils/draw_eaif.py config/{EAIF_PARAM} %s %s %s"

PREFIX = "output"

def get_video_info(msg_stdout : bytes, msg_stderr : bytes) -> Tuple[int, int, int, float]:
    num_frames = 0
    img_h = 0
    img_w = 0
    fps = 0.0
    msg = msg_stderr.decode().split("\n")
    i = 0
    for l in msg:
        i+=1
        if img_w == 0:
            resolutions = re.findall('[0-9]+x[0-9]+', l)
            if resolutions:
                for resolution in resolutions:
                    resoln = resolution.split('x')
                    if "0" in resoln:
                        continue
                    img_h = int(resoln[1])
                    img_w = int(resoln[0])
        if num_frames == 0:
            no_frames = re.findall('frame=[\ ]*[0-9]+', l)
            if (no_frames):
                num_frames = int(no_frames[-1].split("=")[1])
        if fps == 0.0:
            fpsStr = re.findall('\ (\d+(?:\.\d+)?)\ fps,', l)
            if (fpsStr):
                fps = float(fpsStr[0])
    if img_h == 0 or num_frames == 0 or fps == 0:
        log.info("Cannot parse video information!")
        log.info("See %s" % (msg_stderr.decode()))
        return
    return img_h, img_w, num_frames, fps

def run_one_video(path : Tuple[int, int, str]) -> None:
    i, all_vid, path = path
    log.info("Running %d/%d %s" % (i+1, all_vid, path))

    if not os.path.exists(path):
        info("Cannot find %s" % path)
        return
    filename = os.path.basename(path)
    txtname = PREFIX + "/" + filename[:-4] + ".txt"
    binname = PREFIX + "/" + filename[:-4] + ".bin"
    logname = PREFIX + "/" + filename[:-4] + ".log"
    drawname = PREFIX + "/" + filename[:-4] + "_draw.mp4"

    CMD = EXTRACT_SEI % ( "" if "264" in path else "_265", path, txtname)
    log.info(CMD)
    ret, err = cmd(CMD)
    img_h, img_w, num_frames, fps = get_video_info(ret, err)
    log.info("Video info %dx%d fps:%.4f frames:%d" % (img_w, img_h, fps, num_frames))

    CMD = PARSE_SEI % ( "264" if "264" in path else "265", txtname)
    log.info(CMD)
    ret, err = cmd(CMD)
    print(ret.decode(), err.decode())

    CONFIG = UPDATE_CONF % (
        binname,
        path,
        fps,
        img_w, img_h,
        num_frames)
    with open(CONFIG_FILE,"w") as fw:
        fw.write(CONFIG)

    run_name = "exp_" + filename[:-4].replace(".","_")
    run_algo_log = run_name + "_run.log"
    CMD = RUN_EAIF_TEST % (run_name, run_algo_log)
    log.info(CMD)
    ret, err = cmd(CMD)
    print(ret.decode(), err.decode())

    CMD = MV_LOG % (run_algo_log, run_name)
    ret, err = cmd(CMD)

    CMD = DRAW_VIDEO % (path, logname, drawname)
    log.info(CMD)
    ret, err = cmd(CMD)
    print(ret.decode(), err.decode())
    log.info("Complete %d/%d %s" % (i+1, all_vid, path))

def run_all_video(pool : int = 1, paths : Iterator[str] = []) -> None:

    progress_list = [(i,len(paths),path) for i, path in enumerate(paths)]
    with Pool(pool) as p:
        p.map(run_one_video, progress_list)

def cmd(*args : Iterator[str]) -> Tuple[bytes, bytes]:
    p = sp.Popen(*args,shell=True, stdout=sp.PIPE, stderr=sp.PIPE)
    ret, err = p.communicate()
    return ret, err

def main(pool : int, target_path : Iterator[str]):
    set_logging()
    if not os.path.exists(PREFIX):
        os.mkdir(PREFIX)
    run_all_video(pool, target_path)

def help():
    print("\n\tDescription:\n" \
          "\n\tA main program to run eaif simulation. select pool size in program\n" \
          "\n\tUsage:\n\n\t-h/--help     show help message and exit\n" \
          "\n\tExample:\n\t\tpython utils\\run_video.py video-0 video-1 ...\n")
    return

def set_logging() -> logging.Logger:

    log_format = '[%(levelname)s] [%(asctime)s] [%(name)s] - %(message)s'
    logging.basicConfig(
        #filename='app-basic.log',
        level=logging.INFO,
        format=log_format,
        datefmt='%Y-%m-%d %H:%M:%S'
    )
    return logging.getLogger(name="eaif-sim")

if __name__ == "__main__":

    if len(sys.argv) == 1 or "-h" in sys.argv or \
       "--help" in sys.argv:
       help()
       exit()

    path = sys.argv[1:]
    pool = 3 # define in program

    log = set_logging()
    main(pool, path)

