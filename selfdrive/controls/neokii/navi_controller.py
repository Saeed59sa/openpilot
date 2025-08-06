#!/usr/bin/env python3
import json
import os
import select
import signal
import subprocess
import sys
import threading
import time
import socket
import fcntl
import struct
import numpy as np

from collections import deque
from threading import Thread
from cereal import messaging
from openpilot.common.realtime import Ratekeeper

CAMERA_SPEED_FACTOR = 1.05
terminate_flag = threading.Event()

class Port:
  BROADCAST_PORT = 2899
  RECEIVE_PORT = 3843
  LOCATION_PORT = BROADCAST_PORT
  CS_PORT = 3847
  NAVI_GPS_PORT = 3931
  NAVI_OBSTACLES_PORT = 3932
  SEND_OBSTACLE_PORT = 3946

class NaviServer:
  def __init__(self):
    self.sm = messaging.SubMaster(['gpsLocationExternal', 'carState'])

    self.json_road_limit = None
    self.json_traffic_signal = None
    self.active = 0
    self.last_updated = 0
    self.last_updated_active = 0
    self.last_exception = None
    self.lock = threading.Lock()
    self.remote_addr = None

    self.remote_gps_addr = None
    self.last_time_location = 0

    Thread(target=self.broadcast_thread, args=[], daemon=True).start()
    Thread(target=self.update_thread, args=[self.sm], daemon=True).start()
    Thread(target=self.gps_thread, args=[], daemon=True).start()

    subprocess.Popen([os.path.join(os.path.dirname(os.path.abspath(__file__)), 'ngpsd')])
    subprocess.Popen([os.path.join(os.path.dirname(os.path.abspath(__file__)), 'nobsd')])

    self.gps_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    self.location = None

  def gps_thread(self):
    rk = Ratekeeper(3.0, print_delay_threshold=None)
    while not terminate_flag.is_set():
      self.gps_timer()
      rk.keep_time()

  def gps_timer(self):
    try:
      if self.remote_gps_addr is not None:
        self.sm.update(0)
        if self.sm.updated['gpsLocationExternal']:
          self.location = self.sm['gpsLocationExternal']

        if self.location is not None:
          json_location = json.dumps({"location": [
            self.location.latitude,
            self.location.longitude,
            self.location.altitude,
            self.location.speed,
            self.location.bearingDeg,
            self.location.horizontalAccuracy,
            self.location.unixTimestampMillis,
            # self.location.source,
            # self.location.vNED,
            self.location.verticalAccuracy,
            self.location.bearingAccuracyDeg,
            self.location.speedAccuracy,
          ]})

          address = (self.remote_gps_addr[0], Port.LOCATION_PORT)
          self.gps_socket.sendto(json_location.encode(), address)

    except Exception:
      self.remote_gps_addr = None

  def get_broadcast_address(self):
    try:
      with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        ip = fcntl.ioctl(
          s.fileno(),
          0x8919,
          struct.pack('256s', 'wlan0'.encode('utf-8'))
        )[20:24]
        return socket.inet_ntoa(ip)
    except Exception:
      return None

  def broadcast_thread(self):
    broadcast_address = None
    frame = 0

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
      try:
        # sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        while not terminate_flag.is_set():

          try:
            if broadcast_address is None or frame % 10 == 0:
              broadcast_address = self.get_broadcast_address()

            if broadcast_address is not None and self.remote_addr is None:
              print('broadcast', broadcast_address)

              msg = 'EON:ROAD_LIMIT_SERVICE:v1'.encode()
              for i in range(1, 255):
                ip_tuple = socket.inet_aton(broadcast_address)
                new_ip = ip_tuple[:-1] + bytes([i])
                address = (socket.inet_ntoa(new_ip), Port.BROADCAST_PORT)
                sock.sendto(msg, address)
          except Exception:
            pass

          time.sleep(5.)
          frame += 1
      except Exception:
        pass

  def update_thread(self, sm):
    rk = Ratekeeper(10, print_delay_threshold=None)

    while not terminate_flag.is_set():
      sm.update(0)

      if sm.updated['carState']:
        v_ego = sm['carState'].vEgo
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
          data_in_bytes = struct.pack('!f', v_ego)
          sock.sendto(data_in_bytes, ('127.0.0.1', Port.CS_PORT))

      rk.keep_time()

  def send_sdp(self, sock):
    try:
      sock.sendto('EON:ROAD_LIMIT_SERVICE:v1'.encode(), (self.remote_addr[0], Port.BROADCAST_PORT))
    except Exception:
      pass

  def udp_recv(self, sock):
    ret = False
    try:
      ready = select.select([sock], [], [], 1.)
      ret = bool(ready[0])
      if ret:
        data, self.remote_addr = sock.recvfrom(2048)
        json_obj = json.loads(data.decode())

        if 'cmd' in json_obj:
          try:
            os.system(json_obj['cmd'])
            ret = False
          except Exception:
            pass

        if 'request_gps' in json_obj:
          try:
            if json_obj['request_gps'] == 1:
              self.remote_gps_addr = self.remote_addr
            else:
              self.remote_gps_addr = None
            ret = False
          except Exception:
            pass

        if 'echo' in json_obj:
          try:
            echo = json.dumps(json_obj["echo"])
            sock.sendto(echo.encode(), (self.remote_addr[0], Port.BROADCAST_PORT))
            ret = False
          except Exception:
            pass

        if 'echo_cmd' in json_obj:
          try:
            result = subprocess.run(json_obj['echo_cmd'], shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            echo = json.dumps({"echo_cmd": json_obj['echo_cmd'], "result": result.stdout})
            sock.sendto(echo.encode(), (self.remote_addr[0], Port.BROADCAST_PORT))
            ret = False
          except Exception:
            pass

        try:
          self.lock.acquire()
          try:
            if 'active' in json_obj:
              self.active = json_obj['active']
              self.last_updated_active = time.monotonic()
          except Exception:
            pass

          if 'road_limit' in json_obj:
            self.json_road_limit = json_obj['road_limit']
            self.last_updated = time.monotonic()

          if 'traffic_signal' in json_obj:
            self.json_traffic_signal = json_obj['traffic_signal']

        finally:
          self.lock.release()

    except Exception:

      try:
        self.lock.acquire()
        self.json_road_limit = None
        self.json_traffic_signal = None
      finally:
        self.lock.release()

    return ret

  def check(self):
    now = time.monotonic()
    if now - self.last_updated > 6.:
      try:
        self.lock.acquire()
        self.json_road_limit = None
        self.json_traffic_signal = None
      finally:
        self.lock.release()

    if now - self.last_updated_active > 6.:
      self.active = 0
      self.remote_addr = None

  def get_limit_val(self, key, default=None):
    return self.get_json_val(self.json_road_limit, key, default)

  def get_ts_val(self, key, default=None):
    return self.get_json_val(self.json_traffic_signal, key, default)

  def get_json_val(self, json_data, key, default=None):
    if json_data is None:
      return default
    return json_data.get(key, default)

def navi_gps_thread():
  naviGps = messaging.pub_sock('naviGps')
  with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
    sock.bind(('0.0.0.0', Port.NAVI_GPS_PORT))
    while not terminate_flag.is_set():
      try:
        data, address = sock.recvfrom(16)
        floats = struct.unpack('ffff', data)
        dat = messaging.new_message('naviGps', valid=True)
        dat.naviGps.latitude = floats[0]
        dat.naviGps.longitude = floats[1]
        dat.naviGps.heading = floats[2]
        dat.naviGps.speed = floats[3]
        naviGps.send(dat.to_bytes())
      except Exception:
        pass

def navi_obstacles_thread():
  naviObstacles = messaging.pub_sock('naviObstacles')
  with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
    sock.bind(('0.0.0.0', Port.NAVI_OBSTACLES_PORT))
    while not terminate_flag.is_set():
      try:
        data, address = sock.recvfrom(13*4+1)
        dat = messaging.new_message('naviObstacles', valid=True)
        if data[0] == 1:
          floats = struct.unpack('13f', data[1:])
          obstacle = {'valid': True, 'type': 0, 'obstacle': list(floats)}
          dat.naviObstacles.obstacles = [obstacle]
        else:
          dat.naviObstacles.obstacles = []
        naviObstacles.send(dat.to_bytes())
      except Exception:
        pass

def send_obstacle(cam_type, distance, speed, v_ego, s):
  with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
    data_in_bytes = struct.pack('!iffff', int(cam_type), float(distance), float(speed), float(v_ego), float(s))
    sock.sendto(data_in_bytes, ('127.0.0.1', Port.SEND_OBSTACLE_PORT))

def publish_thread(server):
  sm = server.sm
  naviData = messaging.pub_sock('naviData')
  rk = Ratekeeper(10.0, print_delay_threshold=None)
  v_ego_q = deque(maxlen=3)

  while not terminate_flag.is_set():
    sm.update(0)

    dat = messaging.new_message('naviData', valid=True)
    navi = dat.naviData

    server.lock.acquire()
    try:
      navi.active = server.active
      navi.roadLimitSpeed = server.get_limit_val("road_limit_speed", 0)
      navi.isHighway = server.get_limit_val("is_highway", False)
      navi.camType = server.get_limit_val("cam_type", 0)
      navi.camLimitSpeedLeftDist = server.get_limit_val("cam_limit_speed_left_dist", 0)
      navi.camLimitSpeed = server.get_limit_val("cam_limit_speed", 0)
      navi.sectionLimitSpeed = server.get_limit_val("section_limit_speed", 0)
      navi.sectionLeftDist = server.get_limit_val("section_left_dist", 0)
      navi.sectionAvgSpeed = server.get_limit_val("section_avg_speed", 0)
      navi.sectionLeftTime = server.get_limit_val("section_left_time", 0)
      navi.sectionAdjustSpeed = server.get_limit_val("section_adjust_speed", False)
      navi.camSpeedFactor = server.get_limit_val("cam_speed_factor", CAMERA_SPEED_FACTOR)
      navi.currentRoadName = server.get_limit_val("current_road_name", "")
      navi.isNda2 = server.get_limit_val("is_nda2", False)

      ts = {
        'isGreenLightOn': server.get_ts_val("isGreenLightOn", False),
        'isLeftLightOn': server.get_ts_val("isLeftLightOn", False),
        'isRedLightOn': server.get_ts_val("isRedLightOn", False),
        'greenLightRemainTime': server.get_ts_val("greenLightRemainTime", 0),
        'leftLightRemainTime': server.get_ts_val("leftLightRemainTime", 0),
        'redLightRemainTime': server.get_ts_val("redLightRemainTime", 0),
        'distance': server.get_ts_val("distance", 0)
      }
      navi.ts = ts
    finally:
      server.lock.release()

    if sm.updated['carState']:
      v_ego_q.append(sm['carState'].vEgo)

    v_ego = np.mean(v_ego_q) if len(v_ego_q) > 0 else 0.
    t_since_last_update = (time.monotonic() - server.last_updated)
    s_travelled = t_since_last_update * v_ego

    if navi.camLimitSpeedLeftDist > 0:
      navi.camLimitSpeedLeftDist = int(max(navi.camLimitSpeedLeftDist - s_travelled, 0))
    if navi.sectionLeftDist > 0:
      navi.sectionLeftDist = int(max(navi.sectionLeftDist - s_travelled, 0))

    send_obstacle(navi.camType, navi.camLimitSpeedLeftDist, navi.camLimitSpeed / 3.6, v_ego, s_travelled)

    naviData.send(dat.to_bytes())
    server.check()
    rk.keep_time()

def main():
  server = NaviServer()

  Thread(target=navi_gps_thread, args=[], daemon=True).start()
  Thread(target=navi_obstacles_thread, args=[], daemon=True).start()
  Thread(target=publish_thread, args=[server], daemon=True).start()

  with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
    try:
      sock.bind(('0.0.0.0', Port.RECEIVE_PORT))
      sock.setblocking(True)
      while not terminate_flag.is_set():
        server.udp_recv(sock)
        server.send_sdp(sock)

    except Exception as e:
      server.last_exception = e

class SpeedLimiter:
  def __init__(self):
    self.decelerating = False
    self.started_dist = 0
    self.last_limit_speed_left_dist = 0

    self.sock = messaging.sub_sock("naviData")
    self.naviData = None
    self.logMonoTime = 0

  @classmethod
  def instance(cls):
    if not hasattr(cls, "_instance"):
      cls._instance = cls()
    return cls._instance

  def recv(self):
    try:
      dat = messaging.recv_sock(self.sock, wait=False)
      if dat is not None:
        self.logMonoTime = dat.logMonoTime
        self.naviData = dat.naviData
    except Exception:
      pass

  def get_active(self):
    self.recv()
    if self.naviData is not None:
      return self.naviData.active
    return 0

  def get_road_limit_speed(self):
    self.recv()
    if self.naviData is not None:
      return self.naviData.roadLimitSpeed
    return 0

  def get_section_limit_speed(self):
    self.recv()
    if self.naviData is not None:
      return self.naviData.sectionLimitSpeed, self.naviData.sectionLeftDist
    return 0, 0

  def get_cam_type(self):
    self.recv()
    if self.naviData is not None:
      return self.naviData.camType
    return 0

  def get_max_speed(self, cluster_speed, conv):
    self.recv()
    default_return_value = (0, False)

    if self.naviData is None:
      self.decelerating = False
      return default_return_value

    try:
      road_limit_speed = self.naviData.roadLimitSpeed
      is_highway = self.naviData.isHighway
      cam_type = int(self.naviData.camType)
      cam_limit_speed_left_dist = self.naviData.camLimitSpeedLeftDist
      cam_limit_speed = self.naviData.camLimitSpeed
      section_limit_speed = self.naviData.sectionLimitSpeed
      section_left_dist = self.naviData.sectionLeftDist
      section_avg_speed = self.naviData.sectionAvgSpeed
      section_left_time = self.naviData.sectionLeftTime
      section_adjust_speed = self.naviData.sectionAdjustSpeed
      cam_speed_factor = np.clip(self.naviData.camSpeedFactor, 1.0, 1.1)

      min_limit = 40 if is_highway else 20
      max_limit = 120 if is_highway else 100

      if cam_limit_speed_left_dist is not None and cam_limit_speed is not None and cam_limit_speed_left_dist > 0:
        v_ego = conv.to_ms(cluster_speed)
        diff_speed = cluster_speed - (cam_limit_speed * cam_speed_factor)

        safe_dist = v_ego * 3. if cam_type == 22 else v_ego * 8.
        starting_dist = v_ego * 6. if cam_type == 22 else v_ego * 30.

        if self.decelerating and self.last_limit_speed_left_dist > 0 and \
           cam_limit_speed_left_dist < (self.last_limit_speed_left_dist - (v_ego * 5)):
          self.decelerating = False

        if min_limit <= cam_limit_speed <= max_limit and (self.decelerating or cam_limit_speed_left_dist < starting_dist):
          is_limit_zone = not self.decelerating

          if not self.decelerating:
            self.started_dist = cam_limit_speed_left_dist
            self.decelerating = True

          td = self.started_dist - safe_dist
          d = cam_limit_speed_left_dist - safe_dist

          pp = 0
          if d > 0. and td > 0. and diff_speed > 0. and (section_left_dist is None or section_left_dist < 10 or cam_type == 2):
            pp = (d / td) ** 0.6

          self.last_limit_speed_left_dist = cam_limit_speed_left_dist

          if cam_type == 22:
            bump_speed = 28.
            target_speed = bump_speed + int(pp * diff_speed)
          else:
            target_speed = cam_limit_speed * cam_speed_factor + int(pp * diff_speed)

          return target_speed, is_limit_zone

      elif section_left_dist is not None and section_limit_speed is not None and section_left_dist > 0:
        if min_limit <= section_limit_speed <= max_limit:

          is_limit_zone = not self.decelerating
          if not self.decelerating:
            self.decelerating = True

          speed_diff = 0
          if section_adjust_speed is not None and section_adjust_speed:
            speed_diff = (section_limit_speed - section_avg_speed) / 2.
            speed_diff *= np.interp(section_left_dist, [500, 1000], [0., 1.])

          target_speed = section_limit_speed * cam_speed_factor + speed_diff

          return target_speed, is_limit_zone

    except Exception:
      pass

    self.decelerating = False
    return default_return_value

  def get_camera_limit_speed_stock(self, speed_limit_distance, speed_limit, conv):
    if speed_limit_distance <= 0 or speed_limit <= 0:
      return 0, False

    safety_factor = 1.05
    safe_speed_kph = speed_limit * safety_factor

    return self._calculate_deceleration_speed(speed_limit_distance, safe_speed_kph, conv)

  def _calculate_deceleration_speed(self, left_dist, safe_speed_kph, conv):
    safe_time = 7
    safe_decel_rate = 1.2

    safe_speed_ms = conv.to_ms(safe_speed_kph)

    safe_dist = safe_speed_ms * safe_time
    decel_dist = left_dist - safe_dist

    is_limit_zone = False
    if decel_dist > 0:
      if not self.decelerating:
        self.decelerating = True
        is_limit_zone = True

    if decel_dist <= 0:
      self.decelerating = False
      return safe_speed_kph, is_limit_zone

    # v_i^2 = v_f^2 + 2ad (physics formula)
    temp = safe_speed_ms**2 + 2 * safe_decel_rate * decel_dist

    if temp < 0:
      speed_ms = safe_speed_ms
    else:
      speed_ms = np.sqrt(temp)

    calculated_speed = conv.to_clu(speed_ms)
    safe_speed_clu = max(safe_speed_kph, min(255., calculated_speed))

    return safe_speed_clu, is_limit_zone

def signal_handler(sig, frame):
  print('Ctrl+C pressed, exiting.')
  terminate_flag.set()
  sys.exit(0)

if __name__ == "__main__":
  signal.signal(signal.SIGINT, signal_handler)
  main()
