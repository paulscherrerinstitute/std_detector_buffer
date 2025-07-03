/////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "ucx_rdma_endpoint.hpp"
#include <cstring>
#include <stdexcept>

namespace sdcc {

ucx_rdma_endpoint ucx_rdma_endpoint::create_server(std::span<std::byte> buffer)
{
  return {buffer, true};
}

ucx_rdma_endpoint ucx_rdma_endpoint::create_client()
{
  return ucx_rdma_endpoint(false);
}

ucx_rdma_endpoint::ucx_rdma_endpoint(std::span<std::byte> buffer, bool is_server)
    : buffer_(buffer), is_server_(is_server)
{
  ucp_params_t params{};
  params.field_mask = UCP_PARAM_FIELD_FEATURES;
  params.features = UCP_FEATURE_RMA;
  ucp_config_t* config = nullptr;
  check(ucp_config_read(nullptr, nullptr, &config), "ucp_config_read");
  check(ucp_init(&params, config, &ctx_), "ucp_init");
  ucp_config_release(config);

  ucp_worker_params_t wparams{};
  wparams.field_mask = UCP_WORKER_PARAM_FIELD_THREAD_MODE;
  wparams.thread_mode = UCS_THREAD_MODE_SINGLE;
  check(ucp_worker_create(ctx_, &wparams, &worker_), "ucp_worker_create");

  if (is_server_) {
    ucp_mem_map_params_t mmap_params{};
    mmap_params.field_mask = UCP_MEM_MAP_PARAM_FIELD_ADDRESS | UCP_MEM_MAP_PARAM_FIELD_LENGTH |
                             UCP_MEM_MAP_PARAM_FIELD_FLAGS;
    mmap_params.address = buffer_.data();
    mmap_params.length = buffer_.size();
    mmap_params.flags = UCP_MEM_MAP_NONBLOCK;
    check(ucp_mem_map(ctx_, &mmap_params, &memh_), "ucp_mem_map");

    check(ucp_rkey_pack(ctx_, memh_, &rkey_buf_, &rkey_size_), "ucp_rkey_pack");
  }

  check(ucp_worker_get_address(worker_, &worker_addr_, &addr_len_), "ucp_worker_get_address");
}

ucx_rdma_endpoint::ucx_rdma_endpoint(bool is_server)
    : is_server_(is_server)
{
  ucp_params_t params{};
  params.field_mask = UCP_PARAM_FIELD_FEATURES;
  params.features = UCP_FEATURE_RMA;
  ucp_config_t* config = nullptr;
  check(ucp_config_read(nullptr, nullptr, &config), "ucp_config_read");
  check(ucp_init(&params, config, &ctx_), "ucp_init");
  ucp_config_release(config);

  ucp_worker_params_t wparams{};
  wparams.field_mask = UCP_WORKER_PARAM_FIELD_THREAD_MODE;
  wparams.thread_mode = UCS_THREAD_MODE_SINGLE;
  check(ucp_worker_create(ctx_, &wparams, &worker_), "ucp_worker_create");

  check(ucp_worker_get_address(worker_, &worker_addr_, &addr_len_), "ucp_worker_get_address");
}

// Move constructor
ucx_rdma_endpoint::ucx_rdma_endpoint(ucx_rdma_endpoint&& other) noexcept
    : ctx_(std::exchange(other.ctx_, nullptr))
    , worker_(std::exchange(other.worker_, nullptr))
    , memh_(std::exchange(other.memh_, nullptr))
    , buffer_(other.buffer_)
    , rkey_buf_(std::exchange(other.rkey_buf_, nullptr))
    , rkey_size_(std::exchange(other.rkey_size_, 0))
    , worker_addr_(std::exchange(other.worker_addr_, nullptr))
    , addr_len_(std::exchange(other.addr_len_, 0))
    , is_server_(other.is_server_)
{}

// Move assignment
ucx_rdma_endpoint& ucx_rdma_endpoint::operator=(ucx_rdma_endpoint&& other) noexcept
{
  if (this != &other) {
    cleanup();
    ctx_ = std::exchange(other.ctx_, nullptr);
    worker_ = std::exchange(other.worker_, nullptr);
    memh_ = std::exchange(other.memh_, nullptr);
    buffer_ = other.buffer_;
    rkey_buf_ = std::exchange(other.rkey_buf_, nullptr);
    rkey_size_ = std::exchange(other.rkey_size_, 0);
    worker_addr_ = std::exchange(other.worker_addr_, nullptr);
    addr_len_ = std::exchange(other.addr_len_, 0);
    is_server_ = other.is_server_;
  }
  return *this;
}

ucx_rdma_endpoint::~ucx_rdma_endpoint() { cleanup(); }

void ucx_rdma_endpoint::cleanup() noexcept
{
  if (rkey_buf_) ucp_rkey_buffer_release(rkey_buf_);
  if (worker_addr_) ucp_worker_release_address(worker_, worker_addr_);
  if (memh_) ucp_mem_unmap(ctx_, memh_);
  if (worker_) ucp_worker_destroy(worker_);
  if (ctx_) ucp_cleanup(ctx_);
  rkey_buf_ = nullptr;
  worker_addr_ = nullptr;
  memh_ = nullptr;
  worker_ = nullptr;
  ctx_ = nullptr;
}

std::span<const std::byte> ucx_rdma_endpoint::worker_address() const
{
  return {reinterpret_cast<const std::byte*>(worker_addr_), addr_len_};
}

std::span<const std::byte> ucx_rdma_endpoint::rkey_blob() const
{
  return {static_cast<const std::byte*>(rkey_buf_), rkey_size_};
}

std::uintptr_t ucx_rdma_endpoint::buffer_base() const
{
  return reinterpret_cast<std::uintptr_t>(buffer_.data());
}

size_t ucx_rdma_endpoint::buffer_size() const
{
  return buffer_.size();
}

void ucx_rdma_endpoint::write_demo(std::string_view str)
{
  if (str.size() > buffer_.size()) throw std::runtime_error("demo string too large");
  std::memcpy(buffer_.data(), str.data(), str.size());
}

ucp_ep_h ucx_rdma_endpoint::create_ep(std::span<const std::byte> remote_addr)
{
  ucp_ep_params_t ep_params{};
  ep_params.field_mask = UCP_EP_PARAM_FIELD_REMOTE_ADDRESS;
  ep_params.address = reinterpret_cast<const ucp_address_t*>(remote_addr.data());
  ucp_ep_h ep = nullptr;
  check(ucp_ep_create(worker_, &ep_params, &ep), "ucp_ep_create");
  return ep;
}

ucp_rkey_h ucx_rdma_endpoint::unpack_rkey(ucp_ep_h ep, std::span<const std::byte> rkey_blob)
{
  ucp_rkey_h rkey = nullptr;
  check(ucp_ep_rkey_unpack(ep, rkey_blob.data(), &rkey), "ucp_ep_rkey_unpack");
  return rkey;
}

void ucx_rdma_endpoint::check(ucs_status_t st, const char* msg)
{
  if (st != UCS_OK) throw std::runtime_error(msg);
}

} // namespace sdcc
