/*
 * Copyright (C) 2020-present ScyllaDB
 */

/*
 * This file is part of Scylla.
 *
 * Scylla is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Scylla is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scylla.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <seastar/core/coroutine.hh>
#include "service/raft/raft_rpc.hh"
#include "gms/inet_address.hh"
#include "gms/inet_address_serializer.hh"
#include "serializer_impl.hh"
#include "message/msg_addr.hh"
#include "message/messaging_service.hh"
#include "db/timeout_clock.hh"
#include "idl/raft.dist.hh"

namespace service {

static seastar::logger rlogger("raft_rpc");

raft_rpc::raft_rpc(raft_state_machine& sm, netw::messaging_service& ms, raft_address_map<>& address_map, raft::group_id gid, raft::server_id srv_id)
    : _sm(sm), _group_id(std::move(gid)), _server_id(srv_id), _messaging(ms), _address_map(address_map)
{}

future<raft::snapshot_reply> raft_rpc::send_snapshot(raft::server_id id, const raft::install_snapshot& snap, seastar::abort_source& as) {
    return ser::raft_rpc_verbs::send_raft_send_snapshot(&_messaging,
        netw::msg_addr(_address_map.get_inet_address(id)), db::no_timeout, _group_id, _server_id, id, snap);
}

future<> raft_rpc::send_append_entries(raft::server_id id, const raft::append_request& append_request) {
    return ser::raft_rpc_verbs::send_raft_append_entries(&_messaging,
        netw::msg_addr(_address_map.get_inet_address(id)), db::no_timeout, _group_id, _server_id, id, append_request);
}

void raft_rpc::send_append_entries_reply(raft::server_id id, const raft::append_reply& reply) {
    (void)with_gate(_shutdown_gate, [this, id, &reply] {
        return ser::raft_rpc_verbs::send_raft_append_entries_reply(&_messaging, netw::msg_addr(_address_map.get_inet_address(id)), timeout(), _group_id, _server_id, id, reply)
            .handle_exception([id] (std::exception_ptr ex) {
                try {
                    std::rethrow_exception(ex);
                } catch (seastar::rpc::timeout_error&) {
                } catch (...) {
                    rlogger.error("Failed to send append reply to {}: {}", id, std::current_exception());
                }
            });
    });
}

void raft_rpc::send_vote_request(raft::server_id id, const raft::vote_request& vote_request) {
    (void)with_gate(_shutdown_gate, [this, id, vote_request] {
        return ser::raft_rpc_verbs::send_raft_vote_request(&_messaging, netw::msg_addr(_address_map.get_inet_address(id)), timeout(), _group_id, _server_id, id, vote_request)
            .handle_exception([id] (std::exception_ptr ex) {
                try {
                    std::rethrow_exception(ex);
                } catch (seastar::rpc::timeout_error&) {
                } catch (...) {
                    rlogger.error("Failed to send vote request {}: {}", id, ex);
                }
            });
    });
}

void raft_rpc::send_vote_reply(raft::server_id id, const raft::vote_reply& vote_reply) {
    (void)with_gate(_shutdown_gate, [this, id, vote_reply] {
        return ser::raft_rpc_verbs::send_raft_vote_reply(&_messaging, netw::msg_addr(_address_map.get_inet_address(id)), timeout(), _group_id, _server_id, id, vote_reply)
            .handle_exception([id] (std::exception_ptr ex) {
                try {
                    std::rethrow_exception(ex);
                } catch (seastar::rpc::timeout_error&) {
                } catch (...) {
                    rlogger.error("Failed to send vote reply {}: {}", id, ex);
                }
            });
    }); 
}

void raft_rpc::send_timeout_now(raft::server_id id, const raft::timeout_now& timeout_now) {
    (void)with_gate(_shutdown_gate, [this, id, timeout_now] {
        return ser::raft_rpc_verbs::send_raft_timeout_now(&_messaging, netw::msg_addr(_address_map.get_inet_address(id)), timeout(), _group_id, _server_id, id, timeout_now)
            .handle_exception([id] (std::exception_ptr ex) {
                try {
                    std::rethrow_exception(ex);
                } catch (seastar::rpc::timeout_error&) {
                } catch (...) {
                    rlogger.error("Failed to send timeout now {}: {}", id, ex);
                }
            });
    }); 
}

void raft_rpc::send_read_quorum(raft::server_id id, const raft::read_quorum& read_quorum) {
    (void)with_gate(_shutdown_gate, [&] {
        return ser::raft_rpc_verbs::send_raft_read_quorum(&_messaging, netw::msg_addr(_address_map.get_inet_address(id)), timeout(), _group_id, _server_id, id, read_quorum)
            .handle_exception([id] (std::exception_ptr ex) {
                try {
                    std::rethrow_exception(ex);
                } catch (seastar::rpc::timeout_error&) {
                } catch (...) {
                    rlogger.error("Failed to send read barrier {}: {}", id, ex);
                }
            });
    });
}

void raft_rpc::send_read_quorum_reply(raft::server_id id, const raft::read_quorum_reply& check_quorum_reply) {
    (void)with_gate(_shutdown_gate, [&] {
        return ser::raft_rpc_verbs::send_raft_read_quorum_reply(&_messaging, netw::msg_addr(_address_map.get_inet_address(id)), timeout(), _group_id, _server_id, id, check_quorum_reply)
            .handle_exception([id] (std::exception_ptr ex) {
                try {
                    std::rethrow_exception(ex);
                } catch (seastar::rpc::timeout_error&) {
                } catch (...) {
                    rlogger.error("Failed to send read barrier reply {}: {}", id, ex);
                }
            });
    });
}

future<raft::add_entry_reply> raft_rpc::send_add_entry(raft::server_id id, const raft::command& cmd) {
    return ser::raft_rpc_verbs::send_raft_add_entry(&_messaging,
        netw::msg_addr(_address_map.get_inet_address(id)), db::no_timeout, _group_id, _server_id, id, cmd);
}

future<raft::add_entry_reply> raft_rpc::send_modify_config(raft::server_id id,
    const std::vector<raft::server_address>& add,
    const std::vector<raft::server_id>& del) {
   return ser::raft_rpc_verbs::send_raft_modify_config(&_messaging, netw::msg_addr(_address_map.get_inet_address(id)),
       db::no_timeout, _group_id, _server_id, id, add, del);
}

future<raft::read_barrier_reply> raft_rpc::execute_read_barrier_on_leader(raft::server_id id) {
   return ser::raft_rpc_verbs::send_raft_execute_read_barrier_on_leader(&_messaging, netw::msg_addr(_address_map.get_inet_address(id)), db::no_timeout, _group_id, _server_id, id);
}

void raft_rpc::add_server(raft::server_id id, raft::server_info info) {
    // Parse gms::inet_address from server_info
    auto in = ser::as_input_stream(bytes_view(info));
    // Entries explicitly managed via `rpc::add_server` and `rpc::remove_server` should never expire
    // while entries learnt upon receiving an rpc message should be expirable.
    _address_map.set(id, ser::deserialize(in, boost::type<gms::inet_address>()), false);
}

void raft_rpc::remove_server(raft::server_id id) {
    _address_map.erase(id);
}

future<> raft_rpc::abort() {
    return _shutdown_gate.close();
}

void raft_rpc::append_entries(raft::server_id from, raft::append_request append_request) {
    _client->append_entries(from, std::move(append_request));
}

void raft_rpc::append_entries_reply(raft::server_id from, raft::append_reply reply) {
    _client->append_entries_reply(from, std::move(reply));
}

void raft_rpc::request_vote(raft::server_id from, raft::vote_request vote_request) {
    _client->request_vote(from, vote_request);
}

void raft_rpc::request_vote_reply(raft::server_id from, raft::vote_reply vote_reply) {
    _client->request_vote_reply(from, vote_reply);
}

void raft_rpc::timeout_now_request(raft::server_id from, raft::timeout_now timeout_now) {
    _client->timeout_now_request(from, timeout_now);
}

void raft_rpc::read_quorum_request(raft::server_id from, raft::read_quorum check_quorum) {
    _client->read_quorum_request(from, check_quorum);
}

void raft_rpc::read_quorum_reply(raft::server_id from, raft::read_quorum_reply check_quorum_reply) {
    _client->read_quorum_reply(from, check_quorum_reply);
}

future<raft::read_barrier_reply> raft_rpc::execute_read_barrier(raft::server_id from) {
    return _client->execute_read_barrier(from);
}

future<raft::snapshot_reply> raft_rpc::apply_snapshot(raft::server_id from, raft::install_snapshot snp) {
    co_await _sm.transfer_snapshot(_address_map.get_inet_address(from), snp.snp.id);
    co_return co_await _client->apply_snapshot(from, std::move(snp));
}

future<raft::add_entry_reply> raft_rpc::execute_add_entry(raft::server_id from, raft::command cmd) {
    return _client->execute_add_entry(from, std::move(cmd));
}

future<raft::add_entry_reply> raft_rpc::execute_modify_config(raft::server_id from,
    std::vector<raft::server_address> add,
    std::vector<raft::server_id> del) {
    return _client->execute_modify_config(from, std::move(add), std::move(del));
}

} // end of namespace service
