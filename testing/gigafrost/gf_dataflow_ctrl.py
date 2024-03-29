import os
import argparse
import numpy as np

from epics import pv

IOC_NAME_GF1 = 'X02DA-CAM-GF1'
IOC_NAME_GF2 = 'X02DA-CAM-GF2'
EPICS_SETERROR_FAILED_CHANNEL = "failed to set channel %s \n"
EPICS_GETERROR_FAILED_CHANNEL = "failed to get channel %s \n"


class EpicsError(Exception):
    pass


def get(ioc_name, name, as_string=False, get_meta=False):
    """
    Get channel value.

    Parameters
    ----------
    ioc_name : str
        Name of the ioc 
    name : str
        Name of the channel to retrieve the data.
    as_string : bool, optional
        If set to true, return the string representation of the PV.
        Otherwise, the numeric representation is returned.
        (default = False)
    get_meta : bool, optional
        Flag to retrieve the data and metadata. (default=False)

    """

    this_pv = pv.get_pv(pvname="%s:%s" % (ioc_name, name), connect=True)
    if not get_meta:
        value = this_pv.get(as_string=as_string, timeout=10)
        if value is None:
            raise EpicsError(EPICS_GETERROR_FAILED_CHANNEL % name)
        return value
    else:
        value_dict = this_pv.get_with_metadata(
            as_string=as_string, timeout=10)
        # The PV status, which will be 0 for a normal, connected PV
        if value_dict is None or 'value' not in value_dict:
            raise EpicsError(EPICS_GETERROR_FAILED_CHANNEL % name)
        return value_dict


def set(ioc_name, name, value, wait=True):
    """
    Set channel access PV value.

    Parameters
    ----------
    ioc_name : str
        Name of the ioc
    name : str
        Name of the PV
    value
        Value to set. The type must match the PV type.
    wait : bool, optional
        Wait for the change to take effect (default=True)

    """

    this_pv = pv.get_pv(pvname="%s:%s" % (ioc_name, name), connect=True)
    return_code = this_pv.put(value=value, wait=wait,
                                timeout=10)

    if return_code < 0:
        val_str = str(value)
        if len(val_str) > 800:
            val_str = val_str[:780] + " ...\n...\n... " + val_str[-20:]
        raise EpicsError(EPICS_SETERROR_FAILED_CHANNEL %
                            (name, val_str))

def port2byte(port):
    return [(port >> 8) & 0xff, port & 0xff]

def extend_header_table(table, mac, destination_ip, destination_port,
                        source_port):
    """
        Extend the header table by a further entry.

        Parameters
        ----------
        table :
            The table to be extended
        mac : 
            The mac address for the new entry
        destination_ip :
            The destination IP address for the new entry
        destination_port :
            The destination port for the new entry
        source_port :
            The source port for the new entry

        """
    table.extend(mac)
    table.extend(destination_ip)
    table.extend(port2byte(destination_port))
    table.extend(port2byte(source_port))
    return table

def set_udp_header(backend):
    """
    Build the header table for the communication
    """
    # xbl-daq-33 parameters
    if backend == 'xbl-daq-33':
        north_mac = [0x94, 0x40, 0xc9, 0xb4, 0xb8, 0x01]
        south_mac = [0x94, 0x40, 0xc9, 0xb4, 0xa8, 0xd9]
        north_ip = [10, 4, 0, 102]
        south_ip = [10, 0, 0, 102]
    elif backend == 'xbl-daq-35':
        north_mac = [0x94, 0x40, 0xc9, 0xb4, 0xa8, 0xe1]
        south_mac = [0x94, 0x40, 0xc9, 0xb4, 0xa8, 0xb9]
        north_ip = [10, 4, 0, 103]
        south_ip = [10, 0, 0, 103]
    else:
        raise ValueError('Backend address not recognized. Options: xbl-daq-33 or xbl-daq-35.')



    udp_header_table = []
    for i in range(0,64,1):
        for j in range(0,8,1):
            dest_port = 2000 + 8 * i + j
            source_port = 3000+j
            if j < 4:
                extend_header_table(
                    udp_header_table, south_mac, south_ip,
                    dest_port, source_port)
            else:
                extend_header_table(
                    udp_header_table, north_mac, north_ip,
                    dest_port, source_port)
    return udp_header_table

def configure_cam_header(camera, backend, roix, roiy, n_images):
    udp_header_table = set_udp_header(backend)
    set(camera, 'CONN_PARM', udp_header_table, wait=True)
    # stops & prepare soft enable
    set(camera, 'START_CAM', 0)
    set(camera, 'SOFT_ENABLE', 0)
    # prepare camera's configuration
    set(camera, 'EXPOSURE', 0.1)
    set(camera, 'FRAMERATE', 1,0)
    set(camera, 'ROIX', roix)
    set(camera, 'ROIY', roiy)
    set(camera, 'SCAN_ID', 0)
    set(camera, 'CNT_NUM', n_images)
    set(camera, 'CORR_MODE', 5)
    # set config in camera
    set(camera, 'SET_PARAM.PROC', 1)


def get_cam_status():
    status_gf1 = "IDLE" if get(IOC_NAME_GF1, 'START_CAM') == 0 else "RUNNING"
    status_gf2 = "IDLE" if get(IOC_NAME_GF2, 'START_CAM') == 0 else "RUNNING"
    print(f'Status GF1 : {status_gf1}')
    print(f'Status GF2 : {status_gf2}')


def start_dataflow(camera, backend, roix, roiy, n_images):
    configure_cam_header(camera, backend, roix, roiy, n_images)
    # Start & triggers soft enable
    set(camera, 'START_CAM', 1)
    set(camera, 'SOFT_ENABLE', 1)

def stop_dataflow(camera):
    set(camera, 'START_CAM', 0)
    set(camera, 'SOFT_ENABLE', 0)

def validate_cam_config(camera, backend, roix, roiy, n_images):
    get_values_list = [get(camera, 'START_CAM'),
                       get(camera, 'SOFT_ENABLE'), 
                       get(camera, 'EXPOSURE'), 
                       get(camera, 'FRAMERATE'),
                       get(camera, 'ROIX'),
                       get(camera, 'ROIY'),
                       get(camera, 'SCAN_ID'),
                       get(camera, 'CNT_NUM'),
                       get(camera, 'CORR_MODE'),
                       get(camera, 'SET_PARAM.PROC')]
    
    expected_values = [0, 0, 0.1, 1.0, roix, roiy, 0, n_images, 5, 1]
    if any(value != expected_value for value, expected_value in zip(get_values_list, expected_values)):
        raise ValueError(f'Problem with the definition of the configuration on the camera ({camera})')
    if not np.array_equal(np.array(get(camera, 'CONN_PARM')), np.array(set_udp_header(backend))):
        raise ValueError(f'Problem with the header on camera {camera} and configuration to the backend {backend}')

def main():
    parser = argparse.ArgumentParser(description='Script to start and stop the datastream from a gigafrost camera.')

    group = parser.add_mutually_exclusive_group(required=True)

    group.add_argument('--start', action='store_true', help='Start the stream from a gigafrost camera.')
    parser.add_argument('-c', '--camera', type=str, help='The IOC name of the camera.', default='X02DA-CAM-GF2')
    parser.add_argument('-x', '--roix', type=int, help='ROIY', default=2016)
    parser.add_argument('-y', '--roiy', type=int, help='ROIY', default=2016)
    parser.add_argument('-b', '--backend', type=str, help='Backend server that should receive the stream.', default='xbl-daq-33')
    parser.add_argument('-n', '--n_images', default='-1', type=int, help='Number of images to stream (-1 for max).')

    group.add_argument('--stop', action='store_true', help='Stop the stream from a gigafrost camera.')

    group.add_argument('--status', action='store_true', help='Gets the status of the gigafrost cameras.')

    args = parser.parse_args()
    
    os.environ['EPICS_CA_ADDR_LIST']='129.129.99.127 129.129.99.119'

    if args.start:
        camera = args.camera.upper()
        roiy = args.roiy
        roix = args.roix
        n_images = args.n_images
        backend = args.backend.lower()
        
        if get(camera, 'START_CAM') != 1:
            configure_cam_header(camera, backend, roix, roiy, n_images)
            validate_cam_config(camera,backend,roix,roiy,n_images)
            print(f'Starting the camera gigafrost ({camera}) '
              f'with image_shape ({roix, roiy}) '
              f'for {n_images} images.')
            start_dataflow(camera, backend, roix, roiy, n_images)
        else:
            print(f'Camera {camera} is already running...')
    elif args.stop:
        camera = args.camera.upper()
        print(f'Stopping the camera gigafrost ({camera})...')
        stop_dataflow(camera)
    elif args.status:
        get_cam_status()


if __name__ == '__main__':
    main()

