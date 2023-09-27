# System Scope and Context {#section-system-scope-and-context}

Receive data from the detectors (or cameras), transform data into processable grayscale 2D image, provide the data to the user in one of possible manners: as a file, as a stream.

- Metadata + RamBuffer
- array-1.0
- bsread
- hdf5 files
- config file
- deployment file
- REST API 
- influx DB logging 




Commodity HW. low level high performance, standardized solution.  cost of integration, hide complexity of getting data. Scalable, flexible, local support/knowledge, integrated in PSI ecosystem, standardized deployment. Plug-in functionality, Tight integration with the detectors.


- We support up to x(~20 000) receiving events per core per second.
- We support up to 1GB tput/s per core true both for receiving and sending out.


stability > setup simplicity > performance > user experience > testability > debug-ability > feature rich

### **Contents**

System scope and context - as the name suggests - delimits your system
(i.e. your scope) from all its communication partners (neighboring
systems and users, i.e. the context of your system). It thereby
specifies the external interfaces.

If necessary, differentiate the business context (domain specific inputs
and outputs) from the technical context (channels, protocols, hardware).

### **Motivation**

The domain interfaces and technical interfaces to communication partners
are among your system's most critical aspects. Make sure that you
completely understand them.

### **Form**

Various options:

-   Context diagrams

-   Lists of communication partners and their interfaces.

See [Context and Scope](https://docs.arc42.org/section-3/) in the arc42
documentation.

## Business Context {#_business_context}

### **Contents**

Specification of **all** communication partners (users, IT-systems, ...)
with explanations of domain specific inputs and outputs or interfaces.
Optionally you can add domain specific formats or communication
protocols.

### **Motivation**

All stakeholders should understand which data are exchanged with the
environment of the system.

### **Form**

All kinds of diagrams that show the system as a black box and specify
the domain interfaces to communication partners.

Alternatively (or additionally) you can use a table. The title of the
table is the name of your system, the three columns contain the name of
the communication partner, the inputs, and the outputs.

**\<Diagram or Table\>**

**\<optionally: Explanation of external domain interfaces\>**

## Technical Context {#_technical_context}

### **Contents**

Technical interfaces (channels and transmission media) linking your
system to its environment. In addition a mapping of domain specific
input/output to the channels, i.e. an explanation which I/O uses which
channel.

### **Motivation**

Many stakeholders make architectural decision based on the technical
interfaces between the system and its context. Especially infrastructure
or hardware designers decide these technical interfaces.

### **Form**

E.g. UML deployment diagram describing channels to neighboring systems,
together with a mapping table showing the relationships between channels
and input/output.

**\<Diagram or Table\>**

**\<optionally: Explanation of technical interfaces\>**

**\<Mapping Input/Output to Channels\>**
