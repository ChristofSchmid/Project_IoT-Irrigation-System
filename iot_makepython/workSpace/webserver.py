
#webserver.py
import network
import webrepl
import time
from machine import Pin
try:
  import usocket as socket
except:
  import socket

AUTH_OPEN = 0
AUTH_WEP = 1
AUTH_WPA_PSK = 2
AUTH_WPA2_PSK = 3
AUTH_WPA_WPA2_PSK = 4

SSID = "P51D"      #Modify here with SSID
PASSWORD = "4055284055"   #Modify here with PWD
led = Pin(5, Pin.OUT)
ip = "ip get wrong"

def web_page(msg):
  table_str = """<tr><th>Update_time</th><th>Relay</th><th style="color:green">Soil1</th><th style="color:green">Soil2</th><th style="color:green">Soil3</th><th style="color:green">Soil4</th><th style="color:green">Soil5</th><th style="color:green">Soil6</th></tr>"""
  for i in range(10):
    table_str += "<tr><th>" + msg[i]['Update_time'] + "</th><th>" + msg[i]['Relay'] + """</th><th style="color:green">""" + msg[i]['Soil1'] + """</th><th style="color:green">""" + msg[i]['Soil2'] + """</th><th style="color:green">""" + msg[i]['Soil3'] + """</th><th style="color:green">""" + msg[i]['Soil4'] + """</th><th style="color:green">""" + msg[i]['Soil5']+ """</th><th style="color:green">""" + msg[i]['Soil6']+ "</th></tr>"
  table_str += """</table>"""

  html =""" 
<html>

<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta http-equiv="refresh" content="600" />
    
    

<title>LED Button</title>
<style>
        .button1 {
            -webkit-transition-duration: 0.4s;
            transition-duration: 0.4s;
            padding: 16px 32px;
            text-align: center;
            background-color: white;
            color: black;
        }
    </style>
</head>


<body>
    <div id="content" style="text-align:center;background-color:#EEEEEE;height:150px;width:2000px;float:center;">
        <img src="https://makerfabs.com/image/cache/logo11-190x63.png" />
        <h1>Lora IoT Irrigation System</h1>
    </div>
    <div id="menu" style="text-align:center;background-color:#FFD700;height:100px;width:2000px;float:center;">
        <form method="get">
            <button name="led" type="submit" value="water">Water 10 Second</button>
            <button name="led" type="submit" value="off">Relay OFF</button>
            <button name="led" type="submit" value="donothing">Refresh Data</button>
            <button name="led" type="submit" value="readsensor">Read Sensors</button>
            <div class='button1'>
            <button id='btn1' onClick=send1()>R1 ON </button>
            <button id='btn2' onClick=send2()>R1 OFF</button>
            <button id='btn3' onClick=send3()>R2 ON </button>
            <button id='btn4' onClick=send4()>R2 OFF</button>
            <button id='btn5' onClick=send5()>R3 ON </button>
            <button id='btn6' onClick=send6()>R3 OFF</button>
            <button id='btn8' onClick=send8()>R4 OFF</button><br>
            </div>
        </form>

        
    </div>
    <table border="1" align="left" style="background-color: #00dbebcc;width:2000;">
        <caption>Sensors status</caption>
""" + table_str + """
    </table>
    
    <script>
    
function send1(){
  send('/1/');
  document.getElementById("btn1").style.color = "green"; 
  document.getElementById("btn2").style.color = "white";
}
function send2(){
  send('/2/');
  document.getElementById("btn1").style.color = "white"; 
  document.getElementById("btn2").style.color = "green";

}
function send3(){
  send('/3/');
  document.getElementById("btn3").style.color = "green"; 

  document.getElementById("btn4").style.color = "white";
}
function send4(){
  send('/4/');
  document.getElementById("btn3").style.color = "white"; 
  document.getElementById("btn4").style.color = "green";
}
function send5(){
  send('/5/');
  document.getElementById("btn5").style.color = "green"; 
  document.getElementById("btn6").style.color = "white";
}
function send6(){
  send('/6/');
  document.getElementById("btn5").style.color = "white"; 
  document.getElementById("btn6").style.color = "green";
}
function send7(){
  send('/7/');
  document.getElementById("btn7").style.color = "green"; 
  document.getElementById("btn8").style.color = "white";
}
function send8(){
  send('/8/');
  document.getElementById("btn7").style.color = "white"; 
  document.getElementById("btn8").style.color = "green";
}
function send(url){
  var xhr = new XMLHttpRequest();
  xhr.open('GET', url, true);
  xhr.send();
}
</script>
    
</body>

</html>
  """
  return html

def do_connect(ssid,psw):
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    s = wlan.config("mac")
    mac = ('%02x:%02x:%02x:%02x:%02x:%02x').upper() %(s[0],s[1],s[2],s[3],s[4],s[5])
    print("Local MAC:"+mac) #get mac 
    wlan.connect(ssid, psw)
    if not wlan.isconnected():
        print('connecting to network...' + ssid)
        wlan.connect(ssid, psw)

    start = time.ticks_ms() # get millisecond counter
    while not wlan.isconnected():
        time.sleep(1) # sleep for 1 second
        if time.ticks_ms()-start > 20000:
            print("connect timeout!")
            break

    if wlan.isconnected():
        print('network config:', wlan.ifconfig()[0])
        global ip
        ip = str(wlan.ifconfig()[0])
    return wlan

def connect():
 do_connect(SSID,PASSWORD)
 global ip
 return ip




"""
      <a href=\"?led=on\">
        <button style="background-color: #7ED321">RELAY ON</button>
      </a>
"""





