/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>

#include <zmq.h>
#include <mpi.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include "core_buffer/buffer_utils.hpp"
#include "live_writer_config.hpp"
#include "WriterStats.hpp"
#include "JFH5Writer.hpp"
#include "DetWriterConfig.hpp"

using namespace std;
using namespace live_writer_config;

int main(int argc, char* argv[])
{
  if (argc != 2) {
    cout << endl;
    cout << "Usage: std_det_writer [detector_json_filename]" << endl;
    cout << "\tdetector_json_filename: detector config file path." << endl;
    cout << endl;

    exit(-1);
  }

  auto const config = converter::from_json_file(string(argv[1]));

  MPI_Init(nullptr, nullptr);

  int n_writers;
  MPI_Comm_size(MPI_COMM_WORLD, &n_writers);

  int i_writer;
  MPI_Comm_rank(MPI_COMM_WORLD, &i_writer);

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, LIVE_ZMQ_IO_THREADS);
  auto receiver = buffer_utils::connect_socket(ctx, config.detector_name + "-sync");

  const size_t IMAGE_N_BYTES = config.image_width * config.image_height * config.bit_depth / 8;

  JFH5Writer writer(config.detector_name);
  WriterStats stats(config.detector_name, IMAGE_N_BYTES);

  char recv_buffer_meta[512];
  char recv_buffer_data[4838400];
  bool open_run = false;
  bool header_in = false;

  while (true) {
    auto nbytes = zmq_recv(receiver, &recv_buffer_meta, sizeof(recv_buffer_meta), 0);
    rapidjson::Document document;
    if (document.Parse(recv_buffer_meta, nbytes).HasParseError()) {
      std::string error_str(recv_buffer_meta, nbytes);
      throw runtime_error(error_str);
    }

    const string output_file = document["output_file"].GetString();
    const int run_id = document["run_id"].GetInt();
    const int i_image = document["i_image"].GetInt();
    const int n_images = document["n_images"].GetInt();

    const int status = document["status"].GetInt();
    const rapidjson::Value& a = document["shape"];
    const int width = a[0].GetInt();
    const int heigth = a[1].GetInt();
    const int dtype = 2;

    // i_image == n_images -> end of run.
    if (i_image == n_images && open_run == true) {
      writer.close_run();
      stats.end_run();
      open_run = false;

      continue;
    }

    // i_image == 0 -> we have a new run.
    if (i_image == 0 && open_run == false) {
      writer.open_run(output_file, run_id, n_images, heigth, width, dtype);
      open_run = true;
    }

    // data
    auto img_nbytes = zmq_recv(receiver, &recv_buffer_data, sizeof(recv_buffer_data), 0);
    if (img_nbytes != -1 && header_in == true && open_run == true) {
      // Fair distribution of images among writers.
      if (i_image % n_writers == i_writer) {
        stats.start_image_write();
        writer.write_data(run_id, i_image, recv_buffer_data);
        stats.end_image_write();
      }
      header_in = false;
    }

    // Only the first instance writes metadata.
    if (i_writer == 0 && header_in == true && open_run == true) {
      writer.write_meta_gf(run_id, i_image, (uint16_t)run_id, (uint64_t)status);
    }
  }
}
