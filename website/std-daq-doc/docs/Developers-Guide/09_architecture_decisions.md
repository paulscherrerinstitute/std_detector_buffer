# Architecture Decisions {#section-design-decisions}

- elastic
- modularity
- pipe only (reliable source)
- synchronization mechanism
- we want to integrate the performance critical algorithms into pipeline ourselves (in store data path)
- startup-time configuration of services
- no means of communication/synchronization besides the data pipe to services
- common configuration file and common logging for services built by us (InfluxDB)
- separation between data nad metadata streams

### **Contents**

Important, expensive, large scale or risky architecture decisions
including rationales. With \"decisions\" we mean selecting one
alternative based on given criteria.

Please use your judgement to decide whether an architectural decision
should be documented here in this central section or whether you better
document it locally (e.g. within the white box template of one building
block).

Avoid redundancy. Refer to section 4, where you already captured the
most important decisions of your architecture.

### **Motivation**

Stakeholders of your system should be able to comprehend and retrace
your decisions.

### **Form**

Various options:

-   ADR ([Documenting Architecture
    Decisions](https://cognitect.com/blog/2011/11/15/documenting-architecture-decisions))
    for every important decision

-   List or table, ordered by importance and consequences or:

-   more detailed in form of separate sections per decision

See [Architecture Decisions](https://docs.arc42.org/section-9/) in the
arc42 documentation. There you will find links and examples about ADR.
