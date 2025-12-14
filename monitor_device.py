#!/usr/bin/env python3
"""
BT Scanner Device Monitor
Überwacht ein ESP32-BT-Scanner-Gerät und protokolliert wichtige Metriken.
Erkennt Memory-Leaks, Neustarts und Anomalien.
"""

import requests
import time
import json
import sys
from datetime import datetime
from typing import Dict, Optional
import argparse


class DeviceMonitor:
    def __init__(self, device_ip: str, interval: int = 5, log_file: str = "device_monitor.log"):
        self.device_ip = device_ip
        self.base_url = f"http://{device_ip}"
        self.interval = interval
        self.log_file = log_file
        
        # Tracking für Anomalie-Erkennung
        self.last_uptime_seconds = 0
        self.restart_count = 0
        self.min_heap = float('inf')
        self.max_heap = 0
        self.heap_history = []
        self.max_history_size = 100
        
        # Alarm-Schwellwerte
        self.heap_critical_threshold = 50000  # 50KB
        self.heap_warning_threshold = 100000  # 100KB
        
    def parse_uptime(self, uptime_str: str) -> int:
        """Konvertiert Uptime-String (z.B. '2h 15m 30s') in Sekunden."""
        seconds = 0
        parts = uptime_str.split()
        
        for part in parts:
            if 'h' in part:
                seconds += int(part.replace('h', '')) * 3600
            elif 'm' in part:
                seconds += int(part.replace('m', '')) * 60
            elif 's' in part:
                seconds += int(part.replace('s', ''))
        
        return seconds
    
    def fetch_status(self) -> Optional[Dict]:
        """Holt Status vom Gerät."""
        try:
            response = requests.get(f"{self.base_url}/api/status", timeout=5)
            if response.status_code == 200:
                return response.json()
            else:
                self.log_error(f"HTTP {response.status_code}")
                return None
        except requests.exceptions.RequestException as e:
            self.log_error(f"Connection failed: {e}")
            return None
    
    def fetch_devices(self) -> Optional[Dict]:
        """Holt Geräteliste vom Gerät."""
        try:
            response = requests.get(f"{self.base_url}/api/devices", timeout=5)
            if response.status_code == 200:
                return response.json()
            else:
                return None
        except requests.exceptions.RequestException:
            return None
    
    def log_message(self, message: str, level: str = "INFO"):
        """Schreibt formatierte Log-Nachricht."""
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        log_entry = f"[{timestamp}] [{level:7}] {message}"
        
        # Console output mit Farben
        if level == "ERROR":
            print(f"\033[91m{log_entry}\033[0m")  # Rot
        elif level == "WARNING":
            print(f"\033[93m{log_entry}\033[0m")  # Gelb
        elif level == "RESTART":
            print(f"\033[95m{log_entry}\033[0m")  # Magenta
        elif level == "SUCCESS":
            print(f"\033[92m{log_entry}\033[0m")  # Grün
        else:
            print(log_entry)
        
        # File output
        with open(self.log_file, 'a') as f:
            f.write(log_entry + '\n')
    
    def log_error(self, message: str):
        """Logged Fehlermeldung."""
        self.log_message(message, "ERROR")
    
    def analyze_status(self, status: Dict):
        """Analysiert Status-Daten auf Anomalien."""
        uptime_str = status.get('uptime', '0s')
        uptime_seconds = self.parse_uptime(uptime_str)
        heap_free = status.get('heap_free', 0)
        
        # Neustart-Erkennung
        if uptime_seconds < self.last_uptime_seconds:
            self.restart_count += 1
            self.log_message(
                f"🔴 RESTART DETECTED! Count: {self.restart_count} | "
                f"Previous uptime: {self.last_uptime_seconds}s | "
                f"Current uptime: {uptime_seconds}s",
                "RESTART"
            )
        
        self.last_uptime_seconds = uptime_seconds
        
        # Heap-Analyse
        self.heap_history.append(heap_free)
        if len(self.heap_history) > self.max_history_size:
            self.heap_history.pop(0)
        
        self.min_heap = min(self.min_heap, heap_free)
        self.max_heap = max(self.max_heap, heap_free)
        
        # Heap-Warnungen
        if heap_free < self.heap_critical_threshold:
            self.log_message(
                f"🔴 CRITICAL: Free heap {heap_free} bytes (< {self.heap_critical_threshold})",
                "ERROR"
            )
        elif heap_free < self.heap_warning_threshold:
            self.log_message(
                f"⚠️  WARNING: Free heap {heap_free} bytes (< {self.heap_warning_threshold})",
                "WARNING"
            )
        
        # Memory-Leak-Erkennung (Heap sinkt kontinuierlich)
        if len(self.heap_history) >= 20:
            recent_avg = sum(self.heap_history[-10:]) / 10
            older_avg = sum(self.heap_history[-20:-10]) / 10
            heap_drop = older_avg - recent_avg
            
            if heap_drop > 10000:  # 10KB Drop
                self.log_message(
                    f"⚠️  Possible memory leak: Heap dropped {int(heap_drop)} bytes over last 20 samples",
                    "WARNING"
                )
    
    def format_status_line(self, status: Dict) -> str:
        """Erstellt formatierte Status-Zeile."""
        return (
            f"Uptime: {status.get('uptime', 'N/A'):>12} | "
            f"Heap: {status.get('heap_free', 0):>7}B | "
            f"Devices: {status.get('devices', 0):>3} | "
            f"Ever: {status.get('devices_ever', 0):>4} | "
            f"Known: {status.get('known', 0):>3} | "
            f"Present: {status.get('present', 0):>3} | "
            f"WiFi: {status.get('wifi_rssi', 0):>4}dBm | "
            f"Scanning: {status.get('scanning', False)}"
        )
    
    def run(self):
        """Hauptschleife des Monitors."""
        self.log_message(f"Starting device monitor for {self.device_ip}", "SUCCESS")
        self.log_message(f"Interval: {self.interval}s | Log file: {self.log_file}", "INFO")
        self.log_message("-" * 120, "INFO")
        
        consecutive_errors = 0
        sample_count = 0
        
        try:
            while True:
                sample_count += 1
                
                # Status abrufen
                status = self.fetch_status()
                
                if status:
                    consecutive_errors = 0
                    
                    # Anomalie-Analyse
                    self.analyze_status(status)
                    
                    # Status-Zeile ausgeben (alle 10 Samples oder bei wichtigen Events)
                    if sample_count % 10 == 1:
                        self.log_message(self.format_status_line(status), "INFO")
                    
                    # Erweiterte Statistiken alle 60 Samples (~5 Minuten bei 5s Interval)
                    if sample_count % 60 == 0:
                        self.log_extended_stats(status)
                
                else:
                    consecutive_errors += 1
                    if consecutive_errors >= 3:
                        self.log_message(
                            f"Device unreachable for {consecutive_errors * self.interval}s",
                            "ERROR"
                        )
                
                time.sleep(self.interval)
                
        except KeyboardInterrupt:
            self.log_message("\nMonitoring stopped by user", "INFO")
            self.print_summary()
    
    def log_extended_stats(self, status: Dict):
        """Logged erweiterte Statistiken."""
        heap_avg = sum(self.heap_history) / len(self.heap_history) if self.heap_history else 0
        
        self.log_message("-" * 120, "INFO")
        self.log_message(
            f"📊 STATS | "
            f"Restarts: {self.restart_count} | "
            f"Heap: min={self.min_heap}B avg={int(heap_avg)}B max={self.max_heap}B | "
            f"Current Ever Seen: {status.get('devices_ever', 'N/A')}",
            "INFO"
        )
        self.log_message("-" * 120, "INFO")
    
    def print_summary(self):
        """Gibt Zusammenfassung aus."""
        self.log_message("=" * 120, "INFO")
        self.log_message("MONITORING SUMMARY", "SUCCESS")
        self.log_message(f"Total restarts detected: {self.restart_count}", "INFO")
        self.log_message(f"Heap statistics: min={self.min_heap}B max={self.max_heap}B", "INFO")
        if self.heap_history:
            avg_heap = sum(self.heap_history) / len(self.heap_history)
            self.log_message(f"Average heap: {int(avg_heap)}B over {len(self.heap_history)} samples", "INFO")
        self.log_message("=" * 120, "INFO")


def main():
    parser = argparse.ArgumentParser(
        description='Monitor ESP32 BT Scanner device for restarts and memory issues'
    )
    parser.add_argument(
        '--ip',
        default='192.168.2.44',
        help='Device IP address (default: 192.168.2.44)'
    )
    parser.add_argument(
        '--interval',
        type=int,
        default=5,
        help='Polling interval in seconds (default: 5)'
    )
    parser.add_argument(
        '--log',
        default='device_monitor.log',
        help='Log file path (default: device_monitor.log)'
    )
    
    args = parser.parse_args()
    
    monitor = DeviceMonitor(
        device_ip=args.ip,
        interval=args.interval,
        log_file=args.log
    )
    
    monitor.run()


if __name__ == '__main__':
    main()
