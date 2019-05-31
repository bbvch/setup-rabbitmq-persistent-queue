/*
 * (C) Copyright 2019
 * Urs Fässler, bbv Software Services, http://bbv.ch
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "utility.h"

#include "unistd.h"

#include <amqp.h>

#include <memory>
#include <functional>
#include <iostream>
#include <vector>


namespace
{


std::string to_string(const amqp_bytes_t &value)
{
  return std::string{static_cast<char*>(value.bytes), value.len};
}

amqp_envelope_t* consume_message(amqp_envelope_t *envelope, const amqp_connection_state_t &conn)
{
  const auto res = amqp_consume_message(conn, envelope, nullptr, 0);

  if (res.reply_type != AMQP_RESPONSE_NORMAL) {
    std::cerr << "amqp_consume_message: " << res.reply_type << std::endl;
  }

  return (AMQP_RESPONSE_NORMAL == res.reply_type) ? envelope : nullptr;
}


}


int main(int argc, char **argv)
{
  const std::vector<std::string> arg{argv, argv+argc};

  if (arg.size() != 2) {
    std::cout << "expect 1 argument" << std::endl;
    std::cout << "<queue name>" << std::endl;
    return -1;
  }

  const auto hostname = "localhost";
  const auto port = 5672;
  const auto queuename = arg[1];
  const auto channel_number = 1;

  std::cout << "read from queue: \"" << queuename << "\"" << std::endl;

  try {
    const std::unique_ptr<amqp_connection_state_t_, decltype(&destroy_connection)> conn{create_connection(), &destroy_connection};
    const std::unique_ptr<void, std::function<void(void*)>> open_connection{connect(conn.get(), hostname, port), [&conn](void*){disconnect(conn.get());}};
    const std::unique_ptr<const amqp_channel_t, std::function<void(const amqp_channel_t*)>> channel{create_channel(conn.get(), channel_number), [&conn](const amqp_channel_t *channel){destroy_channel(conn.get(), channel);}};


    amqp_basic_consume(conn.get(), *channel.get(), amqp_cstring_bytes(queuename.c_str()), amqp_empty_bytes, 0, 0, 1, amqp_empty_table);
    fail_if_error(amqp_get_rpc_reply(conn.get()), "can not start consuming");

    while (true) {
      amqp_maybe_release_buffers(conn.get());

      const std::unique_ptr<amqp_envelope_t> envelope_memory{new amqp_envelope_t};
      std::unique_ptr<amqp_envelope_t, decltype(&amqp_destroy_envelope)> message{consume_message(envelope_memory.get(), conn.get()), &amqp_destroy_envelope};

      if (!message) {
        std::cout << "nop" << std::endl;
        break;
      }

      std::cout << "channel:          " << message->channel << std::endl;
      std::cout << "consumer tag:     " << to_string(message->consumer_tag) << std::endl;
      std::cout << "delivery tag:     " << message->delivery_tag << std::endl;
      std::cout << "redelivered:      " << message->redelivered << std::endl;
      std::cout << "exchange:         " << to_string(message->exchange) << std::endl;
      std::cout << "routing key:      " << to_string(message->routing_key) << std::endl;

      std::cout << "content type:     " << to_string(message->message.properties.content_type) << std::endl;
      std::cout << "content encoding: " << to_string(message->message.properties.content_encoding) << std::endl;

      std::cout << "message body:     " << to_string(message->message.body) << std::endl;

      if ((message->message.properties._flags & AMQP_BASIC_HEADERS_FLAG) != 0) {
        std::cout << "headers" << std::endl;
        const auto headers = message->message.properties.headers;

        for (int i = 0; i < headers.num_entries; i++) {
          const auto &entry = headers.entries[i];
          std::cout << "  ";
          std::cout << to_string(entry.key);
          std::cout << " (" << entry.value.kind << ")";
          if (entry.value.kind == AMQP_FIELD_KIND_UTF8) {
            std::cout << ": ";
            std::cout << to_string(entry.value.value.bytes);
          }
          std::cout << std::endl;
        }
      }

      const int ack_res = amqp_basic_ack(conn.get(), *channel.get(), message->delivery_tag, 0);
      if (ack_res != 0) {
        std::cerr << "could not ack" << std::endl;
      }
    }

  } catch (std::runtime_error e) {
    std::cerr << e.what() << std::endl;
    return -2;
  }

  return 0;
}
