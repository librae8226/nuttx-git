
/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/

#include <apps/netutils/MQTTLinux.h>
#include <debug.h>

char expired(Timer * timer)
{
  struct timeval now, res;
  gettimeofday(&now, NULL);
  timersub(&timer->end_time, &now, &res);
  return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_usec <= 0);
}

void countdown_ms(Timer * timer, unsigned int timeout)
{
  struct timeval now;
  gettimeofday(&now, NULL);
  struct timeval interval = { timeout / 1000, (timeout % 1000) * 1000 };
  timeradd(&now, &interval, &timer->end_time);
}

void countdown(Timer * timer, unsigned int timeout)
{
  struct timeval now;
  gettimeofday(&now, NULL);
  struct timeval interval = { timeout, 0 };
  timeradd(&now, &interval, &timer->end_time);
}

int left_ms(Timer * timer)
{
  struct timeval now, res;
  gettimeofday(&now, NULL);
  timersub(&timer->end_time, &now, &res);
  nvdbg("left %d ms\n", (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000);
  return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000;
}

void InitTimer(Timer * timer)
{
  timer->end_time = (struct timeval)
  {
  0, 0};
}

int linux_read(Network * n, unsigned char *buffer, int len, int timeout_ms)
{
  struct timeval interval = { timeout_ms / 1000, (timeout_ms % 1000) * 1000 };

  if (interval.tv_sec < 0 || (interval.tv_sec == 0 && interval.tv_usec <= 0))
    {
      interval.tv_sec = 0;
      interval.tv_usec = 100;
    }

  int r;
  r = setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval,
                 sizeof(struct timeval));
  if (r < 0)
    {
      ndbg("ERROR: setsockopt failed, r: %d\n", r);
    }

  int bytes = 0;
  while (bytes < len)
    {
      int rc = recv(n->my_socket, &buffer[bytes], (size_t) (len - bytes), 0);
      if (rc == -1)
        {
          if (errno != ENOTCONN && errno != ECONNRESET)
            {
              bytes = -1;
              break;
            }
        }
      else
        bytes += rc;
    }
  return bytes;
}

int linux_write(Network * n, unsigned char *buffer, int len, int timeout_ms)
{
  struct timeval tv;

  tv.tv_sec = 0;                /* 30 Secs Timeout */
  tv.tv_usec = timeout_ms * 1000;       // Not init'ing this can cause strange
                                        // errors

  setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,
             sizeof(struct timeval));
  int rc = write(n->my_socket, buffer, len);
  return rc;
}

void linux_disconnect(Network * n)
{
  close(n->my_socket);
}

void NewNetwork(Network * n)
{
  n->my_socket = 0;
  n->mqttread = linux_read;
  n->mqttwrite = linux_write;
  n->disconnect = linux_disconnect;
}

/* TODO IPv6 support */
int ConnectNetwork(Network * n, char *addr, int port)
{
  struct sockaddr_in server;
  struct timeval tv;
  int ret;

  if (!n)
    {
      ndbg("ERROR: no entity.\n");
      return -EINVAL;
    }

  n->my_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (n->my_socket < 0)
    {
      ndbg("ERROR: socket failed: %d\n", errno);
      return -EFAULT;
    }

  tv.tv_sec = 10;
  tv.tv_usec = 0;

  (void)setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (FAR const void *)&tv,
                   sizeof(struct timeval));
  (void)setsockopt(n->my_socket, SOL_SOCKET, SO_SNDTIMEO, (FAR const void *)&tv,
                   sizeof(struct timeval));

  /* Get the server address from the host name */

  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  ret = dns_gethostip(addr, &server.sin_addr.s_addr);
  if (ret < 0)
    {
      /* Could not resolve host (or malformed IP address) */

      ndbg("ERROR: Failed to resolve hostname, ret: %d\n", ret);
      ret = -EHOSTUNREACH;
      goto errout;
    }

  /* Connect to server.  First we have to set some fields in the 'server'
   * address structure.  The system will assign me an arbitrary local port that
   * is not in use. */
  ret =
    connect(n->my_socket, (struct sockaddr *)&server,
            sizeof(struct sockaddr_in));
  if (ret < 0)
    {
      ndbg("ERROR: connect failed: %d\n", errno);
      goto errout;
    }

  return OK;

errout:
  close(n->my_socket);
  return ERROR;
}
