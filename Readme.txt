This is a simple daemon for the D-Link DNS-320L NAS system.

  It was developed on knowledge gained from reverse-engineering and spying 
  on the original D-Link system software.
  Parts of this work are based on fan-daemon.py by Lorenzo Martignoni.

  (c) 2013-2024 Andreas Boehler, dev _AT_ aboehler.at


How it works
============

  On bootup, dns320l-daemon is executed and runs as a deamon. On startup,
  It sends the DeviceReady command to the MCU so that the Power LED stops
  blinking.
  Afterwards, it goes to fan control loop.

  If the daemon is killed (e.g. during system shutdown), it sends the 
  DeviceShutdown command with a timeout of 10 seconds to the MCU. After 10
  seconds, the MCU kills power.
  
Installation
============

  Either compile the software on your NAS by typing "make" or cross-compile
  it from your host computer.
  Put the binary to /usr/bin.

Disclaimer
==========

  This program is free software: you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program. If not, see <http://www.gnu.org/licenses/>.

