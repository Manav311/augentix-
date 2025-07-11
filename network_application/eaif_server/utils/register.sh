#!/bin/bash
dst=${1:-client.ini}

if [[ "$dst" == "-h" ]]; then
    echo '  Usage:'
    echo ''
    echo '  register.sh <config.ini:client.ini> <src-img-path:./data/face> <echo-only:0>'
    echo ''
    exit
fi

src_img_path=${2:-./data/face}
echo_only=${3:-0}
ip_head=$(head -2 "$dst")
ip_split=(${ip_head//= / })
ip=${ip_split[1]}
port=${ip_split[3]}

for file in "$src_img_path"/*.jpg; do
    fbase=$(basename -- "$file")
    name="${fbase%.*}"
    echo sed -i '3cquery_api= /facereco/register?face='"$name" "$dst"
    echo sed -i '4cimg_file= '"$file" "$dst"
    echo ./install_linux/eaif_client "$dst"
    if [ $echo_only -eq 1 ]; then
        echo echo only!
    else
        echo echo and exec!;
        sed -i '3cquery_api= /facereco/register?face='"$name" "$dst"
        sed -i '4cimg_file= '"$file" "$dst"
        ./install_linux/eaif_client "$dst"
    fi
done

echo curl -i http://"$ip":"$port"/facereco/query
curl -i http://"$ip":"$port"/facereco/query

