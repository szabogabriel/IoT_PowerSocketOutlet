#!/bin/bash

readonly mqtt_user="user"
readonly mqtt_password="password"
readonly mqtt_server="localhost"
readonly mqtt_port=1883

readonly topic_publish="/cord/update"
readonly topic_response="/cord/status"

function publish_message {
  mosquitto_pub -h $mqtt_server -p $mqtt_port -u $mqtt_user -P $mqtt_password -t $topic_publish -m $1
  sleep 1
}

function status_message {
  mosquitto_rr -h $mqtt_server -p $mqtt_port -u $mqtt_user -P $mqtt_password -t $topic_publish -m "STATUS" -e $topic_response
  sleep 1
}

function main {
  if [ $1 == "status" ]; then
    status_message
    exit 0
  fi
  local port=$1
  local onoff=""
  if [ $2 == "on" ]; then
    onoff="ON"
  fi
  if [ $2 == "off" ]; then
    onoff="OFF"
  fi
  publish_message "$port $onoff"
}

main $@
