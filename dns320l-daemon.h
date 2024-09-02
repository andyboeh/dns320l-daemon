#ifndef DNS320L_DAEMON_H
#define DNS320L_DAEMON_H


typedef struct
{


  int fanPollTime;
  int pollGpio;
  int gpioPollTime;
  int goDaemon;
  int debug;
  char *gpioDir;
  char *portName;
  int tempLow;
  int tempHigh;
  int hysteresis;
  int nRetries;
  int delayShutdown;

} DaemonConfig;

/** <i>Function</i> that reads a GPIO value from sysfs interface.
  @param gpio The GPIO number to read
  @param value Pointer where the value is to be put
  @param gpioDir Pointer containing the sysfs path to the GPIO subdir
  @return The GPIO's value
  */
int gpio_get_value(unsigned int gpio, unsigned int *value);

/** <i>Function</i> that is called by the OS upon sending a signal
    to the application
 @param sig The signal number received
*/
static void sighandler(int sig);

/** <i>Function</i> that checks the first few bytes of the MCU's response
  whether it corresponds to the sent command
  @param buf The buffer to compare
  @param cmd The command that was sent
  @param len The lenght of the command
  @return SUCCESS on success, otherwise ERR_WRONG_ANSWER
*/
int CheckResponse(char *buf, char *cmd, int len);

/** <i>Function</i> that clears the current Serial Port buffer
 by reading some bytes
 @param fd The serial port to work on
 */
void ClearSerialPort(int fd);

/** <i>Function</i> that wraps around the internal send command and handles retry
  @param fd The serial port to work on
  @param cmd The command to send
  @param outArray An array where the response shall be put, can be NULL for no response
  @return SUCCESS, ERR_WRONG_ANSWER or the number of bytes received
  */
int SendCommand(int fd, char *cmd, char *outArray);

/** <i>Function</i> that sends a command to the MCU and waits
  for response and/or ACK.
  @param fd The serial port to work on
  @param cmd The command to send
  @param outArray An array where the response shall be put, can be NULL for no response
  @return SUCCESS, ERR_WRONG_ANSWER or the number of bytes received
  */
int _SendCommand(int fd, char *cmd, char *outArray);

/** <i>Main Function</i>
  @param argc The argument count
  @param argv The argument vector
  @return EXIT_SUCCESS on success, otherwise EXIT_ERROR
*/
int main(int argc, char *argv[]);



#endif //DNS320L_DAEMON_H
