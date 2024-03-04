import argparse
from time import sleep, time

from ..stream_binary import StdStreamSendBinary
from .. import image_metadata_pb2 as daq_proto


def main():
    parser = argparse.ArgumentParser(description='Fake trigger - fake portions ')
    parser.add_argument('output_stream', type=str, help='Address to bind the output stream to.')
    parser.add_argument('-r', '--rep_rate', type=int, help='Repetition rate of the stream.', default=10)

    args = parser.parse_args()
    time_to_sleep = 1 / args.rep_rate

    metadata = daq_proto.ImageMetadata()

    iteration_start = time()
    with StdStreamSendBinary(args.output_stream) as output_stream:
        while True:
            metadata.image_id = (metadata.image_id + 1) % 1000000
            metadata.dtype = daq_proto.ImageMetadataDtype.uint16
            metadata.width = 2016
            metadata.height = 2016
            metadata.size = metadata.height * metadata.width * 2
            metadata.compression = daq_proto.ImageMetadataCompression.none

            output_stream.send_meta(metadata.SerializeToString())

            iteration_end = time()
            time_left_to_sleep = max(0.0, time_to_sleep - (iteration_end - iteration_start))
            sleep(time_left_to_sleep)
            iteration_start = iteration_end


if __name__ == '__main__':
    main()
