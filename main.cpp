#include "utility.h"

#include <amqp.h>

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>
#include <functional>


int main(int argc, char **argv)
{
  const std::vector<std::string> arg{argv, argv+argc};

  if (arg.size() != 3) {
    std::cout << "expect 2 arguments" << std::endl;
    std::cout << "<exchange name> <queue name>" << std::endl;
    return -1;
  }

  const auto hostname = "localhost";
  const auto port = 5672;
  const auto channel_number = 1;
  const auto exchange = arg[1];
  const auto queuename = arg[2];

  std::cout << "creating persistent exchange, queue and binding: \"" << exchange << "\" -> \"" << queuename << "\"" << std::endl;

  try {
    const std::unique_ptr<amqp_connection_state_t_, decltype(&destroy_connection)> conn{create_connection(), &destroy_connection};
    const std::unique_ptr<void, std::function<void(void*)>> open_connection{connect(conn.get(), hostname, port), [&conn](void*){disconnect(conn.get());}};
    const std::unique_ptr<const int, std::function<void(const int*)>> channel{create_channel(conn.get(), channel_number), [&conn](const int *channel){destroy_channel(conn.get(), channel);}};

    // create exchange
    const auto exchange_res = amqp_exchange_declare(conn.get(), *channel.get(), amqp_cstring_bytes(exchange.c_str()), amqp_cstring_bytes("fanout"), 0, 1, 0, 0, amqp_empty_table);
    fail_if(!exchange_res, "can not declare exchange");

    // create queue
    amqp_queue_declare(conn.get(), *channel.get(), amqp_cstring_bytes(queuename.c_str()), 0, 1, 0, 0, amqp_empty_table);
    const auto queue_res = amqp_get_rpc_reply(conn.get());
    fail_if_error(queue_res, "can not declare queue");

    // bind queue to exchange
    amqp_queue_bind(conn.get(), *channel.get(), amqp_cstring_bytes(queuename.c_str()), amqp_cstring_bytes(exchange.c_str()), amqp_empty_bytes, amqp_empty_table);
    const auto bind_res = amqp_get_rpc_reply(conn.get());
    fail_if_error(bind_res, "can not bind queue to exchange");

  } catch (std::runtime_error e) {
    std::cerr << e.what() << std::endl;
    return -2;
  }

  std::cout << "ok" << std::endl;

  return 0;
}
