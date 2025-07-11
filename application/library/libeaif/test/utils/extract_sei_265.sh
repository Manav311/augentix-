echo ffmpeg -y -i $1 -c:v copy -bsf:v 'filter_units=pass_types=39-40' -f rawvideo $2
ffmpeg -y -i $1 -c:v copy -bsf:v 'filter_units=pass_types=39-40' -f rawvideo $2
