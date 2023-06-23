import os
from setuptools import setup


version = os.getenv('PACKAGE_VERSION', '0.0.0')

setup(name="std_buffer",
      version=version,
      maintainer="Paul Scherrer Institute",
      maintainer_email="daq@psi.ch",
      author="Paul Scherrer Institute",
      author_email="daq@psi.ch",
      description="Standard DAQ buffer interface",

      license="GPL3",
      zip_safe=False,

      packages=["std_buffer",
                "std_buffer.gigafrost",
                "std_buffer.jungfrau",
                ],
      )
