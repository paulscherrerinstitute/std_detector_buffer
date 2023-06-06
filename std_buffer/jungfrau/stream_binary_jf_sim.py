#!/usr/bin/python3
import argparse
from time import sleep, time
import numpy

from ..stream_binary import StdStreamSendBinary, ImageMetadata


def generate_simulated_image(image_shape, beam_shape, noise, dtype):
    beam_x = numpy.linspace(-beam_shape[1] + numpy.random.rand(), beam_shape[1] + numpy.random.rand(),
                            image_shape[0])
    beam_y = numpy.linspace(-beam_shape[0] + numpy.random.rand(), beam_shape[0] + numpy.random.rand(),
                            image_shape[1])
    x, y = numpy.meshgrid(beam_y, beam_x)

    image = numpy.exp(-(x ** 2 + y ** 2))
    image += numpy.random.random(image_shape) * noise

    return image.astype(dtype)


def generate_jf_binary_stream(output_stream_address, rep_rate=10,
                              image_shape=(512, 1024), beam_shape=(20, 100), dtype="float32", noise=0.1):
    """Generate a simulated Jungfrau binary stream

    Use this function to generate a ZMQ stream in binary format using the ImageMetadata header. If you want a
    realistic image that could potentially come out of the JF detector the image shape needs to be a multiple of
    1024 in the Y direction and a multiple of 512 in the X direction.

    This function blocks until interrupted (with KeyboardInterrupt).

    :param str output_stream_address: Address to which to bind the output stream.
    :param float rep_rate: Stream repetition rate. How many images per second to generate.
    :param tuple image_shape: Shape of the image in (y_size, x_size) format.
    :param tuple beam_shape: Shape of the beam in the image in (y_size, x_size) format.
    :param str dtype: Data type to generate the image in. Use float32 if simulating a converted image, uint16 otherwise.
    :param float noise: Noise value multiplier. 0.1 is a good starting point.
    :return: None
    """
    time_to_sleep = 1/rep_rate

    meta = ImageMetadata(height=image_shape[0], width=image_shape[1],
                         dtype=ImageMetadata.map_dtype_description_to_value(dtype))
    meta.status = 0
    meta.source_id = 0

    data = generate_simulated_image(image_shape, beam_shape, noise, dtype)

    image_id = 1
    iteration_start = time()
    with StdStreamSendBinary(output_stream_address) as output_stream:
        try:
            while True:
                meta.id = image_id
                output_stream.send(meta, data)

                image_id += 1
                data = generate_simulated_image(image_shape, beam_shape, noise, dtype)

                iteration_end = time()
                time_left_to_sleep = max(0.0, time_to_sleep - (iteration_end - iteration_start))
                sleep(time_left_to_sleep)
                iteration_start = iteration_end

        except KeyboardInterrupt:
            pass


def main():
    parser = argparse.ArgumentParser(description='Jungfrau binary stream generator.')
    parser.add_argument('output_stream', type=str, help='Address to bind the output stream to.')
    parser.add_argument('-n', '--n_modules', type=int, help='Number of modules to simulate', default=1)
    parser.add_argument('-r', '--rep_rate', type=int, help='Repetition rate of the stream.', default=10)

    args = parser.parse_args()
    output_stream = args.output_stream
    n_modules = args.n_modules
    rep_rate = args.rep_rate

    print(f'Starting simulated JF with n_modules {n_modules} and rep_rate {rep_rate} on {output_stream}.')

    image_shape = (512 * n_modules, 1024)
    print(f'Expected image shape: {image_shape}.')

    generate_jf_binary_stream(output_stream, rep_rate, image_shape)

    print('Stopping simulated stream.')


if __name__ == '__main__':
    main()
