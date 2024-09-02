/*

  Simple system daemon for D-Link DNS-320L

  (c) 2013-2024 Andreas Boehler, dev _AT_ aboehler.at

  This code is based on a few other people's work and in parts shamelessly copied.
  The ThermalTable was provided by Lorenzo Martignoni and the fan control 
  algorithm is based on his fan-daemon.py implementation.
  
  The MCU protocol was reverse engineered by strace() calls to up_send_daemon and
  up_read_daemon of the original firmware.

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

*/

#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <signal.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/uio.h>
#include <syslog.h>

#include "dns320l.h"
#include "dns320l-daemon.h"

DaemonConfig stDaemonConfig;

/** @file dns320l-daemon.c
    @brief Implementation of a free system daemon replacement for
           the D-Link DNS-320L NAS
    @author Andreas Boehler, andreas _AT_ aboehler.at
    @version 1.0
    @date 2013/09/12
*/


int gpio_get_value(unsigned int gpio, unsigned int *value)
{
  int fd, len;
  char buf[100];
  char ch;

  len = snprintf(buf, sizeof(buf), "%s/gpio%d/value", stDaemonConfig.gpioDir, gpio);

  fd = open(buf, O_RDONLY);
  if (fd < 0) {
    syslog(LOG_ERR, "gpio/get-value");
    return fd;
  }

  read(fd, &ch, 1);

  if (ch != '0') {
    *value = 1;
  } else {
    *value = 0;
  }

  close(fd);
  return 0;
}

static void sighandler(int sig)
{
  syslog(LOG_DEBUG, "Signal Handler called\n");
  switch(sig)
  {
  case SIGINT:
    exit(EXIT_SUCCESS);
    break;
  case SIGTERM:
    exit(EXIT_SUCCESS);
    break;
  }
}

int CheckResponse(char *buf, char *cmd, int len)
{
  int i;
  int tmp;
  int failure = 0;

  // Attention, 5 is hardcoded here and never checked!
  for(i=0;i<5;i++)
  {
    if(buf[i] != cmd[i])
    {
      syslog(LOG_ERR, "Char %i is %i but should be %i\n", i, buf[i], cmd[i]);
      failure = 1;
      break;
    }
  }
  if(failure)
  {
    for(i=0;i<len;i++)
    {
      syslog(LOG_DEBUG, "Buf/Cmd %i: %i %i\n", i, buf[i], cmd[i]);
    }
    return ERR_WRONG_ANSWER;
  }
/*  if(buf[len-1] != cmd[len-1])
  {
    syslog(LOG_ERR, "Last character does not match! Char %i is %i but should be %i\n", len-1, buf[len-1], cmd[len-1]);
    return ERR_WRONG_ANSWER;
  }
*/
  return SUCCESS;
}

void ClearSerialPort(int fd)
{
  char buf[100];
  struct pollfd fds[1];
  fds[0].fd = fd;
  fds[0].events = POLLIN;
  int n = 0;
  int pollrc;
  pollrc = poll(fds, 1, 0);
  if(pollrc > 0)
  {
    if(fds[0].revents & POLLIN)
    {
      syslog(LOG_DEBUG, "Clearing Serial Port...\n");
      do
      {
        n = read(fd, buf, sizeof(buf));
      } while(n == sizeof(buf));
    }
  }
}

int SendCommand(int fd, char *cmd, char *outArray)
{
  int nRetries = -1;
  int ret;
  do
  {
    ret = _SendCommand(fd, cmd, outArray);
    nRetries++;
    syslog(LOG_DEBUG, "Try number: %i\n", nRetries+1);
  } while((ret != SUCCESS) && (nRetries < stDaemonConfig.nRetries));

  return ret;
}

int _SendCommand(int fd, char *cmd, char *outArray)
{
  int n;
  int i;
  int j;
  ssize_t count;

  char buf[15]; // We need to keep the DateAndTime values here
  // Yes, we're sending byte by byte here - b/c the length of
  // commands and responses can vary!

  ClearSerialPort(fd); // We clear the serial port in case
  // some old data from a previous request is still pending

  i=0;
  do
  {
    count = write(fd, &cmd[i], 1);
    i++;
    usleep(100); // The MCU seems to need some time..
    if(count != 1)
    {
      syslog(LOG_ERR, "Error writing byte %i: %i, count: %i\n", (i-1), cmd[i-1], count);
      return ERR_WRITE_ERROR;
    }
  } while(cmd[i-1] != CMD_STOP_MAGIC);

  i=0;
  do
  {
    n = read(fd, &buf[i], 1);
    i++;
  } while((n == 1) && (buf[i-1] != CMD_STOP_MAGIC));


  if(buf[i-1] != CMD_STOP_MAGIC)
  {
    syslog(LOG_ERR, "Got no stop magic, but read %i bytes!\n", i);
    for(j=0;j<i;j++)
    {
      syslog(LOG_DEBUG, "Buf %i: %i\n", j, buf[j]);
    }
    return ERR_WRONG_ANSWER;
  }
  else
  {
    // If outArray is not NULL, an answer was requested
    if(outArray != NULL)
    {
      if(CheckResponse(buf, cmd, i) != SUCCESS)
      {
        return ERR_WRONG_ANSWER;
      }
      // Copy the answer to the outArray
      for(j=0; j<i; j++)
      {
        outArray[j] = buf[j];
      }
      usleep(20000); // Give the µC some time to answer...

      // Wait for ACK from Serial
      i=0;
      do
      {
        n = read(fd, &buf[i], 1);
        i++;
      } while((n == 1) && (buf[i-1] != CMD_STOP_MAGIC));


      if(buf[i-1] != CMD_STOP_MAGIC)
      {
        syslog(LOG_ERR, "Got no stop magic!\n");
        for(j=0;j<i;j++)
        {
         syslog(LOG_DEBUG, "Buf %i: %i\n", j, buf[j]);
        }

        return ERR_WRONG_ANSWER;
      }

      CheckResponse(buf, AckFromSerial, i);
      syslog(LOG_DEBUG, "Returning %i read bytes\n", n);
      return SUCCESS;
    }
    // Only wait for ACK if no response is expected
    else
    {
      return CheckResponse(buf, AckFromSerial, i);
    }
  }
}

int main(int argc, char *argv[])
{
  char response[500];
  int i;
  pid_t pid;
  pid_t sid;
  int powerBtn;
  int pressed;
  int opt;
  int sleepCount;
  int pollTimeMs;
  int readRtcOnStartup = 0;
  char buf[100];
  char msgBuf[15];
  int temperature;
  int fanSpeed;
  struct sockaddr_in s_name;
  struct pollfd *fds = NULL;
  nfds_t nfds;
  int retval;
  int ret;
  int msgIdx;
  char message[500];
  socklen_t namelength;
  pressed = 0;
  nfds = 1;
  opt = 1;
  sleepCount = 0;
  pollTimeMs = 10; // Sleep 10ms for every loop
  fanSpeed = -1;
  
  stDaemonConfig.goDaemon = 1;
  stDaemonConfig.debug = 0;

  // Parse command line arguments
  while((i = getopt(argc, argv, "fd")) != -1)
  {
    switch(i)
    {
      case 'f':
        stDaemonConfig.goDaemon = 0;
        break;
      case 'd':
        stDaemonConfig.debug = 1;
        stDaemonConfig.goDaemon = 0;
        break;
      case '?':
        if(optopt == 'c')
          fprintf(stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        fprintf(stderr, "Usage: %s [-f] [-d]\n", argv[0]);
        fprintf(stderr, "       where\n");
        fprintf(stderr, "         -f              don't detach\n");
        fprintf(stderr, "         -d              debug (implies -f)\n");
        return EXIT_FAILURE;
    }
  
  }
  
  // Register some signal handlers
  signal(SIGTERM, sighandler);
  signal(SIGINT, sighandler);
  
  stDaemonConfig.portName = "/dev/ttyS1";
  stDaemonConfig.fanPollTime = 15;
  stDaemonConfig.tempLow = 45;
  stDaemonConfig.tempHigh = 50;
  stDaemonConfig.hysteresis = 2;
  stDaemonConfig.gpioPollTime = 1;
  stDaemonConfig.gpioDir = "/sys/class/gpio";
  stDaemonConfig.pollGpio = 1;
  stDaemonConfig.nRetries = 5;
  stDaemonConfig.delayShutdown = 30;

  // Setup syslog
  if(stDaemonConfig.debug)
    setlogmask(LOG_UPTO(LOG_DEBUG));
  else
    setlogmask(LOG_UPTO(LOG_INFO));
  
  if(stDaemonConfig.goDaemon)
    openlog("dns320l-daemon", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
  else
    openlog("dns320l-daemon", LOG_CONS | LOG_PID | LOG_NDELAY | LOG_PERROR, LOG_LOCAL1);
    
  if(stDaemonConfig.goDaemon)
  {
    pid = fork();
    if(pid < 0)
    {
      syslog(LOG_ERR, "Forking failed.\n");
      return EXIT_FAILURE;
    }
    
    if(pid > 0)
    {
      return EXIT_SUCCESS;
    }
    // From here on we are the child process...
    umask(0);
    sid = setsid();
    if(sid < 0)
    {
      syslog(LOG_ERR, "Could not create process group\n");
      return EXIT_FAILURE;
    }
    
    if((chdir("/")) < 0)
    {
       syslog(LOG_ERR, "Could not chdir(\"/\")\n");
       return EXIT_FAILURE;
    }
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
  
  }

  // Send the DeviceReady command to the MCU
  if(SendCommand(fd, DeviceReadyCmd, NULL) == SUCCESS)
    syslog(LOG_INFO, "dns320l-daemon startup complete, going to FanControl mode");
  else
  {
    syslog(LOG_ERR, "Error sending DeviceReady command, exit!\n");
    return EXIT_FAILURE;
  }

  // Go to endless loop and do the following:
  // Get the thermal status
  // Check temperature and adjust fan speeds
  // Wake every 1s to poll the power button GPIO
  // Sleep
  
  while(1)
  {
    sleepCount = 0;
    if(SendCommand(fd, ThermalStatusGetCmd, msgBuf) > ERR_WRONG_ANSWER)
      temperature = msgBuf[5];
    else
      temperature = 0;
    if(temperature > 0)
    {
      temperature = ThermalTable[temperature];
      syslog(LOG_DEBUG, "Read Temperature: %i\n", temperature);
      if(temperature < (stDaemonConfig.tempLow - stDaemonConfig.hysteresis))
      {
        if(fanSpeed != 0)
        {
          syslog(LOG_DEBUG, "Set Fan Stop\n");
          SendCommand(fd, FanStopCmd, NULL);
          fanSpeed = 0;
        }
      }
      else if(temperature < stDaemonConfig.tempLow)
      {
        if(fanSpeed > 1)
        {
          syslog(LOG_DEBUG, "Set Fan Half\n");
          SendCommand(fd, FanHalfCmd, NULL);
          fanSpeed = 1;
        }
      }
      else if(temperature < (stDaemonConfig.tempHigh - stDaemonConfig.hysteresis))
      {
        if(fanSpeed != 1)
        {
          syslog(LOG_DEBUG, "Set Fan Half\n");
          SendCommand(fd, FanHalfCmd, NULL);
          fanSpeed = 1;
        }
      }
      else if(temperature < stDaemonConfig.tempHigh)
      {
        if(fanSpeed < 1)
        {
          syslog(LOG_DEBUG, "Set Fan Half\n");
          SendCommand(fd, FanHalfCmd, NULL);
          fanSpeed = 1;
        }
      }
      else
      {
        if(fanSpeed != 2)
        {
          syslog(LOG_DEBUG, "Set Fan Full\n");
          SendCommand(fd, FanFullCmd, NULL);
          fanSpeed = 2;
        }
      }
    }
    else
    {
      syslog(LOG_ERR, "Error reading Temperature!\n");
    }


    while((sleepCount  * pollTimeMs) < (stDaemonConfig.fanPollTime * 1000))
    {
      if(stDaemonConfig.pollGpio && (((sleepCount * pollTimeMs) % (stDaemonConfig.gpioPollTime* 1000)) == 0))
      {
        if(gpio_get_value(GPIO_BUTTON_POWER, &powerBtn) == 0)
        {
          if((powerBtn == 0) && !pressed)
          {
            pressed = 1;
            syslog(LOG_INFO, "Power Button Pressed, shutting down system!\n");
            DeviceShutdownCmd[5] = (char)stDaemonConfig.delayShutdown;
            SendCommand(fd, DeviceShutdownCmd, NULL);
            execl("/sbin/poweroff", "poweroff", (char *)0);
          }
        }

      }
      sleepCount++;
    }
  }
  closelog();
  return EXIT_SUCCESS;
}
