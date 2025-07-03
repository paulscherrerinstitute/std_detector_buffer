/////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <ucp/api/ucp.h>
#include <span>
#include <string>
#include <stdexcept>
#include <utility>

namespace sdcc {

class ucx_rdma_endpoint
{
public:
  using buffer_view = std::span<std::byte>;
  using const_buffer_view = std::span<const std::byte>;

  static ucx_rdma_endpoint create_server(std::span<std::byte> buffer);
  static ucx_rdma_endpoint create_client();

  ucx_rdma_endpoint(ucx_rdma_endpoint&& other) noexcept;
  ucx_rdma_endpoint& operator=(ucx_rdma_endpoint&& other) noexcept;
  ucx_rdma_endpoint(const ucx_rdma_endpoint&) = delete;
  ucx_rdma_endpoint& operator=(const ucx_rdma_endpoint&) = delete;

  ~ucx_rdma_endpoint();

  [[nodiscard]] std::span<const std::byte> worker_address() const;
  [[nodiscard]] std::span<const std::byte> rkey_blob() const;
  [[nodiscard]] std::uintptr_t buffer_base() const;
  [[nodiscard]] size_t buffer_size() const;

  void write_demo(std::string_view str);

  ucp_ep_h create_ep(std::span<const std::byte> remote_addr);
  ucp_rkey_h unpack_rkey(ucp_ep_h ep, std::span<const std::byte> rkey_blob);

  ucp_worker_h worker() const { return worker_; }
  ucp_context_h context() const { return ctx_; }

private:
  ucx_rdma_endpoint(std::span<std::byte> buffer, bool is_server);
  explicit ucx_rdma_endpoint(bool is_server);

  void cleanup() noexcept;
  static void check(ucs_status_t st, const char* msg);

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
