# DataProxyLibrary
The Data Proxy Library is part of the advertising.com production infrastructure, denoted as DPL in this document. The goal of the DPL is to act as a proxy between C++ applications and various data sources / sinks, and provide data in a way that insulates the client application from the nitty-gritty communication details. The "intelligence" of the DPL is to be embedded completely in an XML configuration file, which should allow operations to:
* modify where & how requests get routed
* determine how runtime parameters are used, translated, defaulted, overridden, or ignored
* control what happens when a particular load, store or delete fails

(More documentation to come, ported from the Aol internal wiki once approved for release). 
