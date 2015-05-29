
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
 *******************************************************************************/

/*
   stdin publisher

   compulsory parameters:

   --topic topic to publish on

   defaulted parameters:

   --host localhost
   --port 1883
   --qos 0
   --delimiters \n
   --clientid stdin_publisher
   --maxdatalen 100

   --userid none
   --password none

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
#include <string.h>

#include <apps/netutils/MQTTClient.h>

void mqttpub_usage(void)
{
  printf("MQTT publisher\n");
  printf("Usage: mqttpub topicname <options>, where options are:\n");
  printf("  --host <hostname> (default is localhost)\n");
  printf("  --port <port> (default is 1883)\n");
  printf("  --qos <qos> (default is 0)\n");
  printf("  --msg <message> (if not provided, use default string)\n");
  printf("  --retained (default is off)\n");
  printf("  --delimiter <delim> (default is \\n)");
  printf("  --clientid <clientid> (default is hostname+timestamp)");
  printf("  --maxdatalen 100\n");
  printf("  --username none\n");
  printf("  --password none\n");
}

struct opts_s
{
  char *clientid;
  char *delimiter;
  int maxdatalen;
  int qos;
  int retained;
  char *username;
  char *password;
  char *host;
  int port;
  int verbose;
  char *msg;
};

void mqttpub_getopts(int argc, char **argv, struct opts_s *p_opts)
{
  int count = 2;

  while (count < argc)
    {
      if (strcmp(argv[count], "--retained") == 0)
        p_opts->retained = 1;
      if (strcmp(argv[count], "--verbose") == 0)
        p_opts->verbose = 1;
      else if (strcmp(argv[count], "--qos") == 0)
        {
          if (++count < argc)
            {
              if (strcmp(argv[count], "0") == 0)
                p_opts->qos = 0;
              else if (strcmp(argv[count], "1") == 0)
                p_opts->qos = 1;
              else if (strcmp(argv[count], "2") == 0)
                p_opts->qos = 2;
              else
                mqttpub_usage();
            }
          else
            mqttpub_usage();
        }
      else if (strcmp(argv[count], "--host") == 0)
        {
          if (++count < argc)
            p_opts->host = argv[count];
          else
            mqttpub_usage();
        }
      else if (strcmp(argv[count], "--port") == 0)
        {
          if (++count < argc)
            p_opts->port = atoi(argv[count]);
          else
            mqttpub_usage();
        }
      else if (strcmp(argv[count], "--msg") == 0)
        {
          if (++count < argc)
            p_opts->msg = argv[count];
          else
            mqttpub_usage();
        }
      else if (strcmp(argv[count], "--clientid") == 0)
        {
          if (++count < argc)
            p_opts->clientid = argv[count];
          else
            mqttpub_usage();
        }
      else if (strcmp(argv[count], "--username") == 0)
        {
          if (++count < argc)
            p_opts->username = argv[count];
          else
            mqttpub_usage();
        }
      else if (strcmp(argv[count], "--password") == 0)
        {
          if (++count < argc)
            p_opts->password = argv[count];
          else
            mqttpub_usage();
        }
      else if (strcmp(argv[count], "--maxdatalen") == 0)
        {
          if (++count < argc)
            p_opts->maxdatalen = atoi(argv[count]);
          else
            mqttpub_usage();
        }
      else if (strcmp(argv[count], "--delimiter") == 0)
        {
          if (++count < argc)
            p_opts->delimiter = argv[count];
          else
            mqttpub_usage();
        }
      count++;
    }
}

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int mqttpub_main(int argc, char *argv[])
#endif
{
  Network n;
  Client c;
  MQTTMessage msg;
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  struct opts_s opts = {"mqtt-publisher", "\n", 100, 0, 0, NULL, NULL, "localhost", 1883, 0, NULL};
  char *topic = NULL;
  char buf[100];
  char msgbuf[100];
  char readbuf[100];
  int rc = 0;

  if (argc < 2)
    {
      mqttpub_usage();
      return -1;
    }

  mqttpub_getopts(argc, argv, &opts);

  topic = argv[1];
  printf("Using topic %s\n", topic);

  NewNetwork(&n);
  if (ConnectNetwork(&n, opts.host, opts.port) != OK)
    {
      printf("Failed to connect network, exit\n");
      return -EFAULT;
    }

  MQTTClient(&c, &n, 1000, (unsigned char *)buf, 100, (unsigned char *)readbuf, 100);

  data.MQTTVersion = 3;
  data.clientID.cstring = opts.clientid;

  rc = MQTTConnect(&c, &data);
  printf("Connected %d\n", rc);

  bzero(msgbuf, sizeof(msgbuf));
  msg.qos = opts.qos;
  msg.retained = false;
  msg.dup = false;
  if (opts.msg == NULL)
    sprintf(msgbuf, "Hi from NuttX! QoS%d message", msg.qos);
  else {
    strcpy(msgbuf, opts.msg);
    printf("custom message\n");
  }
  msg.payload = (void *)msgbuf;
  msg.payloadlen = strlen(msgbuf) + 1;

  rc = MQTTPublish(&c, topic, &msg);
  if (rc != 0)
    printf("Error publish, rc: %d\n", rc);
  if (opts.qos > 0)
    MQTTYield(&c, 100);

  printf("Stopping\n");

  MQTTDisconnect(&c);
  n.disconnect(&n);

  return 0;
}
