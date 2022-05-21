# main.py
from machine import Pin
from 
import network

ssid = "Wifi-Password-OpenToAll"
password = "OpenToAll"
serverName = "https://api.openweathermap.org/data/2.5/onecall?lat=39.8912&lon=-74.9218&exclude=current,minutely,daily,alerts&appid=97316b5f6caa17ae73f2715a11583ab5"
lastTime = 0
elapsedTime = 0

def wifi_setup():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    start = t.ticks_ms()
    while not wlan.isconnected() or t.ticks_diff(t.ticks_ms(), start) > 10000:
        pass
    print("Connected to Wifi; ifconfig:", wlan.ifconfig())

