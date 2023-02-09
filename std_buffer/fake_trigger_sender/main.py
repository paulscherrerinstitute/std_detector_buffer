import argparse
from time import sleep, time

from ..stream_binary import StdStreamSendBinary, ImageMetadata


def main():
    parser = argparse.ArgumentParser(description='Fake trigger - fake portions ')
    parser.add_argument('output_stream', type=str, help='Address to bind the output stream to.')
    parser.add_argument('-r', '--rep_rate', type=int, help='Repetition rate of the stream.', default=10)

    args = parser.parse_args()
    time_to_sleep = 1/args.rep_rate

    meta = ImageMetadata(height=1, width=1,
                         dtype=ImageMetadata.map_dtype_description_to_value('uint16'))

    iteration_start = time()
    with StdStreamSendBinary(args.output_stream) as output_stream:
        while True:
            meta.id = (meta.id + 1) % 10000
            output_stream.send_meta(meta)

            iteration_end = time()
            time_left_to_sleep = max(0.0, time_to_sleep - (iteration_end - iteration_start))
            sleep(time_left_to_sleep)
            iteration_start = iteration_end


if __name__ == '__main__':
    main()
