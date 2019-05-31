/*
 * (C) Copyright 2019
 * Urs Fässler, bbv Software Services, http://bbv.ch
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "utility.h"

#include <unistd.h>

#include <amqp.h>

#include <string.h>

#include <memory>
#include <functional>
#include <vector>
#include <iostream>
#include <map>
#include <algorithm>

namespace
{


amqp_bytes_t fromString(const std::string &value)
{
  amqp_bytes_t result;
  result.bytes = malloc(value.size());
  ::memcpy(result.bytes, value.c_str(), value.size());
  result.len = value.size();

  return result;
}

amqp_table_t createHeaders(const std::map<std::string, std::string> &headers)
{
  amqp_table_t table;
  table.num_entries = headers.size();
  table.entries = static_cast<struct amqp_table_entry_t_ *>(calloc(table.num_entries, sizeof(amqp_table_entry_t)));

  std::transform(headers.cbegin(), headers.cend(), table.entries, [](const std::pair<std::string, std::string> &value){
    amqp_table_entry_t_ entry;

    entry.key = fromString(value.first);
    entry.value.kind = AMQP_FIELD_KIND_UTF8;
    entry.value.value.bytes = fromString(value.second);

    return entry;
  });

  return table;
}

void destroyHeaders(const amqp_table_t &headers)
{
    for (int i = 0; i < headers.num_entries; i++) {
      free(headers.entries[i].key.bytes);
      free(headers.entries[i].value.value.bytes.bytes);
    }
    free(headers.entries);
}


}


int main(int argc, char **argv)
{
  const std::vector<std::string> arg{argv, argv+argc};

  if (arg.size() != 2) {
    std::cout << "expect 1 argument" << std::endl;
    std::cout << "<exchange name>" << std::endl;
    return -1;
  }

  const auto hostname = "localhost";
  const auto port = 5672;
  const auto exchange = arg[1];
  const auto channel_number = 1;

  std::cout << "send to exchange: \"" << exchange << "\"" << std::endl;

  try {
    const std::unique_ptr<amqp_connection_state_t_, decltype(&destroy_connection)> conn{create_connection(), &destroy_connection};
    const std::unique_ptr<void, std::function<void(void*)>> open_connection{connect(conn.get(), hostname, port), [&conn](void*){disconnect(conn.get());}};
    const std::unique_ptr<const amqp_channel_t, std::function<void(const amqp_channel_t*)>> channel{create_channel(conn.get(), channel_number), [&conn](const amqp_channel_t *channel){destroy_channel(conn.get(), channel);}};


    std::size_t messageNr = 0;

    while (true) {
      messageNr++;

      const auto messagebody = "{\"message\": \"Hello World!\", \"number\": " + std::to_string(messageNr) + "}";

      amqp_basic_properties_t props;
      props._flags = 0;
      props.headers.num_entries = 0;

      props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_CONTENT_ENCODING_FLAG | AMQP_BASIC_HEADERS_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
      props.content_type = amqp_cstring_bytes("application/json");
      props.content_encoding = amqp_cstring_bytes("utf-8");
      props.headers = createHeaders(
      {
//        {"gateway:type", "properties"},
        {"gateway:type", "message"},
      });

      props.delivery_mode = 2; /* persistent delivery mode */
      const auto res = amqp_basic_publish(conn.get(), *channel.get(), amqp_cstring_bytes(exchange.c_str()), amqp_empty_bytes, 0, 0, &props, amqp_cstring_bytes(messagebody.c_str()));
      destroyHeaders(props.headers);
      fail_if(res != AMQP_STATUS_OK, "can not publish message");

      std::cout << "sent: " << messagebody << std::endl;

      sleep(3);
    }

  } catch (std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    return -2;
  }

  return 0;
}
