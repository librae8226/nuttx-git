/*******************************************************************************
 * Copyright (c) 2012, 2013 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *   http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial contribution
 *    Ian Craggs - change delimiter option from char to string
 *    Al Stockdill-Mander - Version using the embedded C client
 *******************************************************************************/

/*

   mqtt subscriber

   compulsory parameters:

   topic to subscribe to

   defaulted parameters:

   --host localhost
   --port 1883
   --qos 2
   --delimiter \n
   --clientid stdout_subscriber

   --userid none
   --password none

   for example:

   stdoutsub topic/of/interest --host iot.eclipse.org

*/
#include <nuttx/config.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <debug.h>
#include <signal.h>
#include <string.h>

#include <apps/netutils/MQTTClient.h>

#define SIGINT	2
#define SIGKILL	9
#define SIGTERM	15

volatile int toStop = 0;

void usage(void)
{
  printf("MQTT subscriber\n");
  printf("Usage: mqttsub topicname <options>, where options are:\n");
  printf("  --host <hostname> (default is localhost)\n");
  printf("  --port <port> (default is 1883)\n");
  printf("  --qos <qos> (default is 2)\n");
  printf("  --delimiter <delim> (default is \\n)\n");
  printf("  --clientid <clientid> (default is hostname+timestamp)\n");
  printf("  --username none\n");
  printf("  --password none\n");
  printf
    ("  --showtopics <on or off> (default is on if the topic has a wildcard, else off)\n");
  exit(-1);
}

void cfinish(int sig)
{
  printf("sig: %d\n", sig);
  signal(SIGINT, NULL);
  toStop = 1;
}

struct opts_s
{
  char *clientid;
  int nodelimiter;
  char *delimiter;
  enum QoS qos;
  char *username;
  char *password;
  char *host;
  int port;
  int showtopics;
};

void getopts(int argc, char **argv, struct opts_s *p_opts)
{
  int count = 2;

  while (count < argc)
    {
      if (strcmp(argv[count], "--qos") == 0)
        {
          if (++count < argc)
            {
              if (strcmp(argv[count], "0") == 0)
                p_opts->qos = QOS0;
              else if (strcmp(argv[count], "1") == 0)
                p_opts->qos = QOS1;
              else if (strcmp(argv[count], "2") == 0)
                p_opts->qos = QOS2;
              else
                usage();
            }
          else
            usage();
        }
      else if (strcmp(argv[count], "--host") == 0)
        {
          if (++count < argc)
            p_opts->host = argv[count];
          else
            usage();
        }
      else if (strcmp(argv[count], "--port") == 0)
        {
          if (++count < argc)
            p_opts->port = atoi(argv[count]);
          else
            usage();
        }
      else if (strcmp(argv[count], "--clientid") == 0)
        {
          if (++count < argc)
            p_opts->clientid = argv[count];
          else
            usage();
        }
      else if (strcmp(argv[count], "--username") == 0)
        {
          if (++count < argc)
            p_opts->username = argv[count];
          else
            usage();
        }
      else if (strcmp(argv[count], "--password") == 0)
        {
          if (++count < argc)
            p_opts->password = argv[count];
          else
            usage();
        }
      else if (strcmp(argv[count], "--delimiter") == 0)
        {
          if (++count < argc)
            p_opts->delimiter = argv[count];
          else
            p_opts->nodelimiter = 1;
        }
      else if (strcmp(argv[count], "--showtopics") == 0)
        {
          if (++count < argc)
            {
              if (strcmp(argv[count], "on") == 0)
                p_opts->showtopics = 1;
              else if (strcmp(argv[count], "off") == 0)
                p_opts->showtopics = 0;
              else
                usage();
            }
          else
            usage();
        }
      count++;
    }

}

void printstrbylen(char *msg, char *str, int len)
{
  int i;
  DEBUGASSERT(str != NULL);
  printf("%s (len: %d)\n\t", msg, len);
  for (i = 0; i < len; i++)
    printf("%c", str[i]);
  printf("\n\t");
  for (i = 0; i < len; i++)
    printf("%02x ", str[i]);
  printf("\n");
}

void messageArrived(MessageData * md, struct opts_s *p_opts)
{
  MQTTMessage *message = md->message;
  if (p_opts->showtopics)
    printstrbylen("topic:", md->topicName->lenstring.data,
                  md->topicName->lenstring.len);
  printstrbylen("payload:", (char *)message->payload, message->payloadlen);
}

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int mqttsub_main(int argc, char *argv[])
#endif
{
  int rc = 0;
  unsigned char buf[100];
  unsigned char readbuf[100];
  struct opts_s opts = {(char *)"mqtt-subscriber", 0, (char *)"\n", QOS2, NULL, NULL, (char *)"localhost", 1883, 0};

  if (argc < 2)
    usage();

  char *topic = argv[1];

  if (strchr(topic, '#') || strchr(topic, '+'))
    opts.showtopics = 1;
  if (opts.showtopics)
    printf("topic is %s\n", topic);

  getopts(argc, argv, &opts);

  Network n;
  Client c;

  signal(SIGINT, cfinish);
  signal(SIGTERM, cfinish);

  NewNetwork(&n);
  if (ConnectNetwork(&n, opts.host, opts.port) != OK)
    {
      printf("Failed to connect network, exit\n");
      return -EFAULT;
    }
  MQTTClient(&c, &n, 1000, buf, 100, readbuf, 100);

  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  data.willFlag = 0;
  data.MQTTVersion = 3;
  data.clientID.cstring = opts.clientid;
  data.username.cstring = opts.username;
  data.password.cstring = opts.password;

  data.keepAliveInterval = 10;
  data.cleansession = 1;
  printf("Connecting to %s %d\n", opts.host, opts.port);

  rc = MQTTConnect(&c, &data);
  printf("Connected %d\n", rc);

  printf("Subscribing to %s\n", topic);
  rc = MQTTSubscribe(&c, topic, opts.qos, messageArrived);
  printf("Subscribed %d\n", rc);

  while (!toStop)
    {
      MQTTYield(&c, 1000);
    }

  printf("Stopping\n");

  MQTTDisconnect(&c);
  n.disconnect(&n);

  return 0;
}
