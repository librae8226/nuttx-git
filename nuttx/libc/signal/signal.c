#include <signal.h>

_sa_handler_t signal(int signo, _sa_handler_t handler)
{
  struct sigaction sa;

  sa.sa_handler = handler;
  sa.sa_flags = SA_SIGINFO;
  sigemptyset(&sa.sa_mask);

  if (sigaction(signo, &sa, &sa))
    {
      return (_sa_handler_t) SIG_ERR;
    }
  else
    {
      return (_sa_handler_t) sa.sa_handler;
    }
}
