import datetime


def extract_write_request(request_data):

    if 'output_file' not in request_data:
        raise RuntimeError('Mandatory field "output_file" missing.')
    output_file = request_data['output_file']

    if 'n_images' not in request_data:
        raise RuntimeError('Mandatory field "n_images" missing.')
    n_images = request_data['n_images']

    # if 'sources' not in request_data:
    #     raise RuntimeError('Mandatory field "sources" missing.')
    # sources = request_data['sources']
    # if isinstance(request_data['sources'], list):
    #     raise RuntimeError('Field "sources" must be a list.')

    return build_write_request(output_file=output_file, n_images=n_images)


# TODO: Add back sources to request.
def build_write_request(output_file, n_images):
    return {
        "output_file": output_file,
        'n_images': n_images,
        'timestamp': datetime.datetime.now().timestamp()
    }


def build_user_response(response):
    # TODO: Convert the response into something the user can read.
    return response


