
#print(open('./lora_node.py','rU').read())

from machine import Pin
import time
import config_lora
import random
import webserver
import ntptime
try:
  import usocket as socket
except:
  import socket


 
class Lora_Gate:

    def __init__(self, name, lora, ip):
        print("lora_gate init starting") 
        self.name = name
        self.lora = lora
        self.sensor_adc = "NULL"
        self.relay_status = "NULL"
        self.flag = 0
        self.buff = ""
        self.ip = ip
        self.node_list = Lora_Node_List()
        self.DisplayTimerFlag = 0
        self.read_sensors = 0
        

    #模式配置
    def working(self):
        print(self.node_list.list)
        print("self.show start") 
        self.show_all_status(self.node_list)
        print("MODE_GATE")
        print("on.receive start") 
        self.lora.onReceive(self.on_gate_receiver)
        print("receive start") 
        self.lora.receive()
        print("gate working start") 
        self.gate_working()
        pass
    
    #发送数据
    def sendMessage(self, message):
        self.lora.println(message)
        print("Sending message:")
        print(message)


    #网关模式
    def gate_working(self):
        #获取时间
        try:
            ntptime.settime()
        except:
            self.lora.show_text_wrap('NTP time wrong , please reboot.')
        self.sendMessage("BEGIN")
        onoff = "OFF"
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind(('', 80))
        s.listen(5)
        #设置accept阻塞时间
        s.settimeout(5) #600s on socket
        last_time = 0
        disp_time = 0
        relay_common = 0

        while True:
            print("gate_working main loop running") 
            now_time = time.time()
            if now_time - disp_time > 5 and self.DisplayTimerFlag == 1:
              print("start show all")
              disp_time = now_time
              self.show_all_status(self.node_list)
              #continue
              print("start READ SENSORS SelfReadState: " + str(self.read_sensors) + " Timer: " + str(now_time - last_time) )
              #time.sleep(5)
            if now_time - last_time > 600 or self.read_sensors == 1:
                print("start READ SENSORS SelfReadState: " + str(self.read_sensors) + " Timer: " + str(now_time - last_time) )
                #time.sleep(5)
                
                last_time = now_time 
                self.read_sensors = 0

                tm = time.localtime()
                t_hour = (tm[3] + 1) % 24
                t_min = tm[4]
                t_sec = tm[5]
                time_str = str(t_hour) + ':' + str(t_min) + ':' + str(t_sec)
                print(time_str)

                #Relay收发
                self.sendMessage("PREPARE")
                time.sleep(5)

                last_node_status = {"Relay":"NULL", "Soil1":"NULL", "Soil2": "NULL", "Soil3":"NULL", "Soil4":"NULL", "Soil5":"NULL", "Soil6":"NULL", "Update_time":"NULL"}
                last_node_status["Update_time"] = time_str

                
                self.DisplayTimerFlag = 1
                temp_msg = "RELAY" + onoff
                onoff = ""
                self.sendMessage(temp_msg)
                self.lora.receive()
                self.lora_timeout()
                self.relay_status = self.buff
                self.buff = ""
                last_node_status["Relay"] = self.relay_status
                if last_node_status["Relay"] == "":
                    last_node_status["Relay"] = "NULL"

                for i in range(1,7):
                    soil_index = 'Soil'+str(i)
                    self.sendMessage(soil_index)
                    self.lora.receive()
                    self.lora_timeout()
                    last_node_status[soil_index] = self.buff
                    self.buff = ""
                    if last_node_status[soil_index] == "":
                        last_node_status[soil_index] = "NULL"

                del self.node_list.list[0]
                self.node_list.list.append(last_node_status)

                self.show_all_status(self.node_list)         

                #阻塞lora，等待webserver请求 Block lora, waiting for webserver request
                self.sendMessage("OVER")
                print("Prepare accept")

            while True:
                print("gate_working request loop running") 
                try:
                    print("TRY gate_working request loop running") 
                    conn, addr = s.accept()
                    print('Got a connection from %s' % str(addr))
                    request = conn.recv(2048) #1024
                    request = str(request)
                    print('Content = %s' % request)
                    if request.find("favicon") != -1:
                        conn.close()
                        continue
                    if request is "":
                        conn.close()
                        continue

                    #解析请求并发送控制继电器指令
                    led_on = request.find('/?led=on')
                    led_off = request.find('/?led=off')
                    led_water = request.find('/?led=water')
                    led_readsensor = request.find('/?led=readsensor')
                    if led_on == 6:
                        print('RELAY ON')
                        onoff = 'ON'
                        self.read_sensors = 1
                    if led_off == 6:
                        print('RELAY OFF')
                        onoff = 'OFF'
                        self.read_sensors = 1
                    if led_water == 6:
                        print('RELAY WATER')
                        onoff = 'WATER'
                        self.read_sensors = 0
                    if led_readsensor == 6:
                        print('READ SENSORS')
                        self.read_sensors = 1
                                             
                    response = webserver.web_page(self.node_list.list)
                    conn.send('HTTP/1.1 200 OK\n')
                    conn.send('Content-Type: text/html\n')
                    conn.send('Connection: close\n\n')
                    conn.sendall(response)
                    conn.close()
                    #break #go back to gate_working main loop
                except Exception as e:
                    print(e)
                    break #go back to gate_working main loop


    #网关模式回调
    def on_gate_receiver(self, payload):    
        print("On gate receive starting") 
        rssi = self.lora.packetRssi()
        
        try:
            #print payload可知报文前4字节和后一字节无意义
            if int(payload[0]) == 255:
                length=len(payload)-1   
                payload_string = str((payload[4:length]),'utf-8')
            else:
                payload_string = str(payload,'utf-8')        
            print("Received message:\n{}".format(payload_string))
            self.buff = payload_string
            self.flag = 1

        except Exception as e:
            print(e)
        print("with RSSI {}\n".format(rssi))

    #lora回调超时控制
    def lora_timeout(self):
        print("wait lora callback")
        now = config_lora.millisecond()
        while(self.flag == 0):
            if config_lora.millisecond() - now > 2000:
                print("Callback time out.")
                break
        self.flag = 0
        
    def lora_timeout1(self):
        print("Display time out")
        now = config_lora.millisecond()
        while(1):
            if config_lora.millisecond() - now > 5000:
                print("Changing display")
                break
        

    def show_all_status(self,node_List):
        self.lora.show_text(self.ip)
        
        soil0 = node_List.list[9]['Soil1']
        if soil0 == "AHT10ERR" : 
            pass
        elif soil0 != "NULL" :
            soil0 = soil0[soil0.find('H:'):]
            soil0 = soil0 + soil0[soil0.find('B:'):]


        soil1 = node_List.list[9]['Soil2']
        if soil1 == "AHT10ERR" :
            pass
        elif soil1 != "NULL" :
            soil1 = soil1[soil1.find('H:'):]
            soil1 = soil1 + soil1[soil1.find('B:'):]


        soil2 = node_List.list[9]['Soil3']
        if soil2 == "AHT10ERR" :
            pass
        elif soil2 != "NULL" :
            soil2 = soil2[soil2.find('H:'):]
            soil2 = soil2 + soil2[soil2.find('B:'):]
            
        soil3 = node_List.list[9]['Soil4']
        if soil3 == "AHT10ERR" :
            pass
        elif soil3 != "NULL" :
            soil3 = soil3[soil3.find('H:'):]
            soil3 = soil3 + soil3[soil3.find('B:'):]
            
        soil4 = node_List.list[9]['Soil5']
        if soil4 == "AHT10ERR" :
            pass
        elif soil4 != "NULL" :
            soil4 = soil4[soil4.find('H:'):]
            soil4 = soil4 + soil4[soil4.find('B:'):]
            
            
        soil5 = node_List.list[9]['Soil6']
        if soil5 == "AHT10ERR" :
            pass
        elif soil5 != "NULL" :
            soil5 = soil5[soil5.find('H:'):]
            soil5 = soil5 + soil5[soil5.find('B:'):]
        
        #while self.DisplayTimerFlag == 1:
        self.lora.show_text(self.ip,0,0,clear_first = True)
        self.lora.show_text('R1:'+ node_List.list[9]['Relay'],0,10,clear_first = False)
        self.lora.show_text('S1:'+ soil0,0,20,clear_first = False)
        self.lora.show_text('S2:'+ soil1,0,30,clear_first = False)
        self.lora.show_text('S3:'+ soil2,0,40,clear_first = False)
        self.lora.show_text(node_List.list[9]['Update_time'],0,50,clear_first = False, show_now = True, hold_seconds = 1)
        
        self.lora_timeout1()
          
        self.lora.show_text(self.ip,0,0,clear_first = True)
        self.lora.show_text('R1:'+ node_List.list[9]['Relay'],0,10,clear_first = False)
        self.lora.show_text('S4:'+ soil3,0,20,clear_first = False)
        self.lora.show_text('S5:'+ soil4,0,30,clear_first = False)
        self.lora.show_text('S6:'+ soil5,0,40,clear_first = False) 
        self.lora.show_text(node_List.list[9]['Update_time'],0,50,clear_first = False, show_now = True, hold_seconds = 1)         
          #self.DisplayTimerFlag = 0
          
          
              

class Lora_Node_List:
    def __init__(self):
        self.list = []
        for i in range(10):
            self.list.append({"Relay":"default", "Soil1":"default", "Soil2":"default", "Soil3":"default", "Soil4":"default", "Soil5":"default", "Soil6":"default","Update_time":"00:00:00"})




