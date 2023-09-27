# Architecture Constraints {#section-architecture-constraints}

### Scalability with the module number

We need to scale with the number of detector modules as we don't know what will change and how many will we have. 

### Storage

Data flow needs to match the distributed storage characteristics

### Working set size

The working set size needs to fit within `L3` of a high performance cpu

###  Melanox cards

Technology used at PSI that we have to adhere to

### HDF5 compatibility as an output file

### Real-time API

We need to provide "real-time" (within 5 seconds) feedback of the state of the system

### We need to support experimental user security mechanism 

Defined by PSI

### Restart time within 10 seconds

### Writing throughtput management
