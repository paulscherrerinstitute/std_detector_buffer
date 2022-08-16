from setuptools import setup

setup(name="std_buffer",
      version="1.0.0",
      maintainer="Paul Scherrer Institute",
      maintainer_email="daq@psi.ch",
      author="Paul Scherrer Institute",
      author_email="daq@psi.ch",
      description="Standard DAQ buffer interface",

      license="GPL3",
      zip_safe=False,

      package_dir={
          "std_buffer.tools": 'tools',
          'std_buffer.gigafrost': 'testing/gigafrost',
          'std_buffer.jungfrau': 'testing/jungfrau'
      },
      packages=["std_buffer.tools",
                'std_buffer.gigafrost',
                'std_buffer.jungfrau'
                ],
      )
