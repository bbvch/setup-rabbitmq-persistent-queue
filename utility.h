/*
 * (C) Copyright 2018
 * Urs Fässler, bbv Software Services, http://bbv.ch
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <amqp.h>

#include <string>


void fail_if(bool predicate, const std::string &message);
void fail_if_error(const amqp_rpc_reply_t &reply, const std::string &message);

amqp_connection_state_t create_connection();
void destroy_connection(amqp_connection_state_t conn);

void* connect(const amqp_connection_state_t conn, const std::string &hostname, uint16_t port);
void disconnect(const amqp_connection_state_t conn);

const int* create_channel(const amqp_connection_state_t &conn, int channel);
void destroy_channel(const amqp_connection_state_t conn, const int *chptr);
