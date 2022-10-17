from setuptools import setup

setup(name="std_buffer",
      version="1.0.6",
      maintainer="Paul Scherrer Institute",
      maintainer_email="daq@psi.ch",
      author="Paul Scherrer Institute",
      author_email="daq@psi.ch",
      description="Standard DAQ buffer interface",

      license="GPL3",
      zip_safe=False,

      packages=["std_buffer",
                "std_buffer.gigafrost",
                "std_buffer.jungfrau"
                ],
      )
