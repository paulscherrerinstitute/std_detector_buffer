/////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <ucp/api/ucp.h>
#include <span>
#include <string>
#include <memory>
#include <cstring>
#include <stdexcept>
#include <utility>

namespace sdcc {

class ucx_rdma_endpoint
{
public:
  using buffer_view = std::span<std::byte>;
  using const_buffer_view = std::span<const std::byte>;

  explicit ucx_rdma_endpoint(std::span<std::byte> buffer);
  ucx_rdma_endpoint();
  ucx_rdma_endpoint(ucx_rdma_endpoint&& other) noexcept;
  ucx_rdma_endpoint& operator=(ucx_rdma_endpoint&& other) noexcept;
  ucx_rdma_endpoint(const ucx_rdma_endpoint&) = delete;
  ucx_rdma_endpoint& operator=(const ucx_rdma_endpoint&) = delete;

  ~ucx_rdma_endpoint() { cleanup(); }

  std::span<const std::byte> worker_address() const
  {
    return {reinterpret_cast<const std::byte*>(worker_addr_), addr_len_};
  }
  std::span<const std::byte> rkey_blob() const
  {
    return {reinterpret_cast<const std::byte*>(rkey_buf_), rkey_size_};
  }
  std::uintptr_t buffer_base() const { return reinterpret_cast<std::uintptr_t>(buffer_.data()); }
  size_t buffer_size() const { return buffer_.size(); }

  void write_demo(std::string_view str);

  ucp_ep_h create_ep(std::span<const std::byte> remote_addr);
  ucp_rkey_h unpack_rkey(ucp_ep_h ep, std::span<const std::byte> rkey_blob);

  ucp_worker_h worker() const { return worker_; }
  ucp_context_h context() const { return ctx_; }

private:
  void cleanup() noexcept;

  static void check(ucs_status_t st, const char* msg)
  {
    if (st != UCS_OK) throw std::runtime_error(msg);
  }

  ucp_context_h ctx_ = nullptr;
  ucp_worker_h worker_ = nullptr;
  ucp_mem_h memh_ = nullptr;    // server only
  std::span<std::byte> buffer_; // server only
  void* rkey_buf_ = nullptr;    // server only
  size_t rkey_size_ = 0;        // server only
  ucp_address_t* worker_addr_ = nullptr;
  size_t addr_len_ = 0;
  bool is_server_ = false;
};

} // namespace sdcc
